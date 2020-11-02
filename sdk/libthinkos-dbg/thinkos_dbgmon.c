/* 
 * File:	 thinkos-dbgmon.c
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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#include <thinkos.h>

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <stdbool.h>
#include <sys/dcclog.h>
#include <vt100.h>

#if (THINKOS_ENABLE_DEBUG) 

#ifdef THINKOS_DBGMON_ENABLE_IRQ_MGMT
#warning "Deprecated THINKOS_DBGMON_ENABLE_IRQ_MGMT"
#endif

#define NVIC_IRQ_REGS ((THINKOS_IRQ_MAX + 31) / 32)

struct thinkos_dbgmon {
	int8_t thread_id;
	int8_t break_id;
	uint8_t errno;
	int32_t code;
};

struct thinkos_dbgmon thinkos_dbgmon_rt;

/**
  * __dbgmon_irq_disable_all:
  *
  * Disable all interrupts by clearing the interrupt enable bit
  * of all interrupts on the Nested Vector Interrupt Controller (NVIC).
  *
  * Also the interrupt enable backup is cleared to avoid 
  * interrupts being reenabled by calling __dbgmon_irq_restore_all().
  *
  * The systick interrupt is not disabled.
  */
static void __dbgmon_irq_disable_all(void)
{
	int i;

	DCC_LOG(LOG_WARNING, "NIVC all interrupts disabled!!!");

	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		CM3_NVIC->icer[i] = 0xffffff; /* disable interrupts */
		/* FIXME: clearing the pending interrupt may have a side effect 
		   on the comms irq used by the debug monitor. An alternative 
		   would be to use the force enable list to avoid clearing those
		   in the list. */
#if 0
		CM3_NVIC->icpr[i] = 0xffffff; /* clear pending interrupts */
#endif
	}
}


#if (THINKOS_DBGMON_ENABLE_RST_VEC)
/**
 * __reset_ram_vectors:
 *
 * Copy the default values for the IRQ vectors from the flash into RAM. 
 * 
 * When the a new application replaces the existing one through the GDB
 * or Ymodem some interrupts can be fired due to wrong sequencig of
 * interrupt programming in the application. To avoid potential system
 * crashes the vectors should be initialized to a default value.
 *
 */

void __reset_ram_vectors(void)
{
	/* XXX: 
	   this function assumes the exception vectors defaults to be located 
	   just after the .text section! */
	extern unsigned int __text_end;
	extern unsigned int __ram_vectors;
	extern unsigned int __sizeof_ram_vectors;

	unsigned int size = __sizeof_ram_vectors;
	void * src = &__text_end;
	void * dst = &__ram_vectors;

	DCC_LOG3(LOG_MSG, "dst=%08x src=%08x size=%d", dst, src, size); 
	__thinkos_memcpy32(dst, src, size); 
}
#endif /* THINKOS_DBGMON_ENABLE_RST_VEC */

/* -------------------------------------------------------------------------
 * Debug Monitor API
 * ------------------------------------------------------------------------- */

#define THINKOS_THREAD_LAST (THINKOS_THREADS_MAX + 1)

int dbgmon_thread_inf_get(unsigned int id, struct dbgmon_thread_inf * inf)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();
	unsigned int errno = THINKOS_NO_ERROR;
	struct thinkos_context * ctx;
	unsigned int thread_id = id;
	uint32_t pc = 0;
	uint32_t sp = 0;
	uint32_t ctrl = 0;

	if (thread_id > THINKOS_THREAD_LAST) {
		DCC_LOG(LOG_ERROR, "Invalid thread!");
		return -1;
	}

	if (thread_id == xcpt->active) {
		ctx = &xcpt->ctx.core;
		errno = xcpt->errno;
		pc = ctx->pc;
		sp = xcpt->psp;
	} else if (thread_id == THINKOS_THREAD_IDLE) {
		ctx  = __thinkos_idle_ctx();
		pc = ctx->pc;
		sp = xcpt->msp;
	} else {
		if (thread_id == (unsigned int)thinkos_dbgmon_rt.break_id)
			errno = thinkos_dbgmon_rt.errno;

		ctx = __thinkos_thread_ctx_get(thread_id);
		if (((uint32_t)ctx < 0x10000000) || ((uint32_t)ctx >= 0x30000000)) {
			DCC_LOG2(LOG_ERROR, "<%d> context 0x%08x invalid!!!", 
					 thread_id + 1, ctx);
			return -1;
		}
		ctrl = __thinkos_thread_ctrl_get(thread_id);
		sp = __thinkos_thread_sp_get(thread_id);
	}

	if (inf != NULL) {
		inf->pc = pc;
		inf->sp = sp;
		inf->errno = errno;
		inf->thread_id = id;
		inf->ctrl = ctrl;
		inf->ctx = ctx;
	}

	return 0;
}

