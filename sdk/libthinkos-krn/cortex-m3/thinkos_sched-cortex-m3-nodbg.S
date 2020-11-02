/* 
 * thinkos_sched_cortex-m3-nodbg.S
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

#ifdef THINKOS_ENABLE_SCHED_DEBUG
#endif

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

	/* */
	cmp       r2, THINKOS_THREAD_IDLE
#if (THINKOS_ENABLE_SCHED_DEBUG)
	/* There is no need to update the IDLE context pointer except for 
	   debugging */
	ittt      eq
	pusheq    {r4-r11}
	moveq     r0, sp
	streq     r0, [r3, r2, lsl #2]
#else
	it        eq
	pusheq    {r4-r11}
#endif
	bge       thinkos_sched_select_ready

	mrs       r0, PSP
	stmdb     r0!, {r4-r11}

#if (THINKOS_ENABLE_PROFILING)
thinkos_sched_update_cyccnt:
	/* DWT.SYCCNT */
	ldr.n   r6, .L_cm_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n   r6, [r6, #4]
	/* update the reference */
	/* r5: cycref */
	ldr     r5, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* thinkos_rt.cycref = cyccnt */
	str     r6, [r3, #THINKOS_RT_CYCREF_OFFS]
	/* r6: delta =  cyccnt - cycref */	
	subs    r6, r6, r5
	/* update thread's cycle counter */
	add     r7, r3, #THINKOS_RT_CYCCNT_OFFS
	ldr     r5, [r7, r2, lsl #2]
	/* thinkos_rt.cyccnt[old_thread_id] += delta */
	adds    r5, r6
	str     r5, [r7, r2, lsl #2]
#endif
#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
	/* All threads are privileged. No need to update the control 
	   register. */
#else
	mrs       r4, CONTROL
	orrs      r0, r4
	orr       r0, #CONTROL_SPSEL | CONTROL_nPRIV
#endif

	/* r0: thread stack
	   r1: next active thread 
	   r3: thinkos_rt */
	/* store the old context pointer into the context vector */
	str.w	  r0, [r3, r2, lsl #2]

thinkos_sched_select_ready:
	/* r1 - ready queue
	   r2 - old thread */
	/* if no active go to idle */
	cbz       r1, .L_exit_idle
	/* get a thread from the ready bitmap by counting the
	leading zeros on the bitmap */
	rbit      r1, r1
 	clz       r1, r1

	/* load the new context pointer from the context vector */
	ldr.w	  r0, [r3, r1, lsl #2]

	.thumb_func
thinkos_krn_sched_ctx_restore:
	.global thinkos_krn_sched_ctx_restore
	.type   thinkos_krn_sched_ctx_restore, %function

.L_save_active_and_restore:
	/* r0: new thread stack
	   r1: new active thread id
	   r2: old active thread id 
	   r3: thinkos_rt */
	str       r1, [r3, #THINKOS_RT_ACTIVE_OFFS]

#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
#else
	/* get the control (2 or 3 bits) from the context */
	ands      r7, r0, #CONTROL_MSK
	/* restore the control register */
	msr       CONTROL, r7
	isb
	bic       r0, r0, #CONTROL_MSK
#endif

.L_exit_usr:
	/* restore core context */
	ldmia     r0!, {r4-r11}
	/* restore stack pointer */
	msr       PSP, r0
	/* Synthesizes exception return */
	mov       lr, #CM3_EXC_RET_THREAD_PSP
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	sub       r0, r0, #((CTX_R0) * 4)
	mov       r3, sp
	b         thinkos_sched_dbg
#else
	bx        lr
#endif


.L_exit_idle:
#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
#else
	/* Clear the control register. IDLE thread uses MSP 
	 and is privileged allways. */
	msr       CONTROL, r1
	isb
#endif	
	mov       r1, #THINKOS_THREAD_IDLE
	str       r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#if (THINKOS_ENABLE_SCHED_DEBUG)
	/* load the new context pointer from the context vector */
	ldr.w	  r0, [r3, r1, lsl #2]
	cmp       sp, r0 
	bne       .L_idle_err
#endif
	/* restore core context */
	pop       {r4-r11}
	/* Synthesizes exception return */
	mov       lr, #CM3_EXC_RET_THREAD_MSP
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	mov       r3, sp
	b         thinkos_sched_dbg
#else
	bx        lr
#endif

.L_idle_err:
	bkpt      1
	b         .L_idle_err

	.align  0
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

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


