`timescale 10ns/1ns //`define MASTER_DELAY `define INT_DELAY module pfracbrg_tb; reg  clk = 0; reg  rst = 0; reg  clr = 0; wire brg_clk1; wire brg_clk2; reg [7:0] count1; reg [7:0] count2; initial begin $display("-----------------------------------------"); $dumpfile("pfracbrg_tb.vcd"); $dumpvars(0, pfracbrg_tb);
	end

	/* Make a reset that pulses once. */
	initial 
	begin
		# 0 rst = 0;
		# 2 rst = 1;
		# 20 rst = 0;
		# 80520 $stop;
	end

	/* Make a regular pulsing clock. */
	always #4 clk = !clk;

	pfracbrg #(.CLK_HZ(8000000),
			  .BAUDRATE(2666666),
			  .OVERSAMPLE(1),
			  .RESOLUTION(16),
			  .BLOCKSIZE(1))
		u1 (.clk_i(clk), 
			.rst_i(rst),
			.clr_i(clr),
			.brg_clk_o(brg_clk1));

	fracbrg #(.CLK_HZ(8000000),
			  .BAUDRATE(2666666),
			  .OVERSAMPLE(1),
			  .RESOLUTION(16))
		u2 (.clk_i(clk), 
			.rst_i(rst),
			.clr_i(clr),
			.brg_clk_o(brg_clk2));

	always @(posedge brg_clk1 or posedge rst)
	begin
		if (rst) 
		begin
			count1 <= 0;
		end
		else 
		begin
			count1 <= count1 + 1;
		end
	end

	always @(posedge brg_clk2 or posedge rst)
	begin
		if (rst) 
		begin
			count2 <= 0;
		end
		else 
		begin
			count2 <= count2 + 1;
		end
	end

endmodule 

