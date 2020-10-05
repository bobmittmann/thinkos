/* 
 * File:	 thinkos-monitor.c
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

#if (THINKOS_ENABLE_MONITOR) 

#define MONITOR_PERISTENT_MASK ((1 << MONITOR_RESET) | \
							   (1 << MONITOR_SOFTRST))

#define NVIC_IRQ_REGS ((THINKOS_IRQ_MAX + 31) / 32)

struct thinkos_monitor {
	volatile uint32_t mask;   /* events mask */
	volatile uint32_t events; /* events bitmap */
	uint32_t * ctx;           /* monitor context */
	struct monitor_comm * comm;
	void * param;             /* user supplied parameter */
	int8_t thread_id;
	int8_t break_id;
	uint8_t errno;
	uint8_t enabled;
	int32_t code;
	void (* task)(const struct monitor_comm *, void *);
};

struct thinkos_monitor thinkos_monitor_rt;

uint32_t __attribute__((aligned(8))) 
	thinkos_monitor_stack[THINKOS_MONITOR_STACK_SIZE / 4];
const uint16_t thinkos_monitor_stack_size = sizeof(thinkos_monitor_stack);

void __monitor_context_swap(uint32_t ** pctx); 

/**
  * __monitor_irq_disable_all:
  *
  * Disable all interrupts by clearing the interrupt enable bit
  * of all interrupts on the Nested Vector Interrupt Controller (NVIC).
  *
  * Also the interrupt enable backup is cleared to avoid 
  * interrupts being reenabled by calling __monitor_irq_restore_all().
  *
  * The systick interrupt is not disabled.
  */
static void __monitor_irq_disable_all(void)
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


#if (THINKOS_MONITOR_ENABLE_RST_VEC)
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
#endif /* THINKOS_MONITOR_ENABLE_RST_VEC */

/* -------------------------------------------------------------------------
 * Debug Monitor API
 * ------------------------------------------------------------------------- */

void monitor_signal(int sig) 
{
	struct cm3_scb * scb = CM3_SCB;
	uint32_t evset;
	
	do {
		/* avoid possible race condition on monitor.events */
		evset = __ldrex((uint32_t *)&thinkos_monitor_rt.events);
		evset |= (1 << sig);
	} while (__strex((uint32_t *)&thinkos_monitor_rt.events, evset));

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;

	asm volatile ("isb\n" :  :  : );
}

bool monitor_is_set(int sig) 
{
	return thinkos_monitor_rt.events &  (1 << sig) ? true : false;
}

void monitor_unmask(int sig)
{
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&thinkos_monitor_rt.mask);
		mask |= (1 << sig);
	} while (__strex((uint32_t *)&thinkos_monitor_rt.mask, mask));
}

void monitor_mask(int sig)
{
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&thinkos_monitor_rt.mask);
		mask &= ~(1 << sig);
	} while (__strex((uint32_t *)&thinkos_monitor_rt.mask, mask));
}

void monitor_clear(int sig)
{
	uint32_t evact;
	uint32_t evset;

	do {
		/* avoid possible race condition on monitor.events */
		evset = __ldrex((uint32_t *)&thinkos_monitor_rt.events);
		evact = evset & (1 << sig);
		evset ^= evact;
	} while (__strex((uint32_t *)&thinkos_monitor_rt.events, evset));
}

/* wait for multiple events, return the highest priority (smaller number)
   don't clear the event upon exiting. */
int monitor_select(uint32_t evmsk)
{
	int evset;
	uint32_t save;
	int sig;

	save = thinkos_monitor_rt.mask;
	/* set the local mask */
	evmsk |= save | MONITOR_PERISTENT_MASK;
	DCC_LOG1(LOG_MSG, "evmask=%08x ........ ", evmsk);
	thinkos_monitor_rt.mask = evmsk;
	for(;;) {
		evset = thinkos_monitor_rt.events;
		/* apply local and global masks, 
		   making sure not to mask non maskable events */
		evset &= evmsk;
		/* select the event with highest priority */
		sig = __clz(__rbit(evset & evmsk));

		if (sig < 32)
			break;

		DCC_LOG1(LOG_MSG, "waiting for events evmsk=%08x sleeping...", evmsk);
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
		DCC_LOG(LOG_MSG, "wakeup...");
	} 
	thinkos_monitor_rt.mask = save;

	DCC_LOG1(LOG_MSG, "event=%d", sig);

	return sig;
}


