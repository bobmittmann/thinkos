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

#define __THINKOS_SYS__
#include <thinkos_sys.h>

#if THINKOS_ENABLE_CLOCK
  #define SIZEOF_TCB_CLOCK 4
#else
  #define SIZEOF_TCB_CLOCK 0
#endif
#if THINKOS_ENABLE_THREAD_STAT | THINKOS_ENABLE_TIMESHARE
  #define SIZEOF_TCB_STAT 4
#else
  #define SIZEOF_TCB_STAT 0
#endif
#if THINKOS_ENABLE_THREAD_INFO
  #define SIZEOF_TCB_INF 4
#else
  #define SIZEOF_TCB_INF 0
#endif

#define SIZEOF_TCB (4 + SIZEOF_TCB_CYCCNT + SIZEOF_TCB_CLOCK + \
					SIZEOF_TCB_STAT + SIZEOF_TCB_INF)

#define SIZEOF_TCB_LST ((THINKOS_THREADS_MAX + 1) * SIZEOF_TCB)

#if THINKOS_ENABLE_THREAD_VOID
  #define SIZEOF_VOID_CTX 4
  #define SIZEOF_CTX ((THINKOS_THREADS_MAX + 2) * 4)
#else
  #define SIZEOF_VOID_CTX 0
  #define SIZEOF_CTX ((THINKOS_THREADS_MAX + 1) * 4)
#endif

#if THINKOS_ENABLE_PROFILING
  #if THINKOS_ENABLE_THREAD_VOID
    #define SIZEOF_CYCCNT ((THINKOS_THREADS_MAX + 2) * 4)
  #else
    #define SIZEOF_CYCCNT ((THINKOS_THREADS_MAX + 1) * 4)
  #endif
  #define SIZEOF_CYCREF 4
#else
  #define SIZEOF_CYCCNT 0
  #define SIZEOF_CYCREF 0
#endif

#if THINKOS_ENABLE_CRITICAL
  #define SIZEOF_CRITCNT 4
#else
  #define SIZEOF_CRITCNT 0
#endif

#if THINKOS_ENABLE_TIMESHARE
  #define SIZEOF_SCHED_LM 4
#else
  #define SIZEOF_SCHED_LM 0
#endif

#if THINKOS_ENABLE_CLOCK
  #define SIZEOF_TICKS 4
  #if THINKOS_ENABLE_DMCLOCK
    #define SIZEOF_DMCLOCK 4
  #else
    #define SIZEOF_DMCLOCK 0
  #endif
#else
  #define SIZEOF_TICKS 0
  #define SIZEOF_DMCLOCK 0
#endif

#if THINKOS_ENABLE_DEBUG_STEP
  #define SIZEOF_XCPT_IPSR 2
  #define SIZEOF_STEP_ID  1
  #define SIZEOF_BREAK_ID 1
  #define SIZEOF_STEP_REQ 4
  #define SIZEOF_STEP_SVC 4
#else
  #define SIZEOF_XCPT_IPSR 0
  #define SIZEOF_STEP_ID  0
  #define SIZEOF_BREAK_ID 0
  #define SIZEOF_STEP_REQ 0
  #define SIZEOF_STEP_SVC 0
#endif

#if 0
#define THINKOS_VOID_TCB_OFFS   (0 + SIZEOF_TCB_LST)
#define THINKOS_SCHED_LM_OFFS   (THINKOS_VOID_TCB_OFFS + SIZEOF_VOID_TCB)

#define THINKOS_TICKS_OFFS      (THINKOS_SCHED_LM_OFFS + SIZEOF_SCHED_LM)
#define THINKOS_DMCLOCK_OFFS    (THINKOS_TICKS_OFFS + SIZEOF_TICKS)
#endif

#define THINKOS_IDLE_CTX_OFFS   (4 * THINKOS_THREADS_MAX)
#define THINKOS_VOID_CTX_OFFS   (THINKOS_IDLE_CTX_OFFS + 4)

#define THINKOS_CYCCNT_OFFS     (THINKOS_VOID_CTX_OFFS + SIZEOF_VOID_CTX)

#define THINKOS_CRITCNT_OFFS    (THINKOS_CYCCNT_OFFS + SIZEOF_CYCCNT)

#define THINKOS_XCPT_IPSR_OFFS  (THINKOS_CRITCNT_OFFS + SIZEOF_CRITCNT)
#define THINKOS_STEP_ID_OFFS    (THINKOS_XCPT_IPSR_OFFS + SIZEOF_XCPT_IPSR)
#define THINKOS_BREAK_ID_OFFS   (THINKOS_STEP_ID_OFFS + SIZEOF_STEP_ID)
#define THINKOS_STEP_SVC_OFFS   (THINKOS_BREAK_ID_OFFS + SIZEOF_BREAK_ID)
#define THINKOS_STEP_REQ_OFFS   (THINKOS_STEP_SVC_OFFS + SIZEOF_STEP_SVC)

#define THINKOS_CYCREF_OFFS     (THINKOS_STEP_REQ_OFFS + SIZEOF_STEP_REQ)
#define THINKOS_ACTIVE_OFFS     (THINKOS_CYCREF_OFFS + SIZEOF_CYCREF)
#define THINKOS_READY_OFFS      (THINKOS_ACTIVE_OFFS + 4)

