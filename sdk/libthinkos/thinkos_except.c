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
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>

#include <sys/stm32f.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arch/cortex-m3.h>
#include <sys/delay.h>

#include <sys/dcclog.h>

#if (THINKOS_ENABLE_EXCEPTIONS)

#if DEBUG
  #undef THINKOS_SYSRST_ONFAULT
  #define THINKOS_SYSRST_ONFAULT    0
  #define DCC_EXCEPT_DUMP(XCPT) __xdump(XCPT)
#else
  #define DCC_EXCEPT_DUMP(XCPT)
#endif

struct thinkos_except thinkos_except_buf 
	__attribute__((section(".heap"), aligned(8)));

#if (THINKOS_UNROLL_EXCEPTIONS) 

static void __xcpt_rettobase(struct thinkos_except * xcpt)
{
	int ipsr;

#if DEBUG
	__idump(__func__, cm3_ipsr_get());
	__tdump();
#endif

	ipsr = xcpt->ctx.core.xpsr & 0x1ff;
	if ((ipsr == 0) || (ipsr == CM3_EXCEPT_SVC)) {
		if ((xcpt->active > 0) && (xcpt->active <= THINKOS_THREADS_MAX)) {
#if THINKOS_ENABLE_DEBUG_FAULT
			/* flag the thread as faulty */
			__bit_mem_wr(&thinkos_rt.wq_fault, thinkos_rt.active, 1);
#endif
		} else {
			DCC_LOG(LOG_ERROR, "invalid active thread!");
		}
	} else {
		DCC_LOG(LOG_WARNING, "fault on exception!");
	}

	/* suspend all threads */
	__thinkos_pause_all();

	/* set the active thread to idle */
	thinkos_rt.active = THINKOS_THREAD_IDLE;

	/* Sanity check */
	if (thinkos_rt.wq_ready != 0) {
		DCC_LOG1(LOG_TRACE, "wq_ready=%08x, ready queue not empty!", 
				 thinkos_rt.wq_ready);
		thinkos_rt.wq_ready = 0;
	}

#if THINKOS_ENABLE_TIMESHARE
	if (thinkos_rt.wq_tmshare != 0) {
		DCC_LOG1(LOG_TRACE, "wq_tmshare=%08x, timeshare queue not empty!", 
				 thinkos_rt.wq_tmshare);
		thinkos_rt.wq_tmshare = 0;
	}
#endif

	/* The interrupts where disabled on exception entry.
	   Reenable interrupts */
	cm3_cpsie_i();

	thinkos_exception_dsr(xcpt);
}

static int __xcpt_active_irq(void)
{
	int irqregs;
	int irqbits;
	int irq;
	int i;
	int j;

	irqregs = (CM3_ICTR + 1);
	for (i = 0; i < irqregs ; ++i) {
		irqbits = __rbit(CM3_NVIC->iabr[i]);
		if ((j = __clz(irqbits)) < 32) {
			irq = i * 32 + j;
			return irq;
		}
	}

	return -16;
}

