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
 * @file rtp_test.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stm32f.h>
#include <errno.h>

#include <tcpip/udp.h>
#include <tcpip/tcp.h>
#include <tcpip/ethif.h>
#include <tcpip/route.h>
#include <tcpip/loopif.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <thinkos.h>
#include <sys/console.h>
#include <sys/delay.h>

#define TRACE_LEVEL TRACE_LVL_INF
#include <trace.h>

#include "rtsp.h"
#include "audio.h"
#include "board.h"

void tcpip_init(void);
int webserver_start(void);
void supervisor_init(void);

int stm32f_get_esn(void * arg)
{
	uint64_t * esn = (uint64_t *)arg;
	*esn = *((uint64_t *)STM32F_UID);
	return 0;
}
int network_config(void)
{
	struct ifnet * ifn;
	in_addr_t ip_addr;
	in_addr_t netmask = INADDR_ANY;
	in_addr_t gw_addr = INADDR_ANY;
	char s[64];
	char s1[16];
	char s2[16];
	char * env;
	uint8_t ethaddr[6] = { 0x1c, 0x95, 0x5d, 0x00, 0x00, 0x80};
	uint64_t esn;
	int dhcp;

	/* The UID register is located in a system area not mapped
	   in the MPU user region. We need to escalate the privilege to 
	   have access to this area. */
	thinkos_escalate(stm32f_get_esn, &esn);

	ethaddr[0] = ((esn >>  0) & 0xfc) | 0x02; /* Locally administered MAC */
	ethaddr[1] = ((esn >>  8) & 0xff);
	ethaddr[2] = ((esn >> 16) & 0xff);
	ethaddr[3] = ((esn >> 24) & 0xff);
	ethaddr[4] = ((esn >> 32) & 0xff);
	ethaddr[5] = ((esn >> 40) & 0xff);

	printf("* mac addr: %02x-%02x-%02x-%02x-%02x-%02x\n ", 
		   ethaddr[0], ethaddr[1], ethaddr[2],
		   ethaddr[3], ethaddr[4], ethaddr[5]);

	tcpip_init();

//	if ((env = getenv("IPCFG")) == NULL) {
	if (1) {
		printf("IPCFG not set, using defaults!\n");
		/* default configuration */
		strcpy(s, "192.168.10.100 255.255.255.0 192.168.10.254 0");
		/* set the default configuration */
		setenv("IPCFG", s, 1);
	} else {
		strcpy(s, env);
	}

	if (!inet_aton(strtok(s, " ,"), (struct in_addr *)&ip_addr)) {
		return -1;
	}

	if (inet_aton(strtok(NULL, " ,"), (struct in_addr *)&netmask)) {
		if (inet_aton(strtok(NULL, " ,"), (struct in_addr *)&gw_addr)) {
			dhcp = strtoul(strtok(NULL, ""), NULL, 0);
		}
	}

	/* initialize the Ethernet interface */
	/* configure the ip address */
	ifn = ethif_init(ethaddr, ip_addr, netmask);
//	ifn = loopif_init(ip_addr, netmask);

	ifn_getname(ifn, s);
	ifn_ipv4_get(ifn, &ip_addr, &netmask);
	printf("* netif %s: %s, %s\n", s, 
		   inet_ntop(AF_INET, (void *)&ip_addr, s1, 16),
		   inet_ntop(AF_INET, (void *)&netmask, s2, 16));

	if (gw_addr != INADDR_ANY) {
		/* add the default route (gateway) to ethif */
		ipv4_route_add(INADDR_ANY, INADDR_ANY, gw_addr, ifn);
		printf("* default route gw: %s\n", 
			   inet_ntop(AF_INET, (void *)&gw_addr, s1, 16));
	}

	if (dhcp) {
#if 0
		/* configure the initial ip address */
		dhcp_start();
		/* schedule the interface to be configured through dhcp */
		dhcp_ifconfig(ethif, dhcp_callback);
		printf("DHCP started.\n");
#endif
	}

	return 0;
}

/* -------------------------------------------------------------------------
 * Stdio
 * ------------------------------------------------------------------------- */

void shell_task(void * arg);

uint32_t stdio_shell_stack[512];

