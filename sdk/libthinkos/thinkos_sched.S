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
#include <arch/cortex-m3.h>

#define DCB_DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */


#define CONTROL_nPRIV      (1 << 0)
#define CONTROL_SPSEL      (1 << 1)
#define CONTROL_FPCA       (1 << 2)

#define EXC_RETURN_SPSEL   (1 << 2)
#define EXC_RETURN_nFPCA   (1 << 4)

/* --------------------------------------------------------------------------
 * ThinkOS - Real Time Scheduler 
 * --------------------------------------------------------------------------*/

	.syntax unified
	.cpu cortex-m3

#if (THINKOS_ASM_SCHEDULER)

	.text
	.align	4
	.thumb

	.thumb_func
cm3_pendsv_isr:
	.global cm3_pendsv_isr
	.type   cm3_pendsv_isr, %function
	.size   cm3_pendsv_isr, . - cm3_pendsv_isr

	.thumb_func
thinkos_scheduler:
	.global thinkos_scheduler
	.type   thinkos_scheduler, %function

	/* store context */

thinkos_sched_context_save:
#if !(THINKOS_ENABLE_FPU) && !(THINKOS_ENABLE_IDLE_MSP)
	mrs     r0, PSP
	stmdb   r0!, {r4-r11}
#elif (THINKOS_ENABLE_FPU) && !(THINKOS_ENABLE_IDLE_MSP)
	mrs     r0, PSP
	mov     r12, r0
	stmdb   r0!, {r4-r12,lr}
    tst     lr, #EXC_RETURN_nFPCA
	it      eq
	vstmdbeq  r12!, {s16-s31}
#elif !(THINKOS_ENABLE_FPU) && (THINKOS_ENABLE_IDLE_MSP)
	tst     lr, #EXC_RETURN_SPSEL
	itte    eq
	moveq   r0, sp
	subeq   sp, sp, #(CTX_R0 * 4)
	mrsne   r0, PSP
	stmdb   r0!, {r4-r12,lr}
#else
	tst     lr, #EXC_RETURN_SPSEL
	itte    eq
	moveq   r0, sp
	subeq   sp, sp, #(CTX_R0 * 4) /* reserve spce on MSP stack */
	mrsne   r0, PSP
	movs    r12, r0
	stmdb   r0!, {r4-r12,lr}
	movs    r12, r0
    tst     lr, #EXC_RETURN_nFPCA
	it      eq
	vstmdbeq  r12!, {s16-s31}
#endif

