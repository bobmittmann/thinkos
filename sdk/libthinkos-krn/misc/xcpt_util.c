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
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#include <thinkos.h>
#include <sys/delay.h>
#include <vt100.h>

#if (DEBUG)
  #ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_INFO
  #elif LOG_LEVEL < LOG_INFO
    #undef LOG_LEVEL 
    #define LOG_LEVEL LOG_INFO
  #endif
#endif

#include <sys/dcclog.h>

const char __xcpt_name_lut[16][12] = {
	"Thread",
	"Reset",
	"NMI",
	"HardFault",
	"MemManage",
	"BusFault",
	"UsageFault",
	"Invalid 7",
	"Invalid 8",
	"Invalid 9",
	"Invalid 10",
	"SVCall",
	"DbgMonitor",
	"Invalid 13",
	"PendSV",
	"SysTick"
};

int __scan_stack(void * stack, unsigned int size)
{
	uint32_t * ptr = (uint32_t *)stack;
	unsigned int i;

	for (i = 0; i < size / 4; ++i) {
		if (ptr[i] != 0xdeadbeef)
			break;
	}

	return i * 4;
}

int __scan_stack(void * stack, unsigned int size);
extern uint32_t thinkos_monitor_stack[];
extern const uint16_t thinkos_monitor_stack_size;
extern const uint16_t thinkos_except_stack_size;