const struct thinkos_thread_inf stdio_shell_inf = {
	.stack_ptr = stdio_shell_stack,
	.stack_size = sizeof(stdio_shell_stack),
	.priority = 32,
	.thread_id = 31,
	.paused = false,
	.tag = "SHELL"
};

void stdio_shell_start(void)
{
	FILE * f = stdout;
	thinkos_thread_create_inf((void *)shell_task, (void *)f, &stdio_shell_inf);
}

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

void boost_on(void)
{
	int i;
	
	stm32_gpio_set(IO_BOOST_ON);

	for (i = 0; i < 10; ++i) {
		udelay(10);
		stm32_gpio_set(IO_BOOST_TRIG);
		udelay(10);
		stm32_gpio_clr(IO_BOOST_TRIG);
	}

	stm32_gpio_clr(IO_BOOST_ON);
}

void boost_off(void)
{
	stm32_gpio_clr(IO_BOOST_ON);
	stm32_gpio_set(IO_BOOST_TRIG);
	thinkos_sleep(1);
	stm32_gpio_clr(IO_BOOST_TRIG);
}

#define IO_POLL_PERIOD_MS 100

void __attribute__((noreturn)) io_task(void)
{
	uint32_t clk = thinkos_clock();
	uint32_t count = 0;
	int sw0 = 0;
	int sw1 = 0;
	int ocf = -1; /* overcurrent fault */

	INF("IO task started (thread %d).", thinkos_thread_self());
	sw0 = stm32_gpio_stat(IO_SWITCH) ? 0 : 1;
	sw1 = sw0;

	for (;;) {
		int tmp;
		int sw;

		clk += IO_POLL_PERIOD_MS / 2;
		thinkos_alarm(clk);
		count++;

		if ((count & 4) == 4)
			__led_on(IO_LED2);
		else
			__led_off(IO_LED2);

		sw = stm32_gpio_stat(IO_SWITCH) ? 0 : 1;
		if (sw != sw0) {
			/* Debouncing */
			sw0 = sw;
		} else if ((sw ^ sw1) != 0) {
			/* State change */
			sw1 = sw;
			if (sw) {
				INF("Booster ON!");
				boost_on();
			} else {
				INF("Booster OFF!");
				boost_off();
			}
		}

		tmp = stm32_gpio_stat(IO_BOOST_OCF) ? 1 : 0;
		if (tmp != ocf) {
			ocf = tmp;
			if (ocf) {
				INF("Overcurrent Fault!");
				__led_on(IO_LED1);
			} else {
				INF("Overcurrent Fault Cleared!");
				__led_off(IO_LED1);
			}
		}

	}
}


uint32_t io_stack[128];

const struct thinkos_thread_inf io_inf = {
	.stack_ptr = io_stack,
	.stack_size = sizeof(io_stack),
	.priority = 1,
	.thread_id = 1,
	.paused = false,
	.tag = "IO"
};

void io_init(void)
{
	thinkos_thread_create_inf((void *)io_task, (void *)NULL,
							  &io_inf);
}


struct rtsp_client rtsp;

unsigned int second;
unsigned int minute;
unsigned int hour;

int main(int argc, char ** argv)
{

	uint32_t clk;

	thinkos_udelay_factor(&udelay_factor);

	stdio_init();

	printf("1. NVRAM environment init... \n");
	stm32f_nvram_env_init();

	printf("2. Starting supervisor...\n");
	supervisor_init();

	printf("3. Starting IO... \n");
	io_init();

	printf("3. Configuring network... \n");
	network_config();

	printf("4. Initializing audio subsystem ... \n");
	audio_init();
	line_supv_init();

	printf("5. RTSP client init ... \n");
	rtsp_init(&rtsp, 554);

	printf("6. Starting G.711 RTP client ... \n");
	rtp_g711_start(&rtsp.rtp);

	printf("7. Starting webserver ... \n");
	webserver_start();

	printf("7. Starting shell... \n");
	stdio_shell_start();

	INF("Starting RTSP test");

	clk = thinkos_clock();
	second = 0;
	minute = 0;
	hour = 0;
	for (;;) {
		clk += 1000;
		thinkos_alarm(clk);
		if (++second == 60) {
			second = 0;
			if (++minute == 60) {
				minute = 0;
				hour++;
			}
		}
//		INF("One second tick mark. :) ...");
//		printf("%4d:%02d:%02d tick...\n", hour, minute, second);
	}

	return 0;
}


