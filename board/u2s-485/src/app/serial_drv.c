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
 * @file console.c
 * @brief YARD-ICE UART console
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <thinkos.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/serial.h>

#include "board.h"

#define UART_TX_FIFO_BUF_LEN 64
#define UART_RX_FIFO_BUF_LEN 64
#define UART_RX_FIFO_WATER_MARK 32

#define SERDRV_RX_FLAG_NO   0
#define SERDRV_CTL_FLAG_NO  1
#define VCOM_MODE_FLAG_NO   6

/* Gates */
#define SERDRV_TX_GATE_NO   0

struct serial_drv {
	struct stm32_usart * uart;
#ifndef SERDRV_TX_GATE_NO
	int32_t tx_gate;
#endif
#ifndef SERDRV_RX_FLAG_NO
	int32_t rx_flag;
#endif	
#ifndef SERDRV_CTL_FLAG_NO
	int32_t ctl_flag;
#endif	
	uint32_t * txie;
	uint32_t * tcie;
	uint32_t err_cnt;
	struct {
		volatile uint32_t head;
		volatile uint32_t tail;
		uint8_t buf[UART_TX_FIFO_BUF_LEN];
	} tx_fifo;	
	struct {
		volatile uint32_t head;
		volatile uint32_t tail;
		uint8_t buf[UART_RX_FIFO_BUF_LEN];
	} rx_fifo;	
};

#ifdef SERDRV_RX_FLAG_NO
#define RX_FLAG THINKOS_FLAG_DESC(SERDRV_RX_FLAG_NO)
#else
#define RX_FLAG drv->rx_flag
#endif

#ifdef SERDRV_TX_GATE_NO
#define TX_GATE THINKOS_GATE_DESC(SERDRV_TX_GATE_NO)
#else
#define TX_GATE drv->tx_gate
#endif

#ifdef SERDRV_CTL_FLAG_NO
#define CTL_FLAG THINKOS_FLAG_DESC(SERDRV_CTL_FLAG_NO)
#else
#define CTL_FLAG drv->ctl_flag
#endif

int __serial_read(struct serial_drv * drv, void * buf, 
				unsigned int len, unsigned int tmo)
{
	uint8_t * cp = (uint8_t *)buf;
	unsigned int tail;
	int ret;
	int cnt;
	int n;
	int i;

again:
	if ((ret = thinkos_flag_timedtake(RX_FLAG, tmo)) < 0) {
		return ret;
	}

	tail = drv->rx_fifo.tail;
	cnt = (int32_t)(drv->rx_fifo.head - tail);
	if (cnt == 0) {
		goto again;
	}
	n = MIN(len, cnt);

	for (i = 0; i < n; ++i)
		cp[i] = drv->rx_fifo.buf[tail++ & (UART_RX_FIFO_BUF_LEN - 1)];

	drv->rx_fifo.tail = tail;

	if (cnt > n) {
		thinkos_flag_give(RX_FLAG);
	}

	return n;
}

int __serial_write(struct serial_drv * drv, const void * buf, 
				 unsigned int len)
{
	uint8_t * cp = (uint8_t *)buf;
	int rem = len;

	while (rem) {
		unsigned int head;
		int free;
		int n;
		int i;

		thinkos_gate_wait(TX_GATE);

		head = drv->tx_fifo.head;
		free = UART_TX_FIFO_BUF_LEN - (int32_t)(head - drv->tx_fifo.tail);
		n = MIN(rem, free);
		for (i = 0; i < n; ++i) 
			drv->tx_fifo.buf[head++ & (UART_TX_FIFO_BUF_LEN - 1)] = *cp++;
		drv->tx_fifo.head = head;

		*drv->txie = 1; 

		thinkos_gate_exit(TX_GATE, (free > n));

		rem -= n;
	}

	return len;
}

int __serial_drain(struct serial_drv * drv)
{
	do {
		thinkos_gate_wait(TX_GATE);
		thinkos_gate_exit(TX_GATE, 0);
	} while (((int32_t)drv->tx_fifo.head - drv->tx_fifo.tail) > 0);

	stm32_usart_flush(drv->uart);

	return 0;
}

