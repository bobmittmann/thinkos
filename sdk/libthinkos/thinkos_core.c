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
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#include <thinkos.h>

//#undef THINKOS_ENABLE_SCHED_DEBUG
//#define THINKOS_ENABLE_SCHED_DEBUG 1

void thinkos_idle_task(void);

/* -------------------------------------------------------------------------- 
 * Run Time ThinkOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt thinkos_rt;

/* This is a trick to save memory. We need an area to store
   the Idle thread context, but the Idle thread only uses 4 entries:
   xpsr, pc, lr, r12. The other registers are not used at any time. We 
   claim the space avalilable for this registers as part of the exception 
   stack. */
uint32_t thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

#if THINKOS_ENABLE_SCHED_DEBUG
static inline void __attribute__((always_inline)) 
__dump_context(struct thinkos_context * __ctx) {
	DCC_LOG4(LOG_TRACE, "  r0=%08x  r1=%08x  r2=%08x  r3=%08x", 
			__ctx->r0, __ctx->r1, __ctx->r2, __ctx->r3);
	DCC_LOG4(LOG_TRACE, "  r4=%08x  r5=%08x  r6=%08x  r7=%08x", 
			__ctx->r4, __ctx->r7, __ctx->r6, __ctx->r7);
	DCC_LOG4(LOG_TRACE, "  r8=%08x  r9=%08x r10=%08x r11=%08x", 
			__ctx->r8, __ctx->r9, __ctx->r10, __ctx->r11);
	DCC_LOG4(LOG_TRACE, " r12=%08x  sp=%08x  lr=%08x  pc=%08x", 
			__ctx->r12, __ctx, __ctx->lr, __ctx->pc);
	DCC_LOG1(LOG_TRACE, "xpsr=%08x", __ctx->xpsr);
}

void test_call(struct thinkos_context * ctx)
{
	__dump_context(ctx);
}

static inline void __attribute__((always_inline)) __wait(void) {
	asm volatile ("mov    r3, #1\n"
				  "0:\n"
				  "cbz	r3, 1f\n"
				  "b.n  0b\n"
				  "1:\n" : : : "r3"); 
}
#endif

/* -------------------------------------------------------------------------- 
 * Scheduler
 * --------------------------------------------------------------------------*/

#if (!THINKOS_ASM_SCHEDULER)

static inline struct thinkos_context * __attribute__((always_inline)) 
__sched_entry(void) {
	register struct thinkos_context * ctx asm("r0");
	asm volatile (
#if THINKOS_ENABLE_SCHED_DEBUG
				  "push  {lr}\n"
				  "sub   sp, #16\n"
#endif				  
				  "mrs   %0, PSP\n" 
				  "stmdb %0!, {r4-r11}\n"
				  : "=r" (ctx));
	return ctx;
}

static inline void __attribute__((always_inline)) 
__sched_exit(struct thinkos_context * __ctx) {
	register struct thinkos_context * r0 asm("r0") = __ctx;
	asm volatile (
#if THINKOS_ENABLE_SCHED_DEBUG
				  "add    sp, #16\n"
				  "pop    {lr}\n"
#endif				  
				  "add    r3, %0, #8 * 4\n"
				  "msr    PSP, r3\n"
				  "ldmia  %0, {r4-r11}\n"
				  "bx     lr\n"
				  : : "r" (r0) : "r3"); 
}

static inline void __attribute__((always_inline)) 
__sched_exit_step(struct thinkos_context * ctx, unsigned int thread_id) 
{
	register struct thinkos_context * r0 asm("r0") = ctx;
	register unsigned int r1 asm("r1") = thread_id;

	asm volatile ("movw   r2, #(1 << 5)\n"
				  "msr    BASEPRI, r2\n"
				  : : : "r2"); 
	asm volatile (
#if THINKOS_ENABLE_SCHED_DEBUG
				  "add    sp, #16\n"
				  "pop    {lr}\n"
#endif				  
				  "add    r2, %0, #8 * 4\n"
				  "msr    PSP, r2\n"
				  "ldmia  %0, {r4-r11}\n"
				  : : "r" (r0), "r" (r1) : "r2"); 
	/* CM3_DCB->demcr |= DCB_DEMCR_MON_STEP */
	asm volatile ("movw   r3, #0xedf0\n"
				  "movt   r3, #0xe000\n"
				  "ldr    r2, [r3, #12]\n"
				  "orr.w  r2, r2, #(1 << 18)\n"
				  "str    r2, [r3, #12]\n"
				  "bx     lr\n"
				  : : : "r2", "r3"); 
}

/* --------------------------------------------------------------------------
 * ThinkOS - scheduler 
 * --------------------------------------------------------------------------*/
