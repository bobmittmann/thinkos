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
 * ThinkOS - excption handler stubs
 * --------------------------------------------------------------------------*/
#if THINKOS_ENABLE_EXCEPTIONS
	.syntax unified
	.cpu cortex-m3

	.thumb
	.text
	.align	2
	.global	collect_xcpt_inf
	.thumb
	.thumb_func
	.type	collect_xcpt_inf, %function
	/* r0: return (lr) */
	/* r1: excpt number */
collect_xcpt_inf:
	ldr    r3, .L_xcpt_buf
	stmia  r3, {r4-r11}
	add    r2, r3, #32
	tst    r0, #4 
	ite    eq 
	mrseq  r4, MSP 
	mrsne  r4, PSP 
	ldmia  r4, {r5-r12}
	stmia  r2, {r5-r12}

	mrs    r5, MSP 
	sub    r5, r5, #32
	mrs    r6, PSP 
	tst    r0, #4 
	it     ne 
	subne  r6, r6, #32
	ldr    r7, .L_cm3_scb
	ldr    r7, [r7, #4]
	str    r0, [r3, #64]
	str    r5, [r3, #68]
	str    r6, [r3, #72]
	str    r7, [r3, #76]
	mrs    r5, IPSR
	ldr    r2, .L_thinkos_rt
	ldr    r2, [r2, #THINKOS_ACTIVE_OFFS]
	strb   r5, [r3, #80]
	strb   r2, [r3, #81]
	strb   r1, [r3, #82]

	mov    r7, r0
	mov    r6, r3
	bx     lr
	.align  2
.L_xcpt_buf:
	.word	thinkos_excpt_buf
.L_thinkos_rt:
	.word	thinkos_rt

	.size   collect_xcpt_inf, . - collect_xcpt_inf

	.thumb
	.text
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_bus_fault
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_bus_fault_isr, . - cm3_bus_fault_isr

	.align	2
	.global	cm3_usage_fault_isr
	.thumb
	.thumb_func
	.type	cm3_usage_fault_isr, %function
cm3_usage_fault_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_usage_fault
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_usage_fault_isr, . - cm3_usage_fault_isr

	.align	2
	.global	cm3_mem_manage_isr
	.thumb
	.thumb_func
	.type	cm3_mem_manage_isr, %function
cm3_mem_manage_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_mem_manage
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr


	.align	2
	.global	cm3_hard_fault_isr
	.thumb
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_hard_fault
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_hard_fault_isr, . - cm3_hard_fault_isr

#endif /* THINKOS_ENABLE_EXCEPTIONS */

void __attribute__((naked)) cm3_debug_mon_isr(void)
{
	register struct cm3_except_context * ctx asm("r0");
	.align	2
	.global	cm3_debug_mon_isr
	.thumb
	.thumb_func
	.type	cm3_debug_mon_isr, %function
cm3_debug_mon_isr:
	/* select the context stack according to the content of LR */
	tst lr, #4
	ite eq
	mrseq r0, MSP
	mrsne r0, PSP
	b thinkos_dbgmon_isr
	.size  cm3_debug_mon_isr, . - cm3_debug_mon_isr