/* Exception state dump */
void __xdump(struct thinkos_except * xcpt)
{
#if defined(ENABLE_LOG) && (LOG_LEVEL >= LOG_PANIC)
	uint32_t shcsr;
	uint32_t icsr;
	uint32_t ipsr;
	uint32_t xpsr;
	uint32_t ctrl;
	uint32_t ret;
	uint32_t sp;
	int irqregs;
	int irqbits;
	int i;
	int j;


	ipsr = xcpt->ipsr;
	if (ipsr < 16) { 
		DCC_LOG2(LOG_ERROR, " IPSR=%d (%s)", ipsr, __xcpt_name_lut[ipsr]);
	} else {
		DCC_LOG2(LOG_ERROR, " IPSR=%d (IRQ %d)", ipsr, ipsr - 16);
	}

	irqregs = CM3_ICTR + 1;
	for (i = 0; i < irqregs ; ++i) {
		irqbits = __rbit(CM3_NVIC->iabr[i]);
		while ((j = __clz(irqbits)) < 32) {
			irqbits &= ~(0x80000000 >> j);
			DCC_LOG1(LOG_ERROR, "Active IRQ=%d", i * 32 + j); 
		}
	}

	for (i = 0; i < irqregs ; ++i) {
		irqbits = __rbit(CM3_NVIC->ispr[i]);
		while ((j = __clz(irqbits)) < 32) {
			irqbits &= ~(0x80000000 >> j);
			DCC_LOG1(LOG_ERROR, "Pending IRQ=%d", i * 32 + j); 
		}
	}

	ret  = 0xffffff00 | (xcpt->ret & 0xff);

	sp = (ret & CM3_EXC_RET_SPSEL) ? xcpt->psp : xcpt->msp;
	DCC_LOG1(LOG_ERROR, "ret=%08x", ret); 

	DCC_LOG4(LOG_ERROR, "   R0=%08x  R1=%08x  R2=%08x  R3=%08x", 
			xcpt->ctx.core.r0, xcpt->ctx.core.r1, 
			xcpt->ctx.core.r2, xcpt->ctx.core.r3);
	DCC_LOG4(LOG_ERROR, "   R4=%08x  R5=%08x  R6=%08x  R7=%08x", 
			xcpt->ctx.core.r4, xcpt->ctx.core.r5, 
			xcpt->ctx.core.r6, xcpt->ctx.core.r7);
	DCC_LOG4(LOG_ERROR, "   R8=%08x  R9=%08x R10=%08x R11=%08x", 
			xcpt->ctx.core.r8, xcpt->ctx.core.r9, 
			xcpt->ctx.core.r10, xcpt->ctx.core.r11);
	DCC_LOG4(LOG_ERROR, "  R12=%08x  SP=%08x  LR=%08x  PC=%08x", 
			xcpt->ctx.core.r12, sp, xcpt->ctx.core.lr, xcpt->ctx.core.pc);
	DCC_LOG4(LOG_ERROR, " XPSR=%08x MSP=%08x PSP=%08x RET=%08x", 
			xcpt->ctx.core.xpsr, xcpt->msp, xcpt->psp, ret);
	xpsr = xcpt->ctx.core.xpsr;
	ipsr = xpsr & 0x1ff;
	if (ipsr < 16) { 
		DCC_LOG10(LOG_ERROR, " XPSR={ %c%c%c%c%c %c "
				 "ICI/IT=%02x GE=%1x IPSR=%d (%s) }", 
				 (xpsr & (1 << 31)) ? 'N' : '.',
				 (xpsr & (1 << 30)) ? 'Z' : '.',
				 (xpsr & (1 << 29)) ? 'C' : '.',
				 (xpsr & (1 << 28)) ? 'V' : '.',
				 (xpsr & (1 << 27)) ? 'Q' : '.',
				 (xpsr & (1 << 24)) ? 'T' : '.',
				 ((xpsr >> 19) & 0xc0) | ((xpsr >> 10) & 0x3f),
				 ((xpsr >> 16) & 0x0f),
				 ipsr, __xcpt_name_lut[ipsr]);
	} else {
		DCC_LOG10(LOG_ERROR, " XPSR={ %c%c%c%c%c %c "
				 "ICI/IT=%02x GE=%1x IPSR=%d (IRQ %d) }", 
				 (xpsr & (1 << 31)) ? 'N' : '.',
				 (xpsr & (1 << 30)) ? 'Z' : '.',
				 (xpsr & (1 << 29)) ? 'C' : '.',
				 (xpsr & (1 << 28)) ? 'V' : '.',
				 (xpsr & (1 << 27)) ? 'Q' : '.',
				 (xpsr & (1 << 24)) ? 'T' : '.',
				 ((xpsr >> 19) & 0xc0) | ((xpsr >> 10) & 0x3f),
				 ((xpsr >> 16) & 0x0f),
				 ipsr, ipsr - 16);
	}

#if (THINKOS_ENABLE_FPU)
	if ((xcpt->ret & CM3_EXC_RET_nFPCA) == 0) {
		DCC_LOG4(LOG_ERROR, "   S0=%08x  S1=%08x  S2=%08x  S3=%08x", 
		xcpt->ctx.s0[0], xcpt->ctx.s0[1], xcpt->ctx.s0[2], xcpt->ctx.s0[3]);
		DCC_LOG4(LOG_ERROR, "   S4=%08x  S5=%08x  S6=%08x  S7=%08x", 
		xcpt->ctx.s0[4], xcpt->ctx.s0[5], xcpt->ctx.s0[6], xcpt->ctx.s0[7]);
		DCC_LOG4(LOG_ERROR, "   S8=%08x  S9=%08x S10=%08x S11=%08x", 
		xcpt->ctx.s0[8], xcpt->ctx.s0[9], xcpt->ctx.s0[10], xcpt->ctx.s0[11]);
		DCC_LOG4(LOG_ERROR, "  S12=%08x S13=%08x S14=%08x S15=%08x", 
		xcpt->ctx.s0[12], xcpt->ctx.s0[13], xcpt->ctx.s0[14], xcpt->ctx.s0[15]);
		DCC_LOG4(LOG_ERROR, "  S16=%08x S17=%08x S18=%08x S19=%08x", 
		xcpt->ctx.s1[0], xcpt->ctx.s1[1], xcpt->ctx.s1[2], xcpt->ctx.s1[3]);
		DCC_LOG4(LOG_ERROR, "  S20=%08x S21=%08x S22=%08x S23=%08x", 
		xcpt->ctx.s1[4], xcpt->ctx.s1[5], xcpt->ctx.s1[6], xcpt->ctx.s1[7]);
		DCC_LOG4(LOG_ERROR, "  S24=%08x S25=%08x S26=%08x S27=%08x", 
				 xcpt->ctx.s1[8], xcpt->ctx.s1[9], 
				 xcpt->ctx.s1[10], xcpt->ctx.s1[11]);
		DCC_LOG4(LOG_ERROR, "  S28=%08x S29=%08x S30=%08x S31=%08x", 
				 xcpt->ctx.s1[12], xcpt->ctx.s1[13], 
				 xcpt->ctx.s1[14], xcpt->ctx.s1[15]);
		DCC_LOG1(LOG_ERROR, "FPSCR=%08x", xcpt->ctx.fpscr);
	} else {
		DCC_LOG(LOG_ERROR, "EXC_RETURN.nFPCA=0");
	}
#endif

	ctrl = xcpt->ctrl;
	DCC_LOG3(LOG_ERROR, " CTRL={%s%s%s }",
			 ctrl & CONTROL_FPCA? " FPCA" : "",
			 ctrl & CONTROL_SPSEL? " SPSEL" : "",
			 ctrl & CONTROL_nPRIV ? " nPRIV" : "");

	shcsr = xcpt->shcsr;
	DCC_LOG10(LOG_ERROR, "SHCSR={%s%s%s%s%s%s%s%s%s%s }", 
				 (shcsr & SCB_SHCSR_USGFAULTENA) ? " USGFAULTENA" : "",
				 (shcsr & SCB_SHCSR_BUSFAULTENA) ? " BUSFAULTENA " : "",
				 (shcsr & SCB_SHCSR_MEMFAULTENA) ? " MEMFAULTENA " : "",
				 (shcsr & SCB_SHCSR_SYSTICKACT) ? " SYSTICKACT" : "",
				 (shcsr & SCB_SHCSR_PENDSVACT) ? " PENDSVACT" : "",
				 (shcsr & SCB_SHCSR_MONITORACT) ? " MONITORACT" : "",
				 (shcsr & SCB_SHCSR_SVCALLACT) ? " SVCALLACT" : "",
				 (shcsr & SCB_SHCSR_USGFAULTACT) ?  " USGFAULTACT" : "",
				 (shcsr & SCB_SHCSR_BUSFAULTACT) ?  " BUSFAULTACT" : "",
				 (shcsr & SCB_SHCSR_MEMFAULTACT) ?  " MEMFAULTACT" : "");

	icsr = xcpt->icsr;
	DCC_LOG8(LOG_ERROR, " ICSR={%s%s%s%s%s%s VECTPENDING=%d VECTACTIVE=%d }", 
				 (icsr & SCB_ICSR_NMIPENDSET) ? " NMIPEND" : "",
				 (icsr & SCB_ICSR_PENDSVSET) ? " PENDSV" : "",
				 (icsr & SCB_ICSR_PENDSTSET) ? " PENDST" : "",
				 (icsr & SCB_ICSR_ISRPREEMPT) ? " ISRPREEMPT" : "",
				 (icsr & SCB_ICSR_ISRPENDING) ? " ISRPENDING" : "",
				 (icsr & SCB_ICSR_RETTOBASE) ? " RETTOBASE" : "",
				 (icsr & SCB_ICSR_VECTPENDING) >> 12,
				 (icsr & SCB_ICSR_VECTACTIVE));

	DCC_LOG2(LOG_ERROR, "(active at exception)=%d (active now)=%d", 
			 __xcpt_active_get(xcpt) + 1,
			 __thinkos_active_get() + 1); 
#if 0
	DCC_LOG3(LOG_ERROR, " *   SCR={%s%s%s }", 
			(scr & SCR_SEVONPEND) ? " SEVONPEND" : "",
			(scr & SCR_SLEEPDEEP) ? " SLEEPDEEP" : "",
			(scr & SCR_SLEEPONEXIT) ? " SLEEPONEXIT" : "");
#endif

#if 0
	DCC_LOG3(LOG_ERROR, " *   CCR={%s%s%s }", 
			(ccr & CCR_BP) ? " BP" : "",
			(ccr & CCR_IC) ? " IC" : "",
			(ccr & CCR_DC) ? " DC" : "",
			(ccr & CCR_STKALIGN) ? " STKALIGN" : "",
			(ccr & CCR_BFHFNMIGN) ? " BFHFNMIGN" : "",
			(ccr & CCR_DIV_0_TRP) ? " DIV_0_TRP" : "",
			(ccr & CCR_UNALIGN_TRP) ? " UNALIGN_TRP" : "",
			(ccr & CCR_USERSETMPEND) ? " USERSETMPEND" : "");
			(ccr & CCR_NONBASETHRDENA) ? " NONBASETHRDENA" : "");