void __attribute__((naked, aligned(16))) cm3_pendsv_isr(void)
{
	struct thinkos_context * old_ctx;
	struct thinkos_context * new_ctx;
	uint32_t old_thread_id;
	uint32_t new_thread_id;

	/* save the context */
	old_ctx = __sched_entry();
	/* get the active (current) thread */	
	old_thread_id = thinkos_rt.active;

//	DCC_LOG(LOG_TRACE, "...");

	/* get a thread from the ready bitmap */
	new_thread_id = __clz(__rbit(thinkos_rt.wq_ready));
	/* update the active thread */
	thinkos_rt.active = new_thread_id;
	/* save the old context (SP) */
	thinkos_rt.ctx[old_thread_id] = old_ctx;
	/* get the new context (SP) */
	new_ctx = thinkos_rt.ctx[new_thread_id];

#if THINKOS_ENABLE_PROFILING
	{
		uint32_t cyccnt = CM3_DWT->cyccnt;
		int32_t delta = cyccnt - thinkos_rt.cycref;
		/* update the reference */
		thinkos_rt.cycref = cyccnt;
		/* update thread's cycle counter */
		thinkos_rt.cyccnt[old_thread_id] += delta; 
	}
#endif

#if THINKOS_ENABLE_SCHED_DEBUG
//		DCC_LOG1(LOG_TRACE, "active=%d", new_thread_id);
//		DCC_LOG1(LOG_TRACE, "sp=%08x", cm3_sp_get());
//		__dump_context(new_ctx);
#endif

#if THINKOS_ENABLE_DEBUG_STEP
	if ((1 << new_thread_id) & thinkos_rt.step_req) {
		/* process a step request */
		if ((1 << new_thread_id) & thinkos_rt.step_svc) {
#if THINKOS_ENABLE_SCHED_DEBUG
			DCC_LOG1(LOG_TRACE, "SVC step thread=%d, active is IDLE"
					 , new_thread_id);
#endif
			/* step the IDLE thread instead  */
			thinkos_rt.active = THINKOS_THREAD_IDLE;
			/* set the new context to the idle context */
			new_ctx = thinkos_rt.ctx[THINKOS_THREAD_IDLE];
			/* XXX: reset the IDLE task. There is a problem when stepping
			   at a SVC call. The  __sched_exit_step() disable interrupts,
			   but the idle thread next instruction could potentially be 
			   SVC, which generate a soft IRQ. The fact that the interrupts 
			   are disabled causes a hard fault to the system. 
			   We force the ide thread to start from the beginning where
			   at least one nop instruction will be executed before 
			   a SVC call.
			 */ 
			new_ctx->pc = (uint32_t)thinkos_idle_task;
		} else { 
			uint32_t insn;
			uint16_t * pc;
#if THINKOS_ENABLE_SCHED_DEBUG
			DCC_LOG1(LOG_TRACE, "normal step, active=%d", new_thread_id);
#endif
			/* get the PC value */
			pc = (uint16_t *)new_ctx->pc;
			/* get the next instruction */
			insn = pc[0];
			/* if the thread is running, and it is about to invoke 
			   a system call then we don't step but set the service 
			   flag for stepping on service exit. */
			if ((insn & 0xdf00) == 0xdf00) {
				/* the thread is stepping into a system call */
				thinkos_rt.step_svc |= (1 << new_thread_id);
				goto context_restore;
			}
		}
		/* set the step thread as the current thread ... */
		thinkos_rt.step_id = new_thread_id;
		/* return and step the next instruction */
		__sched_exit_step(new_ctx, new_thread_id);
	} else
context_restore:
#endif
	/* restore the context */
	__sched_exit(new_ctx);
}
#endif /* !THINKOS_ASM_SCHEDULER */

#if THINKOS_ENABLE_CLOCK
static void thinkos_time_wakeup(int thread_id) 
{
#if THINKOS_ENABLE_THREAD_STAT
	int stat;
	/* update the thread status */
	stat = thinkos_rt.th_stat[thread_id];
	thinkos_rt.th_stat[thread_id] = 0;
	/* remove from other wait queue, if any */
	__bit_mem_wr(&thinkos_rt.wq_lst[stat >> 1], thread_id, 0);  
#endif
	/* remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, thread_id, 0);  
	DCC_LOG1(LOG_INFO, "Wakeup %d...", thread_id);
	/* insert into the ready wait queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread_id, 1);  
	__thinkos_preempt();
}
#endif /* THINKOS_ENABLE_CLOCK */

#if THINKOS_ENABLE_TIMESHARE
static void thinkos_timeshare(void) 
{
	int32_t idx;

	idx = thinkos_rt.active;

	/*  */
	thinkos_rt.sched_val[idx] -= thinkos_rt.sched_pri[idx];
	if (thinkos_rt.sched_val[idx] < 0) {
		thinkos_rt.sched_val[idx] += thinkos_rt.sched_limit;
		if (__bit_mem_rd(&thinkos_rt.wq_ready, idx) == 0) {
			DCC_LOG1(LOG_INFO, "thread %d is active but not ready!!!", idx);
		} else {
			/* insert into the CPU wait queue */
			__bit_mem_wr(&thinkos_rt.wq_tmshare, idx, 1);  
			__thinkos_suspend(idx);
			__thinkos_preempt();
		}
	}
}
#endif /* THINKOS_ENABLE_TIMESHARE */


/* --------------------------------------------------------------------------
 * ThinkOS - defered services
 * --------------------------------------------------------------------------*/

#if THINKOS_ENABLE_CLOCK
void __attribute__((aligned(16))) cm3_systick_isr(void)
{
	uint32_t ticks;
	uint32_t wq;
	int j;

	ticks = thinkos_rt.ticks; 
	ticks++;
	thinkos_rt.ticks = ticks; 

	wq = __rbit(thinkos_rt.wq_clock);
	while ((j = __clz(wq)) < 32) {
		wq &= ~(0x80000000 >> j);  
		if ((int32_t)(thinkos_rt.clock[j] - ticks) <= 0) {
			thinkos_time_wakeup(j); 
		}
	}

#if THINKOS_ENABLE_DMCLOCK
	if ((int32_t)(thinkos_rt.dmclock - ticks) == 0) {
		dbgmon_signal(DBGMON_ALARM);
	}
#endif

#if THINKOS_ENABLE_TIMESHARE
	thinkos_timeshare(); 
#endif /* THINKOS_ENABLE_TIMESHARE */

}
#endif /* THINKOS_ENABLE_CLOCK */

