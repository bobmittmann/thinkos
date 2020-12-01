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
  #define DCC_EXCEPT_DUMP(XCPT) __xdump(XCPT)
#else
  #define DCC_EXCEPT_DUMP(XCPT)
#endif

#include <sys/dcclog.h>

void thinkos_krn_fatal_except(struct thinkos_except * xcpt, unsigned int errno)
{
#if DEBUG
	struct thinkos_rt * krn = &thinkos_rt;

	mdelay(500);

	DCC_LOG1(LOG_PANIC, VT_PSH VT_REV VT_FRD
			 "  Fatal exception: %d  " VT_POP, errno);

	mdelay(250);

	__xinfo(xcpt);


	__pdump();

	DCC_EXCEPT_DUMP(xcpt);

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

void thinkos_krn_thread_except(struct thinkos_except * xcpt, 
							   uint32_t errno, uint32_t thread_idx)
{
	struct thinkos_rt * krn = &thinkos_rt;

	DCC_LOG2(LOG_WARNING, VT_PSH VT_REV VT_FYW
			 " Thread fault %d, thread %d " VT_POP, 
			 errno, thread_idx);

	mdelay(250);

	__xinfo(xcpt);

	DCC_EXCEPT_DUMP(xcpt);

	mdelay(250);

	__tdump(krn);

	/* signal the monitor */
	monitor_signal_break(MONITOR_THREAD_FAULT);

#if (THINKOS_ENABLE_MONITOR) 
	DCC_LOG1(LOG_WARNING, VT_PSH VT_FMG VT_REV "    %s" VT_POP, 
			__thread_tag_get(krn, thread_idx));

	__nvic_irq_disable_all();

#if (THINKOS_ENABLE_READY_MASK)
	__thread_disble_all(krn);
#else
	__krn_suspend_all(krn);
#endif

	__thread_fault_raise(krn, thread_idx, errno);

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