#endif

#if (THINKOS_ENABLE_MONITOR)
	DCC_LOG2(LOG_ERROR, "Monitor stack free: %d/%6d", 
			 __scan_stack(thinkos_monitor_stack, thinkos_monitor_stack_size),
			 thinkos_monitor_stack_size); 
	{
		uintptr_t stack = (uintptr_t)thinkos_except_stack;
		unsigned long size = thinkos_except_stack_size;
		stack +=  sizeof(struct thinkos_except);
		size -=  sizeof(struct thinkos_except);

		DCC_LOG2(LOG_ERROR, "EXCEPT stack free: %d/%6d", 
			 __scan_stack((void *)stack, size), thinkos_except_stack_size); 
	}
#endif
	DCC_LOG1(LOG_ERROR, "exceptions count: %d", xcpt->seq); 

#endif
}

/* Interrupts state dump */
void __idump(const char * s, uint32_t ipsr)
{
#if defined(ENABLE_LOG) && (LOG_LEVEL >= LOG_ERROR)
	uint32_t shcsr;
	uint32_t icsr;
	int irqregs;
	int irqbits;
	int i;
	int j;

	DCC_LOG1(LOG_TRACE, "%s() _________________________________", s); 

	irqregs = CM3_ICTR + 1;
	for (i = 0; i < irqregs ; ++i) {
		irqbits = __rbit(CM3_NVIC->iabr[i]);
		while ((j = __clz(irqbits)) < 32) {
			irqbits &= ~(0x80000000 >> j);
			DCC_LOG1(LOG_TRACE, "Active IRQ=%d", i * 32 + j); 
		}
	}

	for (i = 0; i < irqregs ; ++i) {
		irqbits = __rbit(CM3_NVIC->ispr[i]);
		while ((j = __clz(irqbits)) < 32) {
			irqbits &= ~(0x80000000 >> j);
			DCC_LOG1(LOG_TRACE, "Pending IRQ=%d", i * 32 + j); 
		}
	}

	if (ipsr < 16) { 
		DCC_LOG2(LOG_TRACE, " IPSR=%d (%s)", ipsr, __xcpt_name_lut[ipsr]);
	} else {
		DCC_LOG2(LOG_TRACE, " IPSR=%d (IRQ %d)", ipsr, ipsr - 16);
	}
	shcsr = CM3_SCB->shcsr;
	DCC_LOG10(LOG_TRACE, "SHCSR={%s%s%s%s%s%s%s%s%s%s }", 
				 (shcsr & SCB_SHCSR_USGFAULTENA) ? " USGFAULTENA" : "",
				 (shcsr & SCB_SHCSR_BUSFAULTENA) ? " BUSFAULTENA " : "",
				 (shcsr & SCB_SHCSR_MEMFAULTENA) ? " MEMFAULTENA " : "",
				 (shcsr & SCB_SHCSR_SYSTICKACT) ? " SYSTICKACT" : "",
				 (shcsr & SCB_SHCSR_PENDSVACT) ? " PENDSVACT" : "",
				 (shcsr & SCB_SHCSR_MONITORACT) ? " MONITORACT" : "",
				 (shcsr & SCB_SHCSR_SVCALLACT) ? " SVCALLACT" : "",
				 (shcsr & SCB_SHCSR_USGFAULTACT) ?  " USGFAULTACT" : "",
				 (shcsr & SCB_SHCSR_BUSFAULTACT) ?  " BUSFAULTACT" : "",
				 (shcsr & SCB_SHCSR_MEMFAULTACT) ?  " MEMFAULTACT" : "");

	icsr = CM3_SCB->icsr;
	DCC_LOG8(LOG_TRACE, " ICSR={%s%s%s%s%s%s VECTPENDING=%d VECTACTIVE=%d }", 
				 (icsr & SCB_ICSR_NMIPENDSET) ? " NMIPEND" : "",
				 (icsr & SCB_ICSR_PENDSVSET) ? " PENDSV" : "",
				 (icsr & SCB_ICSR_PENDSTSET) ? " PENDST" : "",
				 (icsr & SCB_ICSR_ISRPREEMPT) ? " ISRPREEMPT" : "",
				 (icsr & SCB_ICSR_ISRPENDING) ? " ISRPENDING" : "",
				 (icsr & SCB_ICSR_RETTOBASE) ? " RETTOBASE" : "",
				 (icsr & SCB_ICSR_VECTPENDING) >> 12,
				 (icsr & SCB_ICSR_VECTACTIVE));

	if (ipsr != (icsr & SCB_ICSR_VECTACTIVE))
		DCC_LOG(LOG_ERROR, "IPSR != ICSR.VECTACTIVE");
#endif
}

