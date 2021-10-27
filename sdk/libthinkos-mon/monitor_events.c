/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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
 * @file events.h
 * @brief ThinkOS monitor default event handlers
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/delay.h>

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>

#include <thinkos.h>

#include <sys/dcclog.h>

uint32_t monitor_on_rx_pipe(const struct monitor_comm * comm, 
							uint32_t sigmask)
{
	uint8_t * ptr;
	int cnt;

	/* get a pointer to the head of the pipe.
	   thinkos_console_rx_pipe_ptr() will return the number of 
	   consecutive spaces in the buffer. */
	if ((cnt = thinkos_console_rx_pipe_ptr(&ptr)) > 0) {
		int n;

		/* receive from the COMM driver */
		if ((n = monitor_comm_recv(comm, ptr, cnt)) > 0) {
			/* commit the fifo head */
			thinkos_console_rx_pipe_commit(n);
			if (n == cnt) {
				/* the pipe is full, clear the flag */
				monitor_clear(MONITOR_RX_PIPE);
				/* Wait for RX_PIPE & stop COMM_RECV notifications */
				sigmask |= (1 << MONITOR_RX_PIPE);
				sigmask &= ~(1 << MONITOR_COMM_RCV);
				/* Wait for RX_PIPE */
				DCC_LOG1(LOG_TRACE, "RX_PIPE: fifo full recv=%d", n);
			} else {
				/* Wait for COMM_RECV */
				sigmask |= (1 << MONITOR_RX_PIPE);
				sigmask |= (1 << MONITOR_COMM_RCV);
				DCC_LOG2(LOG_TRACE, "RX_PIPE: free=%d recv=%d ", cnt, n);
			}
		} else {
			monitor_clear(MONITOR_COMM_RCV);
			/* Wait for COMM_RECV */
			sigmask &=  ~(1 << MONITOR_RX_PIPE);
			sigmask |= (1 << MONITOR_COMM_RCV);
			DCC_LOG1(LOG_WARNING, "RX_PIPE: free=%d recv none", cnt);
		}
	} else {
		monitor_clear(MONITOR_RX_PIPE);
		/* Stop receiving */
		sigmask |= (1 << MONITOR_RX_PIPE);
		sigmask &= ~(1 << MONITOR_COMM_RCV);
		DCC_LOG(LOG_WARNING, "RX_PIPE: fifo full");
	}

	return sigmask;
}

uint32_t monitor_on_tx_pipe(const struct monitor_comm * comm, 
							uint32_t sigmask)
{
	uint8_t * ptr;
	int cnt;

	if ((cnt = thinkos_console_tx_pipe_ptr(&ptr)) > 0) {
		int n;
		if ((n = monitor_comm_send(comm, ptr, cnt)) > 0) {
			thinkos_console_tx_pipe_commit(n);
			sigmask |= (1 << MONITOR_TX_PIPE);
			sigmask |= (1 << MONITOR_COMM_EOT);
		} else {
			monitor_clear(MONITOR_COMM_EOT);
			/* Wait for COMM_EOT, stop TX_PIPE notification */
			sigmask &= ~(1 << MONITOR_TX_PIPE);
			sigmask |= (1 << MONITOR_COMM_EOT);
			DCC_LOG1(LOG_WARNING, "TX_PIPE: data=%d sent none", cnt);
		}
	} else {
		monitor_clear(MONITOR_TX_PIPE);
		/* Wait for TX_PIPE, stop COMM_EOT notification */
		sigmask |= (1 << MONITOR_TX_PIPE);
		sigmask &= ~(1 << MONITOR_COMM_EOT);
		DCC_LOG(LOG_WARNING, "TX_PIPE: fifo empty");
	}

	return sigmask;
}

uint32_t monitor_on_comm_ctl(const struct monitor_comm * comm, 
							 uint32_t sigmask)
{
	bool connected;
	int status;

	DCC_LOG1(LOG_MSG, "comm=%08x", comm);

	monitor_clear(MONITOR_COMM_CTL);

	status = monitor_comm_status_get(comm);
	if (status & COMM_ST_CONNECTED) {
		DCC_LOG(LOG_INFO, "connected....");
	}
#if 0
	if (status & COMM_ST_BREAK_REQ ) {
		monitor_comm_break_ack(comm);
		DCC_LOG(LOG_INFO, "break_req....");
	}
#endif
	connected = (status & COMM_ST_CONNECTED) ? true : false;
	thinkos_krn_console_connect_set(connected);

	sigmask &= ~((1 << MONITOR_COMM_EOT) | 
				 (1 << MONITOR_COMM_RCV) |
				 (1 << MONITOR_RX_PIPE));
	sigmask |= (1 << MONITOR_TX_PIPE);

	if (connected) {
		sigmask |= ((1 << MONITOR_COMM_EOT) |
					(1 << MONITOR_COMM_RCV));
	}

	return sigmask;
}


/*
   Default Monitor comm event handler
 */
uint32_t monitor_on_comm_rcv(const struct monitor_comm * comm, 
							 uint32_t sigmask)
{
	return sigmask;
}
