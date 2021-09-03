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
	/* r1: sched.ctrl: [ xcpt | err | svc | thd ] */
	/* r2: sched.ready: [bitmap]  */

	.size   cm3_pendsv_isr, . - cm3_pendsv_isr

	.thumb_func
thinkos_krn_sched_core:
	.global thinkos_krn_sched_core
	.type   thinkos_krn_sched_core , %function

#if 0
	mov  r12, #1
1: 	cmp  r12, #0
	bne  1b
#endif

	/* r2: ready bmp */
	/* r1: sched ctrl [ xcpt | err | svc | thd ] */
	/* r0: krn */
	tst      lr, #EXC_RET_SPSEL
	beq      .L_entry_idle

	/* Save user thread context on PSP */
	mrs      r3, PSP
	/* Save context */
	stmdb    r3!, {r4-r11}

	/* extract the context thread byte (6bits) form control word */
    uxtb.n   r4, r1
	/* get a pointer to the thread context vector */
    lsls.n   r7, r4, #2
	add.n    r7, r0

#if (THINKOS_ENABLE_STACK_LIMIT) || (THINKOS_ENABLE_FPU)
	mov.n    r8, r3
  #if (THINKOS_ENABLE_FPU)
	/* Floating point context */
	tst      lr, #EXC_RETURN_nFPCA
	it       eq
	vstmdbeq r8!, {s16-s31}
  #endif
#endif

	/* Append control */
#if (THINKOS_ENABLE_PRIVILEGED_THREAD) || (THINKOS_ENABLE_FPU)
	mrs      r6, CONTROL
	adds.n   r6, #(CONTROL_SPSEL)
	add.n    r3, r6
