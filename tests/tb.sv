
program tb;

   bit clk;
   initial forever #5 clk = ~clk;

   svc_outbox #(bit[31:0]) toDut = new("dut.inMessageInst");
   bit [31:0] d1 = 'hA000;
   initial begin
      @scemi_utils::scemi_data_avail;
      repeat(10) begin
         @(posedge clk);
         toDut.put(d1);
         d1 = d1 + 1;
      end
      repeat(100) @(posedge clk);
      $finish;
   end
endprogram // tb
