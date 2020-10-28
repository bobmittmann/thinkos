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
	lsr      r0, lr, #2
	and      r0, #7
	tbb      [pc, r0]
.L_entry_tab:
	.byte    ((.L_entry_err - .L_entry_tab)/2) /* 0 */
	.byte    ((.L_entry_err - .L_entry_tab)/2) /* 1 */
	.byte    ((.L_entry_err - .L_entry_tab)/2) /* 2 */
	.byte    ((.L_entry_fpu - .L_entry_tab)/2) /* 3 */
	.byte    ((.L_entry_err - .L_entry_tab)/2) /* 4 */
	.byte    ((.L_entry_err - .L_entry_tab)/2) /* 5 */
	.byte    ((.L_entry_krn - .L_entry_tab)/2) /* 6 */
	.byte    ((.L_entry_usr - .L_entry_tab)/2) /* 7 */
	.align	 1
/*
=========== ============== ============= ============ ===== ===== =====
 EXC_RETURN    Return to   Return stack   Frame type  Priv   Ctrl  Op
=========== ============== ============= ============ ===== ===== =====
 0xFFFFFFE1  Handler mode          Main     Extended  Err    xxx   0 28
 0xFFFFFFE9   Thread mode          Main     Extended  Err    xxx   2 20  
 0xFFFFFFED   Thread mode       Process     Extended  Ufp    101   3 16
 0xFFFFFFF1  Handler mode          Main        Basic  Err    xxx   4 12
 0xFFFFFFF9   Thread mode          Main        Basic  Krn    000   6  4
 0xFFFFFFFD   Thread mode       Process        Basic  Usr    001   7  0
=========== ============== ============= ============ ===== ===== =====
*/
	.align	 1
.L_entry_krn:
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* check the stack limit */
	cmp      sp, r2
	ble      .L_stack_err 
	/* get the thread id */
	and      r2, #0x3f
	/* check the thread identity */
	cmp      r2, #THINKOS_THREAD_IDLE
	bne      .L_krn_entry_err 
#endif
	mov      lr, #CM3_EXC_RET_THREAD_MSP
	stmdb    sp!, {r4-r12, lr}
	mov      r0, sp
	b        thinkos_sched_update_cyccnt

	.align	 1
.L_entry_fpu:
	mrs      r0, PSP
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* check the stack limit */
	cmp      r0, r2
	ble      .L_stack_err 
	/* get the thread id */
	and      r2, r2, #0x3f
#endif
	mov      lr, #CM3_EXC_RET_THREAD_PSP_EXT
	stmdb    r0!, {r4-r12, lr}
	mov      r7, r0
	vstmdb   r7!, {s16-s31}
	orr      r0, #(CONTROL_nPRIV | CONTROL_FPCA)
	b        thinkos_sched_update_cyccnt

	.align	 1
.L_entry_usr:
	mrs      r0, PSP
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* check the stack limit */
	cmp      r0, r2
	ble      .L_stack_err 
	/* get the thread id */
	and      r2, r2, #0x3f
#endif
	mov      lr, #CM3_EXC_RET_THREAD_PSP
	stmdb    r0!, {r4-r12, lr}
	orr      r0, #(CONTROL_nPRIV)