/* wait for a single event and clear the event */
int monitor_expect(int sig)
{
	uint32_t save;
	uint32_t evset;
	uint32_t evmsk;
	uint32_t result;

	save = thinkos_monitor_rt.mask;
	/* set the local mask */
	evmsk = save | (1 << sig) | MONITOR_PERISTENT_MASK;
	thinkos_monitor_rt.mask = evmsk;

	for (;;) {
		/* avoid possible race condition on monitor.events */
		do {
			evset = __ldrex((uint32_t *)&thinkos_monitor_rt.events);
			/* apply local and global masks, 
			   making sure not to mask non maskable events */
			result = evset;
			result &= evmsk;
			if (result & (1 << sig))
				evset &= ~(1 << sig); /* clear the event */
		} while (__strex((uint32_t *)&thinkos_monitor_rt.events, evset));

		if (result != 0)
			break;

		if (evset != 0) {
			DCC_LOG3(LOG_MSG, "expected %08x got %08x/%08x, sleeping...", 
					 (1 << sig), evset, evmsk);
		}
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
		if (evset != 0) {
			DCC_LOG(LOG_MSG, "wakeup...");
		}
	}

	thinkos_monitor_rt.mask = save;

	if ((result & (1 << sig)) == 0) {
		/* unexpected event received */
		sig = -1;
		DCC_LOG3(LOG_WARNING, "expected %08x got %08x/%08x, sleeping...", 
				 (1 << sig), result, evmsk);
	}

	return sig;
}

int monitor_sleep(unsigned int ms)
{
	monitor_clear(MONITOR_ALARM);
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	/* set the clock */
	thinkos_rt.monitor_clock = thinkos_rt.ticks + ms;
#endif
	/* wait for signal */
	return monitor_expect(MONITOR_ALARM);
}

void monitor_alarm(unsigned int ms)
{
	DCC_LOG1(LOG_MSG, "alarm at %d ms!", ms);
	monitor_clear(MONITOR_ALARM);
	monitor_unmask(MONITOR_ALARM);
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	/* set the clock */
	thinkos_rt.monitor_clock = thinkos_rt.ticks + ms;
#endif
}

void monitor_alarm_stop(void)
{
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	/* set the clock in the past so it won't generate a signal */
	thinkos_rt.monitor_clock = thinkos_rt.ticks - 1;
#endif
	/* make sure the signal is cleared */
	monitor_clear(MONITOR_ALARM);
	/* mask the signal */
	monitor_mask(MONITOR_ALARM);
}

int monitor_wait_idle(void)
{
#if (THINKOS_ENABLE_IDLE_HOOKS)
	uint32_t save;
	uint32_t evset;
	uint32_t evmsk;
	uint32_t result;

	/* Issue an idle hook request */
	__idle_hook_req(IDLE_HOOK_NOTIFY_MONITOR);

	save = thinkos_monitor_rt.mask;

	/* set the local mask */
	evmsk = (1 << MONITOR_IDLE);
	thinkos_monitor_rt.mask = evmsk;
	
	do {
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
		do { /* avoid possible race condition on monitor.events */
			evset = __ldrex((uint32_t *)&thinkos_monitor_rt.events);
			result = evset;
			evset &= evmsk; /* clear the event */
		} while (__strex((uint32_t *)&thinkos_monitor_rt.events, evset));
	} while (result == 0);
	thinkos_monitor_rt.mask = save;

	return (result == evmsk) ? 0 : -1;
#else
	return 0;
#endif
}

int monitor_thread_terminate_get(int * code)
{
	int thread_id;

	if ((thread_id = thinkos_monitor_rt.thread_id) >= 0) {
		if (code != NULL) {
			*code = thinkos_monitor_rt.code;

		}
	}

	return thread_id;
}
 
void monitor_signal_thread_terminate(int thread_id, int code) 
{
//	register int r0 asm("r0") = (int)thread_id;
//	register int r1 asm("r1") = (int)code;
	thinkos_monitor_rt.thread_id = thread_id;
	thinkos_monitor_rt.code = code;
	monitor_signal(MONITOR_THREAD_TERMINATE);
//	asm volatile ("udf %0 \n" : : "I" (MONITOR_BKPT_ON_THREAD_TERMINATE), 
//				  "r"(r0), "r"(r1) );
}


