`timescale 10ns/1ns

`define INT_DELAY

module clkdiv_tb;

	reg  clk = 0;
	reg  rst = 0;
	wire clk1;
	wire clk2;
	wire clk3;
	wire clk4;
	wire clk5;
	wire clk6;
	wire clk7;
	wire clk8;

	initial 
	begin
		$display("-----------------------------------------");
		$dumpfile("clkdiv_tb.vcd");
		$dumpvars(0, clkdiv_tb);
	end

	/* Make a reset that pulses once. */
	initial 
	begin
		# 0 rst = 0;
		# 2 rst = 1;
		# 20 rst = 0;
		# 8520 $stop;
	end

	/* Make a regular pulsing clock. */
	always #4 clk = !clk;

	clkdiv #(.DIV(1)) u1 (.clk_i(clk), .rst_i(rst), .clk_o(clk1));
	clkdiv #(.DIV(2)) u2 (.clk_i(clk), .rst_i(rst), .clk_o(clk2));
	clkdiv #(.DIV(3)) u3 (.clk_i(clk), .rst_i(rst), .clk_o(clk3));
	clkdiv #(.DIV(4)) u4 (.clk_i(clk), .rst_i(rst), .clk_o(clk4));
	clkdiv #(.DIV(5)) u5 (.clk_i(clk), .rst_i(rst), .clk_o(clk5));
	clkdiv #(.DIV(6)) u6 (.clk_i(clk), .rst_i(rst), .clk_o(clk6));
	clkdiv #(.DIV(7)) u7 (.clk_i(clk), .rst_i(rst), .clk_o(clk7));
	clkdiv #(.DIV(8)) u8 (.clk_i(clk), .rst_i(rst), .clk_o(clk8));

endmodule 

