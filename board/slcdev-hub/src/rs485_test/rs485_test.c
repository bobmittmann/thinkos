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

void net_start(unsigned int addr);

int net_send(unsigned int addr, const void * buf, unsigned int len);

void net_probe(bool en);

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
	int i;

	if (cfg->magic == CFG_MAGIC)
		addr = cfg->mstp_addr;
	else
		addr = 0;

	stdio_init();

	supervisor_init();

	INF("Starting RS485 test");

	net_start(addr);
	net_probe(true);

	for (i = 0;; ++i) {
		(void)i;
		thinkos_sleep(100);
		net_send(0, "U U U U ", 8);
	}

	return 0;
}


