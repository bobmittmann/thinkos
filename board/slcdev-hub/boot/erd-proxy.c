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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file erd-prox.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <stdlib.h>
#include <stdbool.h>

#include <sys/dcclog.h>
#include <sys/stm32f.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>

struct erd_proxy {
	struct dmon_comm * comm;
};


/* ERD - Embedded Remote Debug */
#define ERD_PROXY_PKTBUF_LEN 512

static int erd_on_fault(struct erd_proxy * proxy, uint32_t * pkt)
{
	return 0;
/*
	int thread_id;

	if (erd->stopped) {
		DCC_LOG(LOG_WARNING, "on fault, suspended already!");
		return 0;
	}

	thread_id = thread_break_id();
	gdb->thread_id.g = thread_id; 

	DCC_LOG1(LOG_TRACE, "suspending (current=%d) ... ...", thread_id);

	target_halt();
	gdb->stopped = true;
	gdb->last_signal = TARGET_SIGNAL_SEGV;

	return rsp_thread_stop_reply(gdb, pkt, thread_id);
*/
}

int erd_on_break(struct erd_proxy * proxy, uint32_t * pkt)
{
	return 0;
/*
	int thread_id;

	DCC_LOG(LOG_TRACE, "on break, suspending... ... ...");

	//gdb->thread_id.g = thread_active();

	target_halt();
	thread_id = thread_any();
	gdb->thread_id.g = thread_id;
	gdb->stopped = true;
	gdb->last_signal = TARGET_SIGNAL_INT;
	
	return rsp_thread_stop_reply(gdb, pkt, thread_id);
*/
}

static int erd_on_step(struct erd_proxy * proxy, uint32_t * pkt)
{
	return 0;
}


static int erd_on_breakpoint(struct erd_proxy * proxy, uint32_t * pkt)
{
	return 0;
}

enum erdrpc_op {
	ERDRPC_SUSPEND       = 1,
	ERDRPC_RESUME        = 2,
	ERDRPC_EXEC          = 3,
	ERDRPC_REBOOT        = 4,
	ERDRPC_KERNELINFO    = 5,
	ERDRPC_MEM_LOCK      = 6,
	ERDRPC_MEM_UNLOCK    = 7,
	ERDRPC_MEM_ERASE     = 8,
	ERDRPC_MEM_READ      = 9,
	ERDRPC_MEM_WRITE     = 10,
	ERDRPC_MEM_SEEK      = 11,
	ERDRPC_MEM_CRC32     = 12,
	ERDRPC_EXCEPTINFO    = 13,
	ERDRPC_THREADINFO    = 14,
};

void dbg_proxy_task(struct dmon_comm * comm)
{
	struct erd_proxy erd_proxy;
	struct erd_proxy * proxy = &erd_proxy;
	uint32_t pkt[ERD_PROXY_PKTBUF_LEN / 4];
	uint32_t hdr;
	uint32_t sigmask;
	uint32_t sigset;

	proxy->comm = comm;

	dmon_breakpoint_clear_all();
	dmon_watchpoint_clear_all();

	SIG_ZERO(sigmask);
	SIG_SET(sigmask, DBGMON_EXCEPT);
	SIG_SET(sigmask, DBGMON_THREAD_FAULT);
	SIG_SET(sigmask, DBGMON_THREAD_STEP);
	SIG_SET(sigmask, DBGMON_COMM_RCV);
	SIG_SET(sigmask, DBGMON_COMM_CTL);
	SIG_SET(sigmask, DBGMON_BREAKPOINT);
#if (THINKOS_ENABLE_CONSOLE)
	SIG_SET(sigmask, DBGMON_TX_PIPE);
#endif
	SIG_SET(sigmask, DBGMON_SOFTRST);

	for(;;) {
		sigset = dbgmon_select(sigmask);

		DCC_LOG1(LOG_MSG, "sig=%08x", sigset);

		if (SIG_ISSET(sigset, DBGMON_SOFTRST)) {
			dbgmon_clear(DBGMON_SOFTRST);
			this_board.softreset();
		}

		if (SIG_ISSET(sigset, DBGMON_COMM_CTL)) {
			DCC_LOG(LOG_TRACE, "Comm Ctl.");
			dbgmon_clear(DBGMON_COMM_CTL);
			if (!dmon_comm_isconnected(comm)) {
				DCC_LOG(LOG_WARNING, "Debug Monitor Comm closed!");
				return;
			}
		}

#if (THINKOS_ENABLE_CONSOLE)
		if (SIG_ISSET(sigset, DBGMON_TX_PIPE)) {
			DCC_LOG(LOG_MSG, "TX Pipe.");
//			if (rsp_console_output(gdb, pkt) <= 0) {
				dbgmon_clear(DBGMON_TX_PIPE);
//			}
		}
#endif


		if (SIG_ISSET(sigset, DBGMON_EXCEPT)) {
			DCC_LOG(LOG_TRACE, "Exception.");
			dbgmon_clear(DBGMON_EXCEPT);
			erd_on_fault(proxy, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_THREAD_FAULT)) {
			DCC_LOG(LOG_TRACE, "Thread fault.");
			dbgmon_clear(DBGMON_THREAD_FAULT);
			erd_on_fault(proxy, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_THREAD_STEP)) {
			DCC_LOG(LOG_INFO, "DBGMON_THREAD_STEP");
			dbgmon_clear(DBGMON_THREAD_STEP);
			erd_on_step(proxy, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_BREAKPOINT)) {
			DCC_LOG(LOG_INFO, "DBGMON_BREAKPOINT");
			dbgmon_clear(DBGMON_BREAKPOINT);
			erd_on_breakpoint(proxy, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_COMM_RCV)) {
			unsigned int op;
			unsigned int len;

			/* get the packet header */
			if (dmon_comm_recv(comm, &hdr, ERD_PROXY_PKTBUF_LEN) != 4) {
				DCC_LOG(LOG_WARNING, "dmon_comm_recv() failed!");
				return;
			}

			op = hdr & 0x7f;
			len = (hdr >> 7) & 0x1ff;
			
			if (len > 0) {
				/* get the packet payload */
				unsigned int rem = len;
				uint8_t * cp = (uint8_t *)pkt;

				while (rem) {
					int n;

					if ((n = dmon_comm_recv(comm, cp, rem)) <= 0) {
						DCC_LOG(LOG_WARNING, "dmon_comm_recv() failed!");
						return;
					}
					cp += n;
					rem -= n;
				}
			}

			switch (op) {
			case ERDRPC_SUSPEND:
				break;

			case ERDRPC_RESUME:
				break;

			case ERDRPC_EXEC:
				break;

			case ERDRPC_REBOOT:
				break;

			case ERDRPC_KERNELINFO:
				break;

			case ERDRPC_MEM_LOCK:
				break;

			case ERDRPC_MEM_UNLOCK:
				break;

			case ERDRPC_MEM_ERASE:
				break;

			case ERDRPC_MEM_READ:
				break;

			case ERDRPC_MEM_WRITE:
				break;

			case ERDRPC_MEM_SEEK:
				break;

			case ERDRPC_MEM_CRC32:
				break;

			case ERDRPC_EXCEPTINFO:
				break;

			case ERDRPC_THREADINFO:
				break;

			default:
				DCC_LOG(LOG_ERROR, "ERD protocol error!");
				return;
			}
		}
	}
}
 
