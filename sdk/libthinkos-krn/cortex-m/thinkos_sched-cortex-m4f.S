/* 
 * thinkos_sched-cortex-m4f.S
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
#include <arch/cortex-m3.h>

#define DCB_DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */

#define CONTROL_nPRIV      (1 << 0)
#define CONTROL_SPSEL      (1 << 1)
#define CONTROL_FPCA       (1 << 2)

#define EXC_RETURN_THREAD  (1 << 1)
#define EXC_RETURN_SPSEL   (1 << 2)
#define EXC_RETURN_nFPCA   (1 << 4)

/* --------------------------------------------------------------------------
 * ThinkOS - Real Time Scheduler 
 * --------------------------------------------------------------------------*/

	.syntax unified
	.cpu cortex-m3

	.text

	.align	3
	.thumb

	.thumb_func
cm3_pendsv_isr:
	.global cm3_pendsv_isr
	.type   cm3_pendsv_isr, %function

	/* r3: thinkos_rt */
	ldr.n    r3, .L_thinkos_rt
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* get the active (current) thread and the ready bitmap */
	ldrd     r12, r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
	and      r2, r12, #(STACK_ALIGN_MSK)
#else
	ldrd     r2, r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#endif

	cmp      r2, #(THINKOS_THREAD_IDLE)

	/* skip invalid ... */
	bgt      thinkos_sched_select_ready
.L_entry_idle:
	ittt     eq
	pusheq   {r4-r11}
    moveq    r0, sp
	beq      thinkos_sched_update_cyccnt

	/* r1: ready, */
	/* r2: active, (thread status) */
	/* r12: stack limit */
	mrs      r0, PSP
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* check the stack limit */
	cmp      r0, r12
	ble      .L_stack_err 
#endif
	stmdb    r0!, {r4-r11}

#if (THINKOS_ENABLE_FPU)
	tst      lr, #EXC_RETURN_nFPCA
	it       eq
	vstmiaeq r0, {s16-s31}
#endif

#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
	mrs      r7, CONTROL
	adds     r0, r7
	adds     r0, #(CONTROL_SPSEL)
#else
	adds     r0, #(CONTROL_SPSEL | CONTROL_nPRIV)
#endif

