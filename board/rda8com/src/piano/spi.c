/* 
 * File:	 spi.c
 * Author:   Vijaykumar Trambadia (vtrambadia@mircomgroup.com)
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

_Pragma("GCC optimize (\"Ofast\")")

#include <sys/stm32f.h>
#include <thinkos.h>

#include <stdio.h>
#include <fixpt.h>

#include "board.h"

#define IRQ_ICE40_SPI STM32_IRQ_SPI1

struct spi_ctrl {
	int16_t mutex;
	int16_t flag;
	volatile uint32_t ctrl;
	volatile uint32_t stat;
	volatile uint32_t seq;
};

/* Chip select high */
static inline void __spi_cs_set(void) {
	stm32_gpio_set(IO_ICE40_SPI_SS);
}

/* Chip select low */
static inline void __spi_cs_clr(void) {
	stm32_gpio_clr(IO_ICE40_SPI_SS);
}

static int __spi_master_init(unsigned int freq)
{
	struct stm32f_spi *spi = ICE40_SPI;
	unsigned int div;
	unsigned int br;

	stm32_gpio_mode(IO_ICE40_SPI_SS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SS, ICE40_SPI_AF);
	stm32_gpio_mode(IO_ICE40_SPI_SCK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SCK, ICE40_SPI_AF);
	stm32_gpio_mode(IO_ICE40_SPI_SDI, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SDI, ICE40_SPI_AF);
	stm32_gpio_mode(IO_ICE40_SPI_SDO, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SDO, ICE40_SPI_AF);

	/* Configure SPI */
	div = (stm32_clk_hz(ICE40_CLK_SPI) / 2) / freq;
	br = ilog2(div);
	if (div > (unsigned int)(1 << br))
		br++;
	if (br > 7)
		br = 7;

	freq = stm32_clk_hz(ICE40_CLK_SPI) / (2 << br);
	printf("SPI: freq=%d Hz %d Sps\n", freq, freq / 16);

	spi->cr1 = 0;
	spi->i2scfgr = 0;
	spi->i2spr = 0;

	/* 8 bits */
	spi->cr2 = SPI_DS(16) | SPI_SSOE | SPI_NSSP;
	/* Enable RX interrupt */
	spi->cr2 |= SPI_RXNEIE;
	/* Master mode, MSB first, Clock default state is 1 */
	spi->cr1 = SPI_BR_SET(br) | SPI_MSTR;

	return 0;
}

static inline void __spi_enable(void)
{
	struct stm32f_spi *spi = ICE40_SPI;
	uint32_t dummy;
	uint32_t sr;

	/* Disable SPI */
	spi->cr1 &= ~SPI_SPE;
	/* Enable RX interrupt */
	spi->cr2 |= SPI_RXNEIE;

	while ((sr = spi->sr) & SPI_RXNE) {
		/* clear input buffer */
		dummy = spi->dr;
		(void)dummy;
		sr = spi->sr;
	}

	/* Enable SPI */
	spi->cr1 |= SPI_SPE;
}

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

static inline uint32_t __spi_io_xfer16(uint32_t txd)
{
	struct stm32f_spi *spi = ICE40_SPI;
	uint32_t sr;

	/* send data */
	spi->dr = txd;
	/* wait for incoming data */
	while (!((sr = spi->sr) & SPI_RXNE))
		thinkos_irq_wait(IRQ_ICE40_SPI);
	return spi->dr;
}

static inline uint32_t __spi_io_xfer8(uint32_t txd)
{
	struct stm32f_spi *spi = ICE40_SPI;
	uint32_t sr;

	/* send data */
	spi->dr = (txd & 0xff);// | (txd << 8);
	/* wait for incoming data */
	while (!((sr = spi->sr) & SPI_RXNE))
		thinkos_irq_wait(IRQ_ICE40_SPI);
	return spi->dr & 0xff;
}

int spi_io_task(struct spi_ctrl * dev)
{
	uint32_t stat[1];
	uint32_t ctrl;
	uint32_t diff;

	__spi_enable();

	ctrl = dev->ctrl;
	stat[1] = 0xffff;
	for (;;) {
		thinkos_sleep(1);
		ctrl = dev->ctrl;
		stat[0] = stat[1];
		stat[1] = __spi_io_xfer16(ctrl);
		if (stat[1] != stat[0]) {
			/* deboucing */
			continue;
		}
		diff = stat[1] ^ dev->stat;
		if (diff != 0) {
//			printf("IO: stat=0x%02x diff=0x%02x\n", stat, diff);
			dev->stat = stat[1];
			thinkos_flag_give(dev->flag);
		}
	}
}

uint32_t spi_io_stack[64] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf spi_io_thread_inf = {
	.stack_ptr = spi_io_stack,
	.stack_size = sizeof(spi_io_stack),
	.priority = 9,
	.thread_id = 9,
	.paused = 0,
	.tag = "SPI-IO"
};

static struct spi_ctrl spi_ctrl_rt;

void spidev_wr(int dat)
{
	struct spi_ctrl * dev = &spi_ctrl_rt;

	thinkos_mutex_lock(dev->mutex);
	thinkos_flag_take(dev->flag);
	dev->ctrl = dat;
	thinkos_mutex_unlock(dev->mutex);
}

int spidev_rd(void)
{
	struct spi_ctrl * dev = &spi_ctrl_rt;
	int stat;

	thinkos_mutex_lock(dev->mutex);

	thinkos_flag_take(dev->flag);
	stat = dev->stat;

	thinkos_mutex_unlock(dev->mutex);

	return stat;
}

void spidev_init(void)
{
	struct spi_ctrl * dev = &spi_ctrl_rt;

	__spi_master_init(11025*31);
	dev->mutex = thinkos_mutex_alloc();

	dev->flag = thinkos_flag_alloc();
	dev->stat = 0;
	dev->ctrl = 0;

	thinkos_thread_create_inf((int (*)(void *))spi_io_task, (void *)dev, 
							  &spi_io_thread_inf);
}