static void __attribute__((noreturn)) 
	__xcpt_unroll(struct thinkos_except * xcpt, uint32_t xpsr)
{
	struct armv7m_except_frame * sf;
	uint32_t xpsr_n;
	uint32_t shcsr;
	uint32_t icsr;
	uint32_t ret;
	uint32_t * sp;
	int ipsr;
	int irq;

	ipsr = cm3_ipsr_get();
#if DEBUG
	__idump(__func__, ipsr);
#endif

	/* increment reentry counter */
	if (++xcpt->unroll > 8) {
		DCC_LOG(LOG_ERROR, "too many reentries...");
#if THINKOS_SYSRST_ONFAULT
		cm3_sysrst();
#endif
		DCC_LOG(LOG_ERROR, "system halt!!");
		for(;;);
	}

	if ((irq = __xcpt_active_irq()) >= 0) {
		/* Return to the next active IRQ */
		xpsr_n = CM_EPSR_T + irq + 16;
		DCC_LOG1(LOG_TRACE, "IRQ %d", irq);
	} else if ((shcsr = CM3_SCB->shcsr) & (SCB_SHCSR_SYSTICKACT | 
										   SCB_SHCSR_PENDSVACT | 
										   SCB_SHCSR_MONITORACT | 
										   SCB_SHCSR_SVCALLACT |
										   SCB_SHCSR_USGFAULTACT |  
										   SCB_SHCSR_BUSFAULTACT |
										   SCB_SHCSR_MEMFAULTACT)) {

		if (ipsr == CM3_EXCEPT_BUS_FAULT && shcsr & SCB_SHCSR_BUSFAULTACT) {
			/* currently servicing bus fault */
			shcsr &= ~SCB_SHCSR_BUSFAULTACT;
		}
		if (ipsr == CM3_EXCEPT_USAGE_FAULT && shcsr & SCB_SHCSR_USGFAULTACT) {
			/* currently servicing usage fault */
			shcsr &= ~SCB_SHCSR_USGFAULTACT;
		}
		if (ipsr == CM3_EXCEPT_MEM_MANAGE && shcsr & SCB_SHCSR_MEMFAULTACT) {
			/* currently servicing memory management fault */
			shcsr &= ~SCB_SHCSR_MEMFAULTACT;
		}

		if (shcsr & SCB_SHCSR_MEMFAULTACT) {
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_MEM_MANAGE;
			DCC_LOG(LOG_TRACE, "MEM_MANAGE");
		} else if (shcsr & SCB_SHCSR_BUSFAULTACT) {
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_BUS_FAULT;
			DCC_LOG(LOG_TRACE, "BUS_FAULT");
		} else if (shcsr & SCB_SHCSR_USGFAULTACT) {
			DCC_LOG(LOG_TRACE, "USAGE_FAULT");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_USAGE_FAULT;
		} else if (shcsr & SCB_SHCSR_SVCALLACT) {
			DCC_LOG(LOG_TRACE, "SVC");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_SVC;
		} else if (shcsr & SCB_SHCSR_MONITORACT) {
			DCC_LOG(LOG_TRACE, "DEBUG_MONITOR");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_DEBUG_MONITOR;
		} else if (shcsr & SCB_SHCSR_PENDSVACT) {
			DCC_LOG(LOG_TRACE, "PENDSV");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_PENDSV;
		} else if (shcsr & SCB_SHCSR_SYSTICKACT) {
			DCC_LOG(LOG_TRACE, "SYSTICK");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_SYSTICK;
		} else {
			xpsr_n = CM_EPSR_T;
		}
	} else {
		xpsr_n = CM_EPSR_T;
	}

	/* Reset the exception stack */
	/* Get the top of the exception stack */
	sp = __thinkos_xcpt_stack_top();

	icsr = CM3_SCB->icsr;
	if (icsr & SCB_ICSR_RETTOBASE) {
		DCC_LOG(LOG_TRACE, "return to base...");
		/* reset the idle thread */
		__thinkos_idle_reset((void *)__xcpt_rettobase, (void *)xcpt);
#if (THINKOS_ENABLE_IDLE_MSP)
		cm3_msp_set((uint32_t)sp);
		ret = CM3_EXC_RET_THREAD_MSP;
#else
		cm3_psp_set((uint32_t)sp);
		ret = CM3_EXC_RET_THREAD_PSP;
#endif
		/* return */
		asm volatile ("bx   %0\n" : : "r" (ret)); 
		for(;;);
	} else {
		DCC_LOG(LOG_TRACE, "return to exception...");
		/* Make room for the exception frame */
		sp -= (sizeof(struct armv7m_except_frame) / sizeof(uint32_t)); 
		sf = (struct armv7m_except_frame *)sp;
		sf->r0 = (uint32_t)xcpt;
		sf->r1 = (uint32_t)xpsr_n;
		sf->xpsr = xpsr;
		sf->pc = (uint32_t)__xcpt_unroll;
		cm3_msp_set((uint32_t)sp);
		ret = CM3_EXC_RET_HANDLER;
		/* return */
		asm volatile ("bx   %0\n" : : "r" (ret)); 
		for(;;);
	} 
}

