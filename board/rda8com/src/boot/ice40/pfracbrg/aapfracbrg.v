`timescale 10ns/1ns

/* ---------------------------------------------------------------------------
 * Fractional Baud Rate Generator
 * ---------------------------------------------------------------------------
 */

module fulladder(input x_i, input y_i, input c_i, output s_o, output c_o);
	wire w;
	assign w = x_i ^ y_i;
	assign s_o = w ^ c_i;
	assign c_o = (w & c_i) | (x_i & y_i);
endmodule

module adder 
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


module pfracbrg
	#(
		parameter CLK_HZ = 24000000,
		parameter BAUDRATE = 38400,
		parameter OVERSAMPLE = 16,
		parameter RESOLUTION = 16,
		parameter LEVEL = 4
	)
	(
		input rst_i,
		input clk_i,
		input clr_i,
		output brg_stb_o,
		output brg_clk_o
	);

	localparam real FREQ = BAUDRATE * OVERSAMPLE;
	localparam real RES2 = (1 << (RESOLUTION - 1));
	localparam real RES1 = RES2 * 2;
	localparam real RDIV = ((FREQ * RES1) / CLK_HZ);
	localparam integer DIV = RDIV;

	localparam integer CNTBITS = RESOLUTION / 2;
	localparam integer LVLBITS = LEVEL;

	wire [CNTBITS-1:0] brg_sum [0:1];
	wire [CNTBITS-1:0] brg_div [0:1];
	wire brg_cy [0:1];

	reg [CNTBITS-1:0] brg_cnt [0:1];
	reg brg_clk [0:1];
	reg brg_stb [0:1];

	assign brg_div[0] = DIV[CNTBITS-1:0];
	assign brg_div[1] = DIV[2*CNTBITS-1:CNTBITS];

	adder #(
			.WIDTH(CNTBITS)
			)
	u1 (.c_i(1'b1), 
		.x_i(brg_cnt[0]),
		.y_i(brg_div[0]),
		.s_o(brg_sum[0]),
		.c_o(brg_cy[0]));

	always @(posedge clk_i or posedge rst_i)
	begin
		if (rst_i) begin
			brg_cnt[0] <= 0;
			brg_clk[0] <= 0;
			brg_stb[0] <= 0;
		end
		else if (clr_i)
		begin
			brg_cnt[0] <= 0;
			brg_clk[0] <= 0;
			brg_stb[0] <= 0;
		end
		else
		begin
			begin
				brg_cnt[0] <= brg_sum[0];
				brg_clk[0] <= brg_cy[0];
				brg_stb[0] <= brg_cy[0] & (brg_cy[0] ^ brg_clk[0]);
			end
		end
	end

	adder #(
			.WIDTH(CNTBITS)
			)
	u2 (.c_i(brg_cy[0]), 
		.x_i(brg_cnt[1]),
		.y_i(brg_div[1]),
		.s_o(brg_sum[1]),
		.c_o(brg_cy[1]));

	always @(posedge clk_i or posedge rst_i)
	begin
		if (rst_i) begin
			brg_cnt[1] <= 0;
			brg_clk[1] <= 0;
			brg_stb[1] <= 0;
		end
		else if (clr_i)
		begin
			brg_cnt[1] <= 0;
			brg_clk[1] <= 0;
			brg_stb[1] <= 0;
		end
		else
		begin
			begin
				brg_cnt[1] <= brg_sum[1];
				brg_clk[1] <= brg_cy[1] ? ~brg_clk[1] : brg_clk[1];
				brg_stb[1] <= brg_cy[1] & (brg_cy[1] ^ brg_clk[1]);
			end
		end
	end

	/* Assign outputs */
	assign brg_clk_o = brg_clk[1];
	assign brg_stb_o = brg_stb[1];

endmodule

