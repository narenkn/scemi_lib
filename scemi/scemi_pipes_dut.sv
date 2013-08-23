
`protect

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

package scemi_utils;
	chandle names[string];
	import "DPI-C" task scemi_pipes_finish();

	task scemi_finish();
		scemi_pipes_finish();
	endtask

	string h2s[int unsigned];
	function string hash2string(int unsigned hash);
		return h2s[hash];
	endfunction
	export "DPI-C" function hash2string;
endpackage

interface SceMiMessageInPort (
	uclock,
	ReceiveReady,
	TransmitReady,
	Message );
//------------------------ ------------------------------
	parameter PortWidth = 1;
	input uclock;
	input ReceiveReady;
	output TransmitReady;
	output [PortWidth-1:0] Message;
//------------------------ ------------------------------

	bit [PortWidth-1:0] Message;
	bit TransmitReady = 0, eom;
	chandle pipe;
	int unsigned num_valid;
	initial begin
		string name = $psprintf("%m");
		pipe = scemi_pipe_c_handle(name);
		scemi_utils::names[name] = pipe;
		scemi_utils::h2s[scemi_shash(name)] = name;
		scemi_pipe_inport_configure(pipe, PortWidth);
		num_valid = 0;
	end

	always @(posedge uclock) begin
		if (null != pipe) begin
			if (ReceiveReady && TransmitReady) begin
				TransmitReady = 0;
				num_valid = 0;
			end

			if (0 == num_valid) begin
				/* On one cycle get the packet and set Transmit */
				scemi_pipe_c_receive_nodata(pipe, 1, num_valid, eom);
				if (1 == num_valid) begin
					for (int unsigned ui1=0; ui1<PortWidth; ui1+=32) begin
						Message[ui1+:32] = scemi_pipe_c_receive_data(pipe);
					end
					TransmitReady = 1;
				end
			end
		end
	end

endinterface

interface SceMiMessageOutPort (
	uclock,
	TransmitReady,
	ReceiveReady,
	Message );
//------------------------ ------------------------------
	parameter PortWidth = 1;
	parameter PortPriority = 0; /* Not used */
	input uclock;
	input TransmitReady;
	output ReceiveReady;
	input [PortWidth-1:0] Message;
//------------------------ ------------------------------

	bit ReceiveReady = 1;
	chandle pipe;
	initial begin
		string name = $psprintf("%m");
		pipe = scemi_pipe_c_handle(name);
		scemi_utils::names[name] = pipe;
		scemi_utils::h2s[scemi_shash(name)] = name;
		scemi_pipe_outport_configure(pipe, PortWidth);
	end
	always @(posedge uclock) begin
		if ((null != pipe) && (TransmitReady)) begin
			ReceiveReady = 0;
			for (int unsigned ui1=0; ui1<PortWidth; ui1+=32) begin
				scemi_pipe_c_send_data(pipe, Message[ui1+:32]);
			end
			scemi_pipe_c_send_nodata(pipe, 1, 0);
			ReceiveReady = 1;
		end
	end
	
endinterface

program scemi_pipes_dut;
	/* For some screwedup reason a program block with initial completion
	is triggering $finish */
	event   scemi_initialize_finish;

	initial begin
		scemi_initialize();
		@scemi_initialize_finish;
	end
endprogram

`endprotect