int dbgmon_errno_get(void)
{
	return  thinkos_dbgmon_rt.errno;
}

int dbgmon_thread_break_get(void)
{
	struct thinkos_context * ctx;
	int thread_id;

	if ((thread_id = thinkos_dbgmon_rt.break_id) >= 0) {
		if ((ctx = __thinkos_thread_ctx_get(thread_id)) == NULL)
			return -1;
	}

	return thread_id;
}

void dbgmon_thread_break_clr(void)
{
	thinkos_dbgmon_rt.break_id = -1;
	thinkos_dbgmon_rt.errno = 0;
}

#if (THINKOS_ENABLE_DEBUG_BKPT) || (THINKOS_ENABLE_DEBUG_WPT)

/* -------------------------------------------------------------------------
 * Debug Breakpoint
 * ------------------------------------------------------------------------- */
#define BP_DEFSZ 2
/* (Flash Patch) Number of instruction address comparators */
#define CM3_FP_NUM_CODE 6
/* (Flash Patch) Number of literal address comparators */
#define CM3_FP_NUM_LIT  2

bool dbgmon_breakpoint_set(uint32_t addr, uint32_t size)
{
	struct cm3_fpb * fpb = CM3_FPB;
	uint32_t comp;
	int i;

	for (i = 0; i < CM3_FP_NUM_CODE; ++i) {
		if ((fpb->comp[i] & COMP_ENABLE) == 0) 
			break;
	}

	if (i == CM3_FP_NUM_CODE) {
		DCC_LOG(LOG_WARNING, "no more breakpoints");
		return false;
	}

	/* use default size if zero */
	size = (size == 0) ? BP_DEFSZ : size;

	if (size == 2) {
		if (addr & 0x00000002) {
			comp = COMP_BP_HIGH | (addr & 0x0ffffffc) | COMP_ENABLE;
		} else {
			comp = COMP_BP_LOW | (addr & 0x0ffffffc) | COMP_ENABLE;
		}
	} else {
		comp = COMP_BP_WORD | (addr & 0x0ffffffc) | COMP_ENABLE;
	}

	fpb->comp[i] = comp;

	DCC_LOG4(LOG_INFO, "bp=%d addr=0x%08x size=%d comp=0x%08x ", i, addr, 
			 size, fpb->comp[i]);

	return true;
}

bool dbgmon_breakpoint_clear(uint32_t addr, uint32_t size)
{
	struct cm3_fpb * fpb = CM3_FPB;
	uint32_t comp;
	int i;

	size = (size == 0) ? BP_DEFSZ : size;

	if (size == 2) {
		if (addr & 0x00000002) {
			comp = COMP_BP_HIGH | (addr & 0x0ffffffc) | COMP_ENABLE;
		} else {
			comp = COMP_BP_LOW | (addr & 0x0ffffffc) | COMP_ENABLE;
		}
	} else {
		comp = COMP_BP_WORD | (addr & 0x0ffffffc) | COMP_ENABLE;
	}

	DCC_LOG2(LOG_INFO, "addr=0x%08x size=%d", addr, size);

	for (i = 0; i < CM3_FP_NUM_CODE; ++i) {
		if ((fpb->comp[i] | COMP_ENABLE) == comp) {
			fpb->comp[i] = 0;
			return true;
		}
	}

	DCC_LOG1(LOG_WARNING, "breakpoint 0x%08x not found!", addr);

	return false;
}

bool dbgmon_breakpoint_disable(uint32_t addr)
{
	struct cm3_fpb * fpb = CM3_FPB;
	int i;

	for (i = 0; i < CM3_FP_NUM_CODE; ++i) {
		if ((fpb->comp[i] & 0x0ffffffc) == (addr & 0x0ffffffc)) {
			DCC_LOG2(LOG_WARNING, "breakpoint %d at 0x%08x disabled!", 
					 i, addr);
			fpb->comp[i] &= ~COMP_ENABLE;
			return true;
		}
	}

	DCC_LOG1(LOG_WARNING, "breakpoint 0x%08x not found!", addr);

	return false;
}

