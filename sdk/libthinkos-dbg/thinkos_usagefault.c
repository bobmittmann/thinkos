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

#if (THINKOS_ENABLE_MONITOR) && (THINKOS_ENABLE_USAGEFAULT_MONITOR)

#ifdef THINKOS_DBGMON_ENABLE_IRQ_MGMT
#warning "Deprecated THINKOS_DBGMON_ENABLE_IRQ_MGMT"
#endif

#if (THINKOS_ENABLE_DEBUG_BKPT && !THINKOS_ENABLE_THREAD_VOID)
#error "Need THINKOS_ENABLE_THREAD_VOID"
#endif

#define DBGMON_PERISTENT_MASK ((1 << DBGMON_RESET) | \
							   (1 << DBGMON_SOFTRST))

#define NVIC_IRQ_REGS ((THINKOS_IRQ_MAX + 31) / 32)

struct thinkos_dbgmon {
	uint32_t * ctx;           /* monitor context */
	struct dbgmon_comm * comm;
	volatile uint32_t mask;   /* events mask */
	volatile uint32_t events; /* events bitmap */
	void * param;             /* user supplied parameter */
	int8_t thread_id;
	int8_t break_id;
	uint8_t errno;
	int32_t code;
	void (* task)(const struct dbgmon_comm *, void *);
};

struct thinkos_dbgmon thinkos_dbgmon_rt;

uint32_t __attribute__((aligned(8))) 
	thinkos_dbgmon_stack[THINKOS_DBGMON_STACK_SIZE / 4];
const uint16_t thinkos_dbgmon_stack_size = sizeof(thinkos_dbgmon_stack);

int dbgmon_context_swap(uint32_t ** pctx); 

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

void dbgmon_signal(int sig) 
{
	uint32_t evset;
	
	do {
		/* avoid possible race condition on dbgmon.events */
		evset = __ldrex((uint32_t *)&thinkos_dbgmon_rt.events);
		evset |= (1 << sig);
	} while (__strex((uint32_t *)&thinkos_dbgmon_rt.events, evset));

	asm volatile ("isb\n" :  :  : );
}

bool dbgmon_is_set(int sig) 
{
	return thinkos_dbgmon_rt.events &  (1 << sig) ? true : false;
}

void dbgmon_unmask(int sig)
{
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&thinkos_dbgmon_rt.mask);
		mask |= (1 << sig);
	} while (__strex((uint32_t *)&thinkos_dbgmon_rt.mask, mask));
}

void dbgmon_mask(int sig)
{
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&thinkos_dbgmon_rt.mask);
		mask &= ~(1 << sig);
	} while (__strex((uint32_t *)&thinkos_dbgmon_rt.mask, mask));
}

void dbgmon_clear(int sig)
{
	uint32_t evact;
	uint32_t evset;

	do {
		/* avoid possible race condition on dbgmon.events */
		evset = __ldrex((uint32_t *)&thinkos_dbgmon_rt.events);
		evact = evset & (1 << sig);
		evset ^= evact;
	} while (__strex((uint32_t *)&thinkos_dbgmon_rt.events, evset));
}

/* wait for multiple events, return the highest priority (smaller number)
   don't clear the event upon exiting. */
int dbgmon_select(uint32_t evmsk)
{
	int evset;
	uint32_t save;
	int sig;

	save = thinkos_dbgmon_rt.mask;
	/* set the local mask */
	evmsk |= save | DBGMON_PERISTENT_MASK;
	DCC_LOG1(LOG_MSG, "evmask=%08x ........ ", evmsk);
	thinkos_dbgmon_rt.mask = evmsk;
	for(;;) {
		evset = thinkos_dbgmon_rt.events;
		/* apply local and global masks, 
		   making sure not to mask non maskable events */
		evset &= evmsk;
		/* select the event with highest priority */
		sig = __clz(__rbit(evset & evmsk));

		if (sig < 32)
			break;

		DCC_LOG1(LOG_MSG, "waiting for events evmsk=%08x sleeping...", evmsk);
		dbgmon_context_swap(&thinkos_dbgmon_rt.ctx); 
		DCC_LOG(LOG_MSG, "wakeup...");
	} 
	thinkos_dbgmon_rt.mask = save;

	DCC_LOG1(LOG_MSG, "event=%d", sig);

	return sig;
}


