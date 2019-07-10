module dut;

   bit clk;
   initial forever #5 clk = ~clk;

   wire inValid;
   wire [31:0] inData;
   SceMiMessageInPort #(.PortWidth(32)) inMessageInst (clk, 1'b1, inValid, inData);

   int         inCnt = 0;
   always @(posedge clk) begin
      if (inValid) begin
         $display("Obtained trans: %h", inData);
         inCnt = inCnt + 1;
      end
      if (inCnt >= 10) begin
         $finish;
      end
   end

endmodule // dut

