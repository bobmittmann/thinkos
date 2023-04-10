/* 
 * Copyright(C) 2015 Robinson Mittmann. All Rights Reserved.
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
 * @file uart-dma.h
 * @brief MST/TP link layer private header
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#ifndef __UART_DMA_H__
#define __UART_DMA_H__

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

enum uart_dma_event {
	UART_EVENT_RX_IDLE    = 1,
	UART_EVENT_RX_BREAK   = 2,
	UART_EVENT_RX_ERROR   = 3,
	UART_EVENT_TIMEOUT    = 4

};

struct uart_dma_op {
	int (* init)(unsigned int);
	int (* rx_en_set)(bool);
	int (* rx_dma_rem)(void);
	int (* event_wait)(void);
	int (* event_timedwait)(uint32_t tmo);
	int (* tx_dma_init)(const void *, size_t);
	int (* tx_dma_start)(const void *, size_t);
	int (* rx_dma_init)(void *, size_t);
};

struct uart_dma_drv {
	const struct uart_dma_op * op;
	const char *tag;
	void * uart_io;
	void * dma_io;
	uint8_t uart_irq;
	uint8_t dma_rx_irq;
	uint8_t dma_tx_irq;
	uint8_t dma_rx_chan;
	uint8_t dma_tx_chan;
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif
#endif /* __UART_DMA_H__ */