/* -------------------------------------------------------------------------- 
 * Scheduler
 * --------------------------------------------------------------------------*/

	.syntax unified
	.cpu cortex-m3

	.align  0
	.thumb
	.section .init

	.thumb_func
_thinkos_sched:
	.global _thinkos_sched
	.type   _thinkos_sched, %function

	mrs     r0, PSP
	stmdb   r0!, {r4-r11}
#if THINKOS_ENABLE_FPU 
	vstmdb.64 r0!, {d0-d15}
#endif
	ldr.n     r7, .L_thinkos_rt
	/* r2 = active, r3 = wq_ready */
	/* get the active (current) thread */	
	/* get a thread from the ready bitmap */
	ldrd    r2, r3, [r7, #THINKOS_ACTIVE_OFFS]
	rbit    r3, r3
 	clz     r3, r3
	str     r3, [r7, #THINKOS_ACTIVE_OFFS]
	/* r2 - old active thread */
	/* r3 - new active thread */
	str.w	r0, [r7, r2, lsl #3]
#if THINKOS_ENABLE_PROFILING
	/* DWT.SYCCNT */
	ldr.n   r4, .L_cm_dwt
	/* r5: cyccnt = CM3_DWT->cyccnt */
	ldr     r5, [r4, #4]
	/* r4: cycref */
	ldr.w   r4, [r7, #THINKOS_CYCREF_OFFS]
    adds    r2, #SIZEOF_CTX 
	/* r6: delta =  cyccnt - cycref */	
	subs    r6, r5, r4
    /* update thread's cycle counter */
	ldr.w	r0, [r7, r2, lsl #3]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	add     r0, r6
	str.w   r0, [r7, r2, lsl #3]
	/* update the reference */
	/* thinkos_rt.cycref = cyccnt */
	str.w   r4, [r7, #THINKOS_CYCREF_OFFS]
#endif
	ldr.w	r0, [r7, r3, lsl #3]

#if THINKOS_ENABLE_DEBUG_STEP
	movs	r4, #1
	lsls	r4, r2
//	ldr.w	r5, [r7, #THINKOS_STEP_REQ_OFFS]
	tst     r4, r5
	beq.n   3f
//	ldr.w	r2, [r7, #THINKOS_STEP_SVC_OFFS]	
	tst     r4, r2
	beq.n   2f

//	ldr     r0, [r7, #THINKOS_IDLE_CTX_OFFS]
	ldr     r2, .L_idle_task
	movs    r4, #THINKOS_THREAD_IDLE
//	str     r4, [r7, #THINKOS_ACTIVE_OFFS]
	str     r2, [r0, #56]

1:
//	strb.w	r1, [r7, #THINKOS_STEP_ID_OFFS]
	movw	r2, #(1 << 5)
	msr	    BASEPRI, r2
	add.w	r2, r0, #32
	msr	    PSP, r2
	ldmia.w	r0, {r4, r5, r6, r7, r8, r9, sl, fp}
	movw	r7, #0xedf0
	movt	r7, #0xe000
	ldr     r2, [r7, #12]
	orr.w	r2, r2, #0x40000
	str     r2, [r7, #12]
	bx      lr
2:
	/* get the PC value */
	ldr     r5, [r0, #56]
	/* get the next instruction */
	ldrh    r5, [r5, #0]
	/* if the thread is running, and it is about to invoke 
	   a system call then we don't step but set the service 
	   flag for stepping on service exit. */
	and.w   r5, r5, #0xdf00
	cmp.w   r5, #0xdf00
	bne.n   1b
    /* the thread is stepping into a system call */
	orrs	r4, r2
//	str.w	r4, [r7, #THINKOS_STEP_SVC_OFFS]
#endif

3:
#if THINKOS_ENABLE_FPU 
	add     r3, r0, #40 * 4
	msr     PSP, r3
	vldmia.64 r0!, {d0-d15}
#else
	add     r3, r0, #8 * 4
	msr     PSP, r3
#endif
	ldmia   r0, {r4-r11}
	bx      lr

.L_thinkos_rt:
	.word	thinkos_rt
.L_cm_dwt:
	.word   0xe0001000
.L_idle_task:
	.word	thinkos_idle_task

	.size   _thinkos_sched, . - _thinkos_sched

#if 0


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

#if THINKOS_ENABLE_DEBUG_STEP
	if ((1 << new_thread_id) & thinkos_rt.step_req) {
		/* process a step request */
		if ((1 << new_thread_id) & thinkos_rt.step_svc) {
			/* step the IDLE thread instead  */
			thinkos_rt.active = THINKOS_THREAD_IDLE;
			/* set the new context to the idle context */
			new_ctx = thinkos_rt.idle_ctx;
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
		dmon_signal(DMON_ALARM);
	}
#endif

#if THINKOS_ENABLE_TIMESHARE
	thinkos_timeshare(); 
#endif /* THINKOS_ENABLE_TIMESHARE */

}
#endif /* THINKOS_ENABLE_CLOCK */

#endif