/* MPU state dump */
void __mpudump(void)
{
#if defined(ENABLE_LOG) && (LOG_LEVEL >= LOG_ERROR)
	struct cm3_mpu * mpu = CM3_MPU;
	uint32_t rbar;
	uint32_t rasr;
	int i;

	DCC_LOG(LOG_TRACE, "MPU _________________________________"); 
	DCC_LOG1(LOG_TRACE, "ctrl=%08x", mpu->ctrl);

	for (i = 0; i < 8; ++i) {
		uint32_t size;

		mpu->rnr = i;
		rasr = mpu->rasr;
		rbar = mpu->rbar;
		DCC_LOG4(LOG_TRACE, "%d rbar=%08x rasr=%08x ADDR=%08x", i, 
				 rbar, rasr, rbar & 0xffffffe0);

		size = (1 << (((rasr >> 1) & 0x1f) + 1)) / 1024;
		if (size > 1024)  
			DCC_LOG9(LOG_TRACE, "  %sAP=%d TEX=%d %s%s%sSRD=%02x %dMB %s",
					 (rasr & (1 << 28)) ? "XN " : "",
					 (rasr >> 24) & 7,
					 (rasr >> 19) & 7,
					 (rasr & (1 << 18)) ? "S " : "",
					 (rasr & (1 << 17)) ? "C " : "",
					 (rasr & (1 << 16)) ? "B " : "",
					 (rasr >> 8) & 0xff,
					 size / 1024,
					 (rasr & 1) ? "enabled" : "disabled");
		else
			DCC_LOG9(LOG_TRACE, "  %sAP=%d TEX=%d %s%s%sSRD=%02x %dKB %s",
					 (rasr & (1 << 28)) ? "XN " : "",
					 (rasr >> 24) & 7,
					 (rasr >> 19) & 7,
					 (rasr & (1 << 18)) ? "S " : "",
					 (rasr & (1 << 17)) ? "C " : "",
					 (rasr & (1 << 16)) ? "B " : "",
					 (rasr >> 8) & 0xff,
					 size,
					 (rasr & 1) ? "enabled" : "disabled");
	}
#endif
}