int __serial_ioctl(struct serial_drv * drv, int opt, 
				   uintptr_t arg1, uintptr_t arg2)
{
	struct stm32_usart * us = drv->uart;
	unsigned int msk = 0;
	unsigned int flowctrl;

	switch (opt) {
	case SERIAL_IOCTL_ENABLE:
		msk |= (arg1 & SERIAL_RX_EN) ? USART_RE : 0;
		msk |= (arg1 & SERIAL_TX_EN) ? USART_TE : 0;
		us->cr1 |= msk;
		break;

	case SERIAL_IOCTL_DISABLE:
		msk |= (arg1 & SERIAL_RX_EN) ? USART_RE : 0;
		msk |= (arg1 & SERIAL_TX_EN) ? USART_TE : 0;
		us->cr1 &= ~msk;
		break;

	case SERIAL_IOCTL_DRAIN:
		__serial_drain(drv);
		break;

	case SERIAL_IOCTL_RESET:
		__serial_drain(drv);
		drv->tx_fifo.head = 0;
		drv->tx_fifo.tail = 0;
		break;

	case SERIAL_IOCTL_STATS_GET: 
		{
			struct serial_stats * stats = (struct serial_stats *)arg1;
			stats->rx_cnt = drv->rx_fifo.head;
			stats->tx_cnt = drv->tx_fifo.tail;
			stats->err_cnt = drv->err_cnt;
			break;
		}

	case SERIAL_IOCTL_CONF_SET: 
		{
			struct serial_config * cfg = (struct serial_config *)arg1;
			uint32_t flags;

			stm32_usart_baudrate_set(us, cfg->baudrate);
			flags = CFG_TO_FLAGS(cfg);
			stm32_usart_mode_set(us, flags);
			flowctrl = SERIAL_FLOWCTRL(flags);
			goto flowctrl_set;
		}
		break;

	case SERIAL_IOCTL_FLOWCTRL_SET: 
		flowctrl = arg1;

flowctrl_set:
		switch (flowctrl) { 
		case SERIAL_FLOWCTRL_NONE:
		case SERIAL_FLOWCTRL_RTSCTS:
		case SERIAL_FLOWCTRL_XONXOFF:
			break;
		}
		break;
	}

	return 0;
}

int __serial_close(struct serial_drv * drv)
{
	struct stm32_usart * us = drv->uart;
	us->cr1 = 0;

	return 0;
}

void stm32f_usart2_task(struct serial_drv * drv)
{
	struct stm32_usart * us = drv->uart;
	uint32_t sr;
	int c;

	for(;;) {
		thinkos_irq_wait(STM32_IRQ_USART2);
		sr = us->sr;

		if (sr & USART_RXNE) {
			uint32_t head;
			int free;
			c = us->dr;

			head = drv->rx_fifo.head;
			free = UART_RX_FIFO_BUF_LEN - (uint8_t)(head - drv->rx_fifo.tail);
			if (free > 0) { 
				drv->rx_fifo.buf[head & (UART_RX_FIFO_BUF_LEN - 1)] = c;
				drv->rx_fifo.head = head + 1;
			}
			if (free < (UART_RX_FIFO_BUF_LEN  - UART_RX_FIFO_WATER_MARK)) 
				thinkos_flag_give(RX_FLAG);

			return;
		}	

		if (sr & USART_IDLE) {
			c = us->dr;
			(void)c;
			thinkos_flag_give(RX_FLAG);
		}

		if (sr & USART_ORE) {
			drv->err_cnt++;
		}

		sr &= us->cr1;

		if (sr & USART_TXE) {
			uint32_t tail = drv->tx_fifo.tail;
			if ((tail == drv->tx_fifo.head)) {
				/* FIFO empty, disable TXE interrupts */
				*drv->txie = 0; 
				/* enable TC interrupts */
				*drv->tcie = 1;
				thinkos_gate_open(TX_GATE);
			} else {
				/* RS485 enable transmitter */ 
				rs485_rxdis();
				rs485_txen();
				c = drv->tx_fifo.buf[tail & (UART_TX_FIFO_BUF_LEN - 1)];
				us->dr = c;
				drv->tx_fifo.tail = tail + 1;
			}
		}

		if (sr & USART_TC) {
			/* RS485 disable ransmitter */ 
			rs485_txdis();
			rs485_rxen();
			/* disable TC interrupts */
			*drv->tcie = 0;
		}
	}
}

struct serial_drv uart2_serial_drv;

uint32_t serial2_stack[256] __attribute__ ((aligned(8)));

const struct thinkos_thread_inf serial2_thread_inf = {
	.stack_ptr = serial2_stack,
	.stack_size = sizeof(serial2_stack),
	.priority = 8,
	.thread_id = 8,
	.paused = false,
	.tag = ""
};

struct serial_drv * serial2_init(void)
{
	struct serial_drv * drv = &uart2_serial_drv;
	struct stm32_usart * uart = STM32_USART2;

	drv->tx_fifo.head = drv->tx_fifo.tail = 0;
	drv->rx_fifo.head = drv->rx_fifo.tail = 0;
	drv->err_cnt = 0;

	drv->txie = CM3_BITBAND_DEV(&uart->cr1, 7);
	drv->tcie = CM3_BITBAND_DEV(&uart->cr1, 6);
	drv->uart = uart;

	thinkos_gate_open(TX_GATE);

	/* clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_USART2);

	stm32_usart_init(uart);
	stm32_usart_baudrate_set(uart, 19200);
	stm32_usart_mode_set(uart, SERIAL_8N1);

	/* enable UART, RX and IDLE interrupts */
	uart->cr1 |= USART_UE | USART_RXNEIE | USART_IDLEIE;
	/* Errors interrupt */
	uart->cr3 |= USART_EIE;

	thinkos_thread_create_inf(C_TASK(stm32f_usart2_task), 
								  C_ARG(drv),
								  &serial2_thread_inf);

	uart->cr1 |= USART_RE | USART_TE;
	return drv;
}

