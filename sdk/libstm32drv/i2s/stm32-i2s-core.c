/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file stm32f-i2s.c
 * @brief STM32F i2s driver
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __STM32_I2S_I__
#include "stm32-i2s-i.h"
#include <errno.h>

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>


#define PLLI2SR 3
#define PLLI2SN 127
#define PLLI2SM 9
#define PLLI2SQ 8
#define PLLI2SP 2

#if defined(STM32F446)
#if (PLLI2SN < 50)
#error "invalid PLLI2SN!"
#endif

#if (PLLI2SN > 432)
#error "invalid PLLI2SN!"
#endif

#if (PLLI2SM > 63)
#error "invalid PLLI2SM!"
#endif

#if (PLLI2SM < 2)
#error "invalid PLLI2SM!"
#endif

#if (PLLI2SR < 2)
#error "invalid PLLI2SR!"
#endif
#endif

#define __VCOI2S_HZ (((uint64_t)STM32_HSE_HZ * PLLI2SN) / PLLI2SM)
#define __I2S_HZ (__VCOI2S_HZ / PLLI2SR)

#if defined(STM32F446)
void i2s_pll_init(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
	uint32_t cr;
	int again;

	rcc->dckcfgr2 = 0;
	rcc->pllsaicfgr = 0;

	cr = rcc->cr;
	/* disable I2SPLL */
	cr &= ~RCC_PLLI2SON;

	/* configure IS2 PLL */
	rcc->plli2scfgr =  RCC_PLLI2SR(PLLI2SR) | RCC_PLLI2SQ(PLLI2SQ) | 
		RCC_PLLI2SP(PLLI2SP) | RCC_PLLI2SN(PLLI2SN) | RCC_PLLI2SM(PLLI2SM);

	/* enable I2SPLL */
	cr |= RCC_PLLI2SON;
	rcc->cr = cr;;

	for (again = 8192; ; again--) {
		cr = rcc->cr;
		if (cr & RCC_PLLI2SRDY)
			break;
		if (again == 0) {
			/* PLL lock fail */
			return;
		}
	}

	rcc->dckcfgr = I2S2SRC_PLLI2S_R | I2S1SRC_PLLI2S_R;
}
#endif


#if defined(STM32F446)
const uint32_t __stm32f_i2s_hz = __I2S_HZ;
#endif

void stm32_spi_i2s_isr(struct stm32_spi_i2s_drv * drv)
{
	struct stm32f_spi * spi = drv->spi;
	uint32_t sr;

	sr = spi->sr;

	DCC_LOG1(LOG_TRACE, "sr=0x%04x", sr);

	if (sr & SPI_FRE) {
		DCC_LOG(LOG_TRACE, "SPI_FRE");
		drv->spi->cr2 &= ~SPI_ERRIE;
	}

	if (sr & SPI_OVR) {
		DCC_LOG(LOG_TRACE, "SPI_OVR");
		drv->spi->cr2 &= ~SPI_ERRIE;
	}
	
	if (sr & SPI_UDR) {
		DCC_LOG(LOG_TRACE, "SPI_UDR");
		drv->spi->cr2 &= ~SPI_ERRIE;
	}

	if (sr & SPI_TXE) {
		DCC_LOG(LOG_TRACE, "SPI_TXE");
		drv->spi->cr2 &= ~SPI_TXEIE;
	}

	if (sr & SPI_RXNE) {
		DCC_LOG(LOG_TRACE, "SPI_RXNE");
		drv->spi->cr2 &= ~SPI_RXNEIE;
	}

	DCC_LOG(LOG_TRACE, "!");
}

#if 0
void stm32_i2s_dma_tx_isr(struct stm32_spi_i2s_drv * drv)
{
	if (drv->tx.dmactl.isr[TCIF_BIT]) {
		drv->tx.dmactl.ifcr[TCIF_BIT] = 1;
		thinkos_flag_give_i(drv->tx_done);
	}
	if (drv->tx.dmactl.isr[TEIF_BIT]) {
		DCC_LOG(LOG_TRACE, "TEIF");
		/* FIXME: DMA transfer error handling... */
		drv->tx.dmactl.ifcr[TEIF_BIT] = 1;
		/* Disable DMA stream */
		drv->tx.dmactl.strm->cr &= ~DMA_EN;
	}

	if (drv->tx.dmactl.isr[FEIF_BIT]) {
		DCC_LOG(LOG_TRACE, "FEIF");
		/* FIXME: DMA fifo error handling... */
		drv->tx.dmactl.ifcr[FEIF_BIT] = 1;
	}
}