#else
	adds.n   r3, r3, #(CONTROL_SPSEL | CONTROL_nPRIV)
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* check the stack limit */
	ldr      r5, [r7, #OFFSETOF_KRN_TH_SL]
	cmp.n    r8, r5
	ble      .L_stack_err
#endif

#if (THINKOS_ENABLE_ERROR_TRAP)
	/* check the kernel scheduler control word */
	lsrs.n   r6, r1, #8
	/* r6: sched.ctrl >> 8 : [ 0x00 | xcp | err | svc ] 

	   r3: active_sp_ctrl : [SP | CONTROL ] 
	   r2: ready bmp 
	   r1: sched.ctrl = [ xcpt | err | svc | act ]  
	 */
    bne.n    thinkos_krn_sched_bypass
#endif

.L_update_cyccnt:

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
	subs.n   r6, r6, r5

	/* update thread's cycle counter */
	ldr      r5, [r7, #OFFSETOF_KRN_TH_CYC]
	/* thinkos_rt.cyccnt[thread_id] += delta */
	adds.n   r5, r6
	str      r5, [r7, #OFFSETOF_KRN_TH_CYC]
#endif

	/* r0: thinkos_rt 
	   r1: sched.ctrl 
	   r2: ready bmp 
	   r3: active_sp_ctrl: [SP | CONTROL ] 
	   r7: thread_ctx[thread_id] */
	/* store the old context pointer into the context vector */
	str.n    r3, [r7]

	/* load the kernel ready mask */
	/* r2: ready bmp */
thinkos_sched_select_ready:
#if (THINKOS_ENABLE_READY_MASK)
	ldr      r5, [r0, #OFFSETOF_KRN_RDY_MSK]
	ands.n   r2, r5
#endif
	/* get a thread from the ready bitmap by counting the
	leading zeros on the bitmap */
	cbz.n    r2, .L_exit_idle
	rbit     r2, r2
 	clz      r2, r2

#if (THINKOS_ENABLE_DEBUG_STEP)
thinkos_sched_step_req_check:
	/* r6: step_req */
	ldr      r6, [r0, #OFFSETOF_KRN_STEP_REQ]
	/* check if the step request bit is set for this thread */
	movs.n	 r5, #1
	lsls.n   r5, r2
	tst.n    r5, r6
	bne.n    thinkos_sched_step
#endif
	/* Get thread number from bitmap index */
 	adds.n   r2, #1
    lsls.n   r7, r2, #2

thinkos_krn_sched_ctx_get:
	/* load the new context pointer from the context vector */
	ldr.n	 r3, [r0, r7]

.L_save_active_and_restore:
	/* r4: old active thread id 
	   r3: new thread stack
	   r2: new active thread id 
	   r0: thinkos_rt */
	strb     r2, [r0, #OFFSETOF_KRN_SCHED_THREAD]

#if (THINKOS_ENABLE_PRIVILEGED_THREAD) || (THINKOS_ENABLE_FPU)
	ands     r0, r3, #(THREAD_CTRL_MSK)
	/* restore the control register */
	msr      CONTROL, r0
	/* restore the stack pointer */
	subs.n   r3, r0
#endif

#if (THINKOS_ENABLE_SANITY_CHECK) && (THINKOS_ENABLE_SCHED_ERROR)
	cbz.n    r3, .L_nullptr_err
#endif

#if (THINKOS_ENABLE_FPU)
	mov.n    r12, r3
	tst      r0, #(CONTROL_FPCA)
	itte     ne
	/* restore FP context */
	vldmdbne r12!, {s16-s31}
	/* Synthesizes exception return */
	movne    lr, #CM3_EXC_RET_THREAD_PSP_EXT
	moveq    lr, #CM3_EXC_RET_THREAD_PSP
#else
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP
#endif

	/* r3: new thread stack [ SP | CONTROL]
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

.L_exit_idle:
	/* Adjust the thread ID */
	movs.n   r2, #THINKOS_THREAD_IDLE
	/* set the active to IDLE */
	strb     r2, [r0, #OFFSETOF_KRN_SCHED_THREAD]

#if (THINKOS_ENABLE_SCHED_ERROR)
    /* load the new context pointer from the context vector */
    ldr.w    r3, [r0, #OFFSETOF_KRN_IDLE_CTX]
    cmp      sp, r3
    bne      .L_idle_stack_ptr_err
#elif (THINKOS_ENABLE_SCHED_DEBUG)
    mov.n     r3, sp
#endif
    /* set the control register */
    movs.n   r2, #0
    msr      CONTROL, r2
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_MSP
	/* restore core context */
	pop      {r4-r11}
	/* return */
#if (THINKOS_ENABLE_SCHED_DEBUG)
	b        thinkos_sched_dbg
#else
	bx       lr
#endif

/* --------------------------------------------------------------------------
 * Scheduler detected errors
 * -------------------------------------------------------------------------- */


#if (THINKOS_ENABLE_STACK_LIMIT)
.L_stack_err: 
	movs.n  r3, #THINKOS_ERR_THREAD_STACKADDR   
	b.n     thinkos_krn_sched_trap
#endif



/* --------------------------------------------------------------------------
 * IDLE 
 * -------------------------------------------------------------------------- */

.L_entry_idle:
#if (THINKOS_ENABLE_ERROR_TRAP)
	/* check the kernel scheduler control word */
	lsrs.n   r3, r1, #8
	/* r6: sched.ctrl >> 8 : [ 0x00 | xcp | err | svc ] 

	   r3: active_sp_ctrl : [SP | CONTROL ] 
	   r2: ready bmp 
	   r1: sched.ctrl = [ xcpt | err | svc | act ]  */
	/* kernel is blocked, just return */
	it      ne
	bxne    lr
#endif

	/* Save system context (IDLE) on MSP */
	push     {r4-r11}
    mov      r3, sp
#if (THINKOS_ENABLE_SANITY_CHECK) && (THINKOS_ENABLE_SCHED_ERROR)
	/* extract the current thread id */
    uxtb.n   r4, r1
	cbz.n    r4, 1f
	cmp.n    r4, #THINKOS_THREAD_IDLE
	bne      .L_idle_entry_err    
1:
#else
	movs.n   r4, #THINKOS_THREAD_IDLE
#endif
    lsls.n   r7, r4, #2
	add.n    r7, r0
#if (THINKOS_ENABLE_STACK_LIMIT) && (THINKOS_ENABLE_SCHED_ERROR)
	/* check the stack limit */
	ldr      r5, [r7, #OFFSETOF_KRN_TH_SL]
	cmp.n    r3, r5
	ble      .L_idle_stack_limit_err 
#endif
    b        .L_update_cyccnt

#if (THINKOS_ENABLE_SANITY_CHECK) && (THINKOS_ENABLE_SCHED_ERROR)
.L_nullptr_err:
	movs.n   r3, #THINKOS_ERR_INVALID_POINTER
	b        thinkos_krn_sched_xcpt
#endif

#if (THINKOS_ENABLE_SCHED_ERROR)
.L_idle_stack_ptr_err:
.L_idle_stack_xcpt_err:
	movs.n   r3, #THINKOS_ERR_IDLE_XCPT
	b        thinkos_krn_sched_xcpt
#endif

#if (THINKOS_ENABLE_SANITY_CHECK) && (THINKOS_ENABLE_SCHED_ERROR)
.L_idle_entry_err:
	movs.n  r3, #THINKOS_ERR_IDLE_ENTRY
	b        thinkos_krn_sched_xcpt
#endif

#if (THINKOS_ENABLE_STACK_LIMIT) && (THINKOS_ENABLE_SCHED_ERROR)
.L_idle_stack_limit_err: 
	movs.n   r3, #THINKOS_ERR_IDLE_STACK
	b        thinkos_krn_sched_xcpt
#endif

	.align  0
.L_thinkos_rt:
	.word	thinkos_rt
#if (THINKOS_ENABLE_PROFILING)
.L_cm_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */
#endif
	.size   thinkos_krn_sched_core, . - thinkos_krn_sched_core


#if (THINKOS_ENABLE_SCHED_ERROR)
	.thumb_func
thinkos_krn_sched_xcpt:
	.global thinkos_krn_sched_xcpt
	.type   thinkos_krn_sched_xcpt, %function
/* FXME: fatal exception... */	
	b       .
	.size   thinkos_krn_sched_xcpt, . - thinkos_krn_sched_xcpt
#endif /* THINKOS_ENABLE_SCHED_ERROR */


/* --------------------------------------------------------------------------
 *
 *
 * -------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_ERROR_TRAP)
	.thumb_func
thinkos_krn_sched_trap:
	.global thinkos_krn_sched_trap
	.type   thinkos_krn_sched_trap, %function

	/*  r1: | err | brk | svc | act |  */
	/* Insert the error code (R3) on R1 */
	lsls.n  r3, r3, #24
	lsls.n  r1, r1, #8
	lsrs.n  r1, r1, #8
	adds.n  r1, r3

	/*  r0: krn */
	/*  r1: | err | brk | svc | act |  */
thinkos_krn_sched_bypass:
	/* store the old context pointer into the context vector */
	str.n    r3, [r7]
#if (THINKOS_ENABLE_SCHED_ERROR)
    /* load the IDLE context pointer from the context vector */
    ldr      r3, [r0, #OFFSETOF_KRN_IDLE_CTX]
    cmp.n    sp, r3
    bne.n    .L_idle_stack_xcpt_err
#elif (THINKOS_ENABLE_SCHED_DEBUG)
    mov.n     r3, sp
#endif
    /* set the control register */
    movs.n   r2, #0
    msr      CONTROL, r2
	/* Synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_MSP
	/* Restore core context */
	pop      {r4-r11}

#if (THINKOS_ENABLE_SCHED_DEBUG)
	push     {r1, lr}
	bl       thinkos_sched_dbg
	pop      {r1, lr}
#endif

	b        thinkos_krn_sched_err_handler

	.size   thinkos_krn_sched_trap, . - thinkos_krn_sched_trap
#endif /* THINKOS_ENABLE_ERROR_TRAP */

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

	ldr      r4, [r0, #OFFSETOF_KRN_STEP_SVC]	
	/* r4: step_svc */
	tst      r6, r4
	bne.n    .L_service_setp
 
.L_normal_step:
	/* get the PC value */
	bic      r7, r3, #(THREAD_CTRL_MSK)
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


