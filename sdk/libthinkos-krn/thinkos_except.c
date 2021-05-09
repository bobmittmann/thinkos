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
	_Static_assert (offsetof(struct thinkos_except, seq) == 
					OFFSETOF_XCPT_SEQ, "OFFSETOF_XCPT_SEQ");

	_Static_assert (offsetof(struct thinkos_except, errno) == 
					OFFSETOF_XCPT_ERRNO, "OFFSETOF_XCPT_ERRNO");

	_Static_assert (offsetof(struct thinkos_except, ipsr) == 
					OFFSETOF_XCPT_IPSR, "OFFSETOF_XCPT_IPSR");

	_Static_assert (offsetof(struct thinkos_except, ret) == 
					OFFSETOF_XCPT_RET, "OFFSETOF_XCPT_RET");

	_Static_assert (offsetof(struct thinkos_except, ack) == 
					OFFSETOF_XCPT_ACK, "OFFSETOF_XCPT_ACK");

	_Static_assert (offsetof(struct thinkos_except, control) == 
					OFFSETOF_XCPT_CONTROL, "OFFSETOF_XCPT_CONTROL");

	_Static_assert (offsetof(struct thinkos_except, ctx) == 
					OFFSETOF_XCPT_CONTEXT, "OFFSETOF_XCPT_CONTEXT");

	_Static_assert (offsetof(struct thinkos_except, msp) == 
					OFFSETOF_XCPT_MSP, "OFFSETOF_XCPT_MSP");

	_Static_assert (offsetof(struct thinkos_except, psp) == 
					OFFSETOF_XCPT_PSP, "OFFSETOF_XCPT_PSP");

	_Static_assert (offsetof(struct thinkos_except, sched) == 
					OFFSETOF_XCPT_SCHED, "OFFSETOF_XCPT_SCHED");

	_Static_assert (offsetof(struct thinkos_except, ready) == 
					OFFSETOF_XCPT_READY, "OFFSETOF_XCPT_READY");

	_Static_assert (offsetof(struct thinkos_except, cfsr) == 
					OFFSETOF_XCPT_CFSR, "OFFSETOF_XCPT_CFSR");

	_Static_assert (offsetof(struct thinkos_except, hfsr) == 
					OFFSETOF_XCPT_HFSR, "OFFSETOF_XCPT_HFSR");

	_Static_assert (offsetof(struct thinkos_except, mmfar) == 
					OFFSETOF_XCPT_MMFAR, "OFFSETOF_XCPT_MMFAR");

	_Static_assert (offsetof(struct thinkos_except, bfar) == 
					OFFSETOF_XCPT_BFAR, "OFFSETOF_XCPT_BFAR");

	_Static_assert (offsetof(struct thinkos_except, icsr) == 
					OFFSETOF_XCPT_ICSR, "OFFSETOF_XCPT_ICSR");

	_Static_assert (offsetof(struct thinkos_except, shcsr) == 
					OFFSETOF_XCPT_SHCSR, "OFFSETOF_XCPT_SHCSR");

#if (THINKOS_ENABLE_PROFILING)
	_Static_assert (offsetof(struct thinkos_except, cycref) == 
					OFFSETOF_XCPT_CYCREF, "OFFSETOF_XCPT_CYCREF");

	_Static_assert (offsetof(struct thinkos_except, cyccnt) == 
					OFFSETOF_XCPT_CYCCNT, "OFFSETOF_XCPT_CYCCNT");
#endif


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
  #define DCC_EXCEPT_DUMP(KRN, XCPT) __xdump(KRN, XCPT)
#else
  #define DCC_EXCEPT_DUMP(KRN, XCPT)
#endif

#include <sys/dcclog.h>

void thinkos_krn_fatal_except(uint32_t errno, struct thinkos_except * xcpt) 
{
#if DEBUG
	struct thinkos_rt * krn = &thinkos_rt;

	mdelay(500);

	DCC_LOG1(LOG_PANIC, VT_PSH VT_REV VT_FRD
			 "  Fatal exception: %d  " VT_POP, errno);

	mdelay(250);

	__xinfo(xcpt);


	__pdump();

	DCC_EXCEPT_DUMP(krn, xcpt);

	mdelay(500);

	__tdump(krn);

	/* kill all threads */
	__thinkos_krn_core_reset(krn);

	/* Enable Interrupts */
	DCC_LOG(LOG_TRACE, "5. enablig interrupts...");
	cm3_cpsie_i();

	/* signal the monitor */
	monitor_signal(MONITOR_KRN_FAULT);
#endif
#if (THINKOS_SYSRST_ONFAULT)
//	thinkos_krn_sysrst();
#endif
}

void thinkos_krn_thread_except(uint32_t errno, struct thinkos_except * xcpt) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	int thread = xcpt->sched.active;

	DCC_LOG2(LOG_WARNING, VT_PSH VT_REV VT_FYW
			 " Thread fault %d, thread %d " VT_POP, 
			 errno, thread);
	(void)krn;
	(void)thread;

#if DEBUG
	mdelay(250);

	__xinfo(xcpt);

	DCC_EXCEPT_DUMP(krn, xcpt);

	mdelay(250);

	__tdump(krn);
#endif

#if (THINKOS_ENABLE_MONITOR) 
	DCC_LOG1(LOG_WARNING, VT_PSH VT_FMG VT_REV "    %s" VT_POP, 
			__thread_tag_get(krn, thread));

	__nvic_irq_disable_all();

#if (THINKOS_ENABLE_READY_MASK)
	__thread_disble_all(krn);
#endif

//	__thread_fault_raise(krn, thread, errno);

	/* Enable Interrupts */
	cm3_cpsie_i();

	monitor_signal_break(MONITOR_THREAD_FAULT);
#else
	__krn_suspend_all(krn);
#endif
}


/* -------------------------------------------------------------------------
   Application fault defered handler 
   ------------------------------------------------------------------------- */
void thinkos_krn_exception_reset(void)
{
	DCC_LOG(LOG_TRACE, "Exception buffer clear.");

#if (THINKOS_ENABLE_STACK_INIT)
	/* initialize thread stack */
	__thinkos_memset32(thinkos_except_stack, 0xdeadbeef, 
					   sizeof(struct thinkos_except));
#elif (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(thinkos_except_stack, 0, 
					   sizeof(struct thinkos_except));
#endif
}

void thinkos_krn_exception_init(void)
{
	struct cm3_scb * scb = CM3_SCB;

	thinkos_krn_exception_reset();

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

#if (THINKOS_SYSRST_ONFAULT)
void __attribute__((naked, noreturn)) cm3_hard_fault_isr(void)
{
	thinkos_krn_sysrst();
}
#endif

void __attribute__((naked, noreturn)) thinkos_krn_xcpt_raise(int errno)
{
	monitor_signal(MONITOR_THREAD_FAULT);
#if (THINKOS_SYSRST_ONFAULT)
	thinkos_krn_sysrst();
#endif
}

#endif /* THINKOS_ENABLE_EXCEPTIONS */

