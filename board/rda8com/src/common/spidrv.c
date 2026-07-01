/* 
 * File:	 spidrv.c
 * Author:   
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

#include <stdio.h>
#include <fixpt.h>
#include <sys/stm32f.h>
#include <thinkos.h>

#include "board.h"

#ifndef SPIDRV_ENABLE_POLLING_TASK 
#define SPIDRV_ENABLE_POLLING_TASK 0
#endif

#ifndef SPIDRV_ENABLE_GPIO_INIT
#define SPIDRV_ENABLE_GPIO_INIT 0
#endif

/* Chip select high */
static inline void __spi_cs_set(void) {
	stm32_gpio_set(IO_ICE40_SPI_SS);
}

/* Chip select low */
static inline void __spi_cs_clr(void) {
	stm32_gpio_clr(IO_ICE40_SPI_SS);
}

static int __spi_master_init(struct stm32f_spi *spi, unsigned int freq)
{
	unsigned int div;
	unsigned int br;

	/* Configure SPI */
	div = (stm32_clk_hz(ICE40_CLK_SPI) / 2) / freq;
	br = ilog2(div);
	if (div > (unsigned int)(1 << br))
		br++;
	if (br > 7)
		br = 7;

	freq = stm32_clk_hz(ICE40_CLK_SPI) / (2 << br);

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

static inline void __spi_enable(struct stm32f_spi *spi)
{
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

static inline uint32_t __spi_io_xfer16(struct stm32f_spi *spi, 
									   int irq, uint32_t txd)
{
	uint32_t sr;

	/* send data */
	spi->dr = txd;
	/* wait for incoming data */
	while (!((sr = spi->sr) & SPI_RXNE)) {
		thinkos_irq_wait(irq);
	}

	return spi->dr;
}

static inline uint32_t __spi_io_xfer8(struct stm32f_spi *spi, 
									  int irq, uint32_t txd)
{
	uint32_t sr;

	/* send data */
	spi->dr = (txd & 0xff);// | (txd << 8);
	/* wait for incoming data */
	while (!((sr = spi->sr) & SPI_RXNE))
		thinkos_irq_wait(irq);
	return spi->dr & 0xff;
}

#if SPIDRV_ENABLE_POLLING_TASK

struct spidrv {
	int16_t mutex;
	int16_t flag;
	volatile uint32_t ctrl;
	volatile uint32_t stat;
};

int spi_io_task(struct spidrv * drv)
{
	struct stm32f_spi *spi = STM32F_SPI1;
	uint32_t stat[1];
	uint32_t ctrl;
	uint32_t diff;

	__spi_enable(spi);

	ctrl = drv->ctrl;
	stat[1] = 0xffff;
	for (;;) {
		thinkos_sleep(1);
		ctrl = drv->ctrl;
		stat[0] = stat[1];
		stat[1] = __spi_io_xfer16(spi, STM32_IRQ_SPI1, ctrl);
		if (stat[1] != stat[0]) {
			/* debouncing */
			continue;
		}
		diff = stat[1] ^ drv->stat;
		if (diff != 0) {
//			printf("IO: stat=0x%02x diff=0x%02x\n", stat, diff);
			drv->stat = stat[1];
			thinkos_flag_give(drv->flag);
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

static struct spidrv spidrv_rt;

void spidrv_wr(int dat)
{
	struct spidrv * drv = &spidrv_rt;

	thinkos_mutex_lock(drv->mutex);
	thinkos_flag_take(drv->flag);
	drv->ctrl = dat;
	thinkos_mutex_unlock(drv->mutex);
}

int spidrv_rd(void)
{
	struct spidrv * drv = &spidrv_rt;
	int stat;

	thinkos_mutex_lock(drv->mutex);

	thinkos_flag_take(drv->flag);
	stat = drv->stat;

	thinkos_mutex_unlock(drv->mutex);

	return stat;
}

#else  /* SPIDRV_ENABLE_POLLING_TASK */

uint32_t spidrv_xfer(uint32_t data) 
{
	struct stm32f_spi *spi = STM32F_SPI1;

	return __spi_io_xfer16(spi, STM32_IRQ_SPI1, data);
}

#endif /* SPIDRV_ENABLE_POLLING_TASK */

void spidrv_master_init(unsigned int freq)
{
	struct stm32f_spi *spi = STM32F_SPI1;
#if SPIDRV_ENABLE_POLLING_TASK 
	struct spidrv * drv = &spidrv_rt;
#endif

	__spi_master_init(spi, freq);

#if SPIDRV_ENABLE_POLLING_TASK 
	drv->mutex = thinkos_mutex_alloc();

	drv->flag = thinkos_flag_alloc();
	drv->stat = 0;
	drv->ctrl = 0;

	thinkos_thread_create_inf((int (*)(void *))spi_io_task, (void *)drv, 
							  &spi_io_thread_inf);
#else
	__spi_enable(spi);
#endif /* SPIDRV_ENABLE_POLLING_TASK */
}