/* Return the an active interrupt wich is different from the 
   interrupt in the current irq. Otherwise returns -16  */

int __xcpt_next_active_irq(int this_irq)
{
	int irqregs;
	int irqbits;
	int irq;
	int i;
	int j;

	irqregs = (CM3_ICTR + 1);
	for (i = 0; i < irqregs ; ++i) {
		irqbits = __rbit(CM3_NVIC->iabr[i]);
		while ((j = __clz(irqbits)) < 32) {
			irq = i * 32 + j;
			if (irq != this_irq)
				return irq;
			irqbits &= ~(0x80000000 >> j);
		}
	}

	return -16;
}

void __xcpt_systick_int_disable(void)
{
#if THINKOS_ENABLE_CLOCK || THINKOS_ENABLE_TIMESHARE
	struct cm3_systick * systick = CM3_SYSTICK;

	systick->csr &= ~SYSTICK_CSR_TICKINT;
	CM3_SCB->icsr = SCB_ICSR_PENDSTCLR;
#endif
}

void __xcpt_systick_int_enable(void)
{
#if THINKOS_ENABLE_CLOCK || THINKOS_ENABLE_TIMESHARE
	struct cm3_systick * systick = CM3_SYSTICK;
	systick->csr |= SYSTICK_CSR_TICKINT;
#endif
}

