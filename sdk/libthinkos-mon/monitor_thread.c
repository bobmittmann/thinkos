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

#if (THINKOS_ENABLE_MONITOR)

//void __attribute__((noreturn)) 
void __monitor_thread_on_exit(unsigned int code)
{
	DCC_LOG1(LOG_WARNING, "code=%d", code);
	monitor_signal(MONITOR_USR_ABORT);
#if (THINKOS_ENABLE_TERMINATE)
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

	if ((ret = thinkos_main_thread_create(task, arg, 
		__monitor_thread_on_exit, true))  < 0) {
		return ret;
	}

	thread_id = ret;
	(void)thread_id;

	/* return in case of fault or abort */	
	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_USR_ABORT);
	sigmask |= (1 << MONITOR_COMM_BRK);

	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_EOT);
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_TX_PIPE);
	sigmask |= (1 << MONITOR_RX_PIPE);

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

		default:
			DCC_LOG1(LOG_WARNING, "unhandled signal: %d", sig);
			return -1;
		}
	}
}

	/*
void monitor_thread_resume(int thread_id)
{
	if (__thinkos_thread_resume(thread_id))
		__thinkos_defer_sched();
} 
	*/

int monitor_thread_create(const struct monitor_comm * comm, 
						thinkos_task_t task, void * arg,
						bool priviledged) 
{
	int ret;

	if ((ret = thinkos_main_thread_create(task, arg, 
		__monitor_thread_on_exit, priviledged))  < 0) {
		return ret;
	}

	return ret;
}

#endif /* THINKOS_ENABLE_MONITOR */

