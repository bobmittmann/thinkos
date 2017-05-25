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

void stm32_i2s_dma_rx_isr(struct stm32_spi_i2s_drv * drv)
{
	if (drv->rx.dmactl.isr[TCIF_BIT]) {
#if I2S_ENABLE_STATS
		drv->stats.rx_dmatc++;
#endif
		DCC_LOG(LOG_TRACE, "TCIF");
		/* clear the RX DMA transfer complete flag */
		drv->rx.dmactl.ifcr[TCIF_BIT] = 1;
/*	XXX: slcdev debug.
 * thinkos_flag_give_i(drv->rx_idle); */
	}
	if (drv->rx.dmactl.isr[TEIF_BIT]) {
#if I2S_ENABLE_STATS
		drv->stats.rx_dmate++;
#endif
		DCC_LOG(LOG_TRACE, "TEIF");
		/* FIXME: DMA transfer error handling... */
		drv->rx.dmactl.ifcr[TEIF_BIT] = 1;
//		thinkos_flag_give_i(drv->rx_idle);
		/* Disable DMA stream */
		drv->rx.dmactl.strm->cr &= ~DMA_EN;
	}
	DCC_LOG(LOG_TRACE, "!");
}

int stm32_i2s_dma_recv(struct stm32_spi_i2s_drv * drv, 
						   void * buf, unsigned int len, unsigned int tmo)
{
	unsigned int ndtr;
	unsigned int cnt;
	uint32_t cr;
	int ret;

	DCC_LOG3(LOG_INFO, "%6d: len=%d tmo=%d", thinkos_clock(), len, tmo);

	/* if the stored buffer pointer is NULL it means that
	   we did not yet prepare other DMA for a transfer,
	   in this case prepare to transfer using the provided 
	   buffer. */
	if (drv->rx.buf_ptr == NULL) {
		DCC_LOG(LOG_MSG, "DMA not prepared...");
		if ((cr = drv->rx.dmactl.strm->cr) & DMA_EN) {
			DCC_LOG(LOG_ERROR, "DMA enabled");
#if 0
			return -11;
#endif
			/* Disable DMA stream */
			drv->rx.dmactl.strm->cr = 0;
			/* Wait for the channel to be ready .. */
			while (drv->rx.dmactl.strm->cr & DMA_EN);
		}

		/* clear transfer complete interrupt flags */
		drv->rx.dmactl.ifcr[TCIF_BIT] = 1; 
		/* Set memory address */
		drv->rx.dmactl.strm->m0ar = buf;
		/* Number of data items to transfer */
		drv->rx.dmactl.strm->ndtr = len;
		/* enable DMA */
		drv->rx.dmactl.strm->cr = cr | DMA_TCIE | DMA_EN;
		ndtr = len;
	} else {
		DCC_LOG(LOG_MSG, "DMA prepared...");
		ndtr = drv->rx.buf_len;
	}

	/* wait for the end of transfer */
//	DCC_LOG1(LOG_TRACE, "thinkos_flag_timedtake(%d)...", tmo);
	if ((ret = thinkos_flag_timedtake(drv->rx_idle, tmo)) < 0) {
		/* if the initially provided 'ndtr' differs from the
		   DMA stream then the transfer already started,
		   in this case we wait until it finishes. */
		if ((cnt = ndtr - drv->rx.dmactl.strm->ndtr) < 2) {
			if (drv->rx.buf_ptr == NULL) {
				/* Disable DMA stream */
				drv->rx.dmactl.strm->cr &= ~(DMA_TCIE | DMA_EN);
			}
			DCC_LOG(LOG_INFO, "timeout...");
			return ret;
		} else {
			DCC_LOG1(LOG_WARNING, "thinkos_flag_take() cnt=%d  ...", cnt);
			thinkos_flag_take(drv->rx_idle);
		}
	}

	/* Number of data items transfered... */
	cnt = ndtr - drv->rx.dmactl.strm->ndtr;

	if (drv->rx.buf_ptr != NULL) {
		/* prepare next transfer */
		drv->rx.buf_ptr = buf;
		drv->rx.buf_len = len;
		if (buf != NULL) {
			/* prepare next transfer */
			/* clear transfer complete interrupt flags */
			drv->rx.dmactl.ifcr[TCIF_BIT] = 1; 
			/* Memory address */
			drv->rx.dmactl.strm->m0ar = (void *)buf;
			/* Number of data items to transfer */
			drv->rx.dmactl.strm->ndtr = len;
			/* enable DMA */
			drv->rx.dmactl.strm->cr |= DMA_TCIE | DMA_EN;
		}
	}

	if (cnt == 0)
		DCC_LOG1(LOG_WARNING, "%6d: DMA XFR cnt == 0!!!", thinkos_clock_i());
	else
		DCC_LOG2(LOG_INFO, "%6d: cnt=%d", thinkos_clock_i(), cnt);

	return cnt;
}