int stm32_i2s_dma_send(struct stm32_spi_i2s_drv * drv, 
					   const int16_t buf[], unsigned int len)
{
	uint32_t idx;
	unsigned int cnt;
	unsigned int rem;
	unsigned int pos;
	int16_t * dst;
	int i;

	if ((pos = drv->tx.pos) == 0) {
		/* wait for the transfer to complete */
		thinkos_flag_take(drv->tx_done);
		DCC_LOG1(LOG_INFO, "idx=%d...", drv->tx.head);
//		idx = drv->tx.idx;
	}

	if ((idx = drv->tx.tail) != (drv->tx.head - 1)) {
		DCC_LOG2(LOG_WARNING, "idx=%d %d", idx, drv->tx.head);
	}

	dst = drv->tx.buf[idx & 1];
	rem = I2S_FRAME_MAX - pos;
	DCC_LOG1(LOG_MSG, "rem=%d", rem);
	cnt = (len > rem) ? rem : len;

	dst += pos;
	for (i = 0; i < cnt; ++i)
		dst[i] = buf[i];

	pos += cnt;
	if (pos == I2S_FRAME_MAX) {
		pos = 0;
		drv->tx.tail = idx + 1;
	}

	drv->tx.pos = pos;
	
	return cnt;
}
#endif

unsigned int stm32_i2s_get_dma_tx_count(struct stm32_spi_i2s_drv * drv)
{
	return drv->tx.dmactl.strm->ndtr;
}

unsigned int stm32_i2s_get_dma_rx_count(struct stm32_spi_i2s_drv * drv)
{
	return drv->rx.dmactl.strm->ndtr;
}

int stm32_i2s_setbuf(struct stm32_spi_i2s_drv * drv,
					 int16_t * buf1, int16_t * buf2, unsigned int len)
{
	bool enabled;
	uint32_t ccr;
	uint32_t cfgr;

	cfgr = drv->spi->i2scfgr;
	
	if (cfgr & SPI_I2SE) {
		enabled = true;
		/* Disable I2S */
		drv->spi->i2scfgr = cfgr & ~SPI_I2SE;
	}

	/* Disable DMA */
	while ((ccr = drv->tx.dmactl.strm->cr) & DMA_EN)
		drv->tx.dmactl.strm->cr = ccr & ~DMA_EN; 

	/* Configure buffers */
	drv->tx.dmactl.strm->m0ar = buf1;
	drv->tx.dmactl.strm->m1ar = buf2;
	drv->tx.dmactl.strm->ndtr = len;

	/* clear transfer complete interrupt flags */
	drv->tx.dmactl.ifcr[TCIF_BIT] = 1; 

	/* */
	drv->tx.head = 0;

	/* enable DMA */
	drv->tx.dmactl.strm->cr |= DMA_EN;

	if (enabled & SPI_I2SE) {
		enabled = true;
		/* Reenable I2S */
		drv->spi->i2scfgr = cfgr;
	}

	return 0;
}

