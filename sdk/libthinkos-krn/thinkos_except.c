/* 
 * File:	 dac-test.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "thinkos_krn-i.h"

#if (THINKOS_ENABLE_EXCEPTIONS)

/* Static sanity check: */
_Static_assert (offsetof(struct thinkos_fault, ctx) == 
				OFFSETOF_FAULT_CONTEXT, "OFFSETOF_FAULT_CONTEXT");

_Static_assert (offsetof(struct thinkos_fault, sp) == 
				OFFSETOF_FAULT_SP, "OFFSETOF_FAULT_SP");

_Static_assert (offsetof(struct thinkos_fault, ret) == 
				OFFSETOF_FAULT_RET, "OFFSETOF_FAULT_RET");

_Static_assert (offsetof(struct thinkos_fault, control) == 
				OFFSETOF_FAULT_CONTROL, "OFFSETOF_FAULT_CONTROL");

_Static_assert (offsetof(struct thinkos_fault, errno) == 
				OFFSETOF_FAULT_ERRNO, "OFFSETOF_FAULT_ERRNO");

_Static_assert (offsetof(struct thinkos_fault, seq) == 
				OFFSETOF_FAULT_SEQ, "OFFSETOF_FAULT_SEQ");

_Static_assert (offsetof(struct thinkos_fault, thread) == 
				OFFSETOF_FAULT_THREAD, "OFFSETOF_FAULT_THREAD");

_Static_assert (offsetof(struct thinkos_fault, ack) == 
				OFFSETOF_FAULT_ACK, "OFFSETOF_FAULT_ACK");

_Static_assert (offsetof(struct thinkos_fault, shcsr) == 
				OFFSETOF_FAULT_SHCSR, "OFFSETOF_FAULT_SHCSR");

_Static_assert (offsetof(struct thinkos_fault, cfsr) == 
				OFFSETOF_FAULT_CFSR, "OFFSETOF_FAULT_CFSR");

_Static_assert (offsetof(struct thinkos_fault, mmfar) == 
				OFFSETOF_FAULT_MMFAR, "OFFSETOF_FAULT_MMFAR");

_Static_assert (offsetof(struct thinkos_fault, bfar) == 
				OFFSETOF_FAULT_BFAR, "OFFSETOF_FAULT_BFAR");

#if 0
_Static_assert (offsetof(struct thinkos_fault, hfsr) == 
				OFFSETOF_FAULT_HFSR, "OFFSETOF_FAULT_HFSR");

_Static_assert (offsetof(struct thinkos_fault, ipsr) == 
				OFFSETOF_FAULT_IPSR, "OFFSETOF_FAULT_IPSR");

_Static_assert (offsetof(struct thinkos_fault, psp) == 
				OFFSETOF_FAULT_PSP, "OFFSETOF_FAULT_PSP");

_Static_assert (offsetof(struct thinkos_fault, sched) == 
				OFFSETOF_FAULT_SCHED, "OFFSETOF_FAULT_SCHED");

_Static_assert (offsetof(struct thinkos_fault, icsr) == 
				OFFSETOF_FAULT_ICSR, "OFFSETOF_FAULT_ICSR");

#if (THINKOS_ENABLE_PROFILING)
_Static_assert (offsetof(struct thinkos_fault, cycref) == 
				OFFSETOF_FAULT_CYCREF, "OFFSETOF_FAULT_CYCREF");

_Static_assert (offsetof(struct thinkos_fault, cyccnt) == 
				OFFSETOF_FAULT_CYCCNT, "OFFSETOF_FAULT_CYCCNT");
#endif

#endif

struct thinkos_fault thinkos_fault_rt __attribute__((aligned(8)));

#if (DEBUG)
/*
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_TRACE
#elif LOG_LEVEL < LOG_TRACE
#undef LOG_LEVEL 
#define LOG_LEVEL LOG_TRACE
#endif
 */
#undef THINKOS_SYSRST_ONFAULT
#define THINKOS_SYSRST_ONFAULT    0
#define DCC_FAULT_DUMP(KRN, XCPT) __xdump(KRN, XCPT)
#else
#define DCC_FAULT_DUMP(KRN, XCPT)
#endif

#include <sys/dcclog.h>

