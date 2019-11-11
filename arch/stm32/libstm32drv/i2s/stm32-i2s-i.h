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
 * @file stm32-i2s-i.h
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


/*****************************************************************************
 * libshell internal (private) header file
 *****************************************************************************/

#ifndef __STM32_I2S_I_H__
#define __STM32_I2S_I_H__

#ifndef __STM32_I2S_I__
#error "Never use <stm32-i2s-i.h> directly; include <sys/i2s.h> instead."
#endif

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/delay.h>
#include <sys/i2s.h>

#include <thinkos.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>

#include <sys/dcclog.h>

#ifndef I2S_IRQ_PRIORITY 
#define I2S_IRQ_PRIORITY IRQ_PRIORITY_REGULAR
#endif

#ifndef STM32F_I2S_SPI_ISR
#define STM32F_I2S_SPI_ISR 0
#endif

#ifndef STM32F_I2S_DMA_RX_ISR
#define STM32F_I2S_DMA_RX_ISR 0
#endif

#ifndef STM32F_I2S_DMA_TX_ISR
#define STM32F_I2S_DMA_TX_ISR 0
#endif

struct stm32_spi_i2s_drv {
	struct stm32f_spi * spi;
	struct {
		struct stm32_dmactl dmactl;
		volatile uint32_t head;
		uint32_t tail;
		uint32_t pos;
	} tx;
	struct {
		struct stm32_dmactl dmactl;
		volatile uint32_t head;
		uint32_t tail;
		uint32_t pos;
	} rx;
};

extern const struct i2s_op stm32_spi_i2s_op;

#ifdef __cplusplus
extern "C" {
#endif

void stm32_spi_i2s_isr(struct stm32_spi_i2s_drv * drv);

int stm32_spi_i2s_init(struct stm32_spi_i2s_drv * drv, 
					   unsigned int samplerate, unsigned int flags,
					   int dma_chan_id);

void stm32_i2s_dma_isr(struct stm32_spi_i2s_drv * drv);

void stm32_i2s_dma_rx_isr(struct stm32_spi_i2s_drv * drv);

void stm32_i2s_dma_tx_isr(struct stm32_spi_i2s_drv * drv);

int stm32_i2s_dma_init(struct stm32_spi_i2s_drv * drv, 
					   unsigned int samplerate, unsigned int flags,
					   int dma_chan_id);

uint32_t stm32_i2s_get_dma_tx_count(struct stm32_spi_i2s_drv * drv);

uint32_t stm32_i2s_get_dma_rx_count(struct stm32_spi_i2s_drv * drv);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_I2S_I_H__ */