int16_t * stm32_i2s_getbuf(struct stm32_spi_i2s_drv * drv)
{
	int16_t * ptr;

	do {
		YAP("IRQ: %d", drv->tx.dmactl.irqno);
		/* wait for the transfer to complete */
		thinkos_irq_wait(drv->tx.dmactl.irqno);
		/* cleck for errors */
		if (drv->tx.dmactl.isr[TEIF_BIT]) {
			/* FIXME: DMA transfer error handling... */
			drv->tx.dmactl.ifcr[TEIF_BIT] = 1;
			/* Disable DMA stream */
			drv->tx.dmactl.strm->cr &= ~DMA_EN;
			break;
		}
		if (drv->tx.dmactl.isr[FEIF_BIT]) {
			WARNS("I2S FEIF");
			/* FIXME: DMA fifo error handling... */
			drv->tx.dmactl.ifcr[FEIF_BIT] = 1;
		}
	} while (!drv->tx.dmactl.isr[TCIF_BIT]);

	/* Acknowledge transfer */
	drv->tx.dmactl.ifcr[TCIF_BIT] = 1;

	/* Return empty buffer */
	if (drv->tx.dmactl.strm->cr & DMA_CT)
		ptr = (int16_t *)drv->tx.dmactl.strm->m0ar;
	else
		ptr = (int16_t *)drv->tx.dmactl.strm->m1ar;

	YAP("DMA buf: %08x", ptr);
	
	return ptr;
}

