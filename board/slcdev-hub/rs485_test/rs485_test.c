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

/* -------------------------------------------------------------------------
 * stdio
 * ------------------------------------------------------------------------- */

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

/* -------------------------------------------------------------------------
 * System Supervision
 * ------------------------------------------------------------------------- */

const char * const trace_lvl_tab[] = {
		"   NONE",
		"  ERROR",
		"WARNING",
		"   INFO",
		"  DEBUG"
};

void __attribute__((noreturn)) supervisor_task(void)
{
	struct trace_entry trace;
	uint32_t clk;

	INF("<%d> started...", thinkos_thread_self());

	trace_tail(&trace);

	clk = thinkos_clock();
	for (;;) {
		struct timeval tv;
		char s[80];

		/* 8Hz periodic task */
		clk += 125;
		thinkos_alarm(clk);

		while (trace_getnext(&trace, s, sizeof(s)) >= 0) {
			trace_ts2timeval(&tv, trace.dt);
			printf("%s %2d.%06d: %s\n", trace_lvl_tab[trace.ref->lvl],
					(int)tv.tv_sec, (int)tv.tv_usec, s);
		}

		trace_flush(&trace);
	}
}


uint32_t supervisor_stack[128];

const struct thinkos_thread_inf supervisor_inf = {
	.stack_ptr = supervisor_stack,
	.stack_size = sizeof(supervisor_stack),
	.priority = 32,
	.thread_id = 31,
	.paused = false,
	.tag = "SUPV"
};

void supervisor_init(void)
{
	trace_init();

	thinkos_thread_create_inf((void *)supervisor_task, (void *)NULL,
							  &supervisor_inf);
}

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

	printf("4. thinkos_thread_create_inf()\n");
	thinkos_thread_create_inf(netrcv_task, ser, &netrcv_inf);
}

struct board_cfg {
    uint32_t magic;
    uint32_t mstp_addr;
    uint32_t ip_addr;
};

#define CFG_MAGIC 0x01020304
#define CFG_ADDR  0x0800ff00

int main(int argc, char ** argv)
{
	struct board_cfg * cfg = (struct board_cfg *)(CFG_ADDR);
	uint8_t addr = 1;
	uint8_t pdu[502];
	int i;

	if (cfg->magic == CFG_MAGIC)
		addr = cfg->mstp_addr;
	else
		addr = *((uint32_t *)STM32F_UID) & 0x1f;

	stdio_init();

	supervisor_init();

	INF("Starting RS485 test");

	net_start(addr);

	for (i = 0;; ++i) {
		thinkos_sleep(100);
	}

	return 0;
}


