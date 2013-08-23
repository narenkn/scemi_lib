#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <stropts.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <map>
#include <queue>
#include "vpi_user.h"
#include "scemi_defines.h"
#include "scemi_pipes.h"

static unsigned char fifoi[RWBUF_SIZE];
static unsigned char fifoo[RWBUF_SIZE];
static svScope       svscope = NULL, svpscope = NULL;
static bool          SceMiFinish = false, data_keeper_vpi = true;

static uint32_t process_tokens(uint32_t cmd, uint32_t num_args, uint32_t idx);
static void * _scemi_pipe_c_handle( uint32_t uid );
static void scemi_data_keeper();
static void scemi_final_cleanup();

struct InBuffer {
	unsigned char *buf;     /* That holds data */
	uint16_t lidx, idx; 		/* read index */
	int  file_id;
	bool init_done;

	InBuffer(unsigned char *b) {
		buf = b;
		lidx = idx = 0;
		init_done = false;
	}
	void Init() {
		errno = 0;
		file_id = open(SVC_FIFO_R, O_RDONLY);
//		DBG_PRINT("%s file_id:%d errno:%d\n", SVC_FIFO_R, file_id, errno);
		assert ((-1 != file_id) && (0 == errno));
		int flags = fcntl(file_id, F_GETFL, 0);
		assert(0 == fcntl(file_id, F_SETFL, flags | O_NONBLOCK));
		init_done = true;
	}
	~InBuffer() {
		close(file_id);
		scemi_final_cleanup();
	}
	void clear() {
		if (lidx == idx) {
			lidx = idx = 0;
			return;
		}
		if (0 != idx) {
			bcopy((char *)buf+idx, (char *)buf, lidx-idx);
			lidx -= idx;
			idx = 0;
		}
	}

	bool get_token_valid(uint32_t t_idx) {
		return init_done && (t_idx < lidx) && ((lidx-t_idx) >= sizeof(uint32_t));
	}

	uint32_t get_token(uint32_t t_idx) {
		uint32_t dt=0, ui1=0;
		for (; (ui1<sizeof(uint32_t)); ui1++, t_idx++) {
			dt |= uint32_t(buf[t_idx])<<(ui1<<3);
		}
		return dt;
	}

	int32_t data_keeper() {
		clear();
		errno = 0;
		int32_t count = read(file_id, buf+idx, RWBUF_SIZE-idx);
		if (count < 0) {
			assert(EAGAIN == errno);
			return errno;
		} else if (0 == count) {
			return 0;
		}
		lidx += count;
		/* We are attempting to read RWBUF_SIZE data. So
		in theory lidx could be == RWBUF_SIZE.  */
		assert(lidx <= RWBUF_SIZE);
		/* Process the data : Packet info
			SCEMI_CMD, [DEST], NUM_ARGS, __VA_ARGS__
			process only if you have on complete transactions
		*/
		do {
			uint32_t cmd = get_token(idx);
			uint32_t num_args = get_token(idx+sizeof(uint32_t));
			if ( get_token_valid(idx+sizeof(uint32_t)) &&
					get_token_valid(idx+((2/*CMD, NUM_ARGS*/+num_args-1)*sizeof(uint32_t))) ) {
					idx += process_tokens(cmd, num_args, idx);
			} else break;
		} while (idx < lidx);
#ifndef SVC_DUT
		if (svscope) {
			svSetScope(svscope);
			scemi_data_avail_trigger();
		}
#endif
		return 0;
	}

	uint32_t process_tokens(uint32_t cmd, uint32_t num_args, uint32_t idx);
};

struct OutBuffer {
	unsigned char *buf;     /* That holds data */
	uint16_t idx; 		/* index */
	int  file_id;
	bool init_done;

	OutBuffer(unsigned char *b) {
		buf = b;
		idx = 0;
		init_done = false;
	}
	void Init() {
		errno = 0;
		file_id = open(SVC_FIFO, O_WRONLY | O_SYNC, 0);
//		DBG_PRINT("%s file_id:%d errno:%d\n", SVC_FIFO, file_id, errno);
		assert ((-1 != file_id) && (0 == errno));
		init_done = true;
	}
	~OutBuffer() {
		close(file_id);
	}

	bool can_put(uint32_t n) {
		return init_done && (((sizeof(uint32_t)*n) + idx) <= RWBUF_SIZE);
	}