void dbgmon_breakpoint_clear_all(void)
{
	struct cm3_fpb * fpb = CM3_FPB;

	__thinkos_memset32(fpb->comp, 0, (CM3_FP_NUM_CODE + CM3_FP_NUM_LIT) * 4);
}

#endif /* THINKOS_ENABLE_DEBUG_BKPT */

#if (THINKOS_ENABLE_DEBUG_WPT)

/* -------------------------------------------------------------------------
 * Debug Watchpoint
 * ------------------------------------------------------------------------- */

#define DWT_MATCHED            (1 << 24)

#define DWT_DATAVADDR1(ADDR)   ((ADDR) << 16)
#define DWT_DATAVADDR0(ADDR)   ((ADDR) << 12)

#define DWT_DATAVSIZE_BYTE     (0 << 10)
#define DWT_DATAVSIZE_HALFWORD (1 << 10)
#define DWT_DATAVSIZE_WORD     (2 << 10)

#define DWT_LNK1ENA            (1 << 9)
#define DWT_DATAVMATCH         (1 << 8)
#define DWT_CYCMATCH           (1 << 7)
#define DWT_EMITRANGE          (1 << 5)

#define DWT_FUNCTION           (0xf << 0)

#define DWT_DATAV_RO_BKP       (5 << 0)
#define DWT_DATAV_WO_BKP       (6 << 0)
#define DWT_DATAV_RW_BKP       (7 << 0)

#define DWT_DATAV_RO_CMP       (9 << 0)
#define DWT_DATAV_WO_CMP       (10 << 0)
#define DWT_DATAV_RW_CMP       (11 << 0)

#define CM3_DWT_NUMCOMP 4

bool dbgmon_watchpoint_set(uint32_t addr, uint32_t size, int access)
{
	struct cm3_dwt * dwt = CM3_DWT;
	uint32_t func;
	int i;

	for (i = 0; i < CM3_DWT_NUMCOMP; ++i) {
		if ((dwt->wp[i].function & DWT_FUNCTION) == 0) 
			break;
	}

	if (i == CM3_DWT_NUMCOMP) {
		DCC_LOG(LOG_WARNING, "no more watchpoints");
		return false;
	}

	if (size == 0)
		return false;

	if (size > 4) {
		/* FIXME: implement ranges... */
		return false;
	}

	dwt->wp[i].comp = addr;

	if (size == 4) {
		func = DWT_DATAVSIZE_WORD;
		dwt->wp[i].mask = 2;
	} else if (size == 2) {
		func = DWT_DATAVSIZE_HALFWORD;
		dwt->wp[i].mask = 1;
	} else {
		func = DWT_DATAVSIZE_BYTE;
		dwt->wp[i].mask = 0;
	}

	if (access == 1) {
		func |= DWT_DATAV_RO_BKP;
	} else if (access == 2) {
		func |= DWT_DATAV_WO_BKP;
	} else {
		func |= DWT_DATAV_RW_BKP;
	}

	dwt->wp[i].function = func;

	DCC_LOG3(LOG_TRACE, "wp=%d addr=0x%08x size=%d", i, addr, size);

	return true;
}

bool dbgmon_watchpoint_clear(uint32_t addr, uint32_t size)
{
	struct cm3_dwt * dwt = CM3_DWT;
	int i;

	DCC_LOG2(LOG_INFO, "addr=0x%08x size=%d", addr, size);

	for (i = 0; i < CM3_DWT_NUMCOMP; ++i) {
		if (((dwt->wp[i].function & DWT_FUNCTION) != 0) && 
			dwt->wp[i].comp == addr) { 
			dwt->wp[i].function = 0;
			dwt->wp[i].comp = 0;
			return true;
		}
	}

	DCC_LOG1(LOG_WARNING, "watchpoint 0x%08x not found!", addr);

	return false;
}


#define DWT_MATCHED            (1 << 24)

#define DWT_DATAVADDR1(ADDR)   ((ADDR) << 16)
#define DWT_DATAVADDR0(ADDR)   ((ADDR) << 12)

#define DWT_DATAVSIZE_BYTE     (0 << 10)
#define DWT_DATAVSIZE_HALFWORD (1 << 10)
#define DWT_DATAVSIZE_WORD     (2 << 10)

