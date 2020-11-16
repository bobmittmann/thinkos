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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <thinkos.h>
#include <sys/dcclog.h>
#include <sys/delay.h>

/* -------------------------------------------------------------------------
 * Fast thread execution
 * ------------------------------------------------------------------------- */

void monitor_thread_destroy(int thread_id)
{
	thread_id = (thread_id > 0) ? thread_id - 1 : 0;

	if (!__thinkos_thread_ctx_is_valid(thread_id)) {
		return;
	}

	DCC_LOG(LOG_MSG, "__thinkos_thread_abort()");
	__thinkos_thread_abort(thread_id);

	monitor_wait_idle();
}

void __attribute__((noreturn)) __monitor_thread_exit_stub(int code)
{
	DCC_LOG1(LOG_WARNING, "code=%d", code);

#if (THINKOS_ENABLE_MONITOR_THREADS)
//	monitor_signal_thread_terminate(0, code);
#endif

	thinkos_abort();
	for(;;);
}

int monitor_thread_create(int (* func)(void *, unsigned int), void * arg)
{
	const struct thinkos_thread_inf * inf = &thinkos_main_inf;
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_thread_initializer init;
	unsigned int thread_idx;
	int ret;

	thread_idx = (inf->thread_id > 0) ? inf->thread_id - 1 : 0;

#if 1
	if (__thinkos_thread_ctx_is_valid(thread_idx)) {
		DCC_LOG2(LOG_WARNING, "thread %d already exists, ctx=%08x", 
				 thread_idx + 1, __thinkos_thread_ctx_get(thread_idx));

		DCC_LOG(LOG_TRACE, "__thinkos_thread_abort()");
		__thinkos_thread_abort(thread_idx);
		monitor_wait_idle();
	}

	/* FIXME: This call is not safe  */
	/* Avoid race condition with kernel handlers */
	while (thinkos_kernel_active()) {
		DCC_LOG(LOG_TRACE, "kernel is active, wait for IDLE!!");
		monitor_wait_idle();
	}
#endif

#if THINKOS_ENABLE_THREAD_ALLOC
	/* force allocate the thread block */
	__bit_mem_wr(&krn->th_alloc, thread_idx, 1);
#endif

	init.stack_base = (uintptr_t)inf->stack_ptr;
	init.stack_size = inf->stack_size;
	init.task_entry = (uintptr_t)func;
	init.task_exit = (uintptr_t)__monitor_thread_exit_stub;
	init.task_arg[0] = (uintptr_t)arg;
	init.task_arg[1] = 0;
	init.task_arg[2] = 0;
	init.task_arg[3] = 0;
	init.priority = 0;
	init.paused = false;
	init.privileged = true;
	init.inf = inf;

	if ((ret = thinkos_krn_thread_init(krn, thread_idx, &init))) {
		__THINKOS_ERROR(THINKOS_THREAD_IDLE, ret);
		thread_idx = ret;
	};

	DCC_LOG1(LOG_WARNING, "thread=%d", thread_idx + 1);

	return thread_idx;
}

void monitor_thread_resume(int thread_id)
{
	if (__thinkos_thread_resume(thread_id))
		__thinkos_defer_sched();
} 

/*
   Exec a thread and wait for termination
 */
int monitor_thread_exec(const struct monitor_comm * comm, 
						int (* task)(void *, unsigned int), void * arg) 
{
	uint32_t sigmask = 0;
	int thread_id;
	int sig;

	thread_id = monitor_thread_create(task, arg);
	(void)thread_id;

	/* return in case of fault or abort */	
	sigmask |= (1 << MONITOR_KRN_EXCEPT);
	sigmask |= (1 << MONITOR_KRN_ABORT);
	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_THREAD_TERMINATE);
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

		default:
			DCC_LOG1(LOG_WARNING, "unhandled signal: %d", sig);
			return -1;
		}
	}
}