	/* Only support integers */
	void put(uint32_t v) {
		/* if can't hold the value, flush */
		if ((sizeof(uint32_t) + idx) > RWBUF_SIZE)
			flush();

		for (uint32_t ui1=0; ui1<sizeof(uint32_t); ui1++, idx++) {
			buf[idx] = v >> (ui1<<3);
		}
		assert(idx <= RWBUF_SIZE);
	}
#if 0
	void put_vaargs(uint32_t n, ...) {
		uint32_t dat;
		va_list argp;

		va_start (argp, n);
		for (uint32_t ui2=0; ui2<n; ui2++) {
			dat = va_arg(argp, uint32_t);

			/* if can't hold the value, flush */
			if ((sizeof(uint32_t) + idx) > RWBUF_SIZE)
				flush();

			for (uint32_t ui1=0; ui1<sizeof(uint32_t); ui1++, idx++) {
				buf[idx] = dat >> (ui1<<3);
			}
		}
		va_end(argp);

		assert (idx <= RWBUF_SIZE);
	}
#endif

	void flush() {
	 	if (! init_done) return;
		if (idx) DBG_PRINT("Flushing Out Stream:");
		for (uint32_t ui1=0; ui1<idx; ui1++) { DBG_PRINT("%3x", buf[ui1]); }
		if (idx) DBG_PRINT("\n");
		write(file_id, buf, idx);
		idx = 0;
	}
};

enum {DIR_OUT, DIR_IN};
typedef std::map<void *, void*> udata_t;
struct Pipe {
	uint32_t uid;
	uint16_t bytes_per_element, callback_threshold, pending_data;
	uint8_t  direction, auto_flush, connected;
	std::queue<uint32_t>       data;
	std::queue<uint32_t>       data_buf;
	udata_t                    udata;
	scemi_pipe_notify_callback notify_callback;
	void*                      notify_context;
	Pipe(uint32_t uid_l) {
		uid = uid_l;
		direction = DIR_OUT;
		bytes_per_element = connected = 0;
		pending_data = callback_threshold = 0;
		notify_callback = NULL;
	}
};

static InBuffer inbuf(fifoi);
static OutBuffer outbuf(fifoo);
typedef std::map<uint32_t, struct Pipe *> hash2path_t;
typedef std::map<uint32_t, std::string> hash2name_t;
static hash2path_t     hash2path;

