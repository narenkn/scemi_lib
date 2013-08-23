
`protect

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
import "DPI-C" task scemi_final_cleanup();
import "DPI-C" task scemi_pipes_finish();
import "DPI-C" function int unsigned scemi_shash(string path);

package scemi_utils;
	import "DPI-C" task scemi_pipes_finish();
	event scemi_data_avail;

	task scemi_finish();
		scemi_pipes_finish();
	endtask

	string h2s[int unsigned];
	function string hash2string(int unsigned hash);
		return h2s[hash];
	endfunction
	export "DPI-C" function hash2string;
endpackage

class svc_mailbox;
	static local chandle names[string];
	chandle pipe;

	function new(string n);
		if (names.exists(n)) begin
			$display("mailbox for:'%s' already exists, try another!", n);
			$finish;
		end
		pipe = scemi_pipe_c_handle(n);
		names[n] = pipe;
		scemi_utils::h2s[scemi_shash(n)] = n;
	endfunction
endclass

class svc_inbox #(type T=bit[31:0]) extends svc_mailbox;
	local int unsigned num_bits;
	bit eom;

	function new(string n);
		T ival;
    super.new(n);
		num_bits = $bits(ival);
	endfunction

	function int unsigned num();
		return scemi_pipe_pending_data(pipe);
	endfunction

	task get(ref T ival);
		int unsigned num_valid;
		do begin
			scemi_pipe_c_receive_nodata(pipe, 1, num_valid, eom);
			if (0 == num_valid) @scemi_utils::scemi_data_avail; /* wait for data avail */
		end while (0 == num_valid);
		for (int unsigned ui1=0; ui1<num_bits; ui1+=32) begin
			ival[ui1+:32] = scemi_pipe_c_receive_data(pipe);
		end
	endtask

	function int try_get(ref T ival);
		int unsigned num_valid;
		scemi_pipe_c_receive_nodata(pipe, 1, num_valid, eom);
		if (num_valid) begin
			for (int unsigned ui1=0; ui1<num_bits; ui1+=32) begin
				ival[ui1+:32] = scemi_pipe_c_receive_data(pipe);
			end
		end
		return num_valid;
	endfunction

endclass

class svc_outbox #(type T=bit[31:0]) extends svc_mailbox;
	local int unsigned num_bits;

	function new(string n);
		T ival;
    super.new(n);
		num_bits = $bits(ival);
	endfunction

	task put(T ival, bit eom=0);
		for (int unsigned ui1=0; ui1<num_bits; ui1+=32) begin
			scemi_pipe_c_send_data(pipe, ival[ui1+:32]);
		end
		scemi_pipe_c_send_nodata(pipe, 1, eom);
	endtask

endclass

program scemi_pipes_tb;

	export "DPI-C" task scemi_data_avail_trigger;
	task scemi_data_avail_trigger();
		-> scemi_utils::scemi_data_avail;
	endtask

	/* For some screwedup reason a program block with initial completion
	is triggering $finish */
	event   scemi_initialize_finish;

	initial begin
		scemi_initialize();
		@scemi_initialize_finish;
	end

endprogram

`endprotect

