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
	struct cm3_scb * scb = CM3_SCB;
	uint32_t hfsr;

	mdelay(500);

	hfsr = scb->hfsr;

	DCC_LOG3(LOG_PANIC, "Hard fault:%s%s%s", 
			 (hfsr & SCB_HFSR_DEBUGEVT) ? " DEBUGEVT" : "",
			 (hfsr & SCB_HFSR_FORCED) ?  " FORCED" : "",
			 (hfsr & SCB_HFSR_VECTTBL) ? " VECTTBL" : "");
	DCC_LOG1(LOG_PANIC, " HFSR=%08x", scb->hfsr);
	DCC_LOG1(LOG_PANIC, " CFSR=%08x", xcpt->cfsr);
	DCC_LOG1(LOG_PANIC, " BFAR=%08x", xcpt->bfar);
	DCC_LOG1(LOG_PANIC, "MMFAR=%08x", xcpt->mmfar);

	if (hfsr & SCB_HFSR_FORCED) {
		uint32_t mmfsr;
		uint32_t bfsr;
		uint32_t ufsr;

		bfsr = SCB_CFSR_BFSR_GET(xcpt->cfsr);
		ufsr = SCB_CFSR_UFSR_GET(xcpt->cfsr);
		mmfsr = SCB_CFSR_MMFSR_GET(xcpt->cfsr);
		(void)bfsr;
		(void)ufsr;
		(void)mmfsr ;

		DCC_LOG1(LOG_PANIC, "BFSR=%08X", bfsr);
		if (bfsr) {
			DCC_LOG7(LOG_PANIC, "    %s%s%s%s%s%s%s", 
					 (bfsr & BFSR_BFARVALID) ? " BFARVALID" : "",
					 (bfsr & BFSR_LSPERR) ? " LSPERR" : "",
					 (bfsr & BFSR_STKERR) ? " STKERR" : "",
					 (bfsr & BFSR_UNSTKERR) ?  " UNSTKERR" : "",
					 (bfsr & BFSR_IMPRECISERR) ?  " IMPRECISERR" : "",
					 (bfsr & BFSR_PRECISERR) ?  " PRECISERR" : "",
					 (bfsr & BFSR_IBUSERR)  ?  " IBUSERR" : "");
		}

		DCC_LOG1(LOG_PANIC, "UFSR=%08X", ufsr);
		if (ufsr) {
			DCC_LOG6(LOG_PANIC, "    %s%s%s%s%s%s", 
					 (ufsr & UFSR_DIVBYZERO)  ? " DIVBYZERO" : "",
					 (ufsr & UFSR_UNALIGNED)  ? " UNALIGNED" : "",
					 (ufsr & UFSR_NOCP)  ? " NOCP" : "",
					 (ufsr & UFSR_INVPC)  ? " INVPC" : "",
					 (ufsr & UFSR_INVSTATE)  ? " INVSTATE" : "",
					 (ufsr & UFSR_UNDEFINSTR)  ? " UNDEFINSTR" : "");
		}
		DCC_LOG1(LOG_PANIC, "MMFSR=%08X", mmfsr);
		if (mmfsr) {
			DCC_LOG6(LOG_PANIC, "    %s%s%s%s%s%s", 
					 (mmfsr & MMFSR_MMARVALID)  ? " MMARVALID" : "",
					 (mmfsr & MMFSR_MLSPERR)  ? " MLSPERR" : "",
					 (mmfsr & MMFSR_MSTKERR)  ? " MSTKERR" : "",
					 (mmfsr & MMFSR_MUNSTKERR)  ? " MUNSTKERR" : "",
					 (mmfsr & MMFSR_DACCVIOL)  ? " DACCVIOL" : "",
					 (mmfsr & MMFSR_IACCVIOL)  ? " IACCVIOL" : "");
		}
	}

	__pdump();

	DCC_EXCEPT_DUMP(xcpt);

	mdelay(500);

	__tdump();

	/* kill all threads */
	__thinkos_core_reset();

#endif
#if (THINKOS_SYSRST_ONFAULT)
	cm3_sysrst();
#endif
	for(;;);
}

void thinkos_krn_thread_except(struct thinkos_except * xcpt)
{
	mdelay(250);

	__xinfo(xcpt);

	__pdump();

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
	struct thinkos_except * xcpt = __thinkos_except_buf();

	__thinkos_memset32(xcpt, 0x00000000,
					   sizeof(struct thinkos_except));
}

void thinkos_krn_exception_init(void)
{
	struct cm3_scb * scb = CM3_SCB;

	DCC_LOG(LOG_TRACE, "Initializing exceptions...");

	thinkos_krn_exception_reset();

#if	(THINKOS_ENABLE_USAGEFAULT) 
	cm3_except_pri_set(CM3_EXCEPT_USAGE_FAULT, EXCEPT_PRIORITY);
#endif
#if	(THINKOS_ENABLE_BUSFAULT)
	cm3_except_pri_set(CM3_EXCEPT_BUS_FAULT, EXCEPT_PRIORITY);
#endif
#if (THINKOS_ENABLE_MPU)
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
	cm3_sysrst();
}
#endif

#endif /* THINKOS_ENABLE_EXCEPTIONS */

const char thinkos_xcp_nm[] = "XCP";

