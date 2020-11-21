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
 * @file monitor.h
 * @brief ThinkOS monitor example
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
/*
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
*/
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>

#include <thinkos.h>
#include <vt100.h>

#include <sys/dcclog.h>



/*
   Default Monitor Task
 */
uint32_t monitor_comm_event_handler(const struct monitor_comm * comm,
									  int32_t sigmask)
{
	uint8_t * ptr;
	int cnt;
	int sig;

	switch (sig) {

	case MONITOR_COMM_RCV:
		DCC_LOG(LOG_MSG, "COMM_RCV: +++++++++++++++++");
		/* get a pointer to the head of the pipe.
		   thinkos_console_rx_pipe_ptr() will return the number of 
		   consecutive spaces in the buffer. */
		if ((cnt = thinkos_console_rx_pipe_ptr(&ptr)) > 0) {
			int n;

			DCC_LOG1(LOG_MSG, "Raw mode RX, cnt=%d", cnt);

			/* receive from the COMM driver */
			if ((n = monitor_comm_recv(comm, ptr, cnt)) > 0) {
				/* commit the fifo head */
				thinkos_console_rx_pipe_commit(n);
				if (n == cnt) {
					/* Wait for RX_PIPE */
					DCC_LOG(LOG_TRACE, "Wait for RX_PIPE && COMM_RECV");
					sigmask |= (1 << MONITOR_COMM_RCV);
					sigmask &=  ~(1 << MONITOR_RX_PIPE);
					//sigmask &=  ~(1 << MONITOR_RX_PIPE);
				} else {
					/* Wait for COMM_RECV */
					DCC_LOG(LOG_TRACE, "Wait for COMM_RECV");
					sigmask |= (1 << MONITOR_COMM_RCV);
					sigmask &=  ~(1 << MONITOR_RX_PIPE);
				}
			} else {
				DCC_LOG1(LOG_ERROR, "monitor_comm_recv n=%d", n);
				/* Wait for COMM_RECV */
				sigmask |= (1 << MONITOR_COMM_RCV);
				sigmask &=  ~(1 << MONITOR_RX_PIPE);
			}
		} else {
			DCC_LOG(LOG_TRACE, "Raw mode RX wait RX_PIPE");
			/* Wait for RX_PIPE */
			sigmask &= ~(1 << MONITOR_COMM_RCV);
			sigmask |= (1 << MONITOR_RX_PIPE);
		}
		break;

	case MONITOR_COMM_CTL:
		monitor_clear(MONITOR_COMM_CTL);

		connected = monitor_comm_isconnected(comm);
		thinkos_krn_console_connect_set(connected);

		sigmask &= ~((1 << MONITOR_COMM_EOT) | 
					 (1 << MONITOR_COMM_RCV) |
					 (1 << MONITOR_RX_PIPE));
		sigmask |= (1 << MONITOR_TX_PIPE);

		if (connected) {
			sigmask |= ((1 << MONITOR_COMM_EOT) |
						(1 << MONITOR_COMM_RCV));
		}

		break;

	case MONITOR_COMM_EOT:
		DCC_LOG(LOG_TRACE, "COMM_EOT");
		/* FALLTHROUGH */
	case MONITOR_TX_PIPE:
		if ((cnt = thinkos_console_tx_pipe_ptr(&ptr)) > 0) {
			int n;
			DCC_LOG1(LOG_TRACE, "TX Pipe: cnt=%d, send...", cnt);
			if ((n = monitor_comm_send(comm, ptr, cnt)) > 0) {
				thinkos_console_tx_pipe_commit(n);
				if (n == cnt) {
					/* Wait for TX_PIPE */
					sigmask |= (1 << MONITOR_TX_PIPE);
					sigmask &= ~(1 << MONITOR_COMM_EOT);
				} else {
					/* Wait for COMM_EOT */
					sigmask |= (1 << MONITOR_COMM_EOT);
					sigmask &= ~(1 << MONITOR_TX_PIPE);
				}
			} else {
				/* Wait for COMM_EOT */
				sigmask |= (1 << MONITOR_COMM_EOT);
				sigmask &=  ~(1 << MONITOR_TX_PIPE);
			}
		} else {
			/* Wait for TX_PIPE */
			DCC_LOG1(LOG_TRACE, "TX Pipe: cnt=%d, wait....", cnt);
			sigmask |= (1 << MONITOR_TX_PIPE);
			sigmask &= ~(1 << MONITOR_COMM_EOT);
		}
		break;

	case MONITOR_RX_PIPE:
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
					/* Wait for RX_PIPE */
					DCC_LOG(LOG_TRACE, 
							"RX_PIPE: Wait for RX_PIPE && COMM_RECV");
					sigmask |= (1 << MONITOR_COMM_RCV);
					sigmask &=  ~(1 << MONITOR_RX_PIPE);
				} else {
					DCC_LOG(LOG_TRACE, "RX_PIPE: Wait for COMM_RECV");
					/* Wait for COMM_RECV */
					sigmask |= (1 << MONITOR_COMM_RCV);
					sigmask &=  ~(1 << MONITOR_RX_PIPE);
				}
			} else {
				/* Wait for COMM_RECV */
				DCC_LOG(LOG_ERROR, "RX_PIPE: Wait for COMM_RECV");
				sigmask |= (1 << MONITOR_COMM_RCV);
				sigmask &=  ~(1 << MONITOR_RX_PIPE);
			}
		} else {
			DCC_LOG1(LOG_ERROR, "RX_PIPE: RX, cnt=%d", cnt);
			/* Wait for RX_PIPE */
			sigmask &= ~(1 << MONITOR_COMM_RCV);
			sigmask |= (1 << MONITOR_RX_PIPE);
		}
		break;

	}

	return sigmask;
}


