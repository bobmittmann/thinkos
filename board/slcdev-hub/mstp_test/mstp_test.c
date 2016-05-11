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
#include <sys/stm32f.h>
#include <sys/delay.h>
#include <errno.h>

#include <thinkos.h>

#include <bacnet/mstp_lnk.h>
#include <sys/console.h>

#define TRACE_LEVEL 7

#include <trace.h>

#include "lattice.h"

#include "board.h"


/* -------------------------------------------------------------------------
 * Test
 * ------------------------------------------------------------------------- */
void io_init(void)
{
	thinkos_udelay_factor(&udelay_factor);

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

    /* USART5 TX */
    stm32_gpio_mode(UART5_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
    stm32_gpio_af(UART5_TX, GPIO_AF8);
    /* USART5 RX */
    stm32_gpio_mode(UART5_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART5_RX, GPIO_AF8);

    stm32_gpio_mode(IO_RS485_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(IO_RS485_RX, RS485_USART_AF);

    stm32_gpio_mode(IO_RS485_TX, ALT_FUNC, PUSH_PULL | SPEED_MED);
    stm32_gpio_af(IO_RS485_TX, RS485_USART_AF);

    stm32_gpio_mode(IO_RS485_MODE, OUTPUT, PUSH_PULL | SPEED_LOW);
    stm32_gpio_set(IO_RS485_MODE);

    /* USART6_TX */
    stm32_gpio_mode(UART6_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
    stm32_gpio_af(UART6_TX, GPIO_AF7);
    /* USART6_RX */
    stm32_gpio_mode(UART6_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART6_RX, GPIO_AF7);
}

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

FILE * trace_file;
volatile bool trace_auto_flush = true;
volatile bool trace_udp_enabled = false;
const char trace_host[] = "192.168.10.1";
const uint16_t trace_port = 1111;

void __attribute__((noreturn)) supervisor_task(void)
{
	struct trace_entry trace;
	uint32_t clk;
	uint32_t dump_tmo;
	int n;

	INF("<%d> started...", thinkos_thread_self());

	/* set the supervisor stream to stdout */
	trace_file = stdout;
	/* enable auto flush */
	trace_auto_flush = true;

	trace_tail(&trace);
#if 0
    struct sockaddr_in sin;
    struct udp_pcb * udp;
    bool udp_enabled;

    if ((udp = udp_alloc()) == NULL) {
        abort();
    }

	if (!inet_aton(trace_host, &sin.sin_addr)) {
		abort();
	}
	sin.sin_port = htons(trace_port);

    if (udp_bind(udp, INADDR_ANY, htons(10)) < 0) {
        abort();
    }

//    if (udp_connect(udp, sin.sin_addr.s_addr, htons(sin.sin_port)) < 0) {
//		abort();
//    }
#endif

	clk = thinkos_clock();
	dump_tmo = clk + 15000;
	for (;;) {
		char msg[80];

		/* 8Hz periodic task */
		clk += 125;
		thinkos_alarm(clk);
//		udp_enabled = trace_udp_enabled;

		while (trace_getnext(&trace, msg, sizeof(msg)) >= 0) {
			struct timeval tv;
			char s[128];

			trace_ts2timeval(&tv, trace.dt);
			if (trace.ref->lvl <= TRACE_LVL_WARN)
				n = sprintf(s, "%s %2d.%06d: %s,%d: %s\n",
						trace_lvl_nm[trace.ref->lvl],
						(int)tv.tv_sec, (int)tv.tv_usec,
						trace.ref->func, trace.ref->line, msg);
			else
				n = sprintf(s, "%s %2d.%06d: %s\n",
						trace_lvl_nm[trace.ref->lvl],
						(int)tv.tv_sec, (int)tv.tv_usec, msg);

            /* write log to local console */
			fwrite(s, n, 1, trace_file);
#if 0
			if (udp_enabled) {
				/* sent log to remote station */
				if (udp_sendto(udp, s, n, &sin) < 0) {
					udp_enabled = false;
				}
			}
#endif
		}

		if (trace_auto_flush)
			trace_flush(&trace);

		if ((int32_t)(clk - dump_tmo) >= 0) {
			const struct thinkos_thread_inf * infbuf[33];
			uint32_t cycbuf[33];
	        uint32_t cycsum = 0;
	        uint32_t cycbusy;
	        uint32_t cycidle;
	        uint32_t cycdiv;
	        uint32_t idle;
	        uint32_t busy;
			int n;
			int i;

			thinkos_critical_enter();

			thinkos_thread_inf(infbuf);
			n = thinkos_cyccnt(cycbuf);

			thinkos_critical_exit();

	        cycsum = 0;
	        for (i = 0; i < n; ++i)
	        	cycsum += cycbuf[i];
	        cycidle = cycbuf[n - 1]; /* The last item is IDLE */
	        cycbusy = cycsum - cycidle;
	        cycdiv = (cycsum + 5000) / 10000;
	        busy = (cycbusy + (cycdiv / 2)) / cycdiv;
	        idle = 1000 - busy;
		    printf("CPU usage: %d.%02d%% busy, %d.%02d%% idle\n",
		    		busy / 100, busy % 100, idle / 100, idle % 100);
			for (i = 0; i < n; ++i) {
				const struct thinkos_thread_inf * inf;
				if (((inf = infbuf[i]) != NULL) && (cycbuf[i] != 0)) {
					uint32_t usage;
				    usage = (cycbuf[i] + cycdiv / 2) / cycdiv;
				    printf("%2d %7s %3d.%02d%%\n", i, inf->tag,
				    		usage / 100, usage % 100);
				}
			}
			dump_tmo = clk + 15000;


		}
	}
}

void supervisor_init(void)
{
	static uint32_t supervisor_stack[256];

	static const struct thinkos_thread_inf supervisor_inf = {
		.stack_ptr = supervisor_stack,
		.stack_size = sizeof(supervisor_stack),
		.priority = 8,
		.thread_id = 2,
		.paused = false,
		.tag = "SUPV"
	};

	trace_init();

	thinkos_thread_create_inf((void *)supervisor_task, (void *)NULL,
							  &supervisor_inf);
}

/* -------------------------------------------------------------------------
 * MS/TP 
 * ------------------------------------------------------------------------- */

int mstp_lnk_task(void * arg)
{
	struct mstp_lnk * mstp = (struct mstp_lnk *)arg;

	printf("MS/TP link task started...\n");

	mstp_lnk_loop(mstp);

	return 0;
}

struct net_stats {
	struct {
		unsigned int octet_cnt;
		unsigned int bcast_cnt;
		unsigned int unicast_cnt;
	} rx;
	struct {
		unsigned int octet_cnt;
		unsigned int bcast_cnt;
		unsigned int unicast_cnt;
	} tx;
};

struct {
	struct mstp_lnk * mstp;
	struct net_stats stats;
} net;

int net_recv_task(void * arg)
{
	struct mstp_lnk * mstp = (struct mstp_lnk *)arg;
	struct mstp_frame_inf inf;
	uint8_t pdu[502];
	int len;
	unsigned int seconds = 10;
	uint32_t clk;

	INF("MS/TP receive task started...");

	clk = thinkos_clock() + seconds * 1000;
	for (;;) {
		len = mstp_lnk_recv(mstp, pdu, sizeof(pdu), &inf);
		net.stats.rx.octet_cnt += len;
		if (inf.daddr == MSTP_ADDR_BCAST)
			net.stats.rx.bcast_cnt++;
		else
			net.stats.rx.unicast_cnt++;

//		printf("RCV: %d->%d (%d) \"%s\"\n", inf.saddr, inf.daddr,
//				len, (char*)pdu);
//		DBG("RCV: %d->%d (%d)", inf.saddr, inf.daddr, len);
		/* Log summary each 5 seconds */
		if ((int32_t)(clk - thinkos_clock()) <= 0) {
			unsigned int rate;
			rate = net.stats.rx.octet_cnt / seconds;

			INF("Receive: %d bcast, %d unicast, %d octets, %d.%03d KBps.",
					net.stats.rx.bcast_cnt, net.stats.rx.unicast_cnt, net.stats.rx.octet_cnt,
					rate / 1000, rate % 1000);

			net.stats.rx.bcast_cnt = 0;
			net.stats.rx.unicast_cnt = 0;
			net.stats.rx.octet_cnt = 0;
			seconds = 10;
			clk += seconds * 1000;
		}
	}

	return 0;
}

uint32_t mstp_lnk_stack[512] __attribute__((section (".ccm")));

const struct thinkos_thread_inf mstp_lnk_inf = {
	.stack_ptr = mstp_lnk_stack,
	.stack_size = sizeof(mstp_lnk_stack),
	.priority = 1,
	.thread_id = 1,
	.paused = 0,
	.tag = "MS/TP"
};

uint32_t net_recv_stack[512];

const struct thinkos_thread_inf net_recv_inf = {
	.stack_ptr = net_recv_stack,
	.stack_size = sizeof(net_recv_stack),
	.priority = 32,
	.thread_id = 3,
	.paused = 0,
	.tag = "NET RCV"
};

struct mstp_lnk * mstp_start(int addr)
{
	struct serial_dev * ser;
	struct mstp_lnk * mstp;

	INF("1. serial port init");
	ser = stm32f_uart1_serial_dma_init(500000, SERIAL_8N1);

	INF("2. mstp_lnk_alloc()");
	mstp = mstp_lnk_alloc();

	INF("3. MS/TP link addr: %d", addr);
	mstp_lnk_init(mstp, "MS/TP1", addr, ser);

	INF("4. thinkos_thread_create_inf()");
	thinkos_thread_create_inf(mstp_lnk_task, mstp, &mstp_lnk_inf);

	thinkos_thread_create_inf(net_recv_task, mstp, &net_recv_inf);

	thinkos_sleep(100);

	printf("5. mstp_lnk_resume()");
	mstp_lnk_resume(mstp);

	return mstp;
}

/* -------------------------------------------------------------------------
 * Test
 * ------------------------------------------------------------------------- */
int test_mode = 0;

int mstp_test_task(void * arg)
{
	struct mstp_lnk * mstp = (struct mstp_lnk *)arg;
	struct mstp_frame_inf inf;
	uint8_t pdu[502];
	uint32_t clk;
	uint32_t tmo;
	unsigned int itval = 5;
	int len;
	int i;

	INF("MS/TP test task started...\n");

	clk = thinkos_clock();
	tmo = clk + itval * 1000;

	for (i = 0;; ++i) {
		if (test_mode != 0) {
			inf.daddr = MSTP_ADDR_BCAST;
			inf.type = FRM_DATA_NO_REPLY;
			len = snprintf((char *)pdu, 501, "%6d"
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog."
					" The quick brown fox jumps over the lazy dog.",
					i);
			if (mstp_lnk_send(mstp, pdu, len, &inf) < 0) {
				ERR("mstp_lnk_send() failed!");
			}
			net.stats.tx.octet_cnt += len;
			net.stats.tx.bcast_cnt++;

			/* Log summary each 10 seconds */
			if ((int32_t)(tmo - clk) <= 0) {
				unsigned int rate;

				rate = net.stats.tx.octet_cnt / itval;

				DBG("Send: %d bcasts, %d octets, %d.%03d KBps.",
						net.stats.tx.bcast_cnt, net.stats.tx.octet_cnt,
						rate / 1000, rate % 1000);

				net.stats.tx.octet_cnt = 0;
				net.stats.tx.bcast_cnt = 0;
				itval = 10;
				tmo += itval * 1000;
			}

			if (test_mode > 1) {
				thinkos_alarm(clk + test_mode * 100);
			}
		} else {
			thinkos_alarm(clk + 100);
		}
		clk = thinkos_clock();
	}

	return 0;
}

uint32_t test_stack[512];

const struct thinkos_thread_inf test_inf = {
	.stack_ptr = test_stack,
	.stack_size = sizeof(test_stack),
	.priority = 32,
	.thread_id = 32,
	.paused = 0,
	.tag = "TEST"
};

void  mstp_test_start(struct mstp_lnk * mstp)
{
	thinkos_thread_create_inf(mstp_test_task, mstp, &test_inf);
}

struct board_cfg {
    uint32_t magic;
    uint32_t mstp_addr;
    uint32_t ip_addr;
};

#define CFG_MAGIC 0x01020304
#define CFG_ADDR  0x0800ff00

void show_netmap(struct mstp_lnk * mstp)
{
	uint8_t map[16];
	unsigned int cnt;
	int i;

	cnt = mstp_lnk_getnetmap(mstp, map, sizeof(map));
	printf("\n---- Network Map ----\n");

	for (i = 0; i < cnt; ++i) {
		printf("%2d - Node %2d\n", i, map[i]);
	}

	printf("\n");
}



void show_menu(void)
{
	printf("\n===== MS/TP test =====\n");
	printf("[s] stop sending\n");
	printf("[1..9] set sending mode\n");
	printf("[n] show network map\n");
	printf("\n");
}

void motd(void)
{
	printf("\n--------- Test Application -------\n");
	printf("\n");
}


extern const uint8_t ice40lp384_bin[];
extern const unsigned int sizeof_ice40lp384_bin;


int main(int argc, char ** argv)
{
	struct board_cfg * cfg = (struct board_cfg *)(CFG_ADDR);
	struct mstp_lnk * mstp;
	int mstp_addr;

	if (cfg->magic == CFG_MAGIC)
		mstp_addr = cfg->mstp_addr;
	else
		mstp_addr = 2;

	io_init();

	stdio_init();

	motd();

    lattice_ice40_configure(ice40lp384_bin, sizeof_ice40lp384_bin);

	supervisor_init();

	INF("Starting MS/TP network (addr=%d)", mstp_addr);

	if ((mstp = mstp_start(mstp_addr)) == NULL) {
		thinkos_sleep(1000);
		return 1;
	}

	INF("Starting MS/TP test");

	mstp_test_start(mstp);

	for (;;) {
		int c = fgetc(stdin);

		switch (c) {
		case '0' ... '9':
			test_mode = c - '0';
			printf("\ntest mode %d\n", test_mode);
			break;
		default:
			show_menu();
		}
	}

	return 0;
}