/* wait for a single event and clear the event */
int dbgmon_expect(int sig)
{
	uint32_t save;
	uint32_t evset;
	uint32_t evmsk;
	uint32_t result;

	save = thinkos_dbgmon_rt.mask;
	/* set the local mask */
	evmsk = save | (1 << sig) | DBGMON_PERISTENT_MASK;
	thinkos_dbgmon_rt.mask = evmsk;

	for (;;) {
		/* avoid possible race condition on dbgmon.events */
		do {
			evset = __ldrex((uint32_t *)&thinkos_dbgmon_rt.events);
			/* apply local and global masks, 
			   making sure not to mask non maskable events */
			result = evset;
			result &= evmsk;
			if (result & (1 << sig))
				evset &= ~(1 << sig); /* clear the event */
		} while (__strex((uint32_t *)&thinkos_dbgmon_rt.events, evset));

		if (result != 0)
			break;

		if (evset != 0) {
			DCC_LOG3(LOG_MSG, "expected %08x got %08x/%08x, sleeping...", 
					 (1 << sig), evset, evmsk);
		}
		dbgmon_context_swap(&thinkos_dbgmon_rt.ctx); 
		if (evset != 0) {
			DCC_LOG(LOG_MSG, "wakeup...");
		}
	}

	thinkos_dbgmon_rt.mask = save;

	if ((result & (1 << sig)) == 0) {
		/* unexpected event received */
		sig = -1;
		DCC_LOG3(LOG_WARNING, "expected %08x got %08x/%08x, sleeping...", 
				 (1 << sig), result, evmsk);
	}

	return sig;
}

int dbgmon_sleep(unsigned int ms)
{
	dbgmon_clear(DBGMON_ALARM);
#if (THINKOS_ENABLE_KRNMON_CLOCK)
	/* set the clock */
	thinkos_rt.dmclock = thinkos_rt.ticks + ms;
#endif
	/* wait for signal */
	return dbgmon_expect(DBGMON_ALARM);
}

void dbgmon_alarm(unsigned int ms)
{
	DCC_LOG1(LOG_MSG, "alarm at %d ms!", ms);
	dbgmon_clear(DBGMON_ALARM);
	dbgmon_unmask(DBGMON_ALARM);
#if (THINKOS_ENABLE_KRNMON_CLOCK)
	/* set the clock */
	thinkos_rt.dmclock = thinkos_rt.ticks + ms;
#endif
}

void dbgmon_alarm_stop(void)
{
#if (THINKOS_ENABLE_KRNMON_CLOCK)
	/* set the clock in the past so it won't generate a signal */
	thinkos_rt.dmclock = thinkos_rt.ticks - 1;
#endif
	/* make sure the signal is cleared */
	dbgmon_clear(DBGMON_ALARM);
	/* mask the signal */
	dbgmon_mask(DBGMON_ALARM);
}

int dbgmon_wait_idle(void)
{
#if (THINKOS_ENABLE_IDLE_HOOKS)
	uint32_t save;
	uint32_t evset;
	uint32_t evmsk;
	uint32_t result;

	/* Issue an idle hook request */
	__idle_hook_req(IDLE_HOOK_NOTIFY_DBGMON);

	save = thinkos_dbgmon_rt.mask;

	/* set the local mask */
	evmsk = (1 << DBGMON_IDLE);
	thinkos_dbgmon_rt.mask = evmsk;
	
	do {
		dbgmon_context_swap(&thinkos_dbgmon_rt.ctx); 
		do { /* avoid possible race condition on dbgmon.events */
			evset = __ldrex((uint32_t *)&thinkos_dbgmon_rt.events);
			result = evset;
			evset &= evmsk; /* clear the event */
		} while (__strex((uint32_t *)&thinkos_dbgmon_rt.events, evset));
	} while (result == 0);
	thinkos_dbgmon_rt.mask = save;

	return (result == evmsk) ? 0 : -1;
#else
	return 0;
#endif
}

