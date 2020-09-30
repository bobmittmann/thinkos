module ice40_comm (
	input  MCLK,
	input  MODE,
	
	input  UART_RXD,
	output UART_TXD,

	inout  COMM,
	
	input  PM_RXD,
	output PM_TXD,

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
	input  SPI_MISO,
	input  SPI_MOSI,
	input  SPI_SCK,

	input  SPI_NSS1,
	output SPI_MISO1,
	input  SPI_MOSI1,
	input  SPI_SCK1,

	input  SAI_FS,
	input  SAI_MCLK,
	input  SAI_SCK,
	input  SAI_SD,

	input  SEL1,
	input  SEL2,
	input  SEL3,
	input  SEL4,
	input  CTL1,
	input  COMM1,
	input  TDMDAT1,
	input  TDMFS1,
	output TDMCK1,

	input  SEL5,
	input  SEL6,
	input  SEL7,
	input  SEL8,
	input  CTL2,
	input  COMM2,
	input  TDMDAT2,
	input  TDMFS2,
	output TDMCK2,

	input  QIO0,
	input  QIO1,
	input  QIO2,
	input  QIO3,
	input  QCLK,
	input  QNCS,
	
	input  I2C_SCL,
	input  I2C_SDA
);

	localparam MCLK_HZ = 79027200;

	wire [15:0] stat;
	wire [15:0] ctrl;
	wire clk;
	wire aux_clk;
	wire slow_clk;
	wire rst;

	assign clk = MCLK;
	assign rst = 0;
	wire brg_clk1;

/*	clkdiv #(
		.DIV(16)
	) clk1 (
		.clk_i(clk),
		.rst_i(rst),
		.clk_o(aux_clk)
	);

	clkdiv #(
		.DIV(16)
	) clk2 (
		.clk_i(aux_clk),
		.rst_i(rst),
		.clk_o(slow_clk)
	);
  
  */
	spi_slave  #(
		.IO_COUNT(16),
		.CPOL(0),
		.CPHA(0)
	) slv (
		.rst_i(rst),

		.nss_i(SPI_NSS1),
		.sck_i(SPI_SCK1),
		.sdi_i(SPI_MOSI1),
		.sdo_o(SPI_MISO1),

		.data_i(stat),
		.data_o(ctrl)
	);

	pfracbrg #(.CLK_HZ(8000000),
			  .BAUDRATE(2666666),
			  .OVERSAMPLE(1),
			  .RESOLUTION(16),
			  .BLOCKSIZE(2))
		u1 (.clk_i(clk), 
			.rst_i(rst),
			.clr_i(0),
			.brg_clk_o(brg_clk1));

	assign stat[0] = SEL1;
	assign stat[1] = SEL2;
	assign stat[2] = SEL3;
	assign stat[3] = SEL4;
	assign stat[7] = CTL1;
	assign stat[4] = COMM1;
	assign stat[5] = TDMDAT1;
	assign stat[6] = brg_clk1;
//	assign TDMCK1 = ctrl[0];

	assign stat[8] = SEL5;
	assign stat[9] = SEL6;
	assign stat[10] = SEL7;
	assign stat[11] = SEL8;
	assign stat[15] = CTL2;
	assign stat[12] = COMM2;
	assign stat[13] = TDMDAT2;
	assign stat[14] = TDMFS2;
	assign TDMCK2 = ctrl[1];

	assign A_TXD = ctrl[8];
	assign B_TXD = ctrl[9];
	assign C_TXD = ctrl[10];
	assign D_TXD = ctrl[11];

	assign A_TXEN = ctrl[12];
	assign B_TXEN = ctrl[13];
	assign C_TXEN = ctrl[14];
	assign D_TXEN = ctrl[15];

	assign UART_TXD = PM_RXD;
	assign PM_TXD = UART_RXD;
endmodule