int stm32_spi_i2s_init(struct stm32_spi_i2s_drv * drv, 
						   unsigned int samplerate, unsigned int flags,
						   int dma_chan_id)
{
	uint32_t mclk_hz;
	uint32_t div;
	uint32_t fs;
	int odd;

	DCC_LOG2(LOG_TRACE, "SPI%d samplerate=%d", 
			 stm32f_spi_lookup(drv->spi) + 1, samplerate);

#if STM32F_I2S_DMA_TX_ISR
	drv->tx_done = thinkos_flag_alloc();
	DCC_LOG2(LOG_TRACE, "tx_done=%d", drv->tx_done);
#endif

#if STM32F_I2S_DMA_RX_ISR
	drv->rx_idle = thinkos_flag_alloc();
	DCC_LOG2(LOG_TRACE, "rx_idle=%d", drv->rx_idle);
#endif

	drv->tx.head = 0;
	drv->tx.tail = 0;
	drv->tx.pos = 0;

	if (flags & I2S_TX_EN) {
		/* -------------------------------------------------------
		   Configure TX DMA stream
		 */
		/* Disable DMA stream */
		drv->tx.dmactl.strm->cr = 0;
		/* Wait for the channel to be ready .. */
		while (drv->tx.dmactl.strm->cr & DMA_EN); 
		drv->tx.dmactl.strm->par = &drv->spi->dr;
		/* configure TX DMA stream */
		drv->tx.dmactl.strm->cr = DMA_CHSEL_SET(dma_chan_id) | 
			DMA_MBURST_1 | DMA_PBURST_1 | DMA_MSIZE_16 | DMA_PSIZE_16 | 
			DMA_CT_M0AR | DMA_DBM | DMA_CIRC | DMA_MINC | DMA_DIR_MTP |
			DMA_TCIE | DMA_TEIE | DMA_DMEIE;
	
		drv->tx.dmactl.strm->m0ar = 0;
		drv->tx.dmactl.strm->m1ar = 0;
		drv->tx.dmactl.strm->ndtr = 0;
		/* cofigure FIFO */
		drv->tx.dmactl.strm->fcr = DMA_FEIE | DMA_DMDIS | DMA_FTH_FULL;

		/* ------------------------------------------------------- 
		 */
	
		DCC_LOG(LOG_TRACE, "TX enabled");
	}

	if (flags & I2S_RX_EN) {
		/* -------------------------------------------------------
		   Configure RX DMA stream
		 */
		/* Disable DMA stream */
		drv->rx.dmactl.strm->cr = 0;
		/* Wait for the channel to be ready .. */
		while (drv->rx.dmactl.strm->cr & DMA_EN); 
		/* peripheral address */
		drv->rx.dmactl.strm->par = &drv->spi->dr;
		/* configure DMA */
		drv->rx.dmactl.strm->cr = DMA_CHSEL_SET(dma_chan_id) | 
			DMA_MSIZE_16 | DMA_PSIZE_16 | DMA_MINC |
			DMA_DIR_PTM | DMA_TCIE | DMA_TEIE;
		/* cofigure FIFO */
		drv->rx.dmactl.strm->fcr = DMA_FEIE | DMA_DMDIS | DMA_FTH_FULL;
		/* ------------------------------------------------------- 
		 */
	}

	/* CR1 is not used in I2S mode */
	drv->spi->cr1 = 0;
	/* Clear configuration */
	drv->spi->cr2 = 0;
	/* Clear pending interrupts */
	drv->spi->sr = 0;

	/* Configure clock */
	mclk_hz = samplerate * 256;
	/* get the total divisior */
#ifdef STM32F446X
	div = (__stm32f_i2s_hz + (mclk_hz / 2)) / mclk_hz;
#else
	div = (stm32f_apb1_hz + (mclk_hz / 2)) / mclk_hz;
#endif
	/* adjust the odd bit and divisor */
	odd = div & 1;
	div /= 2;
	drv->spi->i2spr = SPI_I2SDIV_SET(div) | (odd ? SPI_ODD : 0);

	if (flags & I2S_MCK_EN) {
		DCC_LOG(LOG_TRACE, "MCKO enabled");
		drv->spi->i2spr |= SPI_MCKOE;
		fs = stm32f_i2s_hz / ((32*2)*((2*div)+odd)*4);
		INF("MCLK=%d Hz, ", fs * 256);
	} else
		fs = stm32f_i2s_hz / ((32*2)*((2*div)+odd));

	(void)fs;
	INF("Fs=%d Hz, ", fs);
	INF("I2SCLK=%d Hz, ", stm32f_i2s_hz);
	INF("div=%d odd=%d, ", div, odd);

	/* Configure I2S */
	drv->spi->i2scfgr = SPI_I2SMOD | SPI_I2SSTD_PHILIPS;

	/* Adjust the word length */
	if (flags & I2S_24BITS)
		drv->spi->i2scfgr |= SPI_DATLEN_24;
	else if (flags & I2S_32BITS)
		drv->spi->i2scfgr |= SPI_DATLEN_32;
	else {
		drv->spi->i2scfgr |= SPI_DATLEN_16;
		/* FIXME configure channel length, default 32bits */
		drv->spi->i2scfgr |= SPI_CHLEN;
	}

	/* Select the transfer direction */
	if (flags & I2S_TX_EN)
		drv->spi->i2scfgr |= SPI_I2SCFG_MST_XMT;
	else if (flags & I2S_RX_EN)
		drv->spi->i2scfgr |= SPI_I2SCFG_MST_RCV;

	/* Set the DMA transfer enable bits ac*/
	if (flags & I2S_TX_EN)
		drv->spi->cr2 |= SPI_TXDMAEN;
	if (flags & I2S_RX_EN)
		drv->spi->cr2 |= SPI_RXDMAEN;

#if STM32F_I2S_SPI_ISR
	drv->spi->cr2 |= SPI_ERRIE;
#endif

	/* enable DMA */
#if STM32F_I2S_DMA_TX_ISR
	if (flags & I2S_TX_EN)
		drv->tx.dmactl.strm->cr |= DMA_EN;
#endif

#if STM32F_I2S_DMA_RX_ISR
	if (flags & I2S_RX_EN)
		drv->rx.dmactl.strm->cr |= DMA_EN;
#endif

	return 0;
}

int stm32_i2s_close(struct stm32_spi_i2s_drv * drv)
{
	DCC_LOG(LOG_TRACE, "...");
	return 0;
}

int stm32_i2s_ioctl(struct stm32_spi_i2s_drv * drv, int opt, 
							uintptr_t arg1, uintptr_t arg2)
{
	struct stm32f_spi * spi = drv->spi;

	switch (opt) {
	case I2S_IOCTL_ENABLE:
		if (arg1)
			/* enable I2S */
			spi->i2scfgr |= SPI_I2SE;
		else
			/* disable I2S */
			spi->i2scfgr &= ~SPI_I2SE;
	break;

	default:
		return -EINVAL;
	}

	return 0;
}

const struct i2s_op stm32_spi_i2s_op = {
	.close = (void *)stm32_i2s_close,
	.ioctl = (void *)stm32_i2s_ioctl,
	.setbuf = (void *)stm32_i2s_setbuf,
	.getbuf = (void *)stm32_i2s_getbuf,
	.get_dma_tx_cnt = (void *)stm32_i2s_get_dma_tx_count,
	.get_dma_rx_cnt = (void *)stm32_i2s_get_dma_rx_count
};