int dbgmon_thread_terminate_get(int * code)
{
	int thread_id;

	if ((thread_id = thinkos_dbgmon_rt.thread_id) >= 0) {
		if (code != NULL) {
			*code = thinkos_dbgmon_rt.code;
		}
	}

	return thread_id;
}

static inline void __dbgmon_task_reset(void)
{
	dbgmon_signal(DBGMON_RESET);
	dbgmon_context_swap(&thinkos_dbgmon_rt.ctx); 
}

void __attribute__((naked)) 
	dbgmon_exec(void (* task)(const struct dbgmon_comm *, void *), 
				void * param)
{
	DCC_LOG1(LOG_TRACE, "task=%p", task);
	thinkos_dbgmon_rt.task = task;
	thinkos_dbgmon_rt.param = param;
	__dbgmon_task_reset();
}

#if (THINKOS_ENABLE_THREAD_VOID)
  #define THINKOS_THREAD_LAST (THINKOS_THREADS_MAX + 2)
#else
  #define THINKOS_THREAD_LAST (THINKOS_THREADS_MAX + 1)
#endif

int dbgmon_thread_inf_get(unsigned int id, struct dbgmon_thread_inf * inf)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;
	unsigned int errno = THINKOS_NO_ERROR;
	struct thinkos_context * ctx;
	unsigned int thread_id = id;
	uint32_t pc = 0;
	uint32_t sp = 0;

	if (thread_id > THINKOS_THREAD_LAST) {
		DCC_LOG(LOG_ERROR, "Invalid thread!");
		return -1;
	}

	if (thread_id == xcpt->active) {
		ctx = &xcpt->ctx.core;
		errno = xcpt->errno;
		pc = ctx->pc;
#if (THINKOS_ENABLE_IDLE_MSP) || (THINKOS_ENABLE_FPU)
		sp = (uint32_t)ctx->sp;
		sp += (ctx->ret & CM3_EXC_RET_nFPCA) ? (8*4) : (26*4);
		DCC_LOG3(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
				 "<%d> SP=%08x! RET=[%s]!" _ATTR_POP_, 
				 thread_id + 1, sp, __retstr(ctx->ret));				
#else
		sp = (uint32_t)ctx + sizeof(struct thinkos_context);
#endif
	} else if (thread_id == THINKOS_THREAD_IDLE) {
		ctx  = __thinkos_idle_ctx();
		pc = ctx->pc;
		sp = (uint32_t)ctx + sizeof(struct thinkos_context);
	} else {
		ctx = thinkos_rt.ctx[thread_id];
		if (thread_id == (unsigned int)thinkos_dbgmon_rt.break_id)
			errno = thinkos_dbgmon_rt.errno;

		if (((uint32_t)ctx < 0x10000000) || ((uint32_t)ctx >= 0x30000000)) {
			DCC_LOG2(LOG_ERROR, "<%d> context 0x%08x invalid!!!", 
					 thread_id + 1, ctx);
			return -1;
		}
#if (THINKOS_ENABLE_IDLE_MSP) || (THINKOS_ENABLE_FPU)
		sp = (uint32_t)ctx->sp;
		sp += (ctx->ret & CM3_EXC_RET_nFPCA) ? (8*4) : (26*4);
		DCC_LOG3(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
				 "<%d> SP=%08x! RET=[%s]!" _ATTR_POP_, 
				 thread_id + 1, sp, __retstr(ctx->ret));				
#else
		sp = (uint32_t)ctx + sizeof(struct thinkos_context);
#endif
		pc = ctx->pc;
	}

	if (inf != NULL) {
		inf->pc = pc;
		inf->sp = sp;
		inf->errno = errno;
		inf->thread_id = id;
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
		if ((ctx = thinkos_rt.ctx[thread_id]) == NULL)
			return -1;
	}

	return thread_id;
}

void dbgmon_thread_break_clr(void)
{
	thinkos_dbgmon_rt.break_id = -1;
	thinkos_dbgmon_rt.errno = 0;
}

/* -------------------------------------------------------------------------
 * Debug Monitor Core
 * ------------------------------------------------------------------------- */