#define DWT_LNK1ENA            (1 << 9)
#define DWT_DATAVMATCH         (1 << 8)
#define DWT_CYCMATCH           (1 << 7)
#define DWT_EMITRANGE          (1 << 5)

#define DWT_DATAV_RO_BKP       (5 << 0)
#define DWT_DATAV_WO_BKP       (6 << 0)
#define DWT_DATAV_RW_BKP       (7 << 0)

#define DWT_DATAV_RO_CMP       (9 << 0)
#define DWT_DATAV_WO_CMP       (10 << 0)
#define DWT_DATAV_RW_CMP       (11 << 0)


void dbgmon_watchpoint_clear_all(void)
{
	struct cm3_dwt * dwt = CM3_DWT;
	int i;
	int n;

	n = (dwt->ctrl & DWT_CTRL_NUMCOMP) >> 28;

	DCC_LOG1(LOG_TRACE, "TWD NUMCOMP=%d.", n);

	for (i = 0; i < n; ++i)
		dwt->wp[i].function = 0;
}

#endif /* THINKOS_ENABLE_DEBUG_WPT */

#if (THINKOS_ENABLE_DEBUG_STEP)

/* -------------------------------------------------------------------------
 * Thread stepping
 * ------------------------------------------------------------------------- */

int dbgmon_thread_step(unsigned int thread_id, bool sync)
{
	int ret;

	DCC_LOG2(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
			 "step_req=%08x thread_id=%d"
			 _ATTR_POP_, 
			 thinkos_rt.step_req, thread_id + 1);

	if (CM3_DCB->dhcsr & DCB_DHCSR_C_DEBUGEN) {
		DCC_LOG(LOG_ERROR, "can't step: DCB_DHCSR_C_DEBUGEN !!");
		return -1;
	}

	if (thread_id == THINKOS_THREAD_LAST) {
		DCC_LOG(LOG_ERROR, "void thread, IRQ step!");
		return -1;
	} else {
		if (thread_id >= THINKOS_THREADS_MAX) {
			DCC_LOG1(LOG_ERROR, "thread %d is invalid!", thread_id + 1);
			return -1;
		}

		if (__bit_mem_rd(&thinkos_rt.step_req, thread_id)) {
			DCC_LOG1(LOG_WARNING, "thread %d is step waiting already!", 
					 thread_id + 1);
			return -1;
		}

		DCC_LOG(LOG_MSG, "setting the step_req bit");
		/* request stepping the thread  */
		__bit_mem_wr(&thinkos_rt.step_req, thread_id, 1);
		/* resume the thread */
		__thinkos_thread_resume(thread_id);
		/* make sure to run the scheduler */
		__thinkos_defer_sched();
	}

	if (sync) {
		DCC_LOG(LOG_MSG, "synchronous step, waiting for signal...");
		if ((ret = monitor_expect(MONITOR_THREAD_STEP)) < 0)
			return ret;
	}

	return 0;
}

int dbgmon_thread_step_get(void)
{
	struct thinkos_context * ctx;
	int thread_id;

	if ((thread_id = thinkos_rt.step_id) >= 0) {
		if ((ctx = __thinkos_thread_ctx_get(thread_id)) == NULL)
			return -1;
	}

	return thread_id;
}

void dbgmon_thread_step_clr(void)
{
	thinkos_rt.step_id = -1;
}

#endif /* THINKOS_ENABLE_DEBUG_STEP */



/* -------------------------------------------------------------------------
 * Debug Monitor Core
 * ------------------------------------------------------------------------- */

#if 0
void __except_ctx_cpy(struct thinkos_context * ctx)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();
	uint32_t * dst = (uint32_t *)&xcpt->ctx.core;
	uint32_t * src = (uint32_t *)ctx;
	unsigned int sz = sizeof(struct thinkos_context);
#if (THINKOS_ENABLE_FPU) 
	if ((ctx->ret & CM3_EXC_RET_nFPCA) == 0) {
		src -=16;
		dst -=16;
		sz = sizeof(struct thinkos_fp_context);
	}
#endif

	__thinkos_memcpy32(dst, src, sz);
}
#endif

