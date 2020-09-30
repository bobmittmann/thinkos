`timescale 10ns/1ns

/* ---------------------------------------------------------------------------
 * Fractional Baud Rate Generator
 * ---------------------------------------------------------------------------
 */

module __fbrg_add
	#(
	  parameter WIDTH = 4
	)
	(
		input c_i,
		input [WIDTH-1:0]x_i,
		input [WIDTH-1:0]y_i,
		output [WIDTH-1:0]s_o,
		output c_o
	);

	wire [WIDTH-1:0]w;
	wire [WIDTH-1:0]c;

	assign w[WIDTH-1:0] = x_i[WIDTH-1:0] ^ y_i[WIDTH-1:0];
	assign s_o = w ^ {c[WIDTH-2:0], c_i};
	assign c = (w & {c[WIDTH-2:0], c_i}) | (x_i & y_i);
	assign c_o = c[WIDTH-1];
endmodule

module fracbrg
	#(
		parameter CLK_HZ = 24000000,
		parameter BAUDRATE = 38400,
		parameter OVERSAMPLE = 16,
		parameter RESOLUTION = 16
	)
	(
		input rst_i,
		input clk_i,
		input clr_i,
		output brg_stb_o,
		output brg_clk_o
	);

	localparam real FREQ = BAUDRATE* OVERSAMPLE;
	localparam real RES2 = (1 << (RESOLUTION - 1));
	localparam real RES1 = RES2 * 2;
	localparam real RDIV = ((FREQ * RES1) / CLK_HZ);
	localparam integer DIV = RDIV;

	localparam integer WIDTH = RESOLUTION;

	wire [WIDTH-1:0] brg_sum;
	wire [WIDTH-1:0] brg_div;
	wire brg_cy;

	reg [WIDTH-1:0] brg_cnt;
	reg brg_clk;
	reg brg_stb;

	assign brg_div = DIV[WIDTH-1:0];

	__fbrg_add #( .WIDTH(WIDTH))
	u1 (.c_i(1'b1), 
		.x_i(brg_cnt),
		.y_i(brg_div),
		.s_o(brg_sum),
		.c_o(brg_cy));

	always @(posedge clk_i or posedge rst_i)
	begin
		if (rst_i) begin
			brg_cnt <= 0;
			brg_clk <= 0;
			brg_stb <= 0;
		end
		else if (clr_i)
		begin
			brg_cnt <= 0;
			brg_clk <= 0;
			brg_stb <= 0;
		end
		else
		begin
			brg_cnt <= brg_sum;
			brg_stb <= brg_cy;
			brg_clk <= brg_stb ? ~brg_clk : brg_clk;
		end
	end

	/* Assign outputs */
	assign brg_clk_o = brg_clk;
	assign brg_stb_o = brg_stb;

endmodule

