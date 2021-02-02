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

	/* r0: thinkos_rt */
	ldr.n    r0, .L_thinkos_rt
	ldrd     r1, r2, [r0, #OFFSETOF_KRN_SCHED]
#if 0
	mov  r12, #1
1: 	cmp  r12, #0
	bne  1b
#endif
	ands     r1, r1, r1, asr #9
    bcc      .L_entry_xcept /* C flag clear: exception */
	beq      .L_entry_from_idle /* Z flag set */

	/* r2: ready, */
	/* r1: active */
	/* r12: stack limit */
	mrs      r3, PSP
	stmdb    r3!, {r4-r11}
#if (THINKOS_ENABLE_STACK_LIMIT) || (THINKOS_ENABLE_FPU)
	movs     r4, r3
  #if (THINKOS_ENABLE_FPU)
	tst      lr, #EXC_RETURN_nFPCA
	it       eq
	vstmdbeq r4!, {s16-s31}
  #endif
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* check the stack limit */
//	mov      r7, #OFFSETOF_KRN_TH_SL
//	ldr      r5, [r0, r1, lsl #2]
	add      r7, r0, r1, lsl #2
	ldr      r5, [r7, #OFFSETOF_KRN_TH_SL]
	cmp      r4, r5
	ble      .L_stack_err 
#endif

#if (THINKOS_ENABLE_PRIVILEGED_THREAD) || (THINKOS_ENABLE_FPU)
	mrs      r6, CONTROL
	adds     r3, r6
	adds     r3, #(CONTROL_SPSEL)
#else
	adds     r3, #(CONTROL_SPSEL | CONTROL_nPRIV)
#endif

thinkos_sched_update_cyccnt:
#if (THINKOS_ENABLE_PROFILING)
	/* DWT.SYCCNT */
	ldr.n    r6, .L_cm_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n    r6, [r6, #4]
	/* update the reference */
	/* r5: cycref */
	ldr      r5, [r0, #OFFSETOF_KRN_CYCREF]
	/* thinkos_rt.cycref = cyccnt */
	str      r6, [r0, #OFFSETOF_KRN_CYCREF]
	/* r6: delta =  cyccnt - cycref */	
	subs     r6, r6, r5
	add      r7, r0, r1, lsl #2
	/* update thread's cycle counter */
	ldr      r5, [r7, #OFFSETOF_KRN_TH_CYC]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	adds     r5, r6
	str      r5, [r7, #OFFSETOF_KRN_TH_CYC]
#endif

thinkos_sched_swap:
	/* r3: thread stack
	   r2: next active thread 
	   r0: thinkos_rt */
	/* store the old context pointer into the context vector */
	str      r3, [r0, r1, lsl #2]

	/* r2 - ready queue
	   r1 - old thread id */
thinkos_sched_select_ready:
#if (THINKOS_ENABLE_READY_MASK)
	ldr      r7, [r0, #OFFSETOF_KRN_RDY_MSK]
	ands     r2, r7
	beq      r2, .L_ready_queue_empty
#else
	cbz      r2, .L_ready_queue_empty
#endif
	/* get a thread from the ready bitmap by counting the
	leading zeros on the bitmap */
	rbit     r2, r2
 	clz      r2, r2

#if (THINKOS_ENABLE_DEBUG_STEP)
thinkos_sched_step_req_check:
	/* r4: step_req */
	ldr      r4, [r0, #OFFSETOF_KRN_STEP_REQ]
	/* check if the step request bit is set for this thread */
	movs	 r5, #1
	lsls     r5, r2
	tst      r5, r4
	bne.n    thinkos_sched_step
#endif
	/* Get thread number from bitmap index */
 	adds     r2, #1

thinkos_krn_sched_ctx_get:

	/* load the new context pointer from the context vector */
	ldr.w	 r3, [r0, r2, lsl #2]

	.size   cm3_pendsv_isr, . - cm3_pendsv_isr

	.thumb_func
thinkos_krn_sched_ctx_restore:
	.global thinkos_krn_sched_ctx_restore
	.type   thinkos_krn_sched_ctx_restore, %function

.L_save_active_and_restore:
	/* r3: new thread stack
	   r2: new active thread id 
	   r1: old active thread id 
	   r0: thinkos_rt */
	strb     r2, [r0, #OFFSETOF_KRN_SCHED_ACTIVE]

#if (THINKOS_ENABLE_PRIVILEGED_THREAD) || (THINKOS_ENABLE_FPU)
	ands     r0, r3, #(CONTROL_MSK)
	/* restore the control register */
	msr      CONTROL, r0
	isb
	/* restore the stack pointer */
	subs     r3, r0
	beq      .L_nullptr_err
#else
	cbz      r3, .L_nullptr_err
#endif

#if (THINKOS_ENABLE_FPU)
	movs     r4, r3
	tst      r0, #(CONTROL_FPCA)
#if 1
	itte     ne
	/* restore FP context */
	vldmdbne r4!, {s16-s31}
	/* Synthesizes exception return */
	movne    lr, #CM3_EXC_RET_THREAD_PSP_EXT
	moveq    lr, #CM3_EXC_RET_THREAD_PSP
#else
	beq      4f
#if 0
	mov  r12, #3
3: 	cmp  r12, #0
	bne  3b
#endif

	/* restore FP context */
	vldmdb   r4!, {s16-s31}
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP_EXT
	b        5f
4:
	mov      lr, #CM3_EXC_RET_THREAD_PSP
5:
#endif

#else
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP
#endif

	/* r3: new thread stack
	   r2: new active thread id 
	   r1: old active thread id 
	   r0: new thread ctrl */
	/* restore core context */
	ldmia    r3!, {r4-r11}
	msr      PSP, r3

	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	subs     r3, r3, #(8  * 4)
  #if (THINKOS_ENABLE_PRIVILEGED_THREAD) || (THINKOS_ENABLE_FPU)
	orrs     r3, r0
  #endif
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

.L_entry_xcept:
    bmi      thinkos_sched_break /* N flag set: break condition */
#if 0
	beq      .L_entry_mask_error /* Z flag set */
/* The scheduler mask was not properly set this is either an unexpected
   error condition or was forced by the kernel to reset the idle thread.
 */
#endif
	movs     r3, #~0 /* restore scheduler mask */
	strb     r3, [r0, #OFFSETOF_KRN_SCHED_MASK]
.L_entry_from_idle:
	tst      lr, #EXC_RET_SPSEL
	/* the origin was not IDLE discard context and move on */ 
	bne      thinkos_sched_select_ready
	/* Adjust the thread ID */
	movs     r1, #THINKOS_THREAD_IDLE
	/* Save context on MSP */
	push     {r4-r11}
    mov      r3, sp
	b        thinkos_sched_update_cyccnt

.L_nullptr_err:
#if (THINKOS_ENABLE_SCHED_ERROR)
	bl       thinkos_krn_sched_nullptr_err
	mov      sp, r3
#endif
	/* FALLTRHOUG 
	b       .L_return_idle
	*/

.L_return_idle:
#if (THINKOS_ENABLE_SCHED_ERROR)
	ldr.n    r0, .L_thinkos_rt
	ldrb     r1, [r0, #OFFSETOF_KRN_SCHED_ACTIVE]
#endif

.L_ready_queue_empty:
.L_exit_idle:
#if (THINKOS_ENABLE_SCHED_ERROR)
    /* load the new context pointer from the context vector */
    ldr.w    r3, [r0, #OFFSETOF_KRN_IDLE_CTX]
    cmp      sp, r3
    bne      .L_idle_stack_err
#elif (THINKOS_ENABLE_SCHED_DEBUG)
    mov      r3, sp
#endif
    /* set the control register */
    movs     r2, #0
    msr      CONTROL, r2
    isb
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_MSP

	/* set the active to 0 */
	strb     r2, [r0, #OFFSETOF_KRN_SCHED_ACTIVE]
	/* Adjust the thread ID */
	movs     r2, #THINKOS_THREAD_IDLE

	/* restore core context */
	pop      {r4-r11}
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

#if (THINKOS_ENABLE_SCHED_ERROR)
.L_idle_stack_err: 
	bl      thinkos_krn_sched_idle_err
	mov      sp, r3
	b       .L_return_idle
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
.L_stack_err: 
#if (THINKOS_ENABLE_SCHED_ERROR)
	bl      thinkos_krn_sched_stack_err
	b       .L_return_idle
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



	.thumb_func
thinkos_sched_break:
	.global thinkos_sched_break
	.type   thinkos_sched_break, %function
thinkos_sched_break:
	tst      lr, #EXC_RET_SPSEL
	/* the origin was IDLE save context and move on */ 
	itt      eq
	/* Adjust the thread ID */
	/* Save context on MSP */
	pusheq   {r4-r11}
	beq      .L_do_break

#if (THINKOS_ENABLE_MONITOR)
    /* fault thread */
	ldr      r1, .L_xcpt_buf
	adds     r1, #(OFFSETOF_XCPT_CONTEXT)
	stmia    r1!, {r4-r11} /* r4..r11 */
	mrs      r3, PSP
	ldmia    r3!, {r4-r11}
	stmia    r1!, {r4-r11} /* r4..r11 */
    ldrb     r1, [r0, #(OFFSETOF_KRN_BREAK_ID)]
#endif
.L_do_break:
	bl       thinkos_krn_sched_on_break
	b        .L_return_idle

	.align  0
.L_xcpt_buf: /* exception buffer is at the bottom of the exception stack */
	.word   thinkos_except_stack
	.size    thinkos_sched_break, . - thinkos_sched_break


#if (THINKOS_ENABLE_DEBUG_STEP)
	.thumb_func
thinkos_sched_step:
	.global thinkos_sched_step
	.type   thinkos_sched_step , %function

	/* r3: thread context pointer */
	/* r2: new thread id */
	/* r1: old thread id */
	/* r0: thinkos_rt */
	/* r4: (1 << new_thread_id) */

	/* load the new context pointer from the context vector */
	ldr.w	 r3, [r0, r2, lsl #2]

	ldr      r6, [r0, #OFFSETOF_KRN_STEP_SVC]	
	/* r6: step_svc */
	tst      r4, r6
	bne.n    .L_service_setp
 
.L_normal_step:
	/* get the PC value */
	bics     r7, r3, #CONTROL_MSK
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
	streq    r6, [r0, #OFFSETOF_KRN_STEP_SVC]
	beq      .L_save_active_and_restore

	/* Set the thread step  */
	strb     r2, [r0, #OFFSETOF_KRN_STEP_ID]

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
	strb     r2, [r0, #OFFSETOF_KRN_STEP_ID]
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


	.align  2
.L_cm_dcb:
	.word   CM3_DCB_BASE /* Core Debug Base Address */

	.size   thinkos_sched_step, . - thinkos_sched_step
#endif /* THINKOS_ENABLE_DEBUG_STEP */