static inline void __monitor_task_reset(void)
{
	monitor_signal(MONITOR_RESET);
	__monitor_context_swap(&thinkos_monitor_rt.ctx); 
}

void __attribute__((naked)) 
	monitor_exec(void (* task)(const struct monitor_comm *, void *), 
				void * param)
{
	DCC_LOG1(LOG_TRACE, "task=%p", task);
	thinkos_monitor_rt.task = task;
	thinkos_monitor_rt.param = param;
	__monitor_task_reset();
}

#if (THINKOS_ENABLE_THREAD_VOID)
  #define THINKOS_THREAD_LAST (THINKOS_THREADS_MAX + 2)
#else
  #define THINKOS_THREAD_LAST (THINKOS_THREADS_MAX + 1)
#endif

int monitor_thread_inf_get(unsigned int id, struct monitor_thread_inf * inf)
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
		ctx = __thinkos_thread_ctx_get(thread_id);
		if (thread_id == (unsigned int)thinkos_monitor_rt.break_id)
			errno = thinkos_monitor_rt.errno;

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

int monitor_errno_get(void)
{
	return  thinkos_monitor_rt.errno;
}

int monitor_thread_break_get(void)
{
	struct thinkos_context * ctx;
	int thread_id;

	if ((thread_id = thinkos_monitor_rt.break_id) >= 0) {
		if ((ctx = __thinkos_thread_ctx_get(thread_id)) == NULL)
			return -1;
	}

	return thread_id;
}

void monitor_thread_break_clr(void)
{
	thinkos_monitor_rt.break_id = -1;
	thinkos_monitor_rt.errno = 0;
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

bool monitor_breakpoint_set(uint32_t addr, uint32_t size)
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

bool monitor_breakpoint_clear(uint32_t addr, uint32_t size)
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

bool monitor_breakpoint_disable(uint32_t addr)
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

void monitor_breakpoint_clear_all(void)
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

bool monitor_watchpoint_set(uint32_t addr, uint32_t size, int access)
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

bool monitor_watchpoint_clear(uint32_t addr, uint32_t size)
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


void monitor_watchpoint_clear_all(void)
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

int monitor_thread_step(unsigned int thread_id, bool sync)
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

int monitor_thread_step_get(void)
{
	struct thinkos_context * ctx;
	int thread_id;

	if ((thread_id = thinkos_rt.step_id) >= 0) {
		if ((ctx = __thinkos_thread_ctx_get(thread_id)) == NULL)
			return -1;
	}

	return thread_id;
}

void monitor_thread_step_clr(void)
{
	thinkos_rt.step_id = -1;
}

#endif /* THINKOS_ENABLE_DEBUG_STEP */



/* -------------------------------------------------------------------------
 * Debug Monitor Core
 * ------------------------------------------------------------------------- */

void monitor_null_task(const struct monitor_comm * comm, void * param)
{
	for (;;) {
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
	}
}

static void __attribute__((naked, noreturn)) monitor_bootstrap(void)
{
	const struct monitor_comm * comm = thinkos_monitor_rt.comm; 
	void (* monitor_task)(const struct monitor_comm *, void *);
	void * param = thinkos_monitor_rt.param; 

	param = thinkos_monitor_rt.param; 

	/* Get the new task */
	monitor_task = thinkos_monitor_rt.task; 
	/* Set the new task to NULL */
	thinkos_monitor_rt.task = monitor_null_task;
	
	/* set the clock in the past so it won't generate signals in 
	 the near future */
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	thinkos_rt.monitor_clock = thinkos_rt.ticks - 1;
#endif

	monitor_task(comm, param);

	DCC_LOG(LOG_WARNING, "Debug monitor task returned!");

	__monitor_task_reset();
}

void __attribute__((aligned(16))) __thinkos_monitor_isr(void)
{
	uint32_t sigset = thinkos_monitor_rt.events;
	uint32_t sigmsk = thinkos_monitor_rt.mask;

	DCC_LOG1(LOG_JABBER, "sigset=%08x", sigset);

	/* read SCB Debug Fault Status Register */

	if (sigset & (1 << MONITOR_RESET)) {
		uint32_t * sp;
		DCC_LOG(LOG_TRACE, "MONITOR_RESET");
		sp = &thinkos_monitor_stack[(sizeof(thinkos_monitor_stack) / 4) - 10];
		sp[0] = CM_EPSR_T + CM3_EXCEPT_DEBUG_MONITOR; /* CPSR */
		sp[9] = ((uint32_t)monitor_bootstrap) | 1; /* LR */
		thinkos_monitor_rt.ctx = sp;
		/* clear the RESET event */
		thinkos_monitor_rt.events = sigset & ~(1 << MONITOR_RESET);
		/* make sure the RESET event is not masked */
		sigmsk |= (1 << MONITOR_RESET);
	}

	/* Process monitor events */
	if ((sigset & sigmsk) != 0) {
		DCC_LOG1(LOG_MSG, "monitor ctx=%08x", thinkos_monitor_rt.ctx);
#if DEBUG
		/* TODO: this stack check is very usefull... 
		   Some sort of error to the developer should be raised or
		 force a fault */
		if (thinkos_monitor_rt.ctx < thinkos_monitor_stack) {
			DCC_LOG(LOG_PANIC, "stack overflow!");
			DCC_LOG2(LOG_PANIC, "monitor.ctx=%08x monitor.stack=%08x", 
					 thinkos_monitor_rt.ctx, thinkos_monitor_stack);
			DCC_LOG2(LOG_PANIC, "sigset=%08x sigmsk=%08x", sigset, sigmsk);
		}
#endif
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
	}
}


#if DEBUG
void __attribute__((noinline, noreturn)) 
	monitor_panic(struct thinkos_except * xcpt)
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
 * monitor_soft_reset:
 *
 * Reinitialize the plataform by reseting all ThinkOS subsystems.
 * 
 */

void monitor_soft_reset(void)
{
	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"1. Disable all interrupt on NVIC " _ATTR_POP_); 
	__monitor_irq_disable_all();

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
	monitor_wait_idle();
#endif
#endif

	DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
			"6. Send soft reset signal"  _ATTR_POP_);
	monitor_signal(MONITOR_SOFTRST); 
}