#if 0
void __attribute__((noreturn,noinline)) 
__xcpt_unroll(void (* base)(void *), void  * arg, 
			  uint32_t xpsr, uint32_t cnt)
{
	struct cm3_except_context * sf;
	uint32_t xpsr_n;
	uint32_t shcsr;
	uint32_t icsr;
	uint32_t ret;
	uint32_t * sp;
	int ipsr;
	int irq;

	ipsr = cm3_ipsr_get();
#if DEBUG
	__idump(__func__, ipsr);
#endif

	if ((irq = __xcpt_active_irq()) >= 0) {
		/* Return to the next active IRQ */
		xpsr_n = CM_EPSR_T + irq + 16;
		DCC_LOG1(LOG_TRACE, "IRQ %d", irq);
	} else if ((shcsr = CM3_SCB->shcsr) & (SCB_SHCSR_SYSTICKACT | 
										   SCB_SHCSR_PENDSVACT | 
										   SCB_SHCSR_MONITORACT | 
										   SCB_SHCSR_SVCALLACT |
										   SCB_SHCSR_USGFAULTACT |  
										   SCB_SHCSR_BUSFAULTACT |
										   SCB_SHCSR_MEMFAULTACT)) {

		if (ipsr == CM3_EXCEPT_BUS_FAULT && shcsr & SCB_SHCSR_BUSFAULTACT) {
			/* currently servicing bus fault */
			shcsr &= ~SCB_SHCSR_BUSFAULTACT;
		}
		if (ipsr == CM3_EXCEPT_USAGE_FAULT && shcsr & SCB_SHCSR_USGFAULTACT) {
			/* currently servicing usage fault */
			shcsr &= ~SCB_SHCSR_USGFAULTACT;
		}
		if (ipsr == CM3_EXCEPT_MEM_MANAGE && shcsr & SCB_SHCSR_MEMFAULTACT) {
			/* currently servicing memory management fault */
			shcsr &= ~SCB_SHCSR_MEMFAULTACT;
		}

		if (shcsr & SCB_SHCSR_MEMFAULTACT) {
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_MEM_MANAGE;
			DCC_LOG(LOG_TRACE, "MEM_MANAGE");
		} else if (shcsr & SCB_SHCSR_BUSFAULTACT) {
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_BUS_FAULT;
			DCC_LOG(LOG_TRACE, "BUS_FAULT");
		} else if (shcsr & SCB_SHCSR_USGFAULTACT) {
			DCC_LOG(LOG_TRACE, "USAGE_FAULT");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_USAGE_FAULT;
		} else if (shcsr & SCB_SHCSR_SVCALLACT) {
			DCC_LOG(LOG_TRACE, "SVC");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_SVC;
		} else if (shcsr & SCB_SHCSR_MONITORACT) {
			DCC_LOG(LOG_TRACE, "DEBUG_MONITOR");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_DEBUG_MONITOR;
		} else if (shcsr & SCB_SHCSR_PENDSVACT) {
			DCC_LOG(LOG_TRACE, "PENDSV");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_PENDSV;
		} else if (shcsr & SCB_SHCSR_SYSTICKACT) {
			DCC_LOG(LOG_TRACE, "SYSTICK");
			xpsr_n = CM_EPSR_T + CM3_EXCEPT_SYSTICK;
		} else {
			xpsr_n = CM_EPSR_T;
		}
	} else {
		xpsr_n = CM_EPSR_T;
	}

	/* Reset the exception stack */
	/* Get the top of the exception stack */
	sp = __thinkos_xcpt_stack_top();

	icsr = CM3_SCB->icsr;
	if (icsr & SCB_ICSR_RETTOBASE) {
		DCC_LOG(LOG_TRACE, "return to base...");
		/* reset the idle thread */
		__thinkos_idle_reset(base, arg);
		cm3_msp_set((uint32_t)sp);
#if THINKOS_ENABLE_FPU 
		ret = CM3_EXC_RET_THREAD_MSP_EXT;
#else
		ret = CM3_EXC_RET_THREAD_MSP;
#endif
		/* return */
		asm volatile ("bx   %0\n" : : "r" (ret)); 
		for(;;);
	} else {
		DCC_LOG(LOG_TRACE, "return to exception...");
		/* Make room for the exception frame */
		sp -= (sizeof(struct cm3_except_context) / sizeof(uint32_t)); 
		sf = (struct cm3_except_context *)sp;
		sf->r0 = (uint32_t)base;
		sf->r1 = (uint32_t)arg;
		sf->r2 = (uint32_t)xpsr_n;
		sf->r3 = (uint32_t)cnt + 1;
		sf->xpsr = xpsr;
		sf->pc = (uint32_t)__xcpt_unroll & ~1;
		cm3_msp_set((uint32_t)sp);
		ret = CM3_EXC_RET_HANDLER;
		/* return */
		asm volatile ("bx   %0\n" : : "r" (ret)); 
		for(;;);
	} 

}
#endif
#endif /* THINKOS_UNROLL_EXCEPTIONS */


