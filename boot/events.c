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
   Default Monitor comm event handler
 */
uint32_t monitor_on_comm_rcv(const struct monitor_comm * comm, 
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

	return sigmask;
}

uint32_t monitor_on_comm_ctl(const struct monitor_comm * comm, 
							 uint32_t sigmask)
{
	bool connected;

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

	return sigmask;
}

uint32_t monitor_on_tx_pipe(const struct monitor_comm * comm, 
							uint32_t sigmask)
{
	uint8_t * ptr;
	int cnt;

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

	return sigmask;
}

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

	return sigmask;
}


/*
   Exec a thread and wait for termination
 */
int monitor_thread_exec(const struct monitor_comm * comm, 
						int (* task)(void *), void * arg) 
{
	uint32_t sigmask = 0;
	int thread_id;
	int sig;

#if (THINKOS_ENABLE_THREAD_INFO)
	thread_id = monitor_thread_create(task, arg, &thinkos_main_inf);
#else
	thread_id = monitor_thread_create(task, arg, 0);
#endif
	monitor_thread_resume(thread_id);

	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_TX_PIPE);
	sigmask |= (1 << MONITOR_THREAD_TERMINATE);

	for(;;) {
		switch ((sig = monitor_select(sigmask))) {

		case MONITOR_COMM_RCV:
			sigmask = monitor_on_comm_rcv(comm, sigmask);
			break;

		case MONITOR_COMM_CTL:
			sigmask = monitor_on_comm_ctl(comm, sigmask);
			break;

		case MONITOR_COMM_EOT:
			DCC_LOG(LOG_TRACE, "COMM_EOT");
			/* FALLTHROUGH */
		case MONITOR_TX_PIPE:
			sigmask = monitor_on_tx_pipe(comm, sigmask);
			break;

		case MONITOR_RX_PIPE:
			sigmask = monitor_on_rx_pipe(comm, sigmask);
			break;

		case MONITOR_THREAD_TERMINATE: {
			int code;

			monitor_clear(MONITOR_THREAD_TERMINATE);
			thread_id = monitor_thread_terminate_get(&code);
			(void)code; 
			DCC_LOG2(LOG_TRACE, "/!\\ THREAD_TERMINATE thread_id=%d code=%d",
					thread_id, code);
			return code;
		}

		default:
			return -1;
		}
	}
}

int console_flash_ry_task(const char * tag);

int monitor_flash_ymodem_recv(const struct monitor_comm * comm, 
							 const char * tag)
{
	return monitor_thread_exec(comm, 
							   (int (*)(void *))console_flash_ry_task, 
							   (void *)tag);

}

int console_flash_erase_all_task(const char * tag);

int monitor_flash_erase_all(const struct monitor_comm * comm, 
						const char * tag)
{
	return monitor_thread_exec(comm, 
							   (int (*)(void *))console_flash_erase_all_task, 
							   (void *)tag);

}

