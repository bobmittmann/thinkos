/* 
 * thikos_util.c
 *
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

_Pragma ("GCC optimize (\"Ofast\")")

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>

#if THINKOS_ENABLE_CONSOLE
void dbgmon_console_task(const struct dbgmon_comm * comm, void * param)
{
	uint32_t sigmask = 0;
	uint8_t * ptr;
	int cnt;
	int sig;
	bool connected = false;
	
	DCC_LOG1(LOG_TRACE, "Monitor sp=%08x ...", cm3_sp_get());

	sigmask |= (1 << DBGMON_COMM_RCV);
	sigmask |= (1 << DBGMON_COMM_CTL);
	sigmask |= (1 << DBGMON_TX_PIPE);

	for(;;) {
		switch ((sig = dbgmon_select(sigmask))) {

		case DBGMON_COMM_CTL:
			dbgmon_clear(DBGMON_COMM_CTL);
			DCC_LOG(LOG_MSG, "Comm Ctl.");
			connected = dbgmon_comm_isconnected(comm);
			sigmask &= ~((1 << DBGMON_COMM_EOT) | 
						 (1 << DBGMON_COMM_RCV) |
						 (1 << DBGMON_RX_PIPE));
			__console_connect_set(connected);
			if (connected) {
				DCC_LOG(LOG_TRACE, "Comm connected.");
				sigmask |= (1 << DBGMON_COMM_EOT) |
					(1 << DBGMON_COMM_RCV) |
					(1 << DBGMON_RX_PIPE);
				}
			sigmask |= (1 << DBGMON_TX_PIPE);
		break;


		case DBGMON_COMM_EOT:
			DCC_LOG(LOG_TRACE, "COMM_EOT");
			/* FALLTHROUGH */
		case DBGMON_TX_PIPE:
			DCC_LOG(LOG_MSG, "TX Pipe.");
			DCC_LOG1(LOG_TRACE, "TX Pipe :%d", __console_tx_pipe_ptr(&ptr));
			if ((cnt = __console_tx_pipe_ptr(&ptr)) > 0) {
				if (connected) {
					int n;
					if ((n = dbgmon_comm_send(comm, ptr, cnt)) > 0) {
						__console_tx_pipe_commit(n);
						if (n == cnt) {
							/* Wait for TX_PIPE */
							sigmask |= (1 << DBGMON_TX_PIPE);
							sigmask &= ~(1 << DBGMON_COMM_EOT);
						} else {
							/* Wait for COMM_EOT */
							sigmask |= (1 << DBGMON_COMM_EOT);
							sigmask &= ~(1 << DBGMON_TX_PIPE);
						}
					} else {
						/* Wait for COMM_EOT */
						sigmask |= (1 << DBGMON_COMM_EOT);
						sigmask &=  ~(1 << DBGMON_TX_PIPE);
					}
				} else {
					/* Drain the Rx pipe */
					__console_tx_pipe_commit(cnt);
				}
			} else {
				/* Wait for TX_PIPE */
				sigmask |= (1 << DBGMON_TX_PIPE);
				sigmask &= ~(1 << DBGMON_COMM_EOT);
			}
			break;

		case DBGMON_COMM_RCV:
		case DBGMON_RX_PIPE:
			DCC_LOG(LOG_TRACE, "DBGMON_RX_PIPE");
			/* get a pointer to the head of the pipe.
			   __console_rx_pipe_ptr() will return the number of 
			   consecutive spaces in the buffer. */
			if ((cnt = __console_rx_pipe_ptr(&ptr)) > 0) {
				int n;
			
				DCC_LOG1(LOG_TRACE, "Raw mode RX, cnt=%d", cnt);

				/* receive from the COMM driver */
				if ((n = dbgmon_comm_recv(comm, ptr, cnt)) > 0) {
					/* commit the fifo head */
					__console_rx_pipe_commit(n);
					if (n == cnt) {
						/* Wait for RX_PIPE */
						sigmask &= ~(1 << DBGMON_COMM_RCV);
						sigmask |= (1 << DBGMON_RX_PIPE);
					} else {
						/* Wait for RX_PIPE and/or COMM_RCV */
						sigmask |= (1 << DBGMON_RX_PIPE);
						sigmask |= (1 << DBGMON_COMM_RCV);
					}
				} else {
					/* Wait for COMM_RECV */
					sigmask |= (1 << DBGMON_COMM_RCV);
					sigmask &=  ~(1 << DBGMON_RX_PIPE);
				}
			} else {
				/* Wait for RX_PIPE */
				sigmask &= ~(1 << DBGMON_COMM_RCV);
				sigmask |= (1 << DBGMON_RX_PIPE);
			}
			break;
		}
	}
}

#endif /* THINKOS_ENABLE_CONSOLE */