void __tdump(void)
{
#ifdef DEBUG
	int i;

	DCC_LOG1(LOG_TRACE, "active thread: %d", __thinkos_active_get() + 1);
	for (i = 0; i <= THINKOS_THREADS_MAX; ++i) {
		const struct thinkos_thread_inf * inf;
		if (!__thinkos_thread_ctx_is_valid(i))
			continue;
		if ((inf = __thinkos_thread_inf_get(i)) != NULL) {
#if THINKOS_ENABLE_THREAD_STAT
  #if (THINKOS_ENABLE_STACK_LIMIT)
			DCC_LOG9(LOG_TRACE, "%7s (%2d %3d) SL=%08x SP=%08x PC=%08x LR=%08x %d/%d", 
					 inf->tag, i + 1, i == THINKOS_THREAD_IDLE ? 0 : 
					 __thinkos_thread_stat_wq_get(i),
					 __thinkos_thread_sl_get(i), 
					 __thinkos_thread_sp_get(i), 
					 __thinkos_thread_pc_get(i), 
					 __thinkos_thread_lr_get(i), 
					 __scan_stack(inf->stack_ptr, inf->stack_size), 
					 inf->stack_size);
  #else
			DCC_LOG8(LOG_TRACE, "%7s (%2d %3d) SP=%08x PC=%08x LR=%08x %d/%d", 
					 inf->tag, i + 1, i == THINKOS_THREAD_IDLE ? 0 : 
					 __thinkos_thread_stat_wq_get(i),
					 __thinkos_thread_sp_get(i), 
					 __thinkos_thread_pc_get(i), 
					 __thinkos_thread_lr_get(i), 
					 __scan_stack(inf->stack_ptr, inf->stack_size), 
					 inf->stack_size);
  #endif
#else
			DCC_LOG7(LOG_TRACE, "%7s (%2d) SP=%08x PC=%08x LR=%08x %d/%d", 
					 inf->tag, i + 1, 
					 __thinkos_thread_sp_get(i), 
					 __thinkos_thread_pc_get(i), 
					 __thinkos_thread_lr_get(i), 
					 __scan_stack(inf->stack_ptr, inf->stack_size), 
					 inf->stack_size);
#endif
		} else {
#if (THINKOS_ENABLE_THREAD_STAT)
  #if (THINKOS_ENABLE_STACK_LIMIT)
		DCC_LOG6(LOG_TRACE, "....... (%2d %3d) SL=%08x SP=%08x PC=%08x LR=%08x", 
				 i + 1, __thinkos_thread_stat_wq_get(i),
				 __thinkos_thread_sl_get(i), 
				 __thinkos_thread_sp_get(i), 
				 __thinkos_thread_pc_get(i), 
				 __thinkos_thread_lr_get(i)); 
  #else
		DCC_LOG5(LOG_TRACE, "....... (%2d %3d) SP=%08x PC=%08x LR=%08x", 
				 i + 1, __thinkos_thread_stat_wq_get(i),
				 __thinkos_thread_sp_get(i), 
				 __thinkos_thread_pc_get(i), 
				 __thinkos_thread_lr_get(i)); 
  #endif		 
#else
		DCC_LOG4(LOG_TRACE, "....... (%2d) SP=%08x PC=%08x LR=%08x", 
				 i + 1, __thinkos_thread_sp_get(i), 
				 __thinkos_thread_pc_get(i), 
				 __thinkos_thread_lr_get(i)); 
#endif


		}
	}
	DCC_LOG1(LOG_TRACE, "wq_ready=%08x", thinkos_rt.wq_ready);
#if THINKOS_ENABLE_TIMESHARE
	DCC_LOG1(LOG_TRACE, "wq_tmshare=%08x", thinkos_rt.wq_tmshare);
#endif
#if THINKOS_ENABLE_CLOCK
	DCC_LOG1(LOG_TRACE, "wq_clock=%08x", thinkos_rt.wq_clock);
#endif

#endif /* DEBUG */
}

