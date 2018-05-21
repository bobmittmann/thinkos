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

#define __THINKOS_SYS__
#include <thinkos_sys.h>
#define __THINKOS_DBGMON__
#include <thinkos_dmon.h>
#include <thinkos.h>

#include <sys/param.h>

#if THINKOS_ENABLE_CONSOLE

void __attribute__((noreturn)) dbgmon_console_io_task(struct dbgmon_comm * comm)
{
	uint32_t sigmask;
	uint32_t sigset;
	uint8_t * buf;
	int cnt;
	int len;

	DCC_LOG(LOG_TRACE, "Monitor COMM bypass start...");

	dbgmon_comm_rxflowctrl(comm, true);
	dbgmon_comm_connect(comm);

	DCC_LOG(LOG_INFO, "COMMM connected...");


	sigmask = (1 << DBGMON_THREAD_FAULT);
	sigmask |= (1 << DBGMON_COMM_RCV);
	sigmask |= (1 << DBGMON_COMM_CTL);
	sigmask |= (1 << DBGMON_TX_PIPE);
	sigmask |= (1 << DBGMON_RX_PIPE);
	for(;;) {
		
		sigset = dbgmon_select(sigmask);
		DCC_LOG1(LOG_MSG, "sigset=%08x.", sigset);

		if (sigset & (1 << DBGMON_THREAD_FAULT)) {
			DCC_LOG(LOG_INFO, "Thread fault.");
			dbgmon_clear(DBGMON_THREAD_FAULT);
		}

		if (sigset & (1 << DBGMON_COMM_CTL)) {
			DCC_LOG(LOG_INFO, "Comm Ctl.");
			dbgmon_clear(DBGMON_COMM_CTL);
			if (!dbgmon_comm_isconnected(comm))	
				dbgmon_reset();
		}

		if (sigset & (1 << DBGMON_COMM_RCV)) {
			if ((cnt = __console_rx_pipe_ptr(&buf)) > 0) {
				DCC_LOG1(LOG_INFO, "Comm recv. rx_pipe.free=%d", cnt);
				if ((len = dbgmon_comm_recv(comm, buf, cnt)) > 0)
					__console_rx_pipe_commit(len); 
			} else {
				DCC_LOG(LOG_INFO, "Comm recv. Masking DBGMON_COMM_RCV!");
				sigmask &= ~(1 << DBGMON_COMM_RCV);
			}
		}

		if (sigset & (1 << DBGMON_RX_PIPE)) {
			if ((cnt = __console_rx_pipe_ptr(&buf)) > 0) {
				DCC_LOG1(LOG_INFO, "RX Pipe. rx_pipe.free=%d. "
						 "Unmaksing DBGMON_COMM_RCV!", cnt);
				sigmask |= (1 << DBGMON_COMM_RCV);
			} else {
				DCC_LOG(LOG_INFO, "RX Pipe empty!!!");
			}
			dbgmon_clear(DBGMON_RX_PIPE);
		}

		if (sigset & (1 << DBGMON_TX_PIPE)) {
			DCC_LOG(LOG_MSG, "TX Pipe.");
			if ((cnt = __console_tx_pipe_ptr(&buf)) > 0) {
				len = dbgmon_comm_send(comm, buf, cnt);
				__console_tx_pipe_commit(len); 
			} else {
				DCC_LOG(LOG_INFO, "TX Pipe empty!!!");
				dbgmon_clear(DBGMON_TX_PIPE);
			}
		}
	}
}

#endif /* THINKOS_ENABLE_CONSOLE */

