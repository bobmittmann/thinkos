/* 
 * thinkos_sched-corte-m4f-ndbg.S
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
	.align	4
	.thumb

	.thumb_func
cm3_pendsv_isr:
	.global cm3_pendsv_isr
	.type   cm3_pendsv_isr, %function
	.size   cm3_pendsv_isr, . - cm3_pendsv_isr

	.thumb_func
thinkos_krn_scheduler:
	.global thinkos_krn_scheduler
	.type   thinkos_krn_scheduler, %function

	/* store context */

thinkos_sched_get_active:
#if 0
	mov       r2, 1
1:  cmp       r2, #0
	bne       1b
#endif

	/* r3: thinkos_rt */
	ldr.n     r3, .L_thinkos_rt
	/* get the active (current) thread and the ready bitmap */
	ldrd      r2, r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
	/* r1: ready, */
	/* r2: active, (thread status) */

#if 0
	/* invalid thread id */
	ands      r0, r2, #0x001f
	beq       r0, thinkos_krn_sched_ctx_get
#endif

thinkos_sched_select_ready:
	/* r1 - ready queue
	   r2 - old thread */
	/* get a thread from the ready bitmap by counting the
	leading zeros on the bitmap */
#if (THINKOS_THREADS_MAX < 32) 
	/* force the IDLE thread bit to one */
	orr     r1, #(1 << THINKOS_THREADS_MAX)
#endif
	rbit    r1, r1
 	clz     r1, r1

#if 0
	/* fast return */
	cmp     r1, r2
	it      eq
	bxeq    lr

#endif

	tst     lr, #EXC_RETURN_SPSEL
	itte    eq
	moveq   r0, sp
	subeq   sp, sp, #(CTX_R0 * 4) /* reserve space on MSP stack */
	mrsne   r0, PSP
	movs    r12, r0
thinkos_sched_context_save:
	stmdb   r0!, {r4-r12, lr}
	tst     lr, #EXC_RETURN_nFPCA
	itt     eq
	moveq   r7, r0
	vstmdbeq r7!, {s16-s31}

thinkos_sched_update_cyccnt:
	/* DWT.SYCCNT */
	ldr.n   r6, .L_cm_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n   r6, [r6, #4]
	/* update the reference */
	/* thinkos_rt.cycref = cyccnt */
	str     r6, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* r5: cycref */
	ldr     r5, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* r6: delta =  cyccnt - cycref */	
	subs    r6, r6, r5
	/* update thread's cycle counter */
	adds    r7, r2, #(SIZEOF_CTX / 4)
	ldr     r5, [r3, r7, lsl #2]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	adds    r5, r6
	str     r5, [r3, r7, lsl #2]


	mrs     r7, CONTROL
	/* Alternatively get the control (3bits) from the context */
	orrs    r0, r7
thinkos_sched_swap:
	/* r0: thread stack
	   r1: next active thread 
	   r3: thinkos_rt */
	/* store the old context pointer into the context vector */
	str.w	r0, [r3, r2, lsl #2]
#if 0
	/* store the new control register */
	adds    r6, r2, #THINKOS_RT_CONTROL_OFFS
	strb	r7, [r3, r6]
#endif

thinkos_krn_sched_ctx_get:

	/* load the new context pointer from the context vector */
	ldr.w	r0, [r3, r1, lsl #2]

	.thumb_func
thinkos_krn_sched_ctx_restore:
	.global thinkos_krn_sched_ctx_restore
	.type   thinkos_krn_sched_ctx_restore, %function

	/* r0: new thread stack
	   r1: new active thread
	   r2: old active thread
	   r3: thinkos_rt 
	   r4: new control register */
	str     r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
	
#if 0
	/* load the new control register */
	adds    r4, r1, #THINKOS_RT_CONTROL_OFFS
	ldrb	r4, [r3, r4]
#endif
	/* Alternatively get the control (3bits) from the context */
	ands    r3, r0, #0x07
	bics    r0, r0, #0x07

	tbb      [pc, r3]
.L_exit_tab:
	.byte    ((.L_exit_krn - .L_exit_tab)/2) /* 0 */
	.byte    ((.L_exit_usr - .L_exit_tab)/2) /* 1 */
	.byte    ((.L_exit_err2 - .L_exit_tab)/2) /* 2 */
	.byte    ((.L_exit_usr - .L_exit_tab)/2) /* 3 */
	.byte    ((.L_exit_err4 - .L_exit_tab)/2) /* 4 */
	.byte    ((.L_exit_err5 - .L_exit_tab)/2) /* 5 */
	.byte    ((.L_exit_err6 - .L_exit_tab)/2) /* 6 */
	.byte    ((.L_exit_ufp - .L_exit_tab)/2) /* 7 */

	.align	 1
.L_exit_ufp:
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP_EXT
	/* restore FP context */
	mov      r7, r0
	vldmdb   r7!, {s16-s31}
	/* restore core context */
	ldmia    r0!, {r4-r12, lr}
	/* restore stack pointer */
	msr      PSP, r0
	/* restore the control register */
	msr      CONTROL, r3
	isb
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	sub      r0, r0, #((CTX_R0) * 4)
	mov      r3, sp
	b        thinkos_sched_dbg
#else

	bx       lr
#endif

.L_exit_usr:
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP
	/* restore core context */
	ldmia    r0!, {r4-r12, lr}
	/* restore stack pointer */
	msr      PSP, r0
	/* restore the control register */
	msr      CONTROL, r3
	isb
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	sub      r0, r0, #((CTX_R0) * 4)
	mov      r3, sp
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

.L_exit_krn:
	cmp      sp, r0
	bne      .L_idle_stack_fault
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_MSP
	/* restore core context */
	ldmia    sp!, {r4-r12, lr}
	/* restore the control register */
	msr      CONTROL, r3
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	mov      r3, sp
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

.L_idle_stack_fault:
	mov      lr, r0
	b        .L_idle_stack_fault

.L_exit_err1:
	mov      lr, r1
	b        .L_exit_err1

.L_exit_err2:
	mov      lr, r2
	b        .L_exit_err2

.L_exit_err4:
	mov      lr, r4
	b        .L_exit_err4

.L_exit_err5:
	mov      lr, r5
	b        .L_exit_err5

.L_exit_err6:
	mov      lr, r6
	b        .L_exit_err6

	.align  0
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   thinkos_krn_sched_ctx_restore, . - thinkos_krn_sched_ctx_restore
	.size   thinkos_krn_scheduler, . - thinkos_krn_scheduler


/* FIXME: this is a hack to force linking this file. 
 The linker then will override the weak alias for the cm3_hard_fault_isr() */
	.align 4
	.global	thinkos_sch_nm
	.section .rodata
	.type   thinkos_sch_nm, %object
	.size   thinkos_sch_nm, 4
thinkos_sch_nm:
	.ascii	"SCH\000"