#if 0
void stm32_i2s_dma_tx_isr(struct stm32_spi_i2s_drv * drv)
{
	if (drv->tx.dmactl.isr[TCIF_BIT]) {
		DCC_LOG(LOG_INFO, "TCIF");
		/* clear the TX DMA transfer complete flag */
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
}
#endif

void stm32_i2s_dma_tx_isr(struct stm32_spi_i2s_drv * drv)
{
	if (drv->tx.dmactl.isr[TCIF_BIT]) {
//		DCC_LOG(LOG_TRACE, "TCIF");
		/* clear the TX DMA transfer complete flag */
		drv->tx.dmactl.ifcr[TCIF_BIT] = 1;
		drv->tx.head++;
//		if (drv->tx.dmactl.strm->cr & DMA_CT) {
//			drv->tx.dmactl.strm->m0ar = drv->tx.buf[0];
//			drv->tx.idx = 1;
//			DCC_LOG(LOG_MSG, "1.");
//		} else {
//			drv->tx.dmactl.strm->m1ar = drv->tx.buf[1];
//			drv->tx.idx = 0;
//			DCC_LOG(LOG_MSG, "0.");
//		}
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
		/* FIXME: DMA transfer error handling... */
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

int stm32_i2s_enable(struct stm32_spi_i2s_drv * drv)
{
	/* enable I2S */
	drv->spi->i2scfgr |= SPI_I2SE;

	return 0;
}

int stm32_i2s_setbuf(struct stm32_spi_i2s_drv * drv,
					 int16_t * buf1, int16_t * buf2, unsigned int len)
{
	uint32_t ccr;

	/* disable I2S */
	drv->spi->i2scfgr &= ~SPI_I2SE;
	/* Disable DMA */
	while ((ccr = drv->tx.dmactl.strm->cr) & DMA_EN)
		drv->tx.dmactl.strm->cr = ccr & ~DMA_EN; 

	drv->tx.dmactl.strm->m0ar = buf1;
	drv->tx.dmactl.strm->m1ar = buf2;
	drv->tx.dmactl.strm->ndtr = len;

	/* clear transfer complete interrupt flags */
	drv->tx.dmactl.ifcr[TCIF_BIT] = 1; 

	/* */
	drv->tx.head = 0;

	/* enable DMA */
	drv->tx.dmactl.strm->cr |= DMA_EN;

	return 0;
}


int stm32_spi_i2s_init(struct stm32_spi_i2s_drv * drv, 
						   unsigned int samplerate, unsigned int flags,
						   int dma_chan_id)
{
	uint32_t mclk_hz;
	uint32_t div;
	uint32_t fs;
	int odd;
	int i;

	DCC_LOG2(LOG_TRACE, "SPI%d samplerate=%d", 
			 stm32f_spi_lookup(drv->spi) + 1, samplerate);

	drv->rx_idle = thinkos_flag_alloc();
	drv->tx_done = thinkos_flag_alloc();
	DCC_LOG2(LOG_TRACE, "rx_idle=%d tx_done=%d", drv->rx_idle, 
			 drv->tx_done);

#if I2S_TX_MUTEX
	drv->tx_mutex = thinkos_mutex_alloc(); 
	DCC_LOG1(LOG_TRACE, "tx_mutex=%d", drv->tx_mutex);
#endif

	drv->rx.buf_ptr = NULL;
	drv->rx.buf_len = 0;
	drv->tx.head = 0;
	drv->tx.tail = 0;
	drv->tx.pos = 0;

	for (i = 0; i < I2S_FRAME_MAX; ++i) {
		drv->tx.buf[0][i] = 0;
		drv->tx.buf[1][i] = 0;
	}

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
//		drv->tx.dmactl.strm->cr = DMA_CHSEL_SET(dma_chan_id) | 
//			DMA_MSIZE_16 | DMA_PSIZE_16 | DMA_MINC | 
//			DMA_DIR_MTP | DMA_TCIE | DMA_TEIE;
		drv->tx.dmactl.strm->cr = DMA_CHSEL_SET(dma_chan_id) | 
			DMA_MBURST_1 | DMA_PBURST_1 | DMA_MSIZE_16 | DMA_PSIZE_16 | 
			DMA_CT_M0AR | DMA_DBM | DMA_CIRC | DMA_MINC | DMA_DIR_MTP |
			DMA_TCIE | DMA_TEIE | DMA_DMEIE;
	
		drv->tx.dmactl.strm->m0ar = drv->tx.buf[0];
		drv->tx.dmactl.strm->m1ar = drv->tx.buf[1];
		drv->tx.dmactl.strm->ndtr = I2S_FRAME_MAX;
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
		/* cofigure FIFO */
		//	drv->rx.dmactl.strm->fcr = DMA_FEIE | DMA_DMDIS | DMA_FTH_FULL;
		/* configure DMA */
		drv->rx.dmactl.strm->cr = DMA_CHSEL_SET(dma_chan_id) | 
			DMA_MSIZE_16 | DMA_PSIZE_16 | DMA_MINC |
			DMA_DIR_PTM | DMA_TCIE | DMA_TEIE;
		/* ------------------------------------------------------- 
		 */
	}

	/* CR1 is not used in I2S mode */
	drv->spi->cr1 = 0;
	/* Clear pending interrupts */
	drv->spi->sr = 0;

	drv->spi->cr2 = 0;
	if (flags & I2S_TX_EN)
		drv->spi->cr2 |= SPI_TXDMAEN;
	if (flags & I2S_RX_EN)
		drv->spi->cr2 |= SPI_RXDMAEN;

	mclk_hz = samplerate * 256;

#ifdef STM32F446X
	/* get the total divisior */
	div = (stm32f_i2s_hz + (mclk_hz / 2)) / mclk_hz;
#else
	div = (stm32f_apb1_hz + (mclk_hz / 2)) / mclk_hz;
#endif

	odd = div & 1;
	div /= 2;

	DCC_LOG1(LOG_TRACE, "I2SCLK=%d Hz, ", stm32f_i2s_hz);
	DCC_LOG1(LOG_TRACE, "div=%d, ", div);

	drv->spi->i2spr = SPI_I2SDIV_SET(div) | (odd ? SPI_ODD : 0);
	
	if (flags & I2S_MCK_EN) {
		DCC_LOG(LOG_TRACE, "MCKO enabled");
		drv->spi->i2spr |= SPI_MCKOE;
		fs = stm32f_i2s_hz / ((32*2)*((2*div)+odd)*4);
		DCC_LOG1(LOG_TRACE, "MCLK=%d Hz, ", fs * 256);
	} else
		fs = stm32f_i2s_hz / ((32*2)*((2*div)+odd));

	(void)fs;
	DCC_LOG1(LOG_TRACE, "Fs=%d Hz, ", fs);

	drv->spi->i2scfgr = SPI_I2SMOD | SPI_I2SSTD_PHILIPS;

	if (flags & I2S_24BITS)
		drv->spi->i2scfgr |= SPI_DATLEN_24;
	else if (flags & I2S_32BITS)
		drv->spi->i2scfgr |= SPI_DATLEN_32;
	else {
		drv->spi->i2scfgr |= SPI_DATLEN_16;
		/* FIXME configure channel length, default 32bits */
		drv->spi->i2scfgr |= SPI_CHLEN;
	}


	if (flags & I2S_TX_EN)
		drv->spi->i2scfgr |= SPI_I2SCFG_MST_XMT;
	else if (flags & I2S_RX_EN)
		drv->spi->i2scfgr |= SPI_I2SCFG_MST_RCV;

//	drv->spi->cr2 |= SPI_TXEIE | SPI_RXNEIE | SPI_ERRIE;
	drv->spi->cr2 |= SPI_ERRIE;

	/* enable DMA */
	if (flags & I2S_TX_EN)
		drv->tx.dmactl.strm->cr |= DMA_EN;
	if (flags & I2S_RX_EN)
		drv->rx.dmactl.strm->cr |= DMA_EN;

	return 0;
}

int stm32_i2s_dma_close(struct stm32_spi_i2s_drv * drv)
{
	DCC_LOG(LOG_TRACE, "...");
	return 0;
}

int stm32_i2s_dma_ioctl(struct stm32_spi_i2s_drv * drv, int opt, 
							uintptr_t arg1, uintptr_t arg2)
{
	struct stm32f_spi * spi = drv->spi;
//	unsigned int msk = 0;

	(void)spi;
	DCC_LOG(LOG_TRACE, "...");
	switch (opt) {
#if 0
	case I2S_IOCTL_ENABLE:
		DCC_LOG(LOG_TRACE, "I2S_IOCTL_ENABLE");
		msk |= (arg1 & I2S_RX_EN) ? USART_RE : 0;
		msk |= (arg1 & I2S_TX_EN) ? USART_TE : 0;
		spi->cr1 |= msk;
		break;

	case I2S_IOCTL_DISABLE:
		DCC_LOG(LOG_TRACE, "I2S_IOCTL_DISABLE");
		msk |= (arg1 & I2S_RX_EN) ? USART_RE : 0;
		msk |= (arg1 & I2S_TX_EN) ? USART_TE : 0;
		spi->cr1 &= ~msk;
		break;

	case I2S_IOCTL_DRAIN:
		DCC_LOG(LOG_TRACE, "I2S_IOCTL_DRAIN");
		return stm32f_i2s_dma_drain(drv);

	case I2S_IOCTL_FLOWCTRL_SET: 
		switch (arg1) { 
		case I2S_FLOWCTRL_NONE:
		case I2S_FLOWCTRL_RTSCTS:
			break;
		case I2S_FLOWCTRL_XONXOFF:
			break;
		}
		break;

	case I2S_IOCTL_CONF_SET: 
		{
			struct i2s_config * cfg = (struct i2s_config *)arg1;
			uint32_t flags;
			DCC_LOG(LOG_TRACE, "I2S_IOCTL_CONF_SET");

			stm32_usart_samplerate_set(spi, cfg->samplerate);
			flags = CFG_TO_FLAGS(cfg);
			stm32_usart_mode_set(spi, flags);
		}
		break;

	case I2S_IOCTL_DMA_PREPARE: 
		return stm32f_i2s_dma_prepare(drv, (void *)arg1, arg2);
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

const struct i2s_op stm32_spi_i2s_op = {
	.send = (void *)stm32_i2s_dma_send,
	.recv = (void *)stm32_i2s_dma_recv,
	.close = (void *)stm32_i2s_dma_close,
	.ioctl = (void *)stm32_i2s_dma_ioctl,
	.enable = (void *)stm32_i2s_enable,
	.setbuf = (void *)stm32_i2s_setbuf,
};

