/** 
 * @file fpga.c
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Bootloader descriptor and hardware initialization
 * 
 */

#include <sys/stm32f.h>
#include <sys/delay.h>
#include <lz77.h>

#include "board.h"
#include "ice40hx1k.h"

#ifndef LZ_WINDOW_SIZE
#define LZ_WINDOW_SIZE 8192
#endif

#define LZ_WND_MASK (LZ_WINDOW_SIZE - 1)

/*
 * Send and receives a byte over the SPI port. 
 */
static void conf_wr(int c)
{
	struct stm32f_spi *spi = ICE40_SPI;
	unsigned int sr;

	while (!((sr = spi->sr) & SPI_TXE)) {
		if (sr & SPI_MODF) {
			return;
		}
	}

	spi->dr = c;
}

static int conf_master_start(void)
{
	/* The application processor (AP) begins by driving the iCE40 
	   CRESET_B pin Low, resetting the iCE40 FPGA. Similarly, the AP holds 
	   the iCE40's CSEL pin Low. The AP must hold the CRESET_B pin Low 
	   for at least 200 ns. Ultimately, the AP either releases the CRESET_B 
	   pin and allows it to float High via the 10 KOhm pull-up resistor to 
	   VCCIO_2 or drives CRESET_B High.  */

	stm32_gpio_clr(IO_ICE40_CSEL);
	stm32_gpio_clr(IO_ICE40_CRESET);
	udelay(100);

	/* The iCE40 FPGA enters SPI peripheral mode when the CRESET_B pin 
	   returns High while the CSEL pin is Low. */

	/* reset */
	stm32_gpio_set(IO_ICE40_CRESET);

	udelay(10);

	if (stm32_gpio_stat(IO_ICE40_CDONE)) {
		stm32_gpio_set(IO_ICE40_CSEL);
		return -2;
	}

	/* After driving CRESET_B High or allowing it to float High the AP must 
	   wait a minimum of 800 us, allowing the iCE40 FPGA to clear its 
	   internal configuration memory */
	udelay(1000);

	return 0;
}

static int lz_readvarsize(uint32_t * x, const uint8_t * buf)
{
	uint32_t y, b;
	unsigned int num_bytes;

	/* Read complete value (stop when byte contains zero in 8:th bit) */
	y = 0;
	num_bytes = 0;
	do {
		b = (uint32_t) (*buf++);
		y = (y << 7) | (b & 0x0000007f);
		++num_bytes;
	} while (b & 0x00000080);

	/* Store value in x */
	*x = y;

	/* Return number of bytes read */
	return num_bytes;
}

extern uint8_t __heap_start[];

int lattice_ice40_lz77_configure(const uint8_t buf[], unsigned int max)
{
	uint8_t *wnd = (uint8_t *) (&__heap_start);
	uint8_t marker;
	uint8_t symbol;
	unsigned int i;
	unsigned int inpos;
	unsigned int outpos;
	unsigned int cfgpos;
	unsigned int length;
	unsigned int offset;
	int ret;

	if ((ret = conf_master_start()) < 0)
		return ret;

	/* Get marker symbol from input stream */
	marker = buf[0];
	inpos = 1;

	/* Main decompression loop */
	outpos = 0;
	cfgpos = 0;
	do {
		symbol = buf[inpos++];
		if (symbol == marker) {
			/* We had a marker byte */
			if (buf[inpos] == 0) {
				/* It was a single occurrence of the marker byte */
				wnd[outpos++ & LZ_WND_MASK] = marker;
				++inpos;
			} else {
				/* Extract true length and offset */
				inpos += lz_readvarsize(&length, &buf[inpos]);
				inpos += lz_readvarsize(&offset, &buf[inpos]);
				/* Copy corresponding data from history window */
				for (i = 0; i < length; ++i) {
					uint8_t c;
					c = wnd[(outpos - offset) & LZ_WND_MASK];
					wnd[outpos & LZ_WND_MASK] = c;
					++outpos;
				}
			}
		} else {
			/* No marker, plain copy */
			wnd[(outpos++ & LZ_WND_MASK)] = symbol;
		}

		/* Program */
		while ((outpos - cfgpos) >= 2) {
			int c;
			c = wnd[(cfgpos + 1) & LZ_WND_MASK] |
			    (wnd[cfgpos & LZ_WND_MASK] << 8);
			conf_wr(c);
			cfgpos += 2;
		}

		if (stm32_gpio_stat(IO_ICE40_CDONE))
			break;
	} while (inpos < max);

	/* padding */
	for (i = 0; i < 1024; ++i) {
		if (stm32_gpio_stat(IO_ICE40_CDONE))
			break;
		conf_wr(0x00);
	}

	if (!stm32_gpio_stat(IO_ICE40_CDONE)) {
		stm32_gpio_set(IO_ICE40_CSEL);
		return -3;
	}

/*	After the CDONE output pin goes High, send at least 49 additional 
	dummy bits, effectively 49 additional SPI_SCK 
	clock cycles measured from rising-edge to rising-edge. */
	for (i = 0; i < 8; ++i)
		conf_wr(0x00);

	stm32_gpio_set(IO_ICE40_CSEL);

	return cfgpos;
}

