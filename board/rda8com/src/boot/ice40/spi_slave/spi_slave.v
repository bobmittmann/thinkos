`timescale 10ns/100fs

/* SPI MODE 3
read data (sdin_i) @posedge SCK
*/		
module spi_slave
	#(
		parameter IO_COUNT = 8,
		/* Clock Polarity */
		parameter CPOL = 0, 
		/* Clock Phase */
		parameter CPHA = 0
	)
	(
		input rst_i,

		/* SPI interface */
		input sck_i,
		input nss_i,
		input sdi_i,
		output sdo_o,
		output sck_o,
		output latch_o,

		input [IO_COUNT-1:0]data_i,
		output [IO_COUNT-1:0]data_o
	);

	localparam CNT_BITS = $clog2(IO_COUNT) + 1;
	reg [IO_COUNT-1:0] rdata;
	reg [IO_COUNT-1:0] xfrbuf;
	reg [CNT_BITS-1:0] cnt;
	reg rxd;
	wire ss;
	wire latch;
	wire sck_in;
	wire sck_out;

	assign ss = ~nss_i;

	/* transfer data from shift register on ss high */
	always @(posedge rst_i or posedge nss_i)
	begin
		if (rst_i)
			rdata <= 0;
		else 
			rdata <= xfrbuf;
	end

	assign latch = (cnt == 0);

	generate
		if (CPOL == 0)
		begin
			assign sck_out = !sck_i & !nss_i;
			assign sck_in = sck_i;
		end
		else
		begin
			assign sck_out = sck_i & !nss_i;
			assign sck_in = !sck_i & !nss_i;
		end
	endgenerate

	always @(posedge rst_i or posedge sck_in)
	begin
		if (rst_i)
		begin
			rxd <= 0;
		end
		else 
		begin
			rxd <= sdi_i;
		end
	end

	always @(posedge nss_i or posedge sck_i)
	begin
		if (nss_i)
			cnt <= 0;
		else 
			cnt <= cnt + 1'b1;
	end

   /* SPI transfers are MSB first */
	always @(posedge rst_i or posedge sck_out)
	begin
		if (rst_i)
		begin
			xfrbuf <= 0;
		end
		else 
		begin
			if (latch)
				/* latch data on ss low */
				xfrbuf <= data_i;
			else
				/* shift */
				xfrbuf <= {xfrbuf[IO_COUNT-2:0], rxd};
		end
	end

	assign sdo_o = xfrbuf[IO_COUNT-1];
	assign data_o = rdata;
	assign sck_o = sck_out;
	assign latch_o = latch;

endmodule