extern "C"
unsigned int
scemi_shash(char* s)
{
	unsigned int hash = 5381;
	int c;
	while ((c = *s++)) {
		/* hash = hash * 33 ^ c */
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

uint32_t
InBuffer::process_tokens(uint32_t cmd, uint32_t num_args, uint32_t idx)
{
	uint32_t uid = get_token(idx+(2/*CMD, NUM_ARGS*/+1-1)*sizeof(uint32_t));
	Pipe *p = (Pipe *)_scemi_pipe_c_handle(uid);

	if (SCMD_CONNECT_PATH == cmd) {
		assert(3 == num_args);
		p->bytes_per_element = get_token(idx+(2/*CMD, NUM_ARGS*/+2-1)*sizeof(uint32_t));
		p->direction = get_token(idx+(2/*CMD, NUM_ARGS*/+3-1)*sizeof(uint32_t));
		DBG_PRINT("Obtained command SCMD_CONNECT_PATH: uid:%x bytes_per_element:%u p->direction:%u\n", uid, p->bytes_per_element, p->direction);
		p->connected = 1;
		assert(p->bytes_per_element > 0);
		assert(0 == (p->bytes_per_element % sizeof(svBitVecVal)));
	} else if (SCMD_DATA == cmd) {
		assert(num_args > 1);
		p->data.push(SCMD_DATA);
		p->data.push(num_args-1); /* num args */
		DBG_PRINT("Obtained command SCMD_DATA num_args:%u uid:%x", num_args, uid);
		for (uint32_t t_ui1=2; t_ui1<=num_args; t_ui1++) {
			uint32_t t_ui2 = get_token(idx+(2/*CMD, NUM_ARGS*/+t_ui1-1)*sizeof(uint32_t));
			p->data.push(t_ui2);
			DBG_PRINT(" %u:%8x", t_ui1, t_ui2);
		}
		DBG_PRINT("\n");
		/* callback */
		p->pending_data++;
		if ((NULL != p->notify_callback) && (p->pending_data > p->callback_threshold)) {
			(*(p->notify_callback))(p->notify_context);
		}
	} else if (SCMD_DATA_EOM == cmd) {
		DBG_PRINT("Obtained command SCMD_DATA_EOM\n");
		p->data.push(SCMD_DATA_EOM);
	} else if (SCMD_FINISH == cmd) {
		DBG_PRINT("Obtained command SCMD_FINISH\n");
		SceMiFinish = true;
	} else {
		DBG_PRINT("Unknown cmd %u\n", cmd);
		assert(0);
	}

	return (2/*CMD, ARGS*/+num_args)*sizeof(uint32_t);
}

extern "C"
PLI_INT32
scemi_data_keeper_callback
(struct t_cb_data *cbd)
{
	/* */
	if ( (! inbuf.init_done) || (! outbuf.init_done) ) {
		scemi_data_keeper();
		return 0;
	}

	/* housekeeping 1: flush outbuf */
	int32_t ret_val = inbuf.data_keeper();
	scemi_data_keeper();
	if ((0 != ret_val) && (EAGAIN != ret_val)) {
		DBG_PRINT("Some problem with Communications channel, Triggering Finish\n");
		SceMiFinish = true;
	}

	/* housekeeping 2: flush outbuf */
	outbuf.flush();

	return 0;
}

static
void
scemi_data_keeper()
{
	if (! data_keeper_vpi) return;

  s_cb_data  cbData;
  cbData.reason    = cbNextSimTime;
  cbData.cb_rtn    = scemi_data_keeper_callback;
  cbData.time      = NULL;
  cbData.value     = NULL;
  cbData.obj       = NULL;
  cbData.user_data = NULL;

	vpiHandle cbHandle = vpi_register_cb(&cbData);
  vpi_free_object(cbHandle);
}

static
void * // return: pipe handle
_scemi_pipe_c_handle( uint32_t uid ) // input: path to HDL endpoint instance
{
	/* if pipe available return the same */
	hash2path_t::iterator it = hash2path.find(uid);
	if (it != hash2path.end()) {
		return it->second;
	}

	/* new object */
	Pipe *p = new Pipe(uid);
	hash2path.insert( std::pair<uint32_t, struct Pipe *>(uid, p) );

	return p;
}
void * // return: pipe handle
scemi_pipe_c_handle( const char *endpoint_path ) // input: path to HDL endpoint instance
{
	uint32_t uid = scemi_shash((char *)endpoint_path);
	return _scemi_pipe_c_handle(uid);
}

svBit // return: 1 for input pipe, 0 for output pipe
scemi_pipe_get_direction( void *pipe_handle ) // input: pipe handle
{
	return static_cast<struct Pipe *>(pipe_handle)->direction;
}

int // return: current depth (in elements) of the pipe
scemi_pipe_get_depth( void *pipe_handle ) // input: pipe handle
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	return (int)((RWBUF_SIZE-outbuf.idx) / p->bytes_per_element);
}

int // return: bytes per element
scemi_pipe_get_bytes_per_element( void *pipe_handle ) // input: pipe handle
{
	return static_cast<struct Pipe *>(pipe_handle)->bytes_per_element;
}

svBit
scemi_pipe_set_eom_auto_flush( void *pipe_handle, svBit enabled ) // input: enable/disable
{
	return static_cast<struct Pipe *>(pipe_handle)->auto_flush = enabled;
}

extern "C"
void
scemi_initialize()
{
	static bool init_done = false;

	/* make sure you don't init twice */
	if (init_done) return;
	init_done = true;

#ifdef  SVC_DUT
	inbuf.Init();
	outbuf.Init();
#else
	outbuf.Init();
	inbuf.Init();
#endif
	svscope = svGetScopeFromName(SVC_SV_SCOPE);
	svpscope = svGetScopeFromName(SVC_SVP_SCOPE);

	scemi_data_keeper();
}

static
void
scemi_final_cleanup()
{
	for (hash2path_t::iterator it = hash2path.begin(); it!=hash2path.end(); it++) {
		delete it->second;
	}
}

extern "C"
void
scemi_pipes_finish()
{
	SceMiFinish = true;
	outbuf.put(SCMD_FINISH);
	outbuf.put(0);
	outbuf.flush();
}

void
scemi_pipe_c_send_data (
		void *pipe_handle,
		uint32_t data)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	p->data_buf.push(data);
}