#if (FPGA_RAW_ENABLE)
int lattice_ice40_raw_configure(const uint8_t * buf, unsigned int max)
{
	unsigned int n;
	unsigned int i;
	int ret;

	while ((ret = conf_master_start()) < 0) {
		return ret;
	}

	for (n = 0; n < max; n += 2) {
		int c;

		if (stm32_gpio_stat(IO_ICE40_CDONE))
			break;

		c = buf[n + 1] | (buf[n] << 8);
		conf_wr(c);
	}

	if (n >= max) {
		for (i = 0; i < 128; ++i) {
			if (stm32_gpio_stat(IO_ICE40_CDONE))
				break;
			conf_wr(0x00);
		}
		if (!stm32_gpio_stat(IO_ICE40_CDONE)) {
			stm32_gpio_set(IO_ICE40_CSEL);
			return -2;
		}
	}

/*	After the CDONE output pin goes High, send at least 49 additional 
	dummy bits, effectively 49 additional SPI_SCK 
	clock cycles measured from rising-edge to rising-edge. */
	for (i = 0; i < 7; ++i)
		conf_wr(0x00);

	stm32_gpio_set(IO_ICE40_CSEL);

	return n;
}
#endif

struct hdr {
	uint32_t size;
	uint32_t magic;
	uint32_t algo;
};

int fpga_configure(void)
{
	struct stm32f_spi *spi = ICE40_SPI;
	unsigned int size = (SIZEOF_ICE40HX1K_BIN - 12);
	uint8_t *bin = (uint8_t *)&ice40hx1k_bin[12];
	int ret = 0;
#if (FPGA_RAW_ENABLE)
	const uint32_t * hdr = (uint32_t *)ice40hx1k_bin;
	uint32_t magic = ntohl(hdr[0]);
	uint32_t algo = ntohl(hdr[1]);
#endif

	/* initialize SPI */
	spi->cr1 = 0;
	spi->i2scfgr = 0;
	spi->i2spr = 0;

	/* 16 bits */
	spi->cr2 = SPI_DS(16);
	/* Master mode, MSB first */
	spi->cr1 = SPI_SPE | SPI_BR_SET(2) | SPI_MSTR | SPI_SSM |
	    SPI_SSI | SPI_CPOL | SPI_CPHA;

#if FPGA_RAW_ENABLE
	if ((magic == BCL1_MAGIC) || (algo == ALGO_LZ77)) {
#endif
		ret = lattice_ice40_lz77_configure(bin, size);
#if FPGA_RAW_ENABLE
	} 
	else 
	{
		ret = lattice_ice40_raw_configure(bin, size);
	}
#endif

	return ret;
}