#define SHCSR_ACT_MASK SCB_SHCSR_SYSTICKACT | SCB_SHCSR_PENDSVACT | \
	SCB_SHCSR_MONITORACT | SCB_SHCSR_SVCALLACT | SCB_SHCSR_USGFAULTACT | \
	SCB_SHCSR_BUSFAULTACT | SCB_SHCSR_MEMFAULTACT

uint32_t krn_xcpt_unroll_ipsr_get(struct thinkos_rt * krn)
{
	uint32_t shcsr;
	uint32_t ipsr;
	uint32_t act;
	uint32_t bmp;
	int irqregs;
	int i;
	
	irqregs = (CM3_ICTR + 1);

	/* Find active IRQ */
	for (i = 0; i < irqregs ; ++i) {
		int irqbits;
		int j;
		irqbits = __rbit(CM3_NVIC->iabr[i]);
		if ((j = __clz(irqbits)) < 32) {
			int irq;
			irq = i * 32 + j;
			act = irq + 16;
			__nvic_irq_clrpend_all();
			goto end;
		}
	}

	shcsr = CM3_SCB->shcsr & (SHCSR_ACT_MASK);
	ipsr = cm3_ipsr_get();
	if (ipsr == CM3_EXCEPT_BUS_FAULT) {
		/* currently servicing bus fault */
		shcsr &= ~SCB_SHCSR_BUSFAULTACT;
	} else if (ipsr == CM3_EXCEPT_USAGE_FAULT) {
		/* currently servicing usage fault */
		shcsr &= ~SCB_SHCSR_USGFAULTACT;
	} else if (ipsr == CM3_EXCEPT_MEM_MANAGE) {
		/* currently servicing memory management fault */
		shcsr &= ~SCB_SHCSR_MEMFAULTACT;
	}

	/* check for active exceptions */
	if ((bmp = __rbit(shcsr)) == 0) {
		return 0;
	}

	act = __clz(bmp) + 4;

end:
	uint32_t icsr;
	icsr = CM3_SCB->icsr;

	if (icsr & SCB_ICSR_RETTOBASE) {
		DCC_LOG1(LOG_TRACE, VT_PSH VT_FMG " XCPT %d active RETTOBASE " VT_POP, 
				 act);
		return 0;
	} else {
		DCC_LOG1(LOG_TRACE, VT_PSH VT_FMG " XCPT %d active" VT_POP, 
				 act);
	}
	return act;
}

uint32_t __krn_xcpt_unroll_ipsr_get(struct thinkos_rt * krn)
{
	uint32_t icsr;
//	uint32_t pend;
	uint32_t act;

//	__krn_xcpt_pend_clr(krn);
//	__nvic_irq_disable_all();

	icsr = CM3_SCB->icsr;
	act = ICSR_VECTACTIVE_GET(icsr);
#if 0
	pend = ICSR_VECTPENDING_GET(icsr);
	(void)act;
	(void)pend;
	DCC_LOG4(LOG_WARNING, VT_PSH VT_FMG " VECT %d pend, %d act%s%s" VT_POP, 
			 pend, act, 
			 (icsr & SCB_ICSR_RETTOBASE) ? " RETTTOBASE" : "",
			 (icsr & SCB_ICSR_ISRPENDING) ? " ISRPENDING" : "");

	if (icsr & SCB_ICSR_RETTOBASE) 
		return 0;

	mdelay(125);
#endif

	return act;
}

void thinkos_krn_fatal_except(struct thinkos_rt * krn,
							  struct thinkos_context * ctx,
							  uint32_t sp,
							  uint32_t xpsr)
{
	uint32_t ipsr = xpsr & 0x1ff;
	int i;

	DCC_LOG(LOG_PANIC, VT_PSH VT_REV VT_FRD
			" !!! Fatal exception !!!!" VT_POP);

	mdelay(250);

	DCC_LOG(LOG_PANIC, "1. __hard()...");
	__hard(krn, ctx, sp, ipsr);

	for (i = 0; i < 10; ++i) {
		mdelay(500);
		DCC_LOG1(LOG_PANIC, "Fatal except %d.", i);
	}

	DCC_LOG(LOG_PANIC, "System reset!");
#if (THINKOS_SYSRST_ONFAULT)
	thinkos_krn_sysrst();
#endif
	for(;;);
}

