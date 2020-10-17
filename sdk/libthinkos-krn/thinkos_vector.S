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
#include <arch/cortex-m3.h>

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

/* --------------------------------------------------------------------------
 * ThinkOS - excption handler stubs
 * --------------------------------------------------------------------------*/
#if (THINKOS_ENABLE_EXCEPTIONS)
	.thumb
	.text
	.align	2
	.global	xcpt_collect
	.thumb
	.thumb_func
	.type	xcpt_collect, %function

xcpt_collect:
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
	     r1: except number
	     r2: return (lr) 
	   Return:
	     r0: exception buffer */

	mrs     r3, PRIMASK
	/* disable interrupts */
	cpsid   i
	ldr     r0, .L_xcpt_buf
	strb    r1, [r0, #0] /* errno */
	ldrb    r2, [r0, #1] /* count */
	adds    r2, r2, #1
	strb    r2, [r0, #1] /* count */
	strb    lr, [r0, #2] /* return (lr) */
	strb    r3, [r0, #3] /* PRIMASK */
	mrs     r3, FAULTMASK
	strb    r3, [r0, #4] /* FAULTMASK */
	mrs     r3, BASEPRI 
	strb    r3, [r0, #5] /* BASEPRI */
	mrs     r3, CONTROL
	strb    r3, [r0, #6] /* CONTROL */
	mrs     r3, IPSR
	strb    r3, [r0, #7] /* IPSR */

	adds    r3, r0, #8
#if (THINKOS_ENABLE_FPU)
	/* collect FP data */ 
	vstmia  r3!, {s16-s31}
#endif
	stmia   r3!, {r4-r11} /* r4..r11 */
	tst     lr, #CM3_EXC_RET_SPSEL
	ite     eq 
	mrseq   r4, MSP 
	mrsne   r4, PSP 
#if (THINKOS_ENABLE_IDLE_MSP) || (THINKOS_ENABLE_FPU)
	str     r4, [r3, #0] /* sp */
	str     lr, [r3, #4] /* ret */
	adds    r3, r3, #8
#endif
	ldmia   r4, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
	stmia   r3!, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
#if (THINKOS_ENABLE_FPU)
	vstmia  r3!, {s0-s15}
	vmrs    r4, FPSCR
	stmia   r3!, {r4-r5}
#endif
	mrs     r5, MSP 
	mrs     r6, PSP 
	stmia   r3!, {R5, R6} /* msp, psp */

	ldr     r7, .L_cm3_scb

	ldr     r4, [r7, #SCB_ICSR_OFFS] /* Interrupt Control State */
	ldr     r6, [r7, #SCB_SHCSR_OFFS] /* System Handler Control and State */
	stmia   r3!, {R4, R6} /* icsr, shcsr */
	/* r4 = ICSR */

	ldr     r5, [r7, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	ldr     r6, [r7, #SCB_HFSR_OFF] /* Hard Fault Status */
	stmia   r3!, {R5, R6} /* cfsr, hfsr*/

	ldr     r5, [r7, #SCB_MMFAR_OFFS] /* Mem Manage Address */
	ldr     r6, [r7, #SCB_BFAR_OFFS] /* Bus Fault Address */
	stmia   r3!, {R5, R6} /* mmfar, bfar */

	ldr     r7, .L_thinkos_rt
#if (THINKOS_ENABLE_PROFILING)
	/* DWT.SYCCNT */
	ldr.n   r5, .L_cm3_dwt
	/* r6:  cyccnt = CM3_DWT->cyccnt */
	ldr.n   r5, [r5, #4]
	ldr     r6, [r7, #THINKOS_RT_CYCREF_OFFS]
	stmia   r3!, {R5, R6} /* cyccnt, cycref */
#endif

	ldr     r5, [r7, #THINKOS_RT_ACTIVE_OFFS]
	ldr     r6, [r7, #THINKOS_RT_READY_OFFS]
	stmia   r3!, {R5, R6}

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* get the effective thread id */
	ands    r5, r5, #0x3f
#endif
	/* r4 = ICSR */
	/* r5 = active thread */
	/* r6 = ready bitmap */
	/* r7 = thinkos_rt */
	/* if (CM3_SCB->icsr & SCB_ICSR_RETTOBASE)  */
	ands    r4, #SCB_ICSR_RETTOBASE 
	beq     .L_fault_on_handler
	cmp     r5, #THINKOS_THREAD_IDLE 
	beq     .L_fault_on_idle

.L_thread_except:
	/* Call the thread exception handler */
	bl      thinkos_krn_thread_except

.L_return_to_idle:
	/* thinkos_rt */
	movs    r3, r7
	/* previous thread */
	movs    r2, r5
	/* Prepare to return to IDLE */
	movs    r1, #THINKOS_THREAD_IDLE 
	/* load the IDLE context pointer from the context vector */
	ldr.w	r0, [r3, r1, lsl #2]

	/* enable interrupts */
	cpsie   i
	/* restore context */
	b       thinkos_krn_sched_ctx_restore

.L_fault_on_idle:

	/* Reset IDLE thread */
	bl     thinkos_krn_idle_reset
	/* Reset the exception stack */
	bl     __thinkos_xcpt_stack_top
	subs    r0, r0, #SIZEOF_ARMV7M_BASIC_FRAME 
	mov     sp, r0

	b      .L_thread_except

.L_fault_on_handler:
	/* Reset IDLE thread */
	bl     thinkos_krn_idle_reset
	/* Reset the exception stack */
	bl     __thinkos_xcpt_stack_top
	subs    r0, r0, #SIZEOF_ARMV7M_BASIC_FRAME 
	mov     sp, r0

	/* Call the fatal exception handler */
	bl     thinkos_krn_fatal_except

	b      .L_return_to_idle

	.align  2
.L_xcpt_buf:
	.word	thinkos_except_buf
.L_xcpt_stack:
	.word   thinkos_except_stack
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 
.L_cm3_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   xcpt_collect, . - xcpt_collect

#if	(THINKOS_ENABLE_BUSFAULT)
	.thumb
	.text
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	movs   r1, #THINKOS_ERR_BUS_FAULT
	b      xcpt_collect
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
	movs   r1, #THINKOS_ERR_USAGE_FAULT

#if	(THINKOS_ENABLE_KRNSVC) 
	/* Kernel services -------------------------------------------- */

	/* Check for undefined instruction flag on UFSR */
	ldr    r3, .L_cm3_scb
	ldr    r3, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	tst    r3, #CFSR_UNDEFINSTR /* Undefined instruction ? */
	beq    xcpt_collect
	/* Code running from the process stack (non-privileged) causes a fault.
	   (Select the context stack according to the content of LR */
	tst     lr, #4
	bne     xcpt_collect
	/* Get the call frame stack */
	mov    r0, sp
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
	b      xcpt_collect
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
	movs   r1, #THINKOS_ERR_MEM_MANAGE
	b      xcpt_collect
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
#endif

	.align	2
	.global	cm3_hard_fault_isr
	.thumb
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	movs   r1, #THINKOS_ERR_HARD_FAULT
	b      xcpt_collect
	.size  cm3_hard_fault_isr, . - cm3_hard_fault_isr
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
#if 0
	/* FIXME: add suport for step on interrupt */
	push    {lr}
	bl      thinkos_dbgmon_isr
	pop     {lr}
	cbnz    r0, 1f
	bx      lr
1f:
	/* Disable low priority interrupts */
	mov     r2, #(1 << 5)
	msr	    BASEPRI, r2
	/* Step request */
	ldr.n   r3, .L_cm_dcb
	ldr     r1, [r3, #DCB_DEMCR_OFFS]
	orr     r1, r1, #DCB_DEMCR_MON_STEP
	str     r1, [r3, #DCB_DEMCR_OFFS]
	bx      lr
.L_cm_dcb:
	.word   CM3_DCB_BASE /* Core Debug Base Address */
#endif
	.size   cm3_debug_mon_isr, . - cm3_debug_mon_isr
#endif /* THINKOS_ENABLE_DBGMON */

	.align  2
	.global cm3_svc_isr
	.thumb
	.thumb_func
	.type   cm3_svc_isr, %function
cm3_svc_isr:
	ldr     r3, .L_thinkos_rt_1
	ldr     r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* get the effective thread id */
	ands    r1, r1, #0x3f
#endif

#if (THINKOS_ENABLE_DEBUG_STEP)
	ldr     r2, [r3, #THINKOS_RT_STEP_SVC_OFFS]
	adds    r0, r1, #1
	lsrs    r2, r0
	bcc     1f
	ldr     r3, .L_cm3_scb_1
	mov     r0, #SCB_ICSR_PENDSVSET
	str     r0, [r3, #SCB_ICSR_OFFS]
1:
#endif

#if (THINKOS_ENABLE_IDLE_MSP)
	/* select the context stack according to the content of LR */
	tst     lr, #4
	ite     eq
	mrseq   r0, MSP
	mrsne   r0, PSP
#else
	mrs     r0, PSP
#endif

	/* Get PC from the stack */
	ldr     r2, [r0, #(6 * 4)] 
	/* Read the SVC number from the instruction */
	ldrb    r2, [r2, #-2]
	/* call the service handler demultiplexer */
	b       thinkos_svc_isr

	.align  2
.L_thinkos_rt_1:
	.word   thinkos_rt
.L_cm3_scb_1:
	.word   CM3_SCB_BASE
	.size   cm3_svc_isr, . - cm3_svc_isr
	.align  2
	.global thinkos_vec_nm
	.section .rodata,"a",%progbits
	.type   thinkos_vec_nm, %object
	.size   thinkos_vec_nm, 4
thinkos_vec_nm:
	.ascii  "VEC\000"


