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
	wire [WIDTH:0]c;

	assign c[0] = c_i;
	assign w = x_i ^ y_i;
	assign c[WIDTH:1] = (w & c[WIDTH-1:0]) | (x_i & y_i);
	assign s_o = w ^ c[WIDTH-1:0];
	assign c_o = c[WIDTH];
endmodule


module pfracbrg
	#(
		parameter CLK_HZ = 24000000,
		parameter BAUDRATE = 38400,
		parameter OVERSAMPLE = 16,
		parameter RESOLUTION = 16,
		parameter BLOCKSIZE = 8
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

	localparam integer WIDTH = BLOCKSIZE;
	localparam integer N = RESOLUTION / WIDTH;
	localparam integer REM = RESOLUTION - (N * WIDTH);

	/* TODO: process the reminder block */

	wire [WIDTH-1:0] brg_sum [0:N-1];
	wire [WIDTH-1:0] brg_div [0:N-1];
	wire [N-1:0] brg_ci;
	wire [N-1:0] brg_co;

	reg [WIDTH-1:0] brg_cnt [0:N-1];
	reg [RESOLUTION-1:0] brg_q;
	reg [N-1:0] brg_stb;
	reg brg_clk;

	assign brg_ci[0] = 1'b1;

	/* TODO: process reminder block */
	
	generate
		if (N > 1)
		begin
			assign brg_ci[N-1:1] = brg_stb[N-2:0];
		end
	endgenerate

	always @(posedge clk_i or posedge rst_i)
	begin
		if (rst_i) 
		begin
			brg_q <= DIV[RESOLUTION-1:0];
		end
		else if (clr_i)
		begin
			brg_q <= DIV[RESOLUTION-1:0];
		end
	end

	genvar i;

	generate
		for (i = 0; i < N; i = i + 1) 
		begin
			assign brg_div[i] = brg_q[((i+1)*WIDTH)-1:(i*WIDTH)];

			adder #( .WIDTH(WIDTH) )
			u2 (.c_i(brg_ci[i]), 
				.x_i(brg_cnt[i]),
				.y_i(brg_div[i]),
				.s_o(brg_sum[i]),
				.c_o(brg_co[i]));

			always @(posedge clk_i or posedge rst_i)
			begin
				if (rst_i)
				begin
					brg_cnt[i] <= 0;
					brg_stb[i] <= 0;
				end
				else if (clr_i)
				begin
					brg_cnt[i] <= 0;
					brg_stb[i] <= 0;
				end
				else
				begin
					brg_cnt[i] <= brg_sum[i];
					brg_stb[i] <= brg_co[i];
				end
			end
		end
	endgenerate

	/* TODO: process reminder block */
	generate
		if (REM > 0)
		begin
		end
	endgenerate

	always @(posedge clk_i or posedge rst_i)
	begin
		if (rst_i) 
		begin
			brg_clk <= 0;
		end
		else if (clr_i)
		begin
			brg_clk <= 0;
		end
		else
		begin
			brg_clk <= brg_stb[N-1] ? ~brg_clk : brg_clk;
		end
	end

	/* Assign outputs */
	assign brg_clk_o = brg_clk;
	assign brg_stb_o = brg_stb[N-1];
endmodule