void dbgmon_null_task(const struct dbgmon_comm * comm, void * param)
{
	for (;;) {
		dbgmon_context_swap(&thinkos_dbgmon_rt.ctx); 
	}
}

static void __attribute__((naked, noreturn)) dbgmon_bootstrap(void)
{
	const struct dbgmon_comm * comm = thinkos_dbgmon_rt.comm; 
	void (* dbgmon_task)(const struct dbgmon_comm *, void *);
	void * param = thinkos_dbgmon_rt.param; 

	param = thinkos_dbgmon_rt.param; 

	/* Get the new task */
	dbgmon_task = thinkos_dbgmon_rt.task; 
	/* Set the new task to NULL */
	thinkos_dbgmon_rt.task = dbgmon_null_task;
	
	/* set the clock in the past so it won't generate signals in 
	 the near future */
#if (THINKOS_ENABLE_KRNMON_CLOCK)
	thinkos_rt.dmclock = thinkos_rt.ticks - 1;
#endif

	dbgmon_task(comm, param);

	DCC_LOG(LOG_WARNING, "Debug monitor task returned!");

	__dbgmon_task_reset();
}

void __except_ctx_cpy(struct thinkos_context * ctx)
{
	uint32_t * dst = (uint32_t *)&thinkos_except_buf.ctx.core;
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

int thinkos_dbgmon_isr(struct armv7m_basic_frame * frm, uint32_t ret)
{
	uint32_t sigset = thinkos_dbgmon_rt.events;
	uint32_t sigmsk = thinkos_dbgmon_rt.mask;

	DCC_LOG1(LOG_MSG, "sigset=%08x", sigset);

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
			unsigned int thread_id = thinkos_rt.active;
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
					dbgmon_signal(DBGMON_BREAKPOINT); 
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
					dbgmon_signal(DBGMON_THREAD_FAULT); 
					/* run scheduler */
					__thinkos_defer_sched();
					break;
				} 
#endif /* THINKOS_ENABLE_ERROR_TRAP */

#if (THINKOS_ENABLE_MONITOR_THREADS)
				if (code == DBGMON_BKPT_ON_THREAD_CREATE) {
					DCC_LOG3(LOG_WARNING, _ATTR_PUSH_ _FG_YELLOW_ _REVERSE_
							 " THREAD CREATE" _NORMAL_ _FG_YELLOW_
							 " PC=%08x LR=%08x R0=%d" _ATTR_POP_, 
							 frm->pc, frm->lr, frm->r0);
					thinkos_dbgmon_rt.thread_id = frm->r0;
					thinkos_dbgmon_rt.code = 0;
					dbgmon_signal(DBGMON_THREAD_CREATE);
					break;
				} 
				if (code == DBGMON_BKPT_ON_THREAD_TERMINATE) {
					DCC_LOG4(LOG_WARNING, _ATTR_PUSH_ _FG_YELLOW_ _REVERSE_
							 " THREAD TERMINATE" _NORMAL_ _FG_YELLOW_
							 " PC=%08x LR=%08x R0=%d R1=%d" _ATTR_POP_, 
							 frm->pc, frm->lr, frm->r0, frm->r1);
					thinkos_dbgmon_rt.thread_id = frm->r0;
					thinkos_dbgmon_rt.code = frm->r1;
					dbgmon_signal(DBGMON_THREAD_TERMINATE);
					break;
				} 
#endif /* THINKOS_ENABLE_MONITOR_THREADS */
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
				thinkos_dbgmon_rt.events |= (1 << DBGMON_KRN_EXCEPT);
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
					thinkos_dbgmon_rt.events |= (1 << DBGMON_BREAKPOINT);
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
				thinkos_dbgmon_rt.events |= (1 << DBGMON_KRN_EXCEPT);
#endif /* THINKOS_ENABLE_DEBUG_BKPT */
			} else {
#if (THINKOS_ENABLE_EXCEPTIONS)
				struct thinkos_except * xcpt = &thinkos_except_buf;

				/* FIXME: add support for breakpoints on IRQ */
				DCC_LOG3(LOG_ERROR,_ATTR_PUSH_ _FG_RED_ _REVERSE_
						 " IRQ BKPT " _NORMAL_ _FG_RED_
						 " PC=%08x SP=%08x IPSR=%d"
						 _ATTR_POP_, frm->pc, cm3_msp_get(), ipsr);
				/* suspend all threads */
				__thinkos_pause_all();
				/* record the break thread id */
				thinkos_dbgmon_rt.break_id = thread_id;
				/* save the current state of IPSR */
//				thinkos_rt.xcpt_ipsr = ipsr;
				/* Copy cuurent stack frame into exception
				   buffer */
				__thinkos_memcpy32(&xcpt->ctx.core.r0, frm,
								   sizeof(struct armv7m_basic_frame));
				/* delivers a kernel exception on next round */
				thinkos_dbgmon_rt.events |= (1 << DBGMON_KRN_EXCEPT);
#endif
			}
		} while (0);
		 /* (dfsr & SCB_DFSR_BKPT) */


	}

	if (sigset & (1 << DBGMON_RESET)) {
		uint32_t * sp;
		DCC_LOG(LOG_TRACE, "DBGMON_RESET");
		sp = &thinkos_dbgmon_stack[(sizeof(thinkos_dbgmon_stack) / 4) - 10];
		sp[0] = CM_EPSR_T + CM3_EXCEPT_DEBUG_MONITOR; /* CPSR */
		sp[9] = ((uint32_t)dbgmon_bootstrap) | 1; /* LR */
		thinkos_dbgmon_rt.ctx = sp;
		/* clear the RESET event */
		thinkos_dbgmon_rt.events = sigset & ~(1 << DBGMON_RESET);
		/* make sure the RESET event is not masked */
		sigmsk |= (1 << DBGMON_RESET);
	}

	/* Process monitor events */
	if ((sigset & sigmsk) != 0) {
		DCC_LOG1(LOG_MSG, "monitor ctx=%08x", thinkos_dbgmon_rt.ctx);
#if DEBUG
		/* TODO: this stack check is very usefull... 
		   Some sort of error to the developer should be raised or
		 force a fault */
		if (thinkos_dbgmon_rt.ctx < thinkos_dbgmon_stack) {
			DCC_LOG(LOG_PANIC, "stack overflow!");
			DCC_LOG2(LOG_PANIC, "monitor.ctx=%08x dbgmon.stack=%08x", 
					 thinkos_dbgmon_rt.ctx, thinkos_dbgmon_stack);
			DCC_LOG2(LOG_PANIC, "sigset=%08x sigmsk=%08x", sigset, sigmsk);
		}
#endif
		return dbgmon_context_swap(&thinkos_dbgmon_rt.ctx); 
	}

	DCC_LOG2(LOG_MSG, "Unhandled signal sigset=%08x sigmsk=%08x", 
			 sigset, sigmsk);
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
	dbgmon_signal(DBGMON_SOFTRST); 
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

void thinkos_dbgmon_svc(int32_t arg[], int self)
{
	void (* task)(const struct dbgmon_comm *, void *) = 
		(void (*)(const struct dbgmon_comm *, void *))arg[0];
	struct dbgmon_comm * comm = (void *)arg[1];
	void * param = (void *)arg[2];

	/* set the startup signal */
	thinkos_dbgmon_rt.events = (1 << DBGMON_STARTUP);

	/* Set the persistent signals */
	thinkos_dbgmon_rt.events |= (1 << DBGMON_RESET);
	thinkos_dbgmon_rt.mask = DBGMON_PERISTENT_MASK | (1 << DBGMON_STARTUP);
	thinkos_dbgmon_rt.comm = comm;

	arg[0] = (uint32_t)thinkos_dbgmon_rt.task;
	if (task == NULL)
		/* Set the new task to NULL */
		thinkos_dbgmon_rt.task = dbgmon_null_task;
	else
		thinkos_dbgmon_rt.task = task;

	thinkos_dbgmon_rt.param = param;
	thinkos_dbgmon_rt.ctx = 0;
}

#endif /* THINKOS_ENABLE_MONITOR && THINKOS_ENABLE_USAGEFAULT_MONITOR */


