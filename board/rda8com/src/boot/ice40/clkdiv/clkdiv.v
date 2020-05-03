`timescale 10ns/1ns

/* ---------------------------------------------------------------------------
 * Simple clock divider
 * ---------------------------------------------------------------------------
 */
 
module clkdiv
	#(
		parameter DIV = 2
	)
	(
		input rst_i,
		input clk_i,
		output clk_o
	);

	localparam CNT_BITS = $clog2(DIV);

	wire [CNT_BITS-1:0]cmp;
	reg [CNT_BITS-1:0]cnt;
	reg clk;

	assign cmp = DIV -1;

	generate
		if (DIV == 1)
		begin
			always @(clk_i)
			begin
				clk = clk_i;
			end
		end
		else
		begin
			always @(posedge clk_i or posedge rst_i)
			begin
				if (rst_i) 
				begin
					cnt <= {CNT_BITS{1'b0}};
					clk <= 1'b0;
				end
				else
				begin
					cnt <= (cnt == cmp) ? {CNT_BITS{1'b0}} : cnt + 1'b1;
					clk <= cnt[CNT_BITS - 1];
				end
			end
		end
	endgenerate

	assign clk_o = clk;

endmodule