void __odump(void)
{
#if defined(ENABLE_LOG) && (LOG_LEVEL >= LOG_ERROR)

#if (THINKOS_MUTEX_MAX > 0)
	DCC_LOG3(LOG_TRACE, "    mutex: %2d (%2d .. %2d)", THINKOS_MUTEX_MAX,
			 THINKOS_MUTEX_BASE,
			 THINKOS_MUTEX_BASE + THINKOS_MUTEX_MAX - 1);
#endif
#if (THINKOS_COND_MAX > 0)
	DCC_LOG3(LOG_TRACE, "  condvar: %2d (%2d .. %2d)", THINKOS_COND_MAX,
			 THINKOS_COND_BASE,
			 THINKOS_COND_BASE + THINKOS_COND_MAX - 1);
#endif
#if (THINKOS_SEMAPHORE_MAX > 0)
	DCC_LOG3(LOG_TRACE, "semaphore: %2d (%2d .. %2d)", THINKOS_SEMAPHORE_MAX,
			 THINKOS_SEM_BASE,
			 THINKOS_SEM_BASE + THINKOS_SEMAPHORE_MAX - 1);
#endif
#if (THINKOS_EVENT_MAX > 0)
	DCC_LOG3(LOG_TRACE, "    evset: %2d (%2d .. %2d)", THINKOS_EVENT_MAX,
			 THINKOS_EVENT_BASE,
			 THINKOS_EVENT_BASE + THINKOS_EVENT_MAX - 1);
#endif
#if (THINKOS_FLAG_MAX > 0)
	DCC_LOG3(LOG_TRACE, "     flag: %2d (%2d .. %2d)", THINKOS_FLAG_MAX,
			 THINKOS_FLAG_BASE,
			 THINKOS_FLAG_BASE + THINKOS_FLAG_MAX - 1);
#endif
#if (THINKOS_GATE_MAX > 0)
	DCC_LOG3(LOG_TRACE, "     gate: %2d (%2d .. %2d)", THINKOS_GATE_MAX,
			 THINKOS_GATE_BASE,
			 THINKOS_GATE_BASE + THINKOS_GATE_MAX - 1);
#endif
#if (THINKOS_ENABLE_JOIN)
	DCC_LOG3(LOG_TRACE, "     join: %2d (%2d .. %2d)", THINKOS_THREADS_MAX,
			 THINKOS_JOIN_BASE,
			 THINKOS_JOIN_BASE + THINKOS_THREADS_MAX - 1);
#endif
#if (THINKOS_ENABLE_CONSOLE)
	DCC_LOG2(LOG_TRACE, "  console: (wr:%2d rd:%2d)", 
			THINKOS_WQ_CONSOLE_WR,  THINKOS_WQ_CONSOLE_RD); 
#endif
#if (THINKOS_ENABLE_PAUSE)
	DCC_LOG1(LOG_TRACE, "   paused: (%2d)", THINKOS_WQ_PAUSED); 
#endif
#if (THINKOS_ENABLE_JOIN)
	DCC_LOG1(LOG_TRACE, " canceled: (%2d)", THINKOS_WQ_CANCELED); 
#endif
#if (THINKOS_ENABLE_DEBUG_FAULT)
	DCC_LOG1(LOG_TRACE, "    fault: (%2d)", THINKOS_WQ_FAULT); 
#endif

#endif
}

