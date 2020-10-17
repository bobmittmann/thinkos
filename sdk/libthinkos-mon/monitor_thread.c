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

int monitor_thread_create(int (* func)(void *, unsigned int), void * arg, 
						 const struct thinkos_thread_inf * inf)
{
	int thread_id = (inf->thread_id > 0) ? inf->thread_id - 1 : 0;
	uint32_t sl = (uint32_t)inf->stack_ptr;
	uint32_t sp = sl + inf->stack_size;
	struct thinkos_context * ctx;

	if (__thinkos_thread_ctx_is_valid(thread_id)) {
		DCC_LOG2(LOG_WARNING, "thread %d already exists, ctx=%08x", 
				 thread_id + 1, __thinkos_thread_ctx_get(thread_id));

		DCC_LOG(LOG_TRACE, "__thinkos_thread_abort()");
		__thinkos_thread_abort(thread_id);
		monitor_wait_idle();
	}

	/* FIXME: This call is not safe  */
	/* Avoid race condition with kernel handlers */
	while (thinkos_kernel_active()) {
		DCC_LOG(LOG_TRACE, "kernel is active, wait for IDLE!!");
		monitor_wait_idle();
	}

#if THINKOS_ENABLE_THREAD_ALLOC
	/* allocate the thread block */
	__bit_mem_wr(&thinkos_rt.th_alloc, thread_id, 1);
#endif

	DCC_LOG2(LOG_TRACE, "__thinkos_thread_ctx_init(func=%p arg=%p)", func, arg);
	ctx = __thinkos_thread_ctx_init(thread_id, sp, 
									(uintptr_t)func, (uintptr_t)arg);
	ctx->lr = (uint32_t)__thinkos_thread_terminate_stub;
	DCC_LOG3(LOG_TRACE, "PC=%08X R0=%08x LR=%08x", ctx->pc, ctx->r0, ctx->lr);

#if THINKOS_ENABLE_THREAD_INFO
	__thinkos_thread_inf_set(thread_id, inf);
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	__thinkos_thread_sl_set(thread_id, sl);
#endif

#if (THINKOS_ENABLE_PAUSE)
	__thinkos_thread_pause_set(thread_id);
#endif

#if (THINKOS_ENABLE_DEBUG_FAULT)
	__thinkos_thread_fault_clr(thread_id);
#endif

	/* commit the context to the kernel */ 
	__thinkos_thread_ctx_set(thread_id, ctx, CONTROL_SPSEL | CONTROL_nPRIV);

	return thread_id;
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
			DCC_LOG1(LOG_WARNING, "unhandled signal: %d", sig);
			return -1;
		}
	}
}

