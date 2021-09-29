module ice40_comm (
	input  MCLK,
	
	input  A_RXD,
	output A_TXD,
	output A_TXEN,

	input  B_RXD,
	output B_TXD,
	output B_TXEN,

	input  C_RXD,
	output C_TXD,
	output C_TXEN,

	input  D_RXD,
	output D_TXD,
	output D_TXEN,
	
	input  SPI_NSS,
	output SPI_MISO,
	input  SPI_MOSI,
	input  SPI_SCK,

	input  SEL1,
	input  SEL2,
	input  SEL3,
	input  SEL4,
	input  CTL1,
	input  COMM1,
	input  TDMDAT1,
	input  TDMFS1,
	input  TDMCK1,

	input  SEL5,
	input  SEL6,
	input  SEL7,
	input  SEL8,
	input  CTL2,
	input  COMM2,
	input  TDMDAT2,
	input  TDMFS2,
	input  TDMCK2,

	input  NET_RXD,
	output NET_TXD,

	input  COMM,

	input  SAI_SD,
	input  SAI_MCLK,
	input  SAI_SCK,
	input  SAI_FS,

	output LED1,
	output LED2,

	output IO1,
	output IO2,

	output TP12,
	output TP13,

	output TP71,
	input TP72


);

	localparam MCLK_HZ = 79027200;
	localparam HCLK_HZ = 11289600;
	localparam DCLK_HZ = 2822400;
	localparam ACLK_HZ = 22050;
	localparam SCLK_HSZ = 10;

	wire clk;
	wire rst;

	wire spi_latch;
	wire [15:0] spi_data;
	reg  [1:0] spi_sync;

	wire [15:0] stat;
	reg  [15:0] ctrl;

	reg sig;
	reg [1:0]sw;
	reg [1:0]st;

	wire dsp_clk;
	wire aux_clk;
	wire slow_clk;

	assign clk = MCLK;
	assign rst = 0;


	clkdiv #(
		.DIV(28)
	) clk1 (
		.clk_i(clk),
		.rst_i(rst),
		.clk_o(dsp_clk)
	);

	clkdiv #(
		.DIV(128)
	) clk2 (
		.clk_i(dsp_clk),
		.rst_i(rst),
		.clk_o(aux_clk)
	);

	clkdiv #(
		.DIV(2205)
	) clk3 (
		.clk_i(aux_clk),
		.rst_i(rst),
		.clk_o(slow_clk)
	);
  
  
	spi_slave  #(
		.IO_COUNT(16),
		.CPOL(0),
		.CPHA(0)
	) slv (
		.rst_i(rst),

		.nss_i(SPI_NSS),
		.sck_i(SPI_SCK),
		.sdi_i(SPI_MOSI),
		.sdo_o(SPI_MISO),

		.data_i(stat),
		.data_o(spi_data),
		.latch_o(spi_latch)
	);


	always @(posedge dsp_clk)
	begin
		spi_sync[1] <= spi_sync[0];
		if ((spi_sync[0] ^ spi_sync[1]) & spi_sync[0])
		begin
			ctrl <= spi_data;
		end
	end


	always @(posedge spi_latch or posedge spi_sync[1])
	begin
		if (spi_sync[1])
			spi_sync[0] <= 0;
		else
			spi_sync[0] <= 1'b1;
		
	end


	assign stat[0] = SEL1;
	assign stat[1] = SEL2;
	assign stat[2] = SEL3;
	assign stat[3] = SEL4;
	assign stat[4] = CTL1;
	assign stat[5] = COMM1;
	assign stat[6] = TDMDAT1;
	assign stat[7] = TDMFS1;
	assign stat[8] = TDMCK1;


	assign stat[9] = SEL5;
	assign stat[10] = SEL6;
	assign stat[11] = SEL7;
	assign stat[12] = SEL8;
	assign stat[13] = CTL2;
	assign stat[14] = COMM2;
	assign stat[15] = TDMDAT2;
//	assign stat[15] = TDMFS2;

	assign TDMCK2 = slow_clk;

/*	assign SEL5 = ctrl[0];
	assign SEL6 = ctrl[1];
	assign SEL7 = ctrl[2];
	assign SEL8 = ctrl[3];
	assign CTL2 = ctrl[4];
	assign COMM2 = ctrl[5];
	assign TDMDAT2 = ctrl[6];
	assign TDMFS2 = ctrl[7];
	assign TDMCK2 = ctrl[8];
*/

	assign A_TXD = 1;
	assign B_TXD = 1;
	assign C_TXD = 1;
	assign D_TXD = 1;
	assign A_TXEN = 0;
	assign B_TXEN = 0;
	assign C_TXEN = 0;
	assign D_TXEN = 0;

	assign LED1 = 0;
	assign LED2 = 0;

	assign IO1 = st[0];
	assign IO2 = st[1];

	assign TP71 = sig;

	always @(posedge aux_clk)
	begin
		sig <= !sig;
		sw[0] <= TP72;
		sw[1] <= sw[0];
		if (sw[0] ^ sw[1])
			st <= 2'b10;
		else if (~(sw[0] | sw[1]))
			st <= 2'b01;
		else	
			st <= 2'b00;

	end


//	assign TP12 = IO1;
//	assign TP13 = IO2;


endmodule

