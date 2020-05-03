`timescale 10ns/100fs

module spi_slave_tb;

	reg rst;
	reg clk;

	reg [7:0] wdata;
	wire [7:0] rdata;
	reg nss;
	reg sck;
	wire sdo;
	reg sdi;

	parameter PERIOD = 16;
	parameter real DUTY_CYCLE = 0.5;
	parameter OFFSET = 16;
	initial 
	begin
		$display("-----------------------------------------");
		$dumpfile("spi_slave_tb.vcd");
		$dumpvars(0, spi_slave_tb);
	end

	initial begin  // Clock process for clk
		#OFFSET;
		forever
		begin
			clk = 1'b0;
			#(PERIOD-(PERIOD*DUTY_CYCLE)) clk = 1'b1;
			#(PERIOD*DUTY_CYCLE);
		end
	end

	initial begin
		#0 rst = 1'b1;
		#0 nss = 1'b1;
		#0 sck = 1'b0;
		#0 sdi = 1'b0;
		#0 wdata = 8'b10011001;
		#16 rst = 1'b0;

		#64  nss = 1'b0;

		#16  sck = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  nss = 1'b1;

		#32  nss = 1'b0;

		#16  sck = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  nss = 1'b1;

		#32  nss = 1'b0;

		#16  sck = 1'b0;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  sck = 1'b1;
		#16  sck = 1'b0; sdi = 1'b1;

		#16  nss = 1'b1;
		#256 $stop;
	end


	spi_slave  #(
		.IO_COUNT(8),
		.CPOL(0),
		.CPHA(0)
	) slv (
		 .rst_i(rst),

		 .nss_i(nss),
		 .sck_i(sck),
		 .sdi_i(sdi),
		 .sdo_o(sdo),

		 .data_i(wdata),
		 .data_o(rdata)
		);

endmodule

