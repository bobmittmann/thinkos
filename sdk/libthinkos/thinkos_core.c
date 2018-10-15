/* 
 * thikos_core.c
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#include <sys/delay.h>
#include <thinkos.h>
#include <vt100.h>

void thinkos_idle_task(void);

/* -------------------------------------------------------------------------- 
 * Run Time ThinkOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt thinkos_rt;

uint32_t __attribute__((aligned(8))) thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

const uint16_t thinkos_except_stack_size = sizeof(thinkos_except_stack);


void __thinkos_idle_error(void * arg)
{
	udelay(500000);
	DCC_LOG(LOG_ERROR, "ThinkOS Idle error!!"); 
	udelay(500000);
}

void thinkos_sched_dbg(struct thinkos_context * __ctx, 
					   uint32_t __new_thread_id,
					   uint32_t __prev_thread_id, 
					   uint32_t __sp); 

void __tdump(void);

#if (THINKOS_ENABLE_SANITY_CHECK)
uint32_t thinkos_sched_error(struct thinkos_context * __ctx, 
							 uint32_t __new_thread_id,
							 uint32_t __prev_thread_id, 
							 uint32_t __sp)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();
	struct thinkos_context * ctx = &xcpt->ctx.core;

	DCC_LOG(LOG_ERROR, _ATTR_PUSH_ _FG_RED_ _BRIGHT_ _REVERSE_

			" /!\\ scheduler error! " _ATTR_POP_);
	
#if (THINKOS_ENABLE_SCHED_DEBUG)
	thinkos_sched_dbg(__ctx, __new_thread_id, __prev_thread_id, __sp);
	__context(__ctx, __new_thread_id); 
	__thinkos(&thinkos_rt);
	__tdump();
#endif
	if (__new_thread_id == THINKOS_THREAD_IDLE) {
 		return __thinkos_idle_reset(__thinkos_idle_error, NULL);
	}

	thinkos_rt.break_id = __new_thread_id;
	/* clear IPSR to indicate a thread error */
	thinkos_rt.xcpt_ipsr = 0;
	xcpt->active = __prev_thread_id;
	xcpt->type = THINKOS_ERR_INVALID_STACK;
#if (THINKOS_ENABLE_DEBUG_FAULT)
	/* flag the thread as faulty */
	__thinkos_thread_fault_set(__new_thread_id);
#endif
	/* copy the thread exception context to the exception buffer. */
	__thinkos_memcpy32(ctx, __ctx, sizeof(struct thinkos_context)); 

	__idle_hook_req(IDLE_HOOK_EXCEPT_DONE);

	/* set the active thread to idle */
	thinkos_rt.active = THINKOS_THREAD_IDLE;
	/* get the IDLE context */
	ctx = thinkos_rt.ctx[THINKOS_THREAD_IDLE];
	
 	return (uint32_t)ctx;
}
#endif


bool thinkos_sched_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_PENDSVACT) ? true : false;
}
	
bool thinkos_syscall_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_SVCALLACT) ? true : false;

}

bool thinkos_clock_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_SYSTICKACT) ? true : false;
}

bool thinkos_dbgmon_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_MONITORACT) ? true : false;
}

bool thinkos_kernel_active(void)
{
	return (CM3_SCB->shcsr & (SCB_SHCSR_SYSTICKACT | SCB_SHCSR_PENDSVACT | 
							  SCB_SHCSR_SVCALLACT)) ? true : false;
}

void __thinkos_core_reset(void)
{
	int i;

	/* clear all threads excpet NULL */
	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		thinkos_rt.ctx[i] = 0x00000000;
#if THINKOS_ENABLE_THREAD_STAT
		thinkos_rt.th_stat[i] = 0; 
#endif
#if THINKOS_ENABLE_THREAD_INFO
		thinkos_rt.th_inf[i] = NULL; 
#endif
	}

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_LST_END ; ++i)
		thinkos_rt.wq_lst[i] = 0x00000000;

#if (THINKOS_ENABLE_PROFILING)
	/* Per thread cycle count */
	for (i = 0; i < THINKOS_THREADS_MAX ; ++i)
		thinkos_rt.cyccnt[i] = 0;
#endif

