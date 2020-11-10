/* 
 * thinkos_core.c
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
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

#define SCB_ICSR_OFFS  0x04
#define SCB_SHCSR_OFFS 0x24 /* System Handler Control and State */
#define SCB_CFSR_OFFS  0x28 /* Configurable Fault Status */
#define SCB_HFSR_OFF   0x2c /* Hard Fault Status */
#define SCB_DFSR_OFFS  0x30 /* Debug Fault Status */
#define SCB_MMFAR_OFFS 0x34 /* Mem Manage Address */
#define SCB_BFAR_OFFS  0x38 /* Bus Fault Address */
#define SCB_AFSR_OFFS  0x3c /* Auxiliary Fault Status */

#define DCB_DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */

#define SIZEOF_ARMV7M_BASIC_FRAME (8 *4)

	.syntax unified
	.cpu cortex-m3

	.text
	.align	3
	.thumb

/* --------------------------------------------------------------------------
 * ThinkOS - excption handler stubs
 * --------------------------------------------------------------------------*/
#if (THINKOS_ENABLE_EXCEPTIONS)

#if (THINKOS_ENABLE_HARDFAULT)
	.global	cm3_hard_fault_isr
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	movs   r0, #THINKOS_ERR_HARD_FAULT
	/* Fall-through */
	.size  cm3_hard_fault_isr, . - cm3_hard_fault_isr
#endif

	.global	thinkos_krn_xcpt_raise
	.thumb
	.thumb_func
	.type	thinkos_krn_xcpt_raise, %function

