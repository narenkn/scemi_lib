#ifndef _scemi_pipes_h
#define _scemi_pipes_h

#ifdef __cplusplus
extern "C" {
#endif

	//---------------------------------------------------------------------------
	// scemi_pipe_c_handle()
	//
	// This function retreives an opaque handle representing a transaction
	// input or output pipe given an HDL scope and a pipe ID.
	//---------------------------------------------------------------------------
	void *scemi_pipe_c_handle( // return: pipe handle
			const char *endpoint_path ); // input: path to HDL endpoint instance

	//---------------------------------------------------------------------------
	// scemi_pipe_get_direction()
	// scemi_pipe_get_depth()
	// scemi_pipe_get_bytes_per_element()
	//
	// This function returns the direction, depth, and bytes per element of a previous
	// defined pipe, given the pipe handle
	//----------------------------------------------------------------------------
	svBit scemi_pipe_get_direction( // return: 1 for input pipe, 0 for output pipe
			void *pipe_handle ); // input: pipe handle
	int scemi_pipe_get_depth( // return: current depth (in elements) of the pipe
			void *pipe_handle ); // input: pipe handle
	int scemi_pipe_get_bytes_per_element( // return: bytes per element
			void *pipe_handle ); // input: pipe handle

	//---------------------------------------------------------------------------
	// scemi_pipe_c_send()
	//
	// These is the basic blocking send function for a transaction input pipe.
	// The passed in data is sent to the pipe. If necessary the calling thread
	// is suspended until there is room in the pipe.
	//
	// The eom arg is a flag which is used for user specified end-of-message (eom)
	// indication. It can be used for example to mark the end of a frame containing
	// a sequence of transactions.
	//
	// scemi_pipe_c_receive()
	//
	// This is the basic blocking receive function for a transaction output pipe.
	//
	// The eom argument for this call is an output argument. It is set to the
	// same settings of the flag passed on the send end of the pipe as described
	// above. Thus is can be used by the caller to query whether the current
	// read is one for which an eom was specified when the data was written on
	// the send end.
	//
	// Both the send() and receive() calls are thread-aware. They can be
	// easily implemented using a simple reference implementation that makes
	// use of the non-blocking thread-neutral interface decribed below
	// in conjunction with a selected threading system.
	//---------------------------------------------------------------------------
	void scemi_pipe_c_send(
			void *pipe_handle, // input: pipe handle
			int num_elements, // input: #elements to be written
			const svBitVecVal *data, // input: data
			svBit eom ); // input: end-of-message marker flag (and flush)
	void scemi_pipe_c_receive(
			void *pipe_handle, // input: pipe handle
			int num_elements, // input: #elements to be read
			int *num_elements_valid, // output: #elements that are valid
			svBitVecVal *data, // output: data
			svBit *eom ); // output: end-of-message marker flag (and flush)

	//---------------------------------------------------------------------------
	// scemi_pipe_c_flush()
	//
	// Flush pipelined data.
	//---------------------------------------------------------------------------
	void scemi_pipe_c_flush(
			void *pipe_handle ); // input: pipe handle

	//---------------------------------------------------------------------------
	// scemi_pipe_c_try_send()
	//
	// This is the basic non-blocking send function for a transaction input pipe.
	// If there is room in the pipe for the indicated number of elements, the
	// data is transferred to the pipe and a success status of 1 is returned.
	// Otherwise, nothing is done with the data and a status of 0 is returned.
	//
	// This function is thread-neutral can can be used to create a reference
	// implementation of the blocking send function (scemi_pipe_c_send)
	// over a selected C-based threading environment.
	//
	// scemi_pipe_c_try_receive()
	//
	// This is the basic non-blocking receive function for a transaction output
	// pipe. If the indicated number of elements exist in the pipe, the data is
	// transferred out of the pipe and a success status of 1 is returned.
	// Otherwise, the data in the pipe is left alone and a status of 0 is returned.
	//
	// This function is thread-neutral can can be used to create a reference
	// implementation of the blocking receive function (scemi_pipe_c_receive)
	// over a selected C-based threading environment.
	//
	//---------------------------------------------------------------------------
	int scemi_pipe_c_try_send(
			void *pipe_handle, // input: pipe handle
			int byte_offset, // input: byte offset within data array
			int num_elements, // input: #elements to be written
			const svBitVecVal *data, // input: data
			svBit eom ); // input: end-of-message marker flag
	int scemi_pipe_c_try_receive(
			void *pipe_handle, // input: pipe handle
			int byte_offset, // input: byte offset within data array
			int num_elements, // input: #elements to be read
			svBitVecVal *data, // output: data
			svBit *eom ); // output: end-of-message marker flag

	//---------------------------------------------------------------------------
	// scemi_pipe_c_try_flush()
	//---------------------------------------------------------------------------
	int scemi_pipe_c_try_flush(
			void *pipe_handle ); // input: pipe handle

	//---------------------------------------------------------------------------
	// scemi_pipe_c_in_flush_state()
	//---------------------------------------------------------------------------
	svBit scemi_pipe_c_in_flush_state( // return: whether pipe is in Flush state
			void *pipe_handle ); // input: pipe handle

	//---------------------------------------------------------------------------
	// scemi_pipe_c_can_send()
	//
	// This is function indicates if there is currently space in the pipe
	// for the indicated number of elements meaning that the next call to
	// scemi_pipe_c_send() will succeed without requiring a block.
	//
	// scemi_pipe_c_can_receive()
	//
	// This is function indicates if there is currently at least the indicated
	// number of elements in the pipe meaning that the next call to
	// scemi_pipe_c_receive() will succeed without requiring a block.
	//
	// For both of these calls, a return value 0 indicates that the operation
	// cannot succeed, 1 indicates that it can succeed.
	//---------------------------------------------------------------------------
	int scemi_pipe_c_can_send(
			void *pipe_handle );
	int scemi_pipe_c_can_receive(
			void *pipe_handle );

	//---------------------------------------------------------------------------
	// Notify callback support
	//
	typedef void (*scemi_pipe_notify_callback)(
			void *context ); // input: C model context
	typedef void *scemi_pipe_notify_callback_handle;
	// Handle type denoting registered notify callback.

#ifdef __cplusplus
	scemi_pipe_notify_callback_handle scemi_pipe_set_notify_callback(
			void *pipe_handle, // input: pipe handle
			scemi_pipe_notify_callback notify_callback,
			// input: notify callback function
			void *notify_context, // input: notify context
			int callback_threshold=0 ); // input: threshold for notify callback function
#else // __cplusplus
	scemi_pipe_notify_callback_handle scemi_pipe_set_notify_callback(
			void *pipe_handle, // input: pipe handle
			scemi_pipe_notify_callback notify_callback,
			// input: notify callback function
			void *notify_context, // input: notify context
			int callback_threshold ); // input: threshold for notify callback function
#endif // __cplusplus

	void scemi_pipe_clear_notify_callback(
			scemi_pipe_notify_callback_handle notify_callback_handle );
	// input: notify callback handle
	void *scemi_pipe_get_notify_context( //return: notify context object pointer
			scemi_pipe_notify_callback_handle notify_callback_handle ); // input: notify handle

	//---------------------------------------------------------------------------
	// Per-pipe user data storage support
	//
	void scemi_pipe_put_user_data(
			void *pipe_handle, // input: pipe handle
			void *user_key, // input: user key
			void *user_data); // input: user data
	void *scemi_pipe_get_user_data(
			void *pipe_handle, // input: pipe handle
			void *user_key); // input: user key

	//---------------------------------------------------------------------------
	// Autoflush support
	//
	svBit scemi_pipe_set_eom_auto_flush(
			void *pipe_handle, // input: pipe handle
			svBit enabled ); // input: enable/disable

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // _scemi_pipes_h