int thinkos_dbgmon_isr(struct armv7m_basic_frame * frm, uint32_t ret)
{
	uint32_t xpsr = frm->xpsr;
	uint32_t dfsr;

	/* read SCB Debug Fault Status Register */
	if ((dfsr = CM3_SCB->dfsr) != 0) {

		/* clear the fault */
		CM3_SCB->dfsr = dfsr;

		DCC_LOG5(LOG_INFO, "DFSR=(EXT=%c)(VCATCH=%c)"
				 "(DWTRAP=%c)(BKPT=%c)(HALT=%c)", 
				 dfsr & SCB_DFSR_EXTERNAL ? '1' : '0',
				 dfsr & SCB_DFSR_VCATCH ? '1' : '0',
				 dfsr & SCB_DFSR_DWTTRAP ? '1' : '0',
				 dfsr & SCB_DFSR_BKPT ? '1' : '0',
				 dfsr & SCB_DFSR_HALTED ? '1' : '0');
	
#if (THINKOS_ENABLE_FPU_LS)
		DCC_LOG3(LOG_TRACE, "FPCCR=%08x FPCAR=%08x CTRL=%01x",
				 CM3_SCB->fpccr, CM3_SCB->fpcar, 
				 cm3_control_get()); 
		if (CM3_SCB->fpccr & SCB_FPCCR_LSPACT) {
			/* Save FP context if lazy flag is enabled. */
			uint32_t fpacr = CM3_SCB->fpcar;
			uint32_t fpscr;
			asm volatile ("vstmia %1!, {s0-s15}\n"
						  "vmrs %0, FPSCR\n"
						  "str  %0, [%1]\n"
						  : "=r" (fpscr) : "r" (fpacr));
			DCC_LOG1(LOG_TRACE, "FPSCR=%08x", fpscr);
			/* Clear LSEN flag, preserving the FP lazy save flag */
			CM3_SCB->fpccr = SCB_FPCCR_ASPEN | SCB_FPCCR_LSPEN;
		}
#endif

		if (dfsr & SCB_DFSR_BKPT) do {
			unsigned int thread_id = __thinkos_active_get();
			unsigned int insn;
			unsigned int code; 
			uint16_t * pc;
			int ipsr;

			ipsr = xpsr & 0x1ff;
			(void) ipsr;
			pc = (uint16_t *)frm->pc;
			insn = pc[0];
			code = insn & 0x00ff;
			(void)code;
			insn &= 0xff00;

			/* this is a breakpoint instruction */
			if (insn == 0xbe00) {
				/* Skip breakpoint intruction, by adjusting
				   the call stack...*/
				frm->pc += 2;
				if (CM3_SCB->icsr & SCB_ICSR_RETTOBASE) {
#if (THINKOS_ENABLE_DEBUG_BKPT)
					/* Breakpoint on a thread */
					DCC_LOG4(LOG_TRACE,_ATTR_PUSH_ _FG_GREEN_ _REVERSE_
							 " SOFT BKPT: %2d " _NORMAL_ _FG_GREEN_
							 " PC=%08x SP=%08x IPSR=%d"
							 _ATTR_POP_,
							 thread_id + 1, frm->pc, cm3_psp_get(), ipsr);
					/* suspend the current thread */
					__thinkos_thread_pause(thread_id);
					/* record the break thread id */
					thinkos_dbgmon_rt.break_id = thread_id;
					thinkos_dbgmon_rt.errno = THINKOS_NO_ERROR;
					/* delivers a thread breakpoint on next round */
					monitor_signal(MONITOR_BREAKPOINT); 
					/* run scheduler */
					__thinkos_defer_sched();
#endif
					break;
				} 

				/* Breakpoint on a system call */
#if (THINKOS_ENABLE_ERROR_TRAP)
				if (code > THINKOS_BKPT_EXCEPT_OFF) {
					unsigned int err = code - THINKOS_BKPT_EXCEPT_OFF;
					(void)err;

					/* XXX: a breakpoint is used to indicate a fault or 
					   wrong usage of a system call in ThinkOS. */
					DCC_LOG5(LOG_TRACE,_ATTR_PUSH_ _FG_YELLOW_ _REVERSE_
							 " ERROR %d: %2d " _NORMAL_ _FG_YELLOW_
							 " PC=%08x SP=%08x IPSR=%d"
							 _ATTR_POP_, err,
							 thread_id + 1, frm->pc, cm3_psp_get(), ipsr);

					/* suspend all threads */
					__thinkos_pause_all();
					/* record the break thread id */
					thinkos_dbgmon_rt.break_id = thread_id;
					thinkos_dbgmon_rt.errno = err;
#if (THINKOS_ENABLE_DEBUG_FAULT)
					/* flag the thread as faulty */
					__thinkos_thread_fault_set(thread_id);
#endif
					/* delivers a thread fault on next round */
					monitor_signal(MONITOR_THREAD_FAULT); 
					/* run scheduler */
					__thinkos_defer_sched();
					break;
				} 
#endif /* THINKOS_ENABLE_ERROR_TRAP */

				DCC_LOG4(LOG_WARNING, _ATTR_PUSH_ _FG_YELLOW_ _REVERSE_
						 " KERNEL BKPT" _NORMAL_ _FG_YELLOW_
						 " PC=%08x SP=%08x IPSR=%d"
						 " Bkpt %d on service call"
						 _ATTR_POP_, frm->pc, cm3_msp_get(), ipsr, code);
				__thinkos_pause_all();
				/* record the break thread id */
				thinkos_dbgmon_rt.break_id = thread_id;
				/* save the current state of IPSR */
//				thinkos_rt.xcpt_ipsr = ipsr;
				/* delivers a kernel exception on next round */
				monitor_signal(MONITOR_KRN_EXCEPT);
				break;
			}

			if (CM3_SCB->icsr & SCB_ICSR_RETTOBASE) {
#if (THINKOS_ENABLE_DEBUG_BKPT)
				if ((uint32_t)thread_id < THINKOS_THREADS_MAX) {
					DCC_LOG4(LOG_TRACE,_ATTR_PUSH_ _FG_GREEN_ _REVERSE_
							 " BREAKPOINT: %2d " _NORMAL_ _FG_GREEN_
							 " PC=%08x SP=%08x IPSR=%d"
							 _ATTR_POP_,
							 thread_id + 1, frm->pc, cm3_psp_get(), ipsr);
					/* suspend the current thread */
					__thinkos_thread_pause(thread_id);
					/* disable this breakpoint */
					dbgmon_breakpoint_disable(frm->pc);
					/* record the break thread id */
					thinkos_dbgmon_rt.break_id = thread_id;
					/* run scheduler */
					__thinkos_defer_sched();
					/* delivers a breakpoint swignal on next round */
					monitor_signal(MONITOR_BREAKPOINT);
					break;
				} 

				DCC_LOG4(LOG_ERROR,_ATTR_PUSH_ _FG_RED_ _REVERSE_
						 " BREAKPOINT: %2d " _NORMAL_ 
						 " PC=%08x SP=%08x IPSR=%d Invalid thread!!"
						 _ATTR_POP_,
						 thread_id + 1, frm->pc, cm3_psp_get(), ipsr);
				/* suspend all threads */
				__thinkos_pause_all();
				/* diasble all breakpoints */
				dbgmon_breakpoint_clear_all();
				/* XXX: use IDLE as break thread id */
				thinkos_dbgmon_rt.break_id = THINKOS_THREAD_IDLE; 
				/* delivers a thread fault on next round */
				monitor_signal(MONITOR_KRN_EXCEPT);
#endif /* THINKOS_ENABLE_DEBUG_BKPT */
			} else {
#if (THINKOS_ENABLE_EXCEPTIONS)
				struct thinkos_except * xcpt;
			       xcpt = __thinkos_except_buf();

				/* FIXME: add support for breakpoints on IRQ */
				DCC_LOG3(LOG_ERROR,_ATTR_PUSH_ _FG_RED_ _REVERSE_
						 " IRQ BKPT " _NORMAL_ _FG_RED_
						 " PC=%08x SP=%08x IPSR=%d"
						 _ATTR_POP_, frm->pc, cm3_msp_get(), ipsr);
				/* suspend all threads */
				__thinkos_pause_all();
#if (THINKOS_ENABLE_DEBUG_BKPT)
				/* diasble all breakpoints */
				dbgmon_breakpoint_clear_all();
#endif
				/* record the break thread id */
				thinkos_dbgmon_rt.break_id = thread_id;
				/* save the current state of IPSR */
//				thinkos_rt.xcpt_ipsr = ipsr;
				/* Copy cuurent stack frame into exception
				   buffer */
				__thinkos_memcpy32(&xcpt->ctx.core.r0, frm,
								   sizeof(struct armv7m_basic_frame));
				/* delivers a kernel exception on next round */
				monitor_signal(MONITOR_KRN_EXCEPT);
#endif
			}
		} while (0);
		 /* (dfsr & SCB_DFSR_BKPT) */

#if (THINKOS_ENABLE_DEBUG_WPT)
		if (dfsr & SCB_DFSR_DWTTRAP) {
			unsigned int thread_id = __thinkos_active_get();

			if ((CM3_SCB->icsr & SCB_ICSR_RETTOBASE) == 0) {
				DCC_LOG2(LOG_ERROR, "<<WATCHPOINT>>: exception=%d pc=%08x", 
						 xpsr & 0x1ff, frm->pc);
				DCC_LOG(LOG_ERROR, "invalid breakpoint on exception!!!");
				/* FIXME: add support for breakpoints on IRQ */
				/* record the break thread id */
				thinkos_dbgmon_rt.break_id = thread_id;
				__thinkos_pause_all();

				monitor_signal(MONITOR_BREAKPOINT); 
			} else if ((uint32_t)thread_id < THINKOS_THREADS_MAX) {
				DCC_LOG2(LOG_TRACE, "<<WATCHPOINT>>: thread_id=%d pc=%08x ---", 
						 thread_id + 1, frm->pc);
				/* suspend the current thread */
				__thinkos_thread_pause(thread_id);
				/* record the break thread id */
				thinkos_dbgmon_rt.break_id = thread_id;
				monitor_signal(MONITOR_BREAKPOINT); 
				__thinkos_defer_sched();
			} else {
				DCC_LOG2(LOG_ERROR, "<<WATCHPOINT>>: thread_id=%d pc=%08x ---", 
						 thread_id + 1, frm->pc);
				DCC_LOG(LOG_ERROR, "invalid active thread!!!");
				monitor_signal(MONITOR_BREAKPOINT); 
				/* record the break thread id */
				thinkos_dbgmon_rt.break_id = thread_id;
				__thinkos_pause_all();
			}
		}
#endif /* THINKOS_ENABLE_DEBUG_WPT */

#if (THINKOS_ENABLE_DEBUG_STEP)
		if (dfsr & SCB_DFSR_HALTED) {
			unsigned int thread_id = thinkos_rt.step_id;

			uint32_t demcr;

			demcr = CM3_DCB->demcr;

			DCC_LOG3(LOG_INFO, "DEMCR=(REQ=%c)(PEND=%c)(STEP=%c)", 
					 demcr & DCB_DEMCR_MON_REQ ? '1' : '0',
					 demcr & DCB_DEMCR_MON_PEND ? '1' : '0',
					 demcr & DCB_DEMCR_MON_STEP ? '1' : '0');

			if (demcr & DCB_DEMCR_MON_STEP) {
				int ipsr = (xpsr & 0x1ff);
				/* Restore interrupts. The base priority was
				   set in the scheduler to perform a single step.  */
				cm3_basepri_set(0);

				if ((unsigned int)thread_id < THINKOS_THREADS_MAX) {
					DCC_LOG4(LOG_TRACE,_ATTR_PUSH_ _FG_GREEN_ _REVERSE_
							 " STEP: %2d " _NORMAL_ _FG_GREEN_
							 " PC=%08x SP=%08x IPSR=%d"
							 _ATTR_POP_,
							 thread_id + 1, frm->pc, cm3_psp_get(), ipsr);

					if (ipsr != 0) {
						DCC_LOG4(LOG_ERROR,_ATTR_PUSH_ _FG_RED_ _REVERSE_
								 " STEP: %2d " _NORMAL_ _FG_RED_
								 " PC=%08x SP=%08x IPSR=%d"
								 " Invalid step on exception!!"
								 _ATTR_POP_,
								 thread_id + 1, frm->pc, cm3_msp_get(), ipsr);
						goto step_done;
					}

					/* suspend the thread, this will clear the 
					   step request flag */
					__thinkos_thread_pause(thread_id);
					/* signal the monitor */
					monitor_signal(MONITOR_THREAD_STEP); 
					__thinkos_defer_sched();
				} else {
					DCC_LOG4(LOG_ERROR,_ATTR_PUSH_ _FG_RED_ _REVERSE_
							 " STEP: %2d " _NORMAL_ _FG_RED_
							 " PC=%08x SP=%08x IPSR=%d Invalid thread!!"
							 _ATTR_POP_,
							 thread_id + 1, frm->pc, cm3_psp_get(), ipsr);

				}
				thinkos_dbgmon_rt.break_id = thread_id;
step_done:
				CM3_DCB->demcr = demcr & ~DCB_DEMCR_MON_STEP;
			} else {
				DCC_LOG(LOG_WARNING, "/!\\ SCB_DFSR_HALTED /!\\");
			}
		}
#endif /* THINKOS_ENABLE_DEBUG_STEP */
	}

	return 0;
}