void
scemi_pipe_c_send_nodata (
		void *pipe_handle, // input: pipe handle
		int num_elements, // input: #elements to be written
		svBit eom ) // input: end-of-message marker flag (and flush)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	if (! p->connected) {
		svSetScope(svpscope);
		ERROR("Attempt to send to '%s' before establishing connection\n", hash2string(p->uid));
		return;
	}

	for (uint32_t elem=0; elem<num_elements; elem++) {
		/* */
		uint32_t num_args = (p->bytes_per_element/sizeof(svBitVecVal)) + 1; /* UID */
		outbuf.put(SCMD_DATA);
		outbuf.put(num_args);
		outbuf.put(p->uid);

		/* */
		for (uint32_t ui1=0; ui1<(p->bytes_per_element/sizeof(svBitVecVal)); ui1++) {
			assert(! (p->data_buf.empty()));
			outbuf.put((uint32_t)(p->data_buf.front()));
			p->data_buf.pop();
		}
	}

	/* */
	if (0 != num_elements) {
		if (eom && p->auto_flush) {
			outbuf.put(SCMD_DATA_EOM);
			outbuf.put(1);
			outbuf.put(p->uid);
			outbuf.flush();
		}
	}
}

void
scemi_pipe_c_send (
		void *pipe_handle, // input: pipe handle
		int num_elements, // input: #elements to be written
		const svBitVecVal *data, // input: data
		svBit eom ) // input: end-of-message marker flag (and flush)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	if (! p->connected) {
		ERROR("Attempt to send in pipe '%s' before establishing connection\n", hash2string(p->uid));
		return;
	}

	for (uint32_t elem=0; elem<num_elements; elem++) {
		/* */
		uint32_t num_args = (p->bytes_per_element/sizeof(svBitVecVal)) + 1; /* UID */
		outbuf.put(SCMD_DATA);
		outbuf.put(num_args);
		outbuf.put(p->uid);

		/* */
		for (uint32_t ui1=0; ui1<(p->bytes_per_element/sizeof(svBitVecVal)); ui1++) {
			outbuf.put((uint32_t)data[ui1]);
		}
	}

	/* */
	if (0 != num_elements) {
		if (eom && p->auto_flush) {
			outbuf.put(SCMD_DATA_EOM);
			outbuf.put(1);
			outbuf.put(p->uid);
			outbuf.flush();
		}
	}
}

int /* # elements sent */
scemi_pipe_c_try_send (
		void *pipe_handle, // input: pipe handle
		int byte_offset, // input: byte offset within data array
		int num_elements, // input: #elements to be written
		const svBitVecVal *data, // input: data
		svBit eom ) // input: end-of-message marker flag
{
	scemi_pipe_c_send(pipe_handle, num_elements-byte_offset, data, eom);
	return num_elements;
}

void
scemi_pipe_c_flush( void *pipe_handle ) // input: pipe handle
{
	outbuf.flush();
}
int
scemi_pipe_c_try_flush( void *pipe_handle ) // input: pipe handle
{
	scemi_pipe_c_flush( pipe_handle );
	return 1;
}
svBit // return: whether pipe is in Flush state
scemi_pipe_c_in_flush_state( void *pipe_handle ) // input: pipe handle
{
	return (0 == outbuf.idx) ? 1 : 0;
}
int
scemi_pipe_c_can_send( void *pipe_handle )
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	return (DIR_OUT == p->direction) ? 1 : 0;
}
int
scemi_pipe_c_can_receive( void *pipe_handle )
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	return (DIR_IN == p->direction) ? 1 : 0;
}

void
scemi_pipe_put_user_data (
		void *pipe_handle, // input: pipe handle
		void *user_key, // input: user key
		void *user_data ) // input: user data
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	p->udata.insert( std::pair<void *, void *>(user_key, user_data) );
}

void *
scemi_pipe_get_user_data (
		void *pipe_handle, // input: pipe handle
		void *user_key ) // input: user key
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	udata_t::iterator it = p->udata.find(user_key);
	return (it != p->udata.end()) ? it->second: NULL;
}

uint32_t
scemi_pipe_c_receive_data ( void *pipe_handle )
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	assert (! (p->data_buf.empty()));
	uint32_t ret_val = p->data_buf.front();
	p->data_buf.pop();
	return ret_val;
}

void
scemi_pipe_c_receive_nodata (
		void *pipe_handle, // input: pipe handle
		int num_elements, // input: #elements to be read
		int *num_elements_valid, // output: #elements that are valid
		svBit *eom ) // output: end-of-message marker flag (and flush)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);

	*num_elements_valid = 0;
	*eom = 0;
	if ((p->data.empty()) || !(p->connected)) {
		return;
	}

	uint32_t data_idx = 0;
	do {
		uint32_t cmd = p->data.front(); p->data.pop();
		if (SCMD_DATA == cmd) {
			uint32_t num_args = p->data.front(); p->data.pop();
			for (uint32_t ui1=0; ui1<num_args; data_idx++, ui1++) {
				assert(! p->data.empty());
				p->data_buf.push(p->data.front());
//				DBG_PRINT("Receiving data %x\n", p->data.front());
				p->data.pop();
			}
			(*num_elements_valid)++;
			assert(p->pending_data > 0);
			p->pending_data--;
		} else if (SCMD_DATA_EOM == cmd) {
			*eom = 1;
			break;
		} else { /* Unknown command */
			assert(0);
		}
	} while ((! p->data.empty()) && ((*num_elements_valid) < num_elements));
}