#if (THINKOS_UNROLL_EXCEPTIONS) 
static inline uint32_t __attribute__((always_inline)) cm3_xpsr_get(void) {
	uint32_t ipsr;
	asm volatile ("mrs %0, IPSR\n" : "=r" (ipsr));
	return ipsr;
}

void __attribute__((noreturn)) thinkos_xcpt_return(void)
{
	uint32_t xpsr = cm3_xpsr_get();
	DCC_LOG(LOG_TRACE, "...");
	/* set the active thread to idle */
	thinkos_rt.active = THINKOS_THREAD_IDLE;
	/* Unroll exception chain */
	__xcpt_unroll(NULL, xpsr);
}
#endif

#if THINKOS_STDERR_FAULT_DUMP
void __show_ctrl(uint32_t ctrl)
{
	fprintf(stderr, "[%s ", (ctrl & (1 << 25)) ? "PSP" : "MSP");
	fprintf(stderr, "%s ", (ctrl & (1 << 24)) ? "USER" : "PRIV");
	fprintf(stderr, "PM=%c ", ((ctrl >> 0) & 0x01) + '0');
	fprintf(stderr, "FM=%c ", ((ctrl >> 16) & 0x01) + '0');
	fprintf(stderr, "BPRI=%02x] ", (ctrl >> 8) & 0xff);
}

static void __show_xpsr(uint32_t psr)
{
	fprintf(stderr, "[N=%c ", ((psr >> 31) & 0x01) + '0');
	fprintf(stderr, "Z=%c ", ((psr >> 30) & 0x01) + '0');
	fprintf(stderr, "C=%c ", ((psr >> 29) & 0x01) + '0');
	fprintf(stderr, "V=%c ", ((psr >> 28) & 0x01) + '0');
	fprintf(stderr, "Q=%c ", ((psr >> 27) & 0x01) + '0');
	fprintf(stderr, "ICI/IT=%02x ", ((psr >> 19) & 0xc0) | 
			((psr >> 10) & 0x3f));
	fprintf(stderr, "XCP=%02x]", psr & 0xff);
}