thinkos_krn_xcpt_raise:
/* ThinkOS except processing:

	 1. Collect the special-purpose registers
	 	- Get PRIMASK special-purpose registers
		- Disable Interrupts
		- Get FAULTMASK, BASEPRI, CONTROL and IPSR

	 2. Collect the core registers (context)

	 3. Collect memory mapped system registers
	 	- ICSR, SHCSR, CFSR, HFSR, MMFAR, SCBFAR

	 4. Collect ThinkOS kernel info
	 	- CYCCNT, CYCREF
	 	- active, ready

	 5. Invalidate current thread context
	    - Set the active thread to IDLE
	    - Set the current thread pointer to NULL

	 6. Clear the ready queue 
		- optionally disable the scheduler ? TODO: check this possibility

     7. Does the exception occurred in a thread? (RETTOBASE set on ICSR)
	
	 8. <Y> Is IDLE the current thread ?

	 9. <Y><Y> Reset the IDLE thread. 
	 	- Adjust the IDLE stack
		- Reset the IDLE context pointer
	 	- Possibly adjust the main stack MSP

	10. <Y> TODO: it is safe to call the exception handler?
		 - Do we have enough space on the stack.

	10. <Y> Call the exception handler

	11. <Y> Get the IDLE context pointer
	

	12. Return 
*/

	/* Arguments:
	     r0: except number
	   Return:
	     r0: exception buffer */

	mrs     r12, PRIMASK
	/* disable interrupts */
	cpsid   i
	ldr     r3, .L_xcpt_buf
	strb    r0, [r3, #THINKOS_EXCEPT_ERRNO_OFFS] /* errno */
	ldrb    r0, [r3, #THINKOS_EXCEPT_SEQ_OFFS]  /* sequence */
	adds    r0, r0, #1  /* increment sequence */
	strb    r0, [r3, #THINKOS_EXCEPT_SEQ_OFFS] /* count */

	strb    lr, [r3, #THINKOS_EXCEPT_RET_OFFS] /* return (lr) */
	strb    r12, [r3, #THINKOS_EXCEPT_PRIMASK_OFFS] /* PRIMASK */

	mrs     r0, FAULTMASK
	strb    r0, [r3, #THINKOS_EXCEPT_FAULTMASK_OFFS] /* FAULTMASK */
	mrs     r0, BASEPRI 
	strb    r0, [r3, #THINKOS_EXCEPT_BASEPRI_OFFS] /* BASEPRI */
	mrs     r0, IPSR
	strb    r0, [r3, #THINKOS_EXCEPT_IPSR_OFFS] /* IPSR */
	mrs     r0, CONTROL
	strb    r0, [r3, #THINKOS_EXCEPT_CTRL_OFFS] /* CONTROL */

	mrs     r0, MSP 
	mrs     r1, PSP 
	tst     lr, #CM3_EXC_RET_SPSEL
	ite     eq 
	moveq   r12, r0
	movne   r12, r1
	str     r0, [r3, #THINKOS_EXCEPT_MSP_OFFS]
	str     r1, [r3, #THINKOS_EXCEPT_PSP_OFFS] 

	ldr     r2, .L_cm3_scb
	ldr     r1, [r2, #SCB_ICSR_OFFS] /* Interrupt Control State */
	str     r1, [r3, #THINKOS_EXCEPT_ICSR_OFFS]

	ldr     r0, [r2, #SCB_SHCSR_OFFS] /* System Handler Control and State */
	str     r0, [r3, #THINKOS_EXCEPT_SHCSR_OFFS]

	ldr     r0, [r2, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	str     r0, [r3, #THINKOS_EXCEPT_CFSR_OFFS]
	ldr     r0, [r2, #SCB_HFSR_OFF] /* Hard Fault Status */
	str     r0, [r3, #THINKOS_EXCEPT_HFSR_OFFS]

	ldr     r0, [r2, #SCB_MMFAR_OFFS] /* Mem Manage Address */
	str     r0, [r3, #THINKOS_EXCEPT_MMFAR_OFFS]
	ldr     r0, [r2, #SCB_BFAR_OFFS] /* Bus Fault Address */
	str     r0, [r3, #THINKOS_EXCEPT_BFAR_OFFS]

	/* removes the pending status of the PendSV exception */
	movs    r0, #SCB_ICSR_PENDSVCLR;
	str     r0, [r2, #SCB_ICSR_OFFS]

	ldr     r2, .L_thinkos_rt
	ldr     r0, [r2, #THINKOS_RT_ACTIVE_OFFS]
	str     r0, [r3, #THINKOS_EXCEPT_ACTIVE_OFFS]

	ldr     r0, [r2, #THINKOS_RT_READY_OFFS]
	str     r0, [r3, #THINKOS_EXCEPT_READY_OFFS]

	/* Prepare to return */
	movs    r0, #THINKOS_THREAD_VOID
	str     r0, [r2, #THINKOS_RT_ACTIVE_OFFS]
	/* Force clearing the ready queue */
	movs    r0, #0
	str     r0, [r2, #THINKOS_RT_READY_OFFS]
	/* Set CONTROL to 0 */
    msr     CONTROL, r0
    isb

#if (THINKOS_ENABLE_PROFILING)
	ldr     r0, [r2, #THINKOS_RT_CYCREF_OFFS]
	str     r0, [r3, #THINKOS_EXCEPT_CYCREF_OFFS] 
	ldr     r2, .L_cm3_dwt
	ldr     r0, [r2, #4] /* DWT.SYCCNT */
	str     r0, [r3, #THINKOS_EXCEPT_CYCCNT_OFFS] 
#endif

	movs    r0, r3
	adds    r3, #THINKOS_EXCEPT_CONTEXT_OFFS
#if (THINKOS_ENABLE_FPU)
	vstmia  r3!, {s16-s31}
#endif
	stmia   r3!, {r4-r11} /* r4..r11 */

	/* Basic information is saved. Next call could cause an exception
		TODO: check R12 (PSP/MSP) */

	ldmia   r12, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
	stmia   r3!, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
#if (THINKOS_ENABLE_FPU)
	vstmia  r3!, {s0-s15}
	vmrs    r4, FPSCR
	stmia   r3!, {r4-r5}
#endif

	/* Exception on interrupt handler ? */
	ands    r1, #SCB_ICSR_RETTOBASE 
	beq     .L_reset_idle

	/* Exception returns to idle thread ? */
	cmp     lr, #CM3_EXC_RET_THREAD_MSP
	beq     .L_reset_idle

	/* Exception on idle thread ? */
	ldr     r0, [r3, #THINKOS_EXCEPT_ACTIVE_OFFS]
	cmp     r2, #THINKOS_THREAD_IDLE
	beq     .L_reset_idle

	/* Load the idle context pointer from the context vector */
	ldr     r2, .L_thinkos_rt
	ldr     r7, [r2, #THINKOS_THREAD_IDLE * 4]
	/* Check for stack availability */
	cmp     r7, r3
	ble     .L_reset_idle


	/* Restore idle core context */
	mov     sp, r7
	pop     {r4-r11}
	/* Synthesizes exception return */
	mov     lr, #CM3_EXC_RET_THREAD_MSP
	/* Call the thread exception handler */
	b       thinkos_krn_thread_except

.L_reset_idle:
	/* Reset the exception stack */
	bl      thinkos_krn_idle_reset
	mov     sp, r0
	ldr     r0, .L_xcpt_buf
	/* Restore idle core context */
	pop     {r4-r11}
	/* Synthesizes exception return */
	mov     lr, #CM3_EXC_RET_THREAD_MSP
	/* Call the fatal exception handler */
	b       thinkos_krn_fatal_except

	.align  2
.L_xcpt_buf: /* exception buffer is at the bottom of the exception stack */
	.word   thinkos_except_stack
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 
.L_cm3_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   thinkos_krn_xcpt_raise, . - thinkos_krn_xcpt_raise

#if	(THINKOS_ENABLE_BUSFAULT)
	.thumb
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	movs   r0, #THINKOS_ERR_BUS_FAULT
	b      thinkos_krn_xcpt_raise
	.size  cm3_bus_fault_isr, . - cm3_bus_fault_isr
#endif

#define CFSR_UNDEFINSTR (UFSR_UNDEFINSTR << 16)

#if	(THINKOS_ENABLE_USAGEFAULT) 
	.align	2
	.global	cm3_usage_fault_isr
	.thumb
	.thumb_func
	.type	cm3_usage_fault_isr, %function
cm3_usage_fault_isr:
	movs   r0, #THINKOS_ERR_USAGE_FAULT
#if	(THINKOS_ENABLE_KRNSVC) 
	/* Kernel services -------------------------------------------- */

	/* Check for undefined instruction flag on UFSR */
	ldr     r3, .L_cm3_scb
	ldr     r3, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	tst     r3, #CFSR_UNDEFINSTR /* Undefined instruction ? */
	beq     thinkos_krn_xcpt_raise
	/* Code running from the process stack (non-privileged) causes a fault.
	   (Select the context stack according to the content of LR */
	tst     lr, #4
	bne     thinkos_krn_xcpt_raise
	/* Get the call frame stack */
	mov     r0, sp
	/* Get PC from the stack */
	ldr     r3, [r0, #(6 * 4)] 
	/* Read the Krn SVC number from the instruction */
	ldrb    r1, [r3, #0]
	/* Skip undefined instruction, by adjusting the call stack (PC = PC + 2) */
	adds    r3, r3, #2
	str     r3, [r0, #(6 * 4)] 
	/* Call the kernel service demultiplexer */
	b       thinkos_krnsvc_isr
#else
	b       thinkos_krn_xcpt_raise
#endif

	.size  cm3_usage_fault_isr, . - cm3_usage_fault_isr
#endif

#if	(THINKOS_ENABLE_MEMFAULT)
	.align	2
	.global	cm3_mem_manage_isr
	.thumb
	.thumb_func
	.type	cm3_mem_manage_isr, %function
cm3_mem_manage_isr:
	movs   r0, #THINKOS_ERR_MEM_MANAGE
	b      thinkos_krn_xcpt_raise
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
#endif

#endif /* THINKOS_ENABLE_EXCEPTIONS */

#if (THINKOS_ENABLE_DBGMON)
	.align  2
	.global cm3_debug_mon_isr
	.thumb
	.thumb_func
	.type   cm3_debug_mon_isr, %function
cm3_debug_mon_isr:
	/* select the context stack according to the content of LR */
	mov     r1, lr
	tst     r1, #CM3_EXC_RET_SPSEL
	ite     eq
	mrseq   r0, MSP
	mrsne   r0, PSP
	b       thinkos_dbgmon_isr
	.size   cm3_debug_mon_isr, . - cm3_debug_mon_isr
#endif /* THINKOS_ENABLE_DBGMON */

	.align  2
	.global thinkos_vec_nm
	.section .rodata,"a",%progbits
	.type   thinkos_vec_nm, %object
	.size   thinkos_vec_nm, 4
thinkos_vec_nm:
	.ascii  "VEC\000"
 
