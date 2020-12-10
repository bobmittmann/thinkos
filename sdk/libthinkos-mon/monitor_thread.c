/* 
 * File:	 usb-cdc.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "thinkos_mon-i.h"

/* -------------------------------------------------------------------------
 * Fast thread execution
 * ------------------------------------------------------------------------- */

//void __attribute__((noreturn)) 
void __monitor_thread_on_exit(unsigned int code)
{
	DCC_LOG1(LOG_WARNING, "code=%d", code);
#if 1
	monitor_signal(MONITOR_USR_ABORT);
	thinkos_thread_abort(code);
#else
	thinkos_abort();
#endif
}

/*
   Exec a thread and wait for termination
 */
int monitor_thread_exec(const struct monitor_comm * comm, 
						int (* task)(void *, unsigned int), void * arg) 
{
	uint32_t sigmask = 0;
	int thread_id;
	int ret = 0;
	int sig;

	if ((ret = thinkos_dbg_thread_create(task, arg, __monitor_thread_on_exit, true))  < 0)
		return ret;

	thread_id = ret;
	(void)thread_id;

	/* return in case of fault or abort */	
	sigmask |= (1 << MONITOR_KRN_FAULT);
	sigmask |= (1 << MONITOR_KRN_ABORT);
	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_USR_ABORT);
#if (THINKOS_ENABLE_MONITOR_THREADS)
	sigmask |= (1 << MONITOR_THREAD_TERMINATE);
#endif
	sigmask |= (1 << MONITOR_COMM_BRK);
	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_TX_PIPE);

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

#if (THINKOS_ENABLE_MONITOR_THREADS)
		case MONITOR_THREAD_TERMINATE: {
			monitor_clear(MONITOR_THREAD_TERMINATE);
			int code;

			thread_id = monitor_thread_terminate_get(&code);
			(void)code; 
			DCC_LOG2(LOG_TRACE, "/!\\ THREAD_TERMINATE thread_id=%d code=%d",
					thread_id, code);
			return code;
		}
#endif
		default:
			DCC_LOG1(LOG_WARNING, "unhandled signal: %d", sig);
			return -1;
		}
	}
}

void monitor_thread_resume(int thread_id)
{
	if (__thinkos_thread_resume(thread_id))
		__thinkos_defer_sched();
} 