void print_except_context(struct thinkos_except * xcpt)
{
	uint32_t sp = (xcpt->ret & CM3_EXEC_RET_SPSEL) ? xcpt->psp : xcpt->msp;

	__show_xpsr(xcpt->ctx.xpsr);

	fprintf(stderr, "\n");

	fprintf(stderr, "   r0=%08x", xcpt->ctx.r0);
	fprintf(stderr, "   r4=%08x", xcpt->ctx.r4);
	fprintf(stderr, "   r8=%08x", xcpt->ctx.r8);
	fprintf(stderr, "  r12=%08x", xcpt->ctx.r12);
	fprintf(stderr, " xpsr=%08x\n", xcpt->ctx.xpsr);

	fprintf(stderr, "   r1=%08x", xcpt->ctx.r1);
	fprintf(stderr, "   r5=%08x", xcpt->ctx.r5);
	fprintf(stderr, "   r9=%08x", xcpt->ctx.r9);
	fprintf(stderr, "   sp=%08x", sp);
	fprintf(stderr, "  msp=%08x\n", xcpt->msp);

	fprintf(stderr, "   r2=%08x", xcpt->ctx.r2);
	fprintf(stderr, "   r6=%08x", xcpt->ctx.r6);
	fprintf(stderr, "  r10=%08x", xcpt->ctx.r10);
	fprintf(stderr, "   lr=%08x",  xcpt->ctx.lr);
	fprintf(stderr, "  psp=%08x\n", xcpt->psp);

	fprintf(stderr, "   r3=%08x",  xcpt->ctx.r3);
	fprintf(stderr, "   r7=%08x",  xcpt->ctx.r7);
	fprintf(stderr, "  r11=%08x",  xcpt->ctx.r11);
	fprintf(stderr, "   pc=%08x\n",  xcpt->ctx.pc);
}

static void __dump_bfsr(uint32_t cfsr)
{
	uint32_t bfsr;

	bfsr = SCB_CFSR_BFSR_GET(cfsr);

	fprintf(stderr, "BFSR=0X%08x", bfsr);

	if (bfsr & BFSR_BFARVALID)  
		fprintf(stderr, " BFARVALID");
	if (bfsr & BFSR_LSPERR)
		fprintf(stderr, " LSPERR");
	if (bfsr & BFSR_STKERR)  
		fprintf(stderr, " STKERR");
	if (bfsr & BFSR_UNSTKERR)  
		fprintf(stderr, " UNSTKERR");
	if (bfsr & BFSR_IMPRECISERR)  
		fprintf(stderr, " IMPRECISERR");
	if (bfsr & BFSR_PRECISERR)
		fprintf(stderr, " PRECISERR");
	if (bfsr & BFSR_IBUSERR)  
		fprintf(stderr, " IBUSERR");

	if (bfsr & BFSR_BFARVALID)  {
		fprintf(stderr, "\n * ADDR = 0x%08x", (int)scb->bfar);
	}
}

static void __dump_ufsr(uint32_t cfsr)
{
	uint32_t ufsr;

	ufsr = SCB_CFSR_UFSR_GET(cfsr);

	fprintf(stderr, "UFSR=0x%08x", ufsr);

	if (ufsr & UFSR_DIVBYZERO)  
		fprintf(stderr, " DIVBYZERO");
	if (ufsr & UFSR_UNALIGNED)  
		fprintf(stderr, " UNALIGNED");
	if (ufsr & UFSR_NOCP)  
		fprintf(stderr, " NOCP");
	if (ufsr & UFSR_INVPC)  
		fprintf(stderr, " INVPC");
	if (ufsr & UFSR_INVSTATE)  
		fprintf(stderr, " INVSTATE");
	if (ufsr & UFSR_UNDEFINSTR)  
		fprintf(stderr, " UNDEFINSTR");
}
#endif

/* -------------------------------------------------------------------------
   Fault handlers 
   ------------------------------------------------------------------------- */

void __attribute__((noreturn)) thinkos_xcpt_process(struct thinkos_except * 
													xcpt)
{
	DCC_LOG(LOG_TRACE, "...");
#if (THINKOS_UNROLL_EXCEPTIONS) 
	/* Disable Iterrutps */
	cm3_cpsid_i();
	/* Unroll exception chain */
	__xcpt_unroll(xcpt, xcpt->ctx.core.xpsr);
#else /* THINKOS_UNROLL_EXCEPTIONS */
	/* increment reentry counter */
	xcpt->unroll++;
	/* call exception handler */
	thinkos_exception_dsr(xcpt);
  #if THINKOS_SYSRST_ONFAULT
	DCC_LOG(LOG_WARNING, "system reset...");
	cm3_sysrst();
  #endif
	DCC_LOG(LOG_ERROR, "system halt!!");
#endif /* THINKOS_UNROLL_EXCEPTIONS */
	for(;;);
}