thinkos_sched_update_cyccnt:
#if (THINKOS_ENABLE_PROFILING)
	/* DWT.SYCCNT */
	ldr.n   r6, .L_cm_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n   r6, [r6, #4]
	/* update the reference */
  #if (THINKOS_ENABLE_DEBUG_STEP)
	/* r4: step_req */
	/* r5: cycref */
	ldrd	r4, r5, [r3, #THINKOS_RT_STEP_REQ_OFFS]
  #else
	/* r5: cycref */
	ldr     r5, [r3, #THINKOS_RT_CYCREF_OFFS]
  #endif
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

thinkos_sched_swap:
	/* r0: thread stack
	   r1: next active thread 
	   r3: thinkos_rt */
	/* store the old context pointer into the context vector */
	str.w	r0, [r3, r2, lsl #2]

thinkos_krn_sched_ctx_get:

#if !(THINKOS_ENABLE_STACK_LIMIT)
	/* load the new context pointer from the context vector */
	ldr.w	r0, [r3, r1, lsl #2]
#endif

	.thumb_func
thinkos_krn_sched_ctx_restore:
	.global thinkos_krn_sched_ctx_restore
	.type   thinkos_krn_sched_ctx_restore, %function

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* load the new thread id and stack limit */
	adds    r7, r3, #THINKOS_RT_TH_SL_OFFS
	ldr.w	r1, [r7, r1, lsl #2]
#endif

	/* r0: new thread stack
	   r1: new active thread id and stack limit
	   r2: old active thread id 
	   r3: thinkos_rt */
	str      r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* get the effective thread id */
	ands     r1, r1, #0x3f
	/* load the new context pointer from the context vector */
	ldr      r0, [r3, r1, lsl #2]
#endif
	
	/* get the control (3bits) from the context */
	ands     r7, r0, #0x07
	/* restore the control register */
	msr      CONTROL, r7
	isb
	bics     r0, r0, #0x07

	tbb      [pc, r7]
.L_exit_tab:
	.byte    ((.L_exit_krn - .L_exit_tab)/2) /* 0 */
	.byte    ((.L_exit_usr - .L_exit_tab)/2) /* 1 */
	.byte    ((.L_exit_err2 - .L_exit_tab)/2) /* 2 */
	.byte    ((.L_exit_usr - .L_exit_tab)/2) /* 3 */
	.byte    ((.L_exit_err4 - .L_exit_tab)/2) /* 4 */
	.byte    ((.L_exit_ufp - .L_exit_tab)/2) /* 5 */
	.byte    ((.L_exit_err6 - .L_exit_tab)/2) /* 6 */
	.byte    ((.L_exit_err7 - .L_exit_tab)/2) /* 7 */


	.align	 1
.L_exit_usr:
	/* restore core context */
	ldmia    r0!, {r4-r12, lr}
	/* restore stack pointer */
	msr      PSP, r0
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	sub      r0, r0, #((CTX_R0) * 4)
	mov      r3, sp
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

	.align	 1
.L_exit_ufp:
	/* restore FP context */
	mov      r7, r0
	vldmdb   r7!, {s16-s31}
	/* restore core context */
	ldmia    r0!, {r4-r12, lr}
	/* restore stack pointer */
	msr      PSP, r0
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP_EXT
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	sub      r0, r0, #((CTX_R0) * 4)
	mov      r3, sp
	b        thinkos_sched_dbg
#else

	bx       lr
#endif

.L_exit_krn:
//	cmp      sp, r0
//	bne      .L_idle_stack_fault
	mov      sp, r0
	/* restore core context */
	ldmia    sp!, {r4-r12, lr}
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_MSP
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	mov      r3, sp
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

.L_krn_entry_err:
	mov      r12, #32
	b        .L_krn_entry_err 

.L_entry_err:
	mov      r12, #1
	b        .L_entry_err

.L_stack_err:
	mov      r12, #20
	b        .L_stack_err

.L_idle_stack_fault:
	mov      r12, #0
	b        .L_idle_stack_fault

.L_exit_err1:
	mov      r12, #1
	b        .L_exit_err1

.L_exit_err2:
	mov      r12, #2
	b        .L_exit_err2

.L_exit_err4:
	mov      r12, #3
	b        .L_exit_err4

.L_exit_err6:
	mov      r12, #4
	b        .L_exit_err6

.L_exit_err7:
	mov      r12, #5
	b        .L_exit_err7

.L_stack_align_error:
	mov      r12, #6
	b        .L_stack_align_error

.L_stack_fpca_error:
	mov      r12, #7
	b        .L_stack_fpca_error

.L_control_error:
	mov      r12, #8
	b        .L_control_error

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