/* -------------------------------------------------------------------------
 * ThinkOS kernel level API
 * ------------------------------------------------------------------------- */

void thinkos_monitor_reset(void)
{
#if (THINKOS_ENABLE_DEBUG_BKPT)
	DCC_LOG(LOG_TRACE, "1. clear all breakpoints...");
	monitor_breakpoint_clear_all();
#endif

#if (THINKOS_MONITOR_ENABLE_RST_VEC)
	DCC_LOG(LOG_TRACE, "2. reset RAM vectors...");
	__reset_ram_vectors();
#endif

	thinkos_monitor_rt.thread_id = -1;
	thinkos_monitor_rt.errno = THINKOS_NO_ERROR;
	thinkos_monitor_rt.code = 0;
}

void thinkos_monitor_svc(int32_t arg[], int self)
{
	void (* task)(const struct monitor_comm *, void *) = 
		(void (*)(const struct monitor_comm *, void *))arg[0];
	struct monitor_comm * comm = (void *)arg[1];
	void * param = (void *)arg[2];
	struct cm3_scb * scb = CM3_SCB;

	if (!thinkos_monitor_rt.enabled) {
		DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ _REVERSE_
		" ==== Debug/Monitor startup ==== "_ATTR_POP_);
		thinkos_monitor_reset();
#if (THINKOS_ENABLE_STACK_INIT)
		__thinkos_memset32(thinkos_monitor_stack, 0xdeadbeef, 
						   sizeof(thinkos_monitor_stack));
#endif
		/* set the startup signal */
		thinkos_monitor_rt.events = (1 << MONITOR_STARTUP);
	} 
	
	/* disable monitor */
	DCC_LOG2(LOG_TRACE, "comm=%p task=%p", comm, task);

	/* Set the persistent signals */
	thinkos_monitor_rt.events |= (1 << MONITOR_RESET);
	thinkos_monitor_rt.mask = MONITOR_PERISTENT_MASK | (1 << MONITOR_STARTUP);
	thinkos_monitor_rt.comm = comm;

	arg[0] = (uint32_t)thinkos_monitor_rt.task;
	if (task == NULL)
		/* Set the new task to NULL */
		thinkos_monitor_rt.task = monitor_null_task;
	else
		thinkos_monitor_rt.task = task;

	thinkos_monitor_rt.param = param;
	thinkos_monitor_rt.ctx = 0;

	/* enable monitor and send the reset event */
	thinkos_monitor_rt.enabled = 1;
	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
}

#endif /* THINKOS_ENABLE_MONITOR */