void __attribute__((noreturn)) thinkos_hard_fault(struct thinkos_except * xcpt)
{
#if DEBUG
	struct cm3_scb * scb = CM3_SCB;
	uint32_t hfsr;

	hfsr = scb->hfsr;

	DCC_LOG3(LOG_ERROR, "Hard fault:%s%s%s", 
			 (hfsr & SCB_HFSR_DEBUGEVT) ? " DEBUGEVT" : "",
			 (hfsr & SCB_HFSR_FORCED) ?  " FORCED" : "",
			 (hfsr & SCB_HFSR_VECTTBL) ? " VECTTBL" : "");
	DCC_EXCEPT_DUMP(xcpt);
	DCC_LOG1(LOG_ERROR, " HFSR=%08x", scb->hfsr);
	DCC_LOG1(LOG_ERROR, " CFSR=%08x", xcpt->cfsr);
	DCC_LOG1(LOG_ERROR, " BFAR=%08x", xcpt->bfar);
	DCC_LOG1(LOG_ERROR, "MMFAR=%08x", xcpt->mmfar);

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

		DCC_LOG1(LOG_ERROR, "BFSR=%08X", bfsr);
		if (bfsr) {
			DCC_LOG7(LOG_ERROR, "    %s%s%s%s%s%s%s", 
					 (bfsr & BFSR_BFARVALID) ? " BFARVALID" : "",
					 (bfsr & BFSR_LSPERR) ? " LSPERR" : "",
					 (bfsr & BFSR_STKERR) ? " STKERR" : "",
					 (bfsr & BFSR_UNSTKERR) ?  " UNSTKERR" : "",
					 (bfsr & BFSR_IMPRECISERR) ?  " IMPRECISERR" : "",
					 (bfsr & BFSR_PRECISERR) ?  " PRECISERR" : "",
					 (bfsr & BFSR_IBUSERR)  ?  " IBUSERR" : "");
		}

		DCC_LOG1(LOG_ERROR, "UFSR=%08X", ufsr);
		if (ufsr) {
			DCC_LOG6(LOG_ERROR, "    %s%s%s%s%s%s", 
					 (ufsr & UFSR_DIVBYZERO)  ? " DIVBYZERO" : "",
					 (ufsr & UFSR_UNALIGNED)  ? " UNALIGNED" : "",
					 (ufsr & UFSR_NOCP)  ? " NOCP" : "",
					 (ufsr & UFSR_INVPC)  ? " INVPC" : "",
					 (ufsr & UFSR_INVSTATE)  ? " INVSTATE" : "",
					 (ufsr & UFSR_UNDEFINSTR)  ? " UNDEFINSTR" : "");
		}
		DCC_LOG1(LOG_ERROR, "MMFSR=%08X", mmfsr);
		if (mmfsr) {
			DCC_LOG6(LOG_ERROR, "    %s%s%s%s%s%s", 
					 (mmfsr & MMFSR_MMARVALID)  ? " MMARVALID" : "",
					 (mmfsr & MMFSR_MLSPERR)  ? " MLSPERR" : "",
					 (mmfsr & MMFSR_MSTKERR)  ? " MSTKERR" : "",
					 (mmfsr & MMFSR_MUNSTKERR)  ? " MUNSTKERR" : "",
					 (mmfsr & MMFSR_DACCVIOL)  ? " DACCVIOL" : "",
					 (mmfsr & MMFSR_IACCVIOL)  ? " IACCVIOL" : "");
		}
	}
#endif

