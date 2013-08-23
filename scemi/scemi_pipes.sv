
`protect

interface scemi_input_pipe();

	parameter BYTES_PER_ELEMENT = 1;
	parameter PAYLOAD_MAX_ELEMENTS = 1;
	parameter BUFFER_MAX_ELEMENTS = 1000;
	parameter VISIBILITY_MODE = 0; // must be set to either 1 or 2
	// set to 1 for immediate visibility
	// set to 2 for deferred visibility
	parameter NOTIFICATION_THRESHOLD = BUFFER_MAX_ELEMENTS;
	// Can have a value = 1 or
	// BUFFER_MAX_ELEMENTS
	localparam PAYLOAD_MAX_BITS = PAYLOAD_MAX_ELEMENTS * BYTES_PER_ELEMENT * 8;

	import "DPI-C" task scemi_pipe_outport_configure(chandle pipe_handle, int unsigned num_bits);
	import "DPI-C" task scemi_pipe_inport_configure(chandle pipe_handle, int unsigned num_bits);
	import "DPI-C" function int unsigned scemi_pipe_pending_data (chandle pipe_handle);
	import "DPI-C" function void scemi_pipe_c_send_data (chandle pipe_handle, int unsigned data);
	import "DPI-C" function void scemi_pipe_c_send_nodata (
				input chandle pipe_handle,
				input int unsigned num_elements,
				input bit eom );
	import "DPI-C" function int unsigned scemi_pipe_c_receive_data (chandle pipe_handle);
	import "DPI-C" function void scemi_pipe_c_receive_nodata (
				input chandle pipe_handle,
				input int unsigned num_elements,
				output int unsigned num_elements_valid,
				output bit eom );
	import "DPI-C" function chandle scemi_pipe_c_handle( string endpoint_path );
	import "DPI-C" task scemi_initialize();
	import "DPI-C" function int unsigned scemi_shash(string path);

	task receive (
		input int num_elements, // input: #elements to be read
		output int num_elements_valid, // output: #elements that are valid
		output bit [PAYLOAD_MAX_BITS-1:0] data, // output: data
		output bit eom ); // output: end-of-message marker flag
	endtask

	function int try_receive( // return: #requested elements
														// that are actually received
		input int byte_offset, // input: byte_offset into data, below
		input int num_elements, // input: #elements to be read
		output bit [PAYLOAD_MAX_BITS-1:0] data, // output: data
		output bit eom ); // output: end-of-message marker flag
	endfunction

	function int can_receive(); // return: #elements that can be received
	endfunction

	modport receive_if ( import receive, try_receive, can_receive );

endinterface

interface scemi_output_pipe();

	parameter BYTES_PER_ELEMENT = 1;
	parameter PAYLOAD_MAX_ELEMENTS = 1;
	parameter BUFFER_MAX_ELEMENTS = 1000;
	parameter VISIBILITY_MODE = 0; // must be set to either 1 or 2
	// set to 1 for immediate visibility
	// set to 2 for deferred visibility
	parameter NOTIFICATION_THRESHOLD = BUFFER_MAX_ELEMENTS;
	// Can have a value = 1 or
	// BUFFER_MAX_ELEMENTS
	localparam PAYLOAD_MAX_BITS = PAYLOAD_MAX_ELEMENTS * BYTES_PER_ELEMENT * 8;

	import "DPI-C" task scemi_pipe_outport_configure(chandle pipe_handle, int unsigned num_bits);
	import "DPI-C" task scemi_pipe_inport_configure(chandle pipe_handle, int unsigned num_bits);
	import "DPI-C" function int unsigned scemi_pipe_pending_data (chandle pipe_handle);
	import "DPI-C" function void scemi_pipe_c_send_data (chandle pipe_handle, int unsigned data);
	import "DPI-C" function void scemi_pipe_c_send_nodata (
				input chandle pipe_handle,
				input int unsigned num_elements,
				input bit eom );
	import "DPI-C" function int unsigned scemi_pipe_c_receive_data (chandle pipe_handle);
	import "DPI-C" function void scemi_pipe_c_receive_nodata (
				input chandle pipe_handle,
				input int unsigned num_elements,
				output int unsigned num_elements_valid,
				output bit eom );
	import "DPI-C" function chandle scemi_pipe_c_handle( string endpoint_path );
	import "DPI-C" task scemi_initialize();
	import "DPI-C" function int unsigned scemi_shash(string path);

	task send (
		input int num_elements, // input: #elements to be written
		input bit [PAYLOAD_MAX_BITS-1:0] data, // input: data
		input bit eom ); // input: end-of-message marker flag
	endtask

	task flush;
	endtask

	function int try_send ( // return: #requested elements
												// that are actually sent
		input int byte_offset, // input: byte_offset into data, below
		input int num_elements, // input: #elements to be sent
		input bit [PAYLOAD_MAX_BITS-1:0] data, // input: data
		input bit eom ); // input: end-of-message marker flag
	endfunction

	function int try_flush(); // return: 1 if pipe is successfully flushed
														// i.e. an empty pipe
	endfunction

	function int can_send(); // return: #elements that can be sent
	endfunction

	modport send_if ( import send, flush, try_send, can_send );

endinterface

`endprotect

