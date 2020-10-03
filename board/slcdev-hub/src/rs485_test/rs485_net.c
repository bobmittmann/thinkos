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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file mstp_test.c
 * @brief MS/TP test application
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <errno.h>

#include <sys/stm32f.h>
#include <sys/serial.h>
#include <sys/console.h>
#include <thinkos.h>
#include <trace.h>

#include "board.h"

struct {
	uint8_t addr;
	struct serial_dev * ser;
} net;


/* -------------------------------------------------------------------------
 * RS485
 * ------------------------------------------------------------------------- */

struct serial_dev * rs485_init(void)
{
	struct serial_dev * ser;

    /* IO init */
    stm32_gpio_mode(IO_RS485_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(IO_RS485_RX, RS485_USART_AF);

    stm32_gpio_mode(IO_RS485_TX, ALT_FUNC, PUSH_PULL | SPEED_MED);
    stm32_gpio_af(IO_RS485_TX, RS485_USART_AF);

    stm32_gpio_mode(IO_RS485_MODE, OUTPUT, PUSH_PULL | SPEED_LOW);
    stm32_gpio_set(IO_RS485_MODE);

//	ser = stm32f_uart1_serial_init(500000, SERIAL_8N1);
	ser = stm32f_uart1_serial_dma_init(500000, SERIAL_8N1);
//	ser = stm32f_uart7_serial_init(500000, SERIAL_8N1);

	return ser;
}

int netrcv_task(void * arg)
{
	struct serial_dev * ser = (struct serial_dev *)arg;
	uint8_t buf[512];

	printf("receive task started...\n");

	for (;;) {
		int cnt;

		if ((cnt = serial_recv(ser, buf, 512, 1000)) <= 0) {
			continue;
		}

		printf("cnt=%d\n", cnt);
	}

	return 0;
}

uint32_t netrcv_stack[512];

const struct thinkos_thread_inf netrcv_inf = {
	.stack_ptr = netrcv_stack,
	.stack_size = sizeof(netrcv_stack),
	.priority = 8,
	.thread_id = 1,
	.paused = 0,
	.tag = "NET-RCV"
};

void net_start(int addr)
{
	struct serial_dev * ser;

	printf("1. rs485_init() ...\n");
	if ((ser = rs485_init()) == NULL) {
		return;
	}

	net.ser = ser;
	net.addr = addr;

	printf("4. thinkos_thread_create_inf()\n");
	thinkos_thread_create_inf(netrcv_task, ser, &netrcv_inf);
}

int net_send(unsigned int addr, const void * buf, unsigned int len)
{
	return serial_send(net.ser, buf, len);
}

void net_probe(bool en)
{
	if (en)
	    stm32_gpio_clr(IO_RS485_MODE);
	else
	    stm32_gpio_set(IO_RS485_MODE);
}