#if THINKOS_STDERR_FAULT_DUMP
	fprintf(stderr, "\n---\n");
	fprintf(stderr, "Hard fault:");

	if (hfsr & SCB_HFSR_DEBUGEVT)  
		fprintf(stderr, " DEBUGEVT");
	if (hfsr & SCB_HFSR_FORCED)  
		fprintf(stderr, " FORCED");
	if (hfsr & SCB_HFSR_VECTTBL)  
		fprintf(stderr, " VECTTBL");

	if (hfsr & SCB_HFSR_DEBUGEVT)  
		fprintf(stderr, " DEBUGEVT");

	fprintf(stderr, "\n");

	if (hfsr & SCB_HFSR_FORCED) {
		__dump_bfsr(xcpt->cfsr);
		fprintf(stderr, "\n");
		__dump_ufsr();
		fprintf(stderr, "\n");
	}

	print_except_context(xcpt);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif

	if (xcpt->unroll) {
		DCC_LOG(LOG_ERROR, "unhandled exception ...");
#if THINKOS_SYSRST_ONFAULT
		cm3_sysrst();
#endif
		DCC_LOG(LOG_ERROR, "system halt!!");
		for(;;);
	}

	thinkos_xcpt_process(xcpt);
}

#if	THINKOS_ENABLE_BUSFAULT 
void __attribute__((noreturn)) thinkos_bus_fault(struct thinkos_except * xcpt)
{
#if DEBUG
	uint32_t bfsr = SCB_CFSR_BFSR_GET(xcpt->cfsr);
	DCC_LOG(LOG_ERROR, "!!! Bus fault !!!");
	DCC_LOG2(LOG_ERROR, "BFSR=%08X BFAR=%08x", bfsr, xcpt->bfar);
	if (bfsr) {
		DCC_LOG7(LOG_ERROR, "BFSR={%s%s%s%s%s%s%s }", 
				 (bfsr & BFSR_BFARVALID) ? " BFARVALID" : "",
				 (bfsr & BFSR_LSPERR) ? " LSPERR" : "",
				 (bfsr & BFSR_STKERR) ? " STKERR" : "",
				 (bfsr & BFSR_UNSTKERR) ?  " UNSTKERR" : "",
				 (bfsr & BFSR_IMPRECISERR) ?  " IMPRECISERR" : "",
				 (bfsr & BFSR_PRECISERR) ?  " PRECISERR" : "",
				 (bfsr & BFSR_IBUSERR)  ?  " IBUSERR" : "");
	}
#endif
	DCC_EXCEPT_DUMP(xcpt);

#if THINKOS_STDERR_FAULT_DUMP
	fprintf(stderr, "\n---\n");
	fprintf(stderr, "Bus fault:");

	__dump_bfsr(xcpt->cfsr);

	print_except_context(xcpt);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif

	thinkos_xcpt_process(xcpt);
}
#endif /* THINKOS_ENABLE_BUSFAULT  */

#if	THINKOS_ENABLE_USAGEFAULT 
void __attribute__((noreturn)) thinkos_usage_fault(struct thinkos_except * xcpt)
{
#if DEBUG
	uint32_t ufsr = SCB_CFSR_UFSR_GET(xcpt->cfsr);

	DCC_LOG(LOG_ERROR, "!!! Usage fault !!!");
	DCC_LOG1(LOG_ERROR, "UFSR=%08X", ufsr);
	if (ufsr) {
		DCC_LOG6(LOG_ERROR, "    %s%s%s%s%s%s", 
				 (ufsr & UFSR_DIVBYZERO)  ? " DIVBYZERO" : "",
				 (ufsr & UFSR_UNALIGNED)  ? " UNALIGNED" : "",
				 (ufsr & UFSR_NOCP)  ? " NOCP" : "",
				 (ufsr & UFSR_INVPC)  ? " INVPC" : "",
				 (ufsr & UFSR_INVSTATE)  ? " INVSTATE" : "",
				 (ufsr & UFSR_UNDEFINSTR)  ? " UNDEFINSTR" : "");
	}
#endif

	DCC_EXCEPT_DUMP(xcpt);

#if THINKOS_STDERR_FAULT_DUMP
	fprintf(stderr, "\n---\n");
	fprintf(stderr, "Usage fault:");

	__dump_ufsr();

	print_except_context(xcpt);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif

	thinkos_xcpt_process(xcpt);
}
#endif /* THINKOS_ENABLE_USAGEFAULT  */