#if THINKOS_ENABLE_THREAD_ALLOC
	/* initialize the thread allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.th_alloc, THINKOS_THREADS_MAX); 
#endif

#if (THINKOS_MUTEX_MAX > 0)
#if THINKOS_ENABLE_MUTEX_ALLOC
	/* initialize the mutex locks */
	for (i = 0; i < THINKOS_MUTEX_MAX; i++) 
	thinkos_rt.lock[i] = -1;
	/* initialize the mutex allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.mutex_alloc, THINKOS_MUTEX_MAX); 
#endif
#endif /* THINKOS_MUTEX_MAX > 0 */

#if THINKOS_SEMAPHORE_MAX > 0
	for (i = 0; i < THINKOS_SEMAPHORE_MAX; i++) 
		thinkos_rt.sem_val[i] = 0;
#if THINKOS_ENABLE_SEM_ALLOC
	/* initialize the semaphore allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.sem_alloc, THINKOS_SEMAPHORE_MAX); 
#endif
#endif /* THINKOS_SEMAPHORE_MAX > 0 */

#if THINKOS_ENABLE_COND_ALLOC
	/* initialize the conditional variable allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.cond_alloc, THINKOS_COND_MAX); 
#endif

#if THINKOS_FLAG_MAX > 0
	for (i = 0; i < (THINKOS_FLAG_MAX + 31) / 32; i++) 
		thinkos_rt.flag[i] = 0;
#if THINKOS_ENABLE_FLAG_ALLOC
	/* initialize the flag allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.flag_alloc, THINKOS_FLAG_MAX); 
#endif
#endif /* THINKOS_FLAG_MAX > 0 */

#if THINKOS_EVENT_MAX > 0
	for (i = 0; i < THINKOS_EVENT_MAX ; i++) {
		thinkos_rt.ev[i].pend = 0;
		thinkos_rt.ev[i].mask = 0xffffffff;
	}
#if THINKOS_ENABLE_EVENT_ALLOC
	/* initialize the event set allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.ev_alloc, THINKOS_EVENT_MAX); 
#endif
#endif /* THINKOS_EVENT_MAX > 0 */

#if THINKOS_GATE_MAX > 0
	for (i = 0; i < ((THINKOS_GATE_MAX + 15) / 16); i++) 
		thinkos_rt.gate[i] = 0;
#if THINKOS_ENABLE_GATE_ALLOC
	/* initialize the gate allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.gate_alloc, THINKOS_GATE_MAX); 
#endif
#endif /* THINKOS_GATE_MAX > 0 */

#if (THINKOS_ENABLE_DEBUG_BKPT)
	thinkos_rt.step_id = -1;
	thinkos_rt.break_id = -1;
#if THINKOS_ENABLE_DEBUG_STEP
	thinkos_rt.step_svc = 0;  /* step at service call bitmap */
	thinkos_rt.step_req = 0;  /* step request bitmap */
#endif
#endif

#if (THINKOS_ENABLE_CRITICAL)
	thinkos_rt.critical_cnt = 0;
#endif

#if THINKOS_IRQ_MAX > 0
	__thinkos_irq_reset_all();
#endif
}

void __thinkos_system_reset(void)
{
	DCC_LOG(LOG_WARNING, "/!\\ System reset in progress...");
	/* Return to the Monitor applet with the SOFTRST signal set.
	   The applet should clear the hardware and restore the core interrups.
	   Next context_swap() will continue in __do_soft_reset() ... */

	DCC_LOG(LOG_TRACE, "1. ThinkOS core reset...");
	__thinkos_core_reset();

#if THINKOS_ENABLE_EXCEPTIONS
	DCC_LOG(LOG_TRACE, "2. exception reset...");
	__exception_reset();
#endif

	DCC_LOG(LOG_TRACE, "5. reset RAM vectors...");
	__dbgmon_reset();

	/* Enable Interrupts */
	DCC_LOG(LOG_TRACE, "6. enablig interrupts...");
	cm3_cpsie_i();
}

void __thinkos_kill_all(void) 
{
	int i;

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_LST_END ; ++i)
		thinkos_rt.wq_lst[i] = 0x00000000;

#if THINKOS_ENABLE_THREAD_VOID 
	/* discard current thread context */
	if (thinkos_rt.active != THINKOS_THREAD_IDLE)
		thinkos_rt.active = THINKOS_THREAD_VOID;
#else
	DCC_LOG(LOG_PANIC, "can't set current thread to void!"); 
#endif
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}

