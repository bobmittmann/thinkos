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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arch/cortex-m3.h>
#include <sys/delay.h>
#include <vt100.h>

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

void __attribute__((noreturn)) 
	thinkos_krn_fatal_except(struct thinkos_except * xcpt)
{
#if DEBUG
	mdelay(500);

	DCC_LOG(LOG_PANIC, "Fatal exception");

	mdelay(250);

	__xinfo(xcpt);


	__pdump();

	DCC_EXCEPT_DUMP(xcpt);

	mdelay(500);

	__tdump();

	/* kill all threads */
	__thinkos_core_reset();

#endif
#if (THINKOS_SYSRST_ONFAULT)
	thinkos_krn_sysrst();
#endif
	for(;;);
}

void thinkos_krn_thread_except(struct thinkos_except * xcpt)
{
	mdelay(250);

	__xinfo(xcpt);

	DCC_EXCEPT_DUMP(xcpt);

	mdelay(250);

	__tdump();

	/* signal the monitor */
	monitor_signal(MONITOR_THREAD_FAULT);
}


/* -------------------------------------------------------------------------
   Application fault defered handler 
   ------------------------------------------------------------------------- */
void thinkos_krn_exception_reset(void)
{
	DCC_LOG(LOG_ERROR, "Exception buffer reset...");

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

#if	(THINKOS_ENABLE_USAGEFAULT) 
	DCC_LOG(LOG_TRACE, "Initializing usage fault...");
	cm3_except_pri_set(CM3_EXCEPT_USAGE_FAULT, EXCEPT_PRIORITY);
#endif
#if	(THINKOS_ENABLE_BUSFAULT)
	DCC_LOG(LOG_TRACE, "Initializing bus fault...");
	cm3_except_pri_set(CM3_EXCEPT_BUS_FAULT, EXCEPT_PRIORITY);
#endif
#if (THINKOS_ENABLE_MPU)
	DCC_LOG(LOG_TRACE, "Initializing memory management fault...");
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

const char thinkos_xcp_nm[] = "XCP";