thinkos_get_active:
	/* r3: thinkos_rt */
	ldr.n     r3, .L_thinkos_rt
	/* get the active (current) thread */
	ldr       r2, [r3, #THINKOS_RT_ACTIVE_OFFS]
	/* r2: active, */

#if (THINKOS_ENABLE_PROFILING)
thinkos_sched_update_cyccnt:
	/* DWT.SYCCNT */
	ldr.n   r6, .L_cm_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n   r6, [r6, #4]
  #if (THINKOS_ENABLE_DEBUG_STEP)
	/* r4: step_req */
	/* r5: cycref */
	ldrd	r4, r5, [r3, #THINKOS_RT_STEP_REQ_OFFS]
  #else
	/* r5: cycref */
	ldr     r5, [r3, #THINKOS_RT_CYCREF_OFFS]
  #endif
	/* update the reference */
	/* thinkos_rt.cycref = cyccnt */
	str     r6, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* r6: delta =  cyccnt - cycref */	
	subs    r6, r6, r5
    /* update thread's cycle counter */
    adds    r7, r2, #(SIZEOF_CTX / 4)
	ldr     r5, [r3, r7, lsl #2]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	adds    r5, r6
	str     r5, [r3, r7, lsl #2]
#else
  #if (THINKOS_ENABLE_DEBUG_STEP)
thinkos_sched_step_req_check:
	/* r4: step_req */
	ldr.w	r4, [r3, #THINKOS_RT_STEP_REQ_OFFS]
  #endif
#endif /* THINKOS_ENABLE_PROFILING */

	/* store the old context pointer into the context vector */
	str.w	r0, [r3, r2, lsl #2]

thinkos_sched_select_ready:
	ldr     r1, [r3, #THINKOS_RT_READY_OFFS]
	/* r1 - ready queue
	   r2 - old thread */
	/* get a thread from the ready bitmap by counting the
	 leading zeros on the bitmap */
	rbit    r1, r1
 	clz     r1, r1

#if (THINKOS_THREADS_MAX < 32) 
	cmp    r1, #32
	it     eq
	moveq  r1, #THINKOS_THREADS_MAX
#endif

thinkos_sched_swap:
	/* r0: thread stack
	   r1: next active thread 
	   r3: thinkos_rt */
	ldr.w	r0, [r3, r1, lsl #2]

#if (THINKOS_ENABLE_DEBUG_STEP)
	/* check whether the step request bit is set for
	   this thread */
	movs	r5, #1
	lsls	r5, r1
	tst     r5, r4
	bne.n   thinkos_sched_step
#endif

.L_save_active:
	/* r0: thread stack
	   r1: new active thread
	   r3: thinkos_rt */
	str     r1, [r3, #THINKOS_RT_ACTIVE_OFFS]

.L_context_restore:
#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	ldmia   r0!, {r4-r12,lr}
  #if (THINKOS_ENABLE_SANITY_CHECK) 
	cmp     r0, r12
	bne    .L_sched_error
  #endif	
#else
	ldmia   r0!, {r4-r11}
#endif

#if (THINKOS_ENABLE_IDLE_MSP)
	tst     lr, #EXC_RETURN_SPSEL
	ittee   ne
	msrne   PSP, r0
	movne   r3, #CONTROL_nPRIV | CONTROL_SPSEL /* non-privileged mode */
	moveq   sp, r0
	moveq   r3, 0 /* privileged mode MSP */
	msr CONTROL, r3
	isb
#else
	msr     PSP, r0
#endif

#if (THINKOS_ENABLE_FPU)
    tst     lr, #EXC_RETURN_nFPCA
	itt     eq
	subeq     r3, r0, #((CTX_R0) * 4)
	vldmdbeq  r3!, {s16-s31}
#endif

#if (THINKOS_ENABLE_SCHED_DEBUG)
	sub     r0, r0, #((CTX_R0) * 4)
	mov     r3, sp
	b       thinkos_sched_dbg
#else
	bx      lr
#endif

thinkos_sched_on_error:
#if (THINKOS_ENABLE_SANITY_CHECK) && \
	((THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP))
.L_sched_error:
	/* Disable interrupts */
	cpsid   i
	sub     r0, r0, #((CTX_R0) * 4)
	mov     r3, sp
	bl      thinkos_sched_error
	/* Reenable interrupts */
	cpsie   i
	/* Try again */
    b       thinkos_get_active
#endif

	.align  0
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   thinkos_scheduler, . - thinkos_scheduler


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
	ldr     r6, [r3, #THINKOS_RT_STEP_SVC_OFFS]	
	/* r6: step_svc */
	tst     r4, r6
	beq.n   .L_normal_step
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
	 */

	/* set the new context to the idle context */
	/* Get the idle contex t*/
	ldr     r0, [r3, #THINKOS_RT_IDLE_CTX_OFFS]
	ldr     r6, .L_idle_task
	movs    r5, #THINKOS_THREAD_IDLE
	str     r5, [r3, #THINKOS_RT_ACTIVE_OFFS]
	/* set the PC */
	str     r6, [r0, #(CTX_PC * 4)]
	/* step the IDLE thread instead  */
	b       .L_restore_and_step

.L_normal_step:
	/* get the PC value */
	ldr     r5, [r0, #(CTX_PC * 4)]
	/* load the next instruction */
	ldrb    r5, [r5, #1]
	/* if the thread is running, and it is about to invoke 
	   a system call then we don't step but set the service 
	   flag for stepping on service exit. */
	and     r5, r5, #0xdf
	cmp     r5, #0xdf
	bne.n   .L_restore_and_step
    /* the thread is stepping into a system call */
	orrs    r6, r4
	str     r6, [r3, #THINKOS_RT_STEP_SVC_OFFS]
	b       .L_context_restore

thinkos_sched_restore_and_step:

.L_restore_and_step:
	/* Return and step */
	strb    r1, [r3, #THINKOS_RT_STEP_ID_OFFS]
	ldr.n   r6, .L_cm_dcb
	/* Disable all exceptions. They wil be automatically restored
	 when returning from this handler. */
	cpsid   f
	/* Request Debug/Monitor step */
	ldr     r5, [r6, #DCB_DEMCR_OFFS]
	orr     r5, r5, #DCB_DEMCR_MON_STEP
	str     r5, [r6, #DCB_DEMCR_OFFS]
	/* Mask low priority interrupts except debug monitor */
	mov     r2, #(1 << 5)
	msr	    BASEPRI, r2

	b       .L_context_restore

	.align  2
.L_idle_task:
	.word	thinkos_idle_task
.L_cm_dcb:
	.word   CM3_DCB_BASE /* Core Debug Base Address */

	.size   thinkos_sched_step, . - thinkos_sched_step
#endif /* THINKOS_ENABLE_DEBUG_STEP */

	.thumb_func
thinkos_sched_context_restore:
	.global thinkos_sched_context_restore
	.type   thinkos_sched_context_restore, %function

	b       .L_context_restore

	.size   thinkos_sched_context_restore, . - thinkos_sched_context_restore

/* FIXME: this is a hack to force linking this file. 
 The linker then will override the weak alias for the cm3_hard_fault_isr() */
	.align 2
	.global	thinkos_sch_nm
	.section .rodata
	.type   thinkos_sch_nm, %object
	.size   thinkos_sch_nm, 4
thinkos_sch_nm:
	.ascii	"SCH\000"


#endif /* THINKOS_ASM_SCHEDULER */