#if DEBUG
void __attribute__((noinline, noreturn)) 
	dbgmon_panic(struct thinkos_except * xcpt)
{
	asm volatile ("ldmia %0!, {r4-r11}\n"
				  "add   %0, %0, #16\n"
				  "mov   r12, sp\n"
				  "msr   PSP, r12\n"
				  "ldr   r12, [%0]\n"
				  "ldr   r13, [%0, #4]\n"
				  "ldr   r14, [%0, #8]\n"
				  "sub   %0, %0, #16\n"
				  "ldmia %0, {r0-r4}\n"
				  : : "r" (xcpt));
	for(;;);
}
#endif


/**
 * dbgmon_soft_reset:
 *
 * Reinitialize the plataform by reseting all ThinkOS subsystems.
 * 
 */

void dbgmon_soft_reset(void)
{
	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"1. Disable all interrupt on NVIC " _ATTR_POP_); 
	__dbgmon_irq_disable_all();

	//DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
	//		"2. Kill all threads"  _ATTR_POP_); 
	//__thinkos_kill_all(); 

	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"3. Core reset..."   _ATTR_POP_); 
	__thinkos_core_reset();

#if (THINKOS_ENABLE_EXCEPTIONS)
	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"4. Except reset..."   _ATTR_POP_); 
	__exception_reset();