void thinkos_krn_fault_handler(struct thinkos_rt * krn,
							   struct thinkos_fault * fault,
							   uint32_t xcptno,
							   uint32_t sp)
{
	/* Stop the scheduler */
	__krn_sched_xcp_set(krn, xcptno);

	/* Disable all vectored interrupts on NVIC */
	__nvic_irq_disable_all();

#if (THINKOS_ENABLE_READY_MASK)
	__thread_disble_all(krn);
#endif

	/* Clear the SVC call pending to avoid returning to a service routine
	   as this will cause another stack fault */
	__krn_svc_pend_clr(krn);

	DCC_LOG1(LOG_WARNING, VT_PSH VT_REV VT_FYW " Kernel exception: %d " VT_POP, 
			 xcptno);

	/* preempt the scheduler */
	__krn_preempt(krn);

	/* Reenable CPU interrupts */
	cm3_cpsie_i();
}

/* ARM exceptions in a thread context are recorded in
   the exception buffer. Then a common handling
   function takes care of signaling the modules
   that may be affected by this condition. */
void thinkos_krn_except_err_handler(struct thinkos_rt * krn,
									struct thinkos_fault * fault,
									uint32_t errno,
									uint32_t thread)
{
	/* Stop the scheduler */
	__krn_sched_err_set(krn, errno);

	/* Disable all vectored interrupts on NVIC */
//	__nvic_irq_disable_all();

	/* Clear the SVC call pending to avoid returning to a service routine
	   as this will cause another stack fault */
	__krn_svc_pend_clr(krn);

	DCC_LOG2(LOG_TRACE, VT_PSH VT_REV VT_FYW
			 " Exception: error %d on thread %d " VT_POP, 
			 errno, thread);

	DCC_FAULT_DUMP(krn, fault);

	/* preempt the scheduler. It will run shortly ater this exception 
	 * is unstacked. */
	__krn_preempt(krn);

	/* Reenable CPU interrupts */
	cm3_cpsie_i();
}

/* -------------------------------------------------------------------------
   Application fault deferred handler 
   ------------------------------------------------------------------------- */
void thinkos_krn_fault_clr(void)
{
	struct cm3_scb * scb = CM3_SCB;
	uint32_t cfsr;

	/* Clear CFSR (Configurable Fault Status Register) */
	cfsr = scb->cfsr;
	scb->cfsr = cfsr;

	if (cfsr)  {
		DCC_LOG1(LOG_TRACE, VT_PSH VT_FMG  "CFSR=0x%8x" VT_POP, cfsr);
	}

#if (THINKOS_ENABLE_MEMORY_CLEAR)
	DCC_LOG(LOG_TRACE, "Exception clear.");
	__thinkos_memset32(&thinkos_fault_rt, 0, sizeof(struct thinkos_fault));
#endif
}

void thinkos_krn_exception_init(void)
{
	struct cm3_scb * scb = CM3_SCB;

	thinkos_krn_fault_clr();

#if	(THINKOS_ENABLE_USAGEFAULT) 
	DCC_LOG(LOG_TRACE, "USAGE fault enabled.");
	cm3_except_pri_set(CM3_EXCEPT_USAGE_FAULT, EXCEPT_PRIORITY);
#endif
#if	(THINKOS_ENABLE_BUSFAULT)
	DCC_LOG(LOG_TRACE, "BUS fault enabled.");
	cm3_except_pri_set(CM3_EXCEPT_BUS_FAULT, EXCEPT_PRIORITY);
#endif
#if (THINKOS_ENABLE_MPU)
	DCC_LOG(LOG_TRACE, "Mem management fault enabled.");
	cm3_except_pri_set(CM3_EXCEPT_MEM_MANAGE, EXCEPT_PRIORITY);
#endif

	scb->shcsr = 0 
#if	(THINKOS_ENABLE_USAGEFAULT)
		| SCB_SHCSR_USGFAULTENA 
#endif
#if	(THINKOS_ENABLE_BUSFAULT)
		| SCB_SHCSR_BUSFAULTENA
#endif
#if (THINKOS_ENABLE_MEMFAULT)
		| SCB_SHCSR_MEMFAULTENA
#endif
		;
}

#else /* THINKOS_ENABLE_EXCEPTIONS */

#endif /* THINKOS_ENABLE_EXCEPTIONS */