void
scemi_pipe_c_receive (
		void *pipe_handle, // input: pipe handle
		int num_elements, // input: #elements to be read
		int *num_elements_valid, // output: #elements that are valid
		svBitVecVal *data, // output: data
		svBit *eom ) // output: end-of-message marker flag (and flush)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);

	*num_elements_valid = 0;
	*eom = 0;
	if ((p->data.empty()) || !(p->connected)) {
		return;
	}

	uint32_t data_idx = 0;
	do {
		uint32_t cmd = p->data.front(); p->data.pop();
		if (SCMD_DATA == cmd) {
			uint32_t num_args = p->data.front(); p->data.pop();
			for (uint32_t ui1=0; ui1<num_args; data_idx++, ui1++) {
				assert(! p->data.empty());
				data[data_idx] = p->data.front();
//				DBG_PRINT("Receiving data %x\n", p->data.front());
				p->data.pop();
			}
			(*num_elements_valid)++;
			assert(p->pending_data > 0);
			p->pending_data--;
		} else if (SCMD_DATA_EOM == cmd) {
			*eom = 1;
			break;
		} else { /* Unknown command */
			assert(0);
		}
	} while ((! p->data.empty()) && ((*num_elements_valid) < num_elements));
}

int /* # elements received */
scemi_pipe_c_try_receive (
		void *pipe_handle, // input: pipe handle
		int byte_offset, // input: byte offset within data array
		int num_elements, // input: #elements to be read
		svBitVecVal *data, // output: data
		svBit *eom ) // output: end-of-message marker flag
{
	int num_elem_received = 0;

	/* num_elem_received is returned as byte_offset after mult with p->bytes_per_element */
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	uint32_t offset_svbitvec = byte_offset / p->bytes_per_element;

	scemi_pipe_c_receive(pipe_handle, num_elements, &num_elem_received, data+offset_svbitvec, eom);

	return num_elem_received;
}

scemi_pipe_notify_callback_handle
scemi_pipe_set_notify_callback (
		void *pipe_handle, // input: pipe handle
		scemi_pipe_notify_callback notify_callback, // input: notify callback function
		void *notify_context, // input: notify context
		int callback_threshold ) // input: threshold for notify callback function
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	p->notify_callback = notify_callback;
	p->notify_context = notify_context;
	p->callback_threshold = callback_threshold;

	return pipe_handle;
}

void
scemi_pipe_clear_notify_callback (
		scemi_pipe_notify_callback_handle notify_callback_handle )
{
	Pipe *p = static_cast<struct Pipe *>(notify_callback_handle);
	p->notify_callback = NULL;
}

void * //return: notify context object pointer
scemi_pipe_get_notify_context(
		scemi_pipe_notify_callback_handle notify_callback_handle ) // input: notify handle
{
	Pipe *p = static_cast<struct Pipe *>(notify_callback_handle);
	return p->notify_context;
}

uint32_t
scemi_pipe_pending_data(void *pipe_handle)
{
	return static_cast<struct Pipe *>(pipe_handle)->pending_data;
}

extern "C"
void
scemi_pipe_outport_configure(void *pipe_handle, int unsigned num_bits)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	/* */
	p->bytes_per_element = ((num_bits+31)/32)*sizeof(uint32_t);
	p->direction = DIR_OUT;
	p->connected = 1;
	/* */
	outbuf.put(SCMD_CONNECT_PATH);
	outbuf.put(3);
	outbuf.put(p->uid);
	outbuf.put(p->bytes_per_element);
	outbuf.put(p->direction);
}

extern "C"
void
scemi_pipe_inport_configure(void *pipe_handle, int unsigned num_bits)
{
	Pipe *p = static_cast<struct Pipe *>(pipe_handle);
	/* */
	p->bytes_per_element = ((num_bits+31)/32)*sizeof(uint32_t);
	p->direction = DIR_IN;
	p->connected = 1;
	/* */
	outbuf.put(SCMD_CONNECT_PATH);
	outbuf.put(3);
	outbuf.put(p->uid);
	outbuf.put(p->bytes_per_element);
	outbuf.put(p->direction);
}