void __pdump(void)
{
#if defined(ENABLE_LOG) && (LOG_LEVEL >= LOG_ERROR)
	uint32_t shcsr;
	uint32_t icsr;

	shcsr = CM3_SCB->shcsr;
	DCC_LOG10(LOG_TRACE, "SHCSR={%s%s%s%s%s%s%s%s%s%s }", 
				 (shcsr & SCB_SHCSR_USGFAULTENA) ? " USGFAULTENA" : "",
				 (shcsr & SCB_SHCSR_BUSFAULTENA) ? " BUSFAULTENA " : "",
				 (shcsr & SCB_SHCSR_MEMFAULTENA) ? " MEMFAULTENA " : "",
				 (shcsr & SCB_SHCSR_SYSTICKACT) ? " SYSTICKACT" : "",
				 (shcsr & SCB_SHCSR_PENDSVACT) ? " PENDSVACT" : "",
				 (shcsr & SCB_SHCSR_MONITORACT) ? " MONITORACT" : "",
				 (shcsr & SCB_SHCSR_SVCALLACT) ? " SVCALLACT" : "",
				 (shcsr & SCB_SHCSR_USGFAULTACT) ?  " USGFAULTACT" : "",
				 (shcsr & SCB_SHCSR_BUSFAULTACT) ?  " BUSFAULTACT" : "",
				 (shcsr & SCB_SHCSR_MEMFAULTACT) ?  " MEMFAULTACT" : "");

	icsr = CM3_SCB->icsr;
	DCC_LOG8(LOG_TRACE, " ICSR={%s%s%s%s%s%s VECTPENDING=%d VECTACTIVE=%d }", 
				 (icsr & SCB_ICSR_NMIPENDSET) ? " NMIPEND" : "",
				 (icsr & SCB_ICSR_PENDSVSET) ? " PENDSV" : "",
				 (icsr & SCB_ICSR_PENDSTSET) ? " PENDST" : "",
				 (icsr & SCB_ICSR_ISRPREEMPT) ? " ISRPREEMPT" : "",
				 (icsr & SCB_ICSR_ISRPENDING) ? " ISRPENDING" : "",
				 (icsr & SCB_ICSR_RETTOBASE) ? " RETTOBASE" : "",
				 (icsr & SCB_ICSR_VECTPENDING) >> 12,
				 (icsr & SCB_ICSR_VECTACTIVE));

	DCC_LOG(LOG_TRACE, " Exceptions priorities:");
	DCC_LOG1(LOG_TRACE, "       SVC: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_SVC));
	/* SysTick interrupt has to have a lower priority then SVC,
	 to not preempt SVC */
	DCC_LOG1(LOG_TRACE, "   SysTick: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_SYSTICK));
	/* PendSV interrupt has to have the lowest priority among
	   regular interrupts (higher number) */
	DCC_LOG1(LOG_TRACE, "    PendSv: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_PENDSV));

#if	(THINKOS_ENABLE_USAGEFAULT) 
	DCC_LOG1(LOG_TRACE, "  UsgFault: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_USAGE_FAULT));
#endif
#if	(THINKOS_ENABLE_BUSFAULT)
	DCC_LOG1(LOG_TRACE, "  BusFault: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_BUS_FAULT));
#endif
#if (THINKOS_ENABLE_MPU)
	DCC_LOG1(LOG_TRACE, " MemManage: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_MEM_MANAGE));
#endif
#if (THINKOS_ENABLE_DEBUG)
	DCC_LOG1(LOG_TRACE, "    DbgMon: %d", 
	         cm3_except_pri_get(CM3_EXCEPT_DEBUG_MONITOR));
#endif

#endif
}

void __xinfo(struct thinkos_except * xcpt)
{
#if defined(ENABLE_LOG) && (LOG_LEVEL >= LOG_ERROR)
	int err = xcpt->errno;

	(void)err;

	mdelay(100);
	DCC_LOG2(LOG_ERROR, VT_PSH VT_FBK VT_BRD
			 "/!\\ Exception %d [%s] /!\\" VT_POP, 
			 err, thinkos_err_name_lut[err]);

	if (err == THINKOS_ERR_HARD_FAULT) {
		uint32_t hfsr = xcpt->hfsr;
		DCC_LOG3(LOG_PANIC, VT_PSH VT_BRI VT_FRD 
				 "Hard fault:%s%s%s    " VT_POP, 
				 (hfsr & SCB_HFSR_DEBUGEVT) ? " DEBUGEVT" : "",
				 (hfsr & SCB_HFSR_FORCED) ?  " FORCED" : "",
				 (hfsr & SCB_HFSR_VECTTBL) ? " VECTTBL" : "");
	}

	if ((err == THINKOS_ERR_HARD_FAULT) || (err == THINKOS_ERR_BUS_FAULT)) {
		uint32_t mmfsr = SCB_CFSR_MMFSR_GET(xcpt->cfsr);
		uint32_t mmfar = xcpt->mmfar;
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
	}

	if ((err == THINKOS_ERR_HARD_FAULT) || (err == THINKOS_ERR_USAGE_FAULT)) {
		uint32_t ufsr = SCB_CFSR_UFSR_GET(xcpt->cfsr);
		DCC_LOG1(LOG_ERROR, "UFSR=%08X", ufsr);
		if (ufsr) {
			DCC_LOG6(LOG_PANIC, "    %s%s%s%s%s%s", 
					 (ufsr & UFSR_DIVBYZERO)  ? " DIVBYZERO" : "",
					 (ufsr & UFSR_UNALIGNED)  ? " UNALIGNED" : "",
					 (ufsr & UFSR_NOCP)  ? " NOCP" : "",
					 (ufsr & UFSR_INVPC)  ? " INVPC" : "",
					 (ufsr & UFSR_INVSTATE)  ? " INVSTATE" : "",
					 (ufsr & UFSR_UNDEFINSTR)  ? " UNDEFINSTR" : "");
		}
	}

	if ((err == THINKOS_ERR_HARD_FAULT) || (err == THINKOS_ERR_BUS_FAULT)) {
		uint32_t bfsr = SCB_CFSR_BFSR_GET(xcpt->cfsr);
		DCC_LOG2(LOG_ERROR, "BFSR=%08X BFAR=%08x", bfsr, xcpt->bfar);
		if (bfsr) {
			DCC_LOG7(LOG_ERROR, "     %s%s%s%s%s%s%s", 
					 (bfsr & BFSR_BFARVALID) ? " BFARVALID" : "",
					 (bfsr & BFSR_LSPERR) ? " LSPERR" : "",
					 (bfsr & BFSR_STKERR) ? " STKERR" : "",
					 (bfsr & BFSR_UNSTKERR) ?  " UNSTKERR" : "",
					 (bfsr & BFSR_IMPRECISERR) ?  " IMPRECISERR" : "",
					 (bfsr & BFSR_PRECISERR) ?  " PRECISERR" : "",
					 (bfsr & BFSR_IBUSERR)  ?  " IBUSERR" : "");
		}
	}
#endif
}