#endif

#if 0
#if (THINKOS_ENABLE_IDLE_HOOKS)
	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"5. Wait IDLE signal..."   _ATTR_POP_); 
	dbgmon_wait_idle();
#endif
#endif

	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"6. Send soft reset signal"  _ATTR_POP_);
	monitor_signal(MONITOR_SOFTRST); 
}

/* -------------------------------------------------------------------------
 * ThinkOS kernel level API
 * ------------------------------------------------------------------------- */

void thinkos_dbgmon_reset(void)
{
#if (THINKOS_ENABLE_DEBUG_BKPT)
	DCC_LOG(LOG_TRACE, "1. clear all breakpoints...");
	dbgmon_breakpoint_clear_all();
#endif

#if (THINKOS_DBGMON_ENABLE_RST_VEC)
	DCC_LOG(LOG_TRACE, "2. reset RAM vectors...");
	__reset_ram_vectors();
#endif

	thinkos_dbgmon_rt.thread_id = -1;
	thinkos_dbgmon_rt.errno = THINKOS_NO_ERROR;
	thinkos_dbgmon_rt.code = 0;
}

void __thinkos_dbgmon_init(void)
{
	struct cm3_dcb * dcb = CM3_DCB;
	uint32_t demcr; 

	if (((demcr = dcb->demcr) & DCB_DEMCR_MON_EN) == 0) {
		DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
		" ==== Debug/Monitor startup ==== "_ATTR_POP_);
		thinkos_dbgmon_reset();
#if (THINKOS_ENABLE_DEBUG_STEP)
		/* clear the step request */
		demcr &= ~DCB_DEMCR_MON_STEP;
		/* enable the FPB unit */
		CM3_FPB->ctrl = FP_KEY | FP_ENABLE;
#endif

	/* XXX: enabling all vector catch? Not sure if this is
	   really necessary. At least the breakpoint somehow depends
	   on this on certain Cortex-M4 processors !!!!... */
//	demcr |= DCB_DEMCR_VC_HARDERR | DCB_DEMCR_VC_INTERR | DCB_DEMCR_VC_BUSERR |
//		DCB_DEMCR_VC_STATERR | DCB_DEMCR_VC_CHKERR | DCB_DEMCR_VC_NOCPERR |
//		DCB_DEMCR_VC_MMERR | DCB_DEMCR_VC_CORERESET;

		/* clear semaphore */
		demcr &= ~(DCB_DEMCR_MON_REQ );
	} 
	
	/* disable monitor and clear semaphore */
	dcb->demcr = demcr & ~(DCB_DEMCR_MON_EN | DCB_DEMCR_MON_PEND);

	/* enable monitor and send the reset event */
	dcb->demcr = demcr | DCB_DEMCR_MON_EN | DCB_DEMCR_MON_PEND;
}

#endif /* THINKOS_ENABLE_MONITOR && !THINKOS_ENABLE_USAGEFAULT_MONITOR */


