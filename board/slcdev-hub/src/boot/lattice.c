/* 
 * File:	 spi-test.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <sys/stm32f.h>
#include <sys/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <sys/dcclog.h>

#include "board.h"

/*--------------------------------------------------------------------------
 * Lattice ICE40 FPGA Configuration
 ---------------------------------------------------------------------------*/

/*
 * Initialize IO pins and SPI low level device 
 */
static int lattice_ice40_io_init(unsigned int freq)
{
	struct stm32f_spi * spi = ICE40_SPI;
	unsigned int div;
	int br;

	/* Enable peripheral clock */;
	stm32_clk_enable(STM32_RCC, ICE40_CLK_SPI);

	/* Configure IO pins */
	stm32_gpio_mode(IO_ICE40_CDONE, INPUT, PULL_UP);

	stm32_gpio_set(IO_ICE40_CRESET);
	stm32_gpio_mode(IO_ICE40_CRESET, OUTPUT, SPEED_MED);

	stm32_gpio_set(IO_ICE40_SPI_SS);
	stm32_gpio_mode(IO_ICE40_SPI_SS, OUTPUT, SPEED_MED);

	stm32_gpio_mode(IO_ICE40_SPI_SCK, ALT_FUNC, PUSH_PULL | SPEED_LOW);
	stm32_gpio_af(IO_ICE40_SPI_SCK, ICE4_SPI_AF);

	stm32_gpio_mode(IO_ICE40_SPI_SDO, ALT_FUNC, PULL_UP);
	stm32_gpio_af(IO_ICE40_SPI_SDO, ICE4_SPI_AF);

	stm32_gpio_mode(IO_ICE40_SPI_SDI, ALT_FUNC, PUSH_PULL | SPEED_MED);
	stm32_gpio_af(IO_ICE40_SPI_SDI, ICE4_SPI_AF);

	/* Configure SPI */
	div = stm32_clk_hz(ICE40_CLK_SPI) / freq / 2;
	br = 31 - __clz(div);
	if (div > (1 << br))
		br++;

    DCC_LOG3(LOG_TRACE, "SPI freq=%d div=%d br=%d", freq, div, br);

	spi->cr1 = 0;
	spi->cr2 = 0;
	spi->i2scfgr = 0;
	spi->i2spr = 0;

	/* Master mode, MSB first */
	spi->cr1 = SPI_SPE | SPI_BR_SET(br) | SPI_MSTR | SPI_SSM | SPI_SSI;

	return 0;
}

static int conf_start(void)
{
/* The application processor (AP) begins by driving the iCE40 
   CRESET_B pin Low, resetting the iCE40 FPGA. Similarly, the AP holds 
   the iCE40's SPI_SS_B pin Low. The AP must hold the CRESET_B pin Low 
   for at least 200 ns. Ultimately, the AP either releases the CRESET_B 
   pin and allows it to float High via the 10 KOhm pull-up resistor to 
   VCCIO_2 or drives CRESET_B High.  */

/* The iCE40 FPGA enters SPI peripheral mode when the CRESET_B pin 
   returns High while the SPI_SS_B pin is Low. */
	stm32_gpio_clr(IO_ICE40_SPI_SS);

	/* reset */
	stm32_gpio_clr(IO_ICE40_CRESET);
	udelay(1);
	stm32_gpio_set(IO_ICE40_CRESET);

	if (stm32_gpio_stat(IO_ICE40_CDONE)) {
		stm32_gpio_set(IO_ICE40_SPI_SS);
		return -1;
	}

/* After driving CRESET_B High or allowing it to float High the AP must 
   wait a minimum of 300 �s, allowing the iCE40 FPGA to clear its 
   internal configuration memory */
	udelay(300);

	return 0;
}

/*
 * Send and receives a byte over the SPI port. 
 */
static void conf_wr(int c)
{
	struct stm32f_spi * spi = ICE40_SPI;

	stm32f_spi_putc(spi, c);

	c = stm32f_spi_getc(spi);
	(void)c;
}

/*
 * Configure an ICE40 FPGA device 
 */
int lattice_ice40_configure(const uint8_t * buf, unsigned int max)
{
	int ret;
	int n;
	int i;

	lattice_ice40_io_init(50000);
	
	DCC_LOG2(LOG_TRACE, "bin=0x%08x max=%d", buf, max);

	while ((ret = conf_start()) < 0) {
		DCC_LOG(LOG_ERROR, "conf_start() failed!");
		return ret;
	}

	for (n = 0; n < max; ++n) {
		if (stm32_gpio_stat(IO_ICE40_CDONE))
			break;
		conf_wr(buf[n]);
	}

	if (n >= max) {
		for (i = 0; i < 128; ++i) {
			if (stm32_gpio_stat(IO_ICE40_CDONE))
				break;
			conf_wr(0x00);
		}
		if (!stm32_gpio_stat(IO_ICE40_CDONE)) {
			stm32_gpio_set(IO_ICE40_SPI_SS);
			return -2;
		}
	}

	DCC_LOG1(LOG_TRACE, "%d bytes", n);

/*	After the CDONE output pin goes High, send at least 49 additional 
	dummy bits, effectively 49 additional SPI_SCK 
	clock cycles measured from rising-edge to rising-edge. */
	for (i = 0; i < 7; ++i)
		conf_wr(0x00);

	stm32_gpio_set(IO_ICE40_SPI_SS);

	return n;
}

