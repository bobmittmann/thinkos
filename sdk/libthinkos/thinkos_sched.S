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
#include <arch/cortex-m3.h>

#define DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */

/* --------------------------------------------------------------------------
 * ThinkOS - Real Time Scheduler 
 * --------------------------------------------------------------------------*/

	.syntax unified
	.cpu cortex-m3

	.thumb
	.text
	.align	4
	.global cm3_pendsv_isr
	.thumb
	.thumb_func
	.type   cm3_pendsv_isr, %function
cm3_pendsv_isr:

	mrs     r0, PSP
	stmdb   r0!, {r4-r11}
#if THINKOS_ENABLE_FPU 
	vstmdb.64 r0!, {d0-d15}
#endif
	.size   cm3_pendsv_isr, . - cm3_pendsv_isr

#if THINKOS_ASM_SCHEDULER
	.thumb_func
thinkos_scheduler:
	.global thinkos_scheduler
	.type   thinkos_scheduler, %function
	/* r7: thinkos_rt */
	ldr.n     r7, .L_thinkos_rt
	/* get the active (current) thread and the
	 ready wait queue in a single reading. 
	 XXX: this double reading assumes the order of
	 the active and wq_ready fields. */	
	/* r2: active, 
	   r3: wq_ready */
	ldrd    r2, r3, [r7, #THINKOS_RT_ACTIVE_OFFS]
	/* store the old context pointer into the context vector */
	/* r0: thread stack
	   r2: old active thread */
	str.w	r0, [r7, r2, lsl #2]
	/* get a thread from the ready bitmap by counting the
	 leading zeros on the bitmap */
	rbit    r3, r3
 	clz     r3, r3
	/* r3: new active thread */
	str     r3, [r7, #THINKOS_RT_ACTIVE_OFFS]
#if THINKOS_ENABLE_PROFILING
	/* DWT.SYCCNT */
	ldr.n   r1, .L_cm_dwt
	/* r1: cyccnt = CM3_DWT->cyccnt */
	ldr     r1, [r1, #4]
#if THINKOS_ENABLE_DEBUG_STEP
	/* r4: step_req */
	/* r5: cycref */
	ldrd	r4, r5, [r7, #THINKOS_RT_STEP_REQ_OFFS]
#else
	/* r5: cycref */
	ldr     r5, [r7, #THINKOS_RT_CYCREF_OFFS]
#endif
	/* update the reference */
	/* thinkos_rt.cycref = cyccnt */
	str     r1, [r7, #THINKOS_RT_CYCREF_OFFS]
	/* r1: delta =  cyccnt - cycref */	
	subs    r1, r1, r5
    /* update thread's cycle counter */
    adds    r2, #(SIZEOF_CTX / 4)
	ldr     r5, [r7, r2, lsl #2]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	adds    r5, r1
	str     r5, [r7, r2, lsl #2]
#else
#if THINKOS_ENABLE_DEBUG_STEP
	/* r4: step_req */
	ldr.w	r4, [r7, #THINKOS_RT_STEP_REQ_OFFS]
#endif
#endif

	ldr.w	r0, [r7, r3, lsl #2]

#if THINKOS_ENABLE_DEBUG_STEP
	movs	r1, #1
	lsls	r1, r3
	tst     r1, r4
	bne.n   thinkos_sched_step
#endif

.L_context_restore:
	/* Restore the context and return */
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

	.align  0
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   thinkos_scheduler, . - thinkos_scheduler
#endif /* THINKOS_ASM_SCHEDULER */


#if THINKOS_ENABLE_DEBUG_STEP
	.thumb_func
thinkos_sched_step:
	.global thinkos_sched_step
	.type   thinkos_sched_step , %function

	/* r0: thread context pointer */
	/* r1: (1 << new_thread_id) */
	/* r3: new thread id */
	/* r7: thinkos_rt */
	ldr.w	r2, [r7, #THINKOS_RT_STEP_SVC_OFFS]	
	/* r2: step_svc */
	tst     r1, r2
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
	   at a SVC call. The  L_restore_and_step code disable interrupts,
	   but the idle thread next instruction could potentially be
	   SVC, which generate a soft IRQ. The fact that the interrupts
	   are disabled causes a hard fault to the system.
	   We force the ide thread to start from the beginning where
	   at least one NOP instruction will be executed before
	   a SVC call.
	 */

	/* set the new context to the idle context */
	/* Get the idle contex t*/
	ldr     r0, [r7, #THINKOS_RT_IDLE_CTX_OFFS]
	ldr     r2, .L_idle_task
	movs    r4, #THINKOS_THREAD_IDLE
	str     r4, [r7, #THINKOS_RT_ACTIVE_OFFS]
	/* set the PC */
	str     r2, [r0, #56]
	/* step the IDLE thread instead  */
	b       .L_restore_and_step

.L_normal_step:
	/* get the PC value */
	ldr     r5, [r0, #56]
	/* load the next instruction */
	ldrb    r5, [r5, #1]
	/* if the thread is running, and it is about to invoke 
	   a system call then we don't step but set the service 
	   flag for stepping on service exit. */
	and     r5, r5, #0xdf
	cmp     r5, #0xdf
	bne.n   .L_restore_and_step
    /* the thread is stepping into a system call */
	orrs	r2, r1
	str.w	r2, [r7, #THINKOS_RT_STEP_SVC_OFFS]
	b       .L_context_restore

.L_restore_and_step:
	/* Return and step */
	strb    r3, [r7, #THINKOS_RT_STEP_ID_OFFS]
	mov     r2, #(1 << 5)
	msr	    BASEPRI, r2
#if THINKOS_ENABLE_FPU 
	add     r3, r0, #40 * 4
	msr     PSP, r3
	vldmia.64 r0!, {d0-d15}
#else
	add     r3, r0, #8 * 4
	msr     PSP, r3
#endif
	ldmia   r0, {r4-r11}
	ldr.n   r3, .L_cm_dcb
	ldr     r1, [r3, #DEMCR_OFFS]
	orr     r1, r1, #0x40000
	str     r1, [r3, #DEMCR_OFFS]
	bx      lr

	.align  2
.L_idle_task:
	.word	thinkos_idle_task
.L_cm_dcb:
	.word   CM3_DCB_BASE /* Core Debug Base Address */


	.size   thinkos_sched_step, . - thinkos_sched_step

/* FIXME: this is a hack to force linking this file. 
 The linker then will override the weak alias for the cm3_hard_fault_isr() */
	.align 2
	.global	thinkos_sch_nm
	.section .rodata
	.type   thinkos_sch_nm, %object
	.size   thinkos_sch_nm, 4
thinkos_sch_nm:
	.ascii	"SCH\000"

#endif