#if THINKOS_ENABLE_MEMFAULT
void __attribute__((noreturn)) thinkos_mem_manage(struct thinkos_except * xcpt)
{
#if DEBUG
	uint32_t mmfsr = SCB_CFSR_MMFSR_GET(xcpt->cfsr);
	uint32_t mmfar = xcpt->mmfar;
	DCC_LOG(LOG_ERROR, "!!! Mem Management !!!");
	DCC_LOG2(LOG_ERROR, "MMFSR=%08X MMFAR=%08x", mmfsr, xcpt->mmfar);
	if (mmfsr) {
		DCC_LOG6(LOG_ERROR, "    %s%s%s%s%s%s", 
				 (mmfsr & MMFSR_MMARVALID)  ? " MMARVALID" : "",
				 (mmfsr & MMFSR_MLSPERR)  ? " MLSPERR" : "",
				 (mmfsr & MMFSR_MSTKERR)  ? " MSTKERR" : "",
				 (mmfsr & MMFSR_MUNSTKERR)  ? " MUNSTKERR" : "",
				 (mmfsr & MMFSR_DACCVIOL)  ? " DACCVIOL" : "",
				 (mmfsr & MMFSR_IACCVIOL)  ? " IACCVIOL" : "");
	}
	if ((mmfsr & MMFSR_MMARVALID) && (mmfar == 0))
		DCC_LOG(LOG_ERROR, "Null pointer!!!");
#endif

	DCC_EXCEPT_DUMP(xcpt);

#if THINKOS_STDERR_FAULT_DUMP
	fprintf(stderr, "\n---\n");
	fprintf(stderr, "Mem Manager:");
	print_except_context(xcpt);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif

	thinkos_xcpt_process(xcpt);
}
#endif

/* -------------------------------------------------------------------------
   Application fault defered handler 
   ------------------------------------------------------------------------- */

void thinkos_default_exception_dsr(struct thinkos_except * xcpt)
{
}

void thinkos_exception_dsr(struct thinkos_except *) 
	__attribute__((weak, alias("thinkos_default_exception_dsr")));

struct thinkos_except * __thinkos_except_buf(void)
{
	return &thinkos_except_buf;
}

void __exception_reset(void)
{
	DCC_LOG(LOG_INFO, "clearing exception buffer!");
#if THINKOS_ENABLE_EXCEPT_CLEAR
	__thinkos_memset32(&thinkos_except_buf, 0x00000000,
					   sizeof(struct thinkos_except));
#else
	thinkos_except_buf.ipsr = 0;
	thinkos_except_buf.type = 0;
	thinkos_except_buf.unroll = 0;
#endif
	DCC_LOG(LOG_INFO, "/!\\ clearing active thread in exception buffer!");
	thinkos_except_buf.active = -1;
}

void thinkos_exception_init(void)
{
	struct cm3_scb * scb = CM3_SCB;

	scb->shcsr = 0
#if	THINKOS_ENABLE_USAGEFAULT 
		| SCB_SHCSR_USGFAULTENA 
#endif
#if	THINKOS_ENABLE_BUSFAULT 
		| SCB_SHCSR_BUSFAULTENA
#endif
#if THINKOS_ENABLE_MEMFAULT
		| SCB_SHCSR_MEMFAULTENA
#endif
		;

	__exception_reset();
}

const char thinkos_xcp_nm[] = "XCP";

#else /* THINKOS_ENABLE_EXCEPTIONS */

#if THINKOS_SYSRST_ONFAULT
void __attribute__((naked, noreturn)) cm3_hard_fault_isr(void)
{
	cm3_sysrst();
}
#endif

#endif /* THINKOS_ENABLE_EXCEPTIONS */

