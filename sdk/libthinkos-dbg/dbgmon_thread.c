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
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>
#include <sys/dcclog.h>

/* -------------------------------------------------------------------------
 * Fast thread execution
 * ------------------------------------------------------------------------- */

void dbgmon_thread_destroy(int thread_id)
{
	thread_id = (thread_id > 0) ? thread_id - 1 : 0;

	if (thinkos_rt.ctx[thread_id] == NULL) {
		return;
	}

	DCC_LOG(LOG_MSG, "__thinkos_thread_abort()");
	__thinkos_thread_abort(thread_id);

	dbgmon_wait_idle();
}

int dbgmon_thread_create(int (* func)(void *), void * arg, 
						 const struct thinkos_thread_inf * inf)
{
	int thread_id = (inf->thread_id > 0) ? inf->thread_id - 1 : 0;
	uint32_t sp = (uint32_t)inf->stack_ptr + inf->stack_size;
	struct thinkos_context * ctx;

	if (thinkos_rt.ctx[thread_id] != NULL) {
		DCC_LOG2(LOG_WARNING, "thread %d already exists, ctx=%08x", 
				 thread_id + 1, thinkos_rt.ctx[thread_id]);

		DCC_LOG(LOG_TRACE, "__thinkos_thread_abort()");
		__thinkos_thread_abort(thread_id);
		dbgmon_wait_idle();
	}

	/* FIXME: This call is not safe  */
	/* Avoid race condition with kernel handlers */
	while (thinkos_kernel_active()) {
		DCC_LOG(LOG_TRACE, "kernel is active, wait for IDLE!!");
		dbgmon_wait_idle();
	}

#if THINKOS_ENABLE_THREAD_ALLOC
	/* allocate the thread block */
	__bit_mem_wr(&thinkos_rt.th_alloc, thread_id, 1);
#endif

	DCC_LOG2(LOG_TRACE, "__thinkos_thread_init(func=%p arg=%p)", func, arg);
	ctx = __thinkos_thread_init(thread_id, sp, func, arg);
	ctx->lr = (uint32_t)__thinkos_thread_terminate_stub;
	DCC_LOG3(LOG_TRACE, "PC=%08X R0=%08x LR=%08x", ctx->pc, ctx->r0, ctx->lr);

#if THINKOS_ENABLE_THREAD_INFO
	DCC_LOG(LOG_TRACE, "__thinkos_thread_inf_set()");
	__thinkos_thread_inf_set(thread_id, inf);
#endif

	return thread_id;
}

void dbgmon_thread_resume(int thread_id)
{
	if (__thinkos_thread_resume(thread_id))
		__thinkos_defer_sched();
} 