thinkos_sched_update_cyccnt:
#if (THINKOS_ENABLE_PROFILING)
	/* DWT.SYCCNT */
	ldr.n    r6, .L_cm_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n    r6, [r6, #4]
	/* update the reference */
	/* r5: cycref */
	ldr      r5, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* thinkos_rt.cycref = cyccnt */
	str      r6, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* r6: delta =  cyccnt - cycref */	
	subs     r6, r6, r5
	/* update thread's cycle counter */
	add      r7, r3, #THINKOS_RT_CYCCNT_OFFS
	ldr      r5, [r7, r2, lsl #2]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	adds     r5, r6
	str      r5, [r7, r2, lsl #2]
#endif

thinkos_sched_swap:
	/* r0: thread stack
	   r1: next active thread 
	   r3: thinkos_rt */
	/* store the old context pointer into the context vector */
	str.w	 r0, [r3, r2, lsl #2]

thinkos_sched_select_ready:
#if (THINKOS_ENABLE_RUNMASK)
	ldr      r7, [r3, #THINKOS_RT_RUNMASK_OFFS]
	ands     r1, r7
#endif
	/* r1 - ready queue
	   r2 - old thread id */
	cbz      r1, .L_ready_queue_empty
	/* get a thread from the ready bitmap by counting the
	leading zeros on the bitmap */
	rbit     r1, r1
 	clz      r1, r1

thinkos_krn_sched_ctx_get:

#if (THINKOS_ENABLE_DEBUG_STEP)
thinkos_sched_step_req_check:
	/* r4: step_req */
	ldr      r4, [r3, #THINKOS_RT_STEP_REQ_OFFS]
	/* check if the step request bit is set for this thread */
	movs	 r5, #1
	lsls     r5, r1
	tst      r5, r4
	bne.n    thinkos_sched_step
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* load the new thread id and stack limit */
	adds     r7, r3, #THINKOS_RT_TH_SL_OFFS
	ldr.w	 r12, [r7, r1, lsl #2]
	/* get the effective thread id */
	ands     r1, r12, #(STACK_ALIGN_MSK)
#endif
	/* load the new context pointer from the context vector */
	ldr.w	 r0, [r3, r1, lsl #2]

	.size   cm3_pendsv_isr, . - cm3_pendsv_isr

	.thumb_func
thinkos_krn_sched_ctx_restore:
	.global thinkos_krn_sched_ctx_restore
	.type   thinkos_krn_sched_ctx_restore, %function

.L_save_active_and_restore:
#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
	/* get the control (3bits) from the context */
	ands     r7, r0, #CONTROL_MSK
	/* restore the control register */
	msr      CONTROL, r7
	isb
	/* restore stack pointer */
	subs     r0, r7
#endif
	/* r0: new thread stack
	   r1: new active thread id and stack limit
	   r2: old active thread id 
	   r3: thinkos_rt */
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* r12: active stack limit  */
	str      r12, [r3, #THINKOS_RT_ACTIVE_OFFS]
#else
	str      r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#endif

.L_exit_usr:
#if (THINKOS_ENABLE_FPU)
	tst      r7, #(CONTROL_FPCA)
	itte     ne
	/* restore FP context */
	vldmiane r12, {s16-s31}
	/* Synthesizes exception return */
	movne    lr, #CM3_EXC_RET_THREAD_PSP_EXT
	moveq    lr, #CM3_EXC_RET_THREAD_PSP
#else
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP
#endif
	/* restore core context */
	ldmia    r0!, {r4-r11}
	msr      PSP, r0
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	mov      r3, sp
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

.L_exit_idle:
.L_ready_queue_empty:
#if (THINKOS_ENABLE_SCHED_ERROR)
    /* load the new context pointer from the context vector */
    ldr.w    r0, [r3, #THINKOS_RT_IDLE_CTX_OFFS]
    cmp      sp, r0
    bne      .L_idle_err
#endif
    /* set the control register */
    movs     r1, #0
    msr      CONTROL, r1
    isb

	movs     r1, #THINKOS_THREAD_IDLE
	str      r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
	/* restore core context */
	pop      {r4-r11}
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_MSP
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	mov      r3, sp
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

#if (THINKOS_ENABLE_SCHED_ERROR)
.L_idle_err: 
	b       thinkos_krn_sched_idle_err
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
.L_stack_err: 
#if (THINKOS_ENABLE_SCHED_ERROR)
	b       thinkos_krn_sched_stack_err
#endif
#endif

	.align  0
.L_thinkos_rt:
	.word	thinkos_rt
#if (THINKOS_ENABLE_PROFILING)
.L_cm_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */
#endif
	.size   thinkos_krn_sched_ctx_restore, . - thinkos_krn_sched_ctx_restore


#if (THINKOS_ENABLE_DEBUG_STEP)
	.thumb_func
thinkos_sched_step:
	.global thinkos_sched_step
	.type   thinkos_sched_step , %function

	/* r0: thread context pointer */
	/* r1: new thread id */
	/* r2: old thread id */
	/* r3: thinkos_rt */
	/* r4: (1 << new_thread_id) */

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* load the new thread id and stack limit */
	adds     r7, r3, #THINKOS_RT_TH_SL_OFFS
	ldr.w	 r12, [r7, r1, lsl #2]
	/* get the effective thread id */
	ands     r1, r12, #(STACK_ALIGN_MSK)
#endif
	/* load the new context pointer from the context vector */
	ldr.w	 r0, [r3, r1, lsl #2]

	ldr      r6, [r3, #THINKOS_RT_STEP_SVC_OFFS]	
	/* r6: step_svc */
	tst      r4, r6
	bne.n    .L_service_setp
 
.L_normal_step:
	/* get the PC value */
	bics     r7, r0, #CONTROL_MSK
	ldr      r5, [r7, #(CTX_PC * 4)]
	/* load the next instruction */
	ldrb     r5, [r5, #1]
	/* if the thread is running, and it is about to invoke 
	   a system call then we don't step but set the service 
	   flag for stepping on service exit. */
	and      r5, r5, #0xdf
	cmp      r5, #0xdf
	itt      eq
	/* the thread is stepping into a system call */
	orreq    r6, r4
	streq    r6, [r3, #THINKOS_RT_STEP_SVC_OFFS]
	beq      .L_save_active_and_restore

	/* Set the thread step  */
	strb     r1, [r3, #THINKOS_RT_STEP_ID_OFFS]

.L_restore_and_step:
	ldr.n    r6, .L_cm_dcb
	/* Disable all exceptions. They wil be automatically restored
	 when returning from this handler. */
	cpsid    f
	/* Request Debug/Monitor step */
	ldr      r5, [r6, #DCB_DEMCR_OFFS]
	orr      r5, r5, #DCB_DEMCR_MON_STEP
	str      r5, [r6, #DCB_DEMCR_OFFS]
	/* Mask low priority interrupts except debug monitor */
	mov      r5, #(1 << 5)
	msr      BASEPRI, r5

	b       .L_save_active_and_restore

.L_service_setp:
	/* this thread got a step request when calling a service.
	   We allowed for the system call to go through as if it
	   was a single instruction. Which make sense from the point of
	   view of the thread. Now the thread is returning from the
	   service call. We need to stop the system and rise a 
	   step break event.
	   But we don't want to step the real thread, we choose to step
	   the idle thread instead. */

	/* XXX: reset the IDLE task. There is a problem when stepping
	   at a SVC call. The ".L_restore_and_step" code disable interrupts,
	   but the idle thread next instruction could potentially be
	   SVC, which generate a soft IRQ. The fact that the interrupts
	   are disabled causes a hard fault to the system.
	   We force the idle thread to start from the beginning where
	   at least one NOP instruction will be executed before
	   a SVC call.
	   Resolved: no SVC calls from IDLE.
	 */

	/* Set the thread step  */
	strb     r1, [r3, #THINKOS_RT_STEP_ID_OFFS]
	/* step the IDLE thread instead  */
	ldr.n    r6, .L_cm_dcb
	/* Disable all exceptions. They wil be automatically restored
	 when returning from this handler. */
	cpsid    f
	/* Request Debug/Monitor step */
	ldr      r5, [r6, #DCB_DEMCR_OFFS]
	orr      r5, r5, #DCB_DEMCR_MON_STEP
	str      r5, [r6, #DCB_DEMCR_OFFS]
	/* Mask low priority interrupts except debug monitor */
	mov      r5, #(1 << 5)
	msr      BASEPRI, r5
	b        .L_exit_idle


	.align  4
.L_cm_dcb:
	.word   CM3_DCB_BASE /* Core Debug Base Address */

	.size   thinkos_sched_step, . - thinkos_sched_step
#endif /* THINKOS_ENABLE_DEBUG_STEP */

