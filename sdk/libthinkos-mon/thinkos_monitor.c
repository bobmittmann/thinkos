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
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>

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
	uint32_t * ctx;           /* monitor context */
	/* task entry point */
	void (* task)(const struct monitor_comm *, void *);
	const struct monitor_comm * comm;
	/* user supplied parameter */
	void * param;             
	/* break thread id */
	int8_t brk_thread_id;
	uint8_t brk_code;

	/* error thread id */
	int8_t err_thread_id;
	/* error code */
	uint8_t err_code;

	/* entry/exit signal thread id */
	int8_t ret_thread_id;
	/* entry/exit signal thread exit code */
	int32_t ret_code;
};

struct thinkos_monitor thinkos_monitor_rt;

uint32_t __attribute__((aligned(16))) 
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
		evset = __ldrex((uint32_t *)&thinkos_rt.monitor.events);
		evset |= (1 << sig);
	} while (__strex((uint32_t *)&thinkos_rt.monitor.events, evset));

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;

	asm volatile ("isb\n" :  :  : );
}

bool monitor_is_set(int sig) 
{
	return thinkos_rt.monitor.events &  (1 << sig) ? true : false;
}

void monitor_unmask(int sig)
{
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&thinkos_rt.monitor.mask);
		mask |= (1 << sig);
	} while (__strex((uint32_t *)&thinkos_rt.monitor.mask, mask));
}

void monitor_mask(int sig)
{
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&thinkos_rt.monitor.mask);
		mask &= ~(1 << sig);
	} while (__strex((uint32_t *)&thinkos_rt.monitor.mask, mask));
}

void monitor_clear(int sig)
{
	uint32_t evact;
	uint32_t evset;

	do {
		/* avoid possible race condition on monitor.events */
		evset = __ldrex((uint32_t *)&thinkos_rt.monitor.events);
		evact = evset & (1 << sig);
		evset ^= evact;
	} while (__strex((uint32_t *)&thinkos_rt.monitor.events, evset));
}

/* wait for multiple events, return the highest priority (smaller number)
   don't clear the event upon exiting. */
int monitor_select(uint32_t evmsk)
{
	int evset;
	uint32_t save;
	int sig;

	save = thinkos_rt.monitor.mask;
	/* set the local mask */
	evmsk |= save | MONITOR_PERISTENT_MASK;
	DCC_LOG1(LOG_MSG, "evmask=%08x ........ ", evmsk);
	thinkos_rt.monitor.mask = evmsk;
	for(;;) {
		evset = thinkos_rt.monitor.events;
		/* apply local and global masks, 
		   making sure not to mask non maskable events */
		evset &= evmsk;
		/* select the event with highest priority */
		sig = __clz(__rbit(evset & evmsk));

		if (sig < 32)
			break;

		DCC_LOG2(LOG_MSG, "waiting evmsk=%08x, sp=%08x sleeping...", 
				 evmsk, cm3_sp_get());
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
		DCC_LOG1(LOG_MSG, "wakeup, sp=%08x ...", cm3_sp_get());
	} 
	thinkos_rt.monitor.mask = save;

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

	save = thinkos_rt.monitor.mask;
	/* set the local mask */
	evmsk = save | (1 << sig) | MONITOR_PERISTENT_MASK;
	thinkos_rt.monitor.mask = evmsk;

	for (;;) {
		/* avoid possible race condition on monitor.events */
		do {
			evset = __ldrex((uint32_t *)&thinkos_rt.monitor.events);
			/* apply local and global masks, 
			   making sure not to mask non maskable events */
			result = evset;
			result &= evmsk;
			if (result & (1 << sig))
				evset &= ~(1 << sig); /* clear the event */
		} while (__strex((uint32_t *)&thinkos_rt.monitor.events, evset));

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

	thinkos_rt.monitor.mask = save;

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

	save = thinkos_rt.monitor.mask;

	/* set the local mask */
	evmsk = (1 << MONITOR_IDLE);
	thinkos_rt.monitor.mask = evmsk;
	
	do {
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
		do { /* avoid possible race condition on monitor.events */
			evset = __ldrex((uint32_t *)&thinkos_rt.monitor.events);
			result = evset;
			evset &= evmsk; /* clear the event */
		} while (__strex((uint32_t *)&thinkos_rt.monitor.events, evset));
	} while (result == 0);
	thinkos_rt.monitor.mask = save;

	return (result == evmsk) ? 0 : -1;
#else
	return 0;
#endif
}

int monitor_thread_terminate_get(int * code)
{
	int thread_id;

	if ((thread_id = thinkos_monitor_rt.ret_thread_id) >= 0) {
		if (code != NULL) {
			*code = thinkos_monitor_rt.ret_code;

		}
	}

	return thread_id;
}
 
void monitor_signal_thread_terminate(int thread_id, int code) 
{
	thinkos_monitor_rt.ret_thread_id = thread_id;
	thinkos_monitor_rt.ret_code = code;
	monitor_signal(MONITOR_THREAD_TERMINATE);
}

void monitor_signal_error(int thread_id, int errno) 
{
	thinkos_monitor_rt.err_thread_id = thread_id;
	thinkos_monitor_rt.err_code = errno;
	monitor_signal(MONITOR_THREAD_TERMINATE);
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
	struct thinkos_except * xcpt = __thinkos_except_buf();
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
		if (thread_id == (unsigned int)thinkos_monitor_rt.brk_thread_id)
			errno = thinkos_monitor_rt.err_code;

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
	return  thinkos_monitor_rt.err_code;
}

int monitor_thread_break_get(void)
{
	struct thinkos_context * ctx;
	int thread_id;

	if ((thread_id = thinkos_monitor_rt.brk_thread_id) >= 0) {
		if ((ctx = __thinkos_thread_ctx_get(thread_id)) == NULL)
			return -1;
	}

	return thread_id;
}

void monitor_thread_break_clr(void)
{
	thinkos_monitor_rt.brk_thread_id = -1;
	thinkos_monitor_rt.brk_code = 0;
}

/* -------------------------------------------------------------------------
 * ThinkOS Monitor Core
 * ------------------------------------------------------------------------- */

void monitor_null_task(const struct monitor_comm * comm, void * param)
{
	for (;;) {
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
	}
}

/*
 * This is a wrapper for the monitor entry function (task).
 * It's pourpose is to get the arguments to the monitor entry function
 * as well as to set a fallback function if the task returns.
 */
static void __attribute__((naked, noreturn)) monitor_bootstrap(void)
{
	const struct monitor_comm * comm = thinkos_monitor_rt.comm; 
	void (* monitor_task)(const struct monitor_comm *, void *);
	void * param = thinkos_monitor_rt.param; 

	param = thinkos_monitor_rt.param; 

	/* Get the new task */
	monitor_task = thinkos_monitor_rt.task; 
	/* Set the new task to the default NULL in 
	   case the new task returns; */
	thinkos_monitor_rt.task = monitor_null_task;
	
	/* set the clock in the past so it won't generate signals in 
	 the near future */
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	thinkos_rt.monitor_clock = thinkos_rt.ticks - 1;
#endif

	DCC_LOG2(LOG_TRACE, "PC=%08x SP=0x%08x!", monitor_task, cm3_sp_get());
	monitor_task(comm, param);

	DCC_LOG(LOG_WARNING, "Debug monitor task returned!");

	__monitor_task_reset();
}

/* Prepare the execution environment to invoke the new monitor 
 task. */
static void __thinkos_monitor_on_reset(void)
{
	uint32_t * sp;

	sp = &thinkos_monitor_stack[(sizeof(thinkos_monitor_stack) / 4) - 10];
	sp[0] = CM_EPSR_T + CM3_EXCEPT_SYSTICK; /* CPSR */
	sp[9] = ((uintptr_t)monitor_bootstrap) | 1; /* LR */
	thinkos_monitor_rt.ctx = sp;
}

uint32_t __attribute__((aligned(16))) __thinkos_monitor_isr(void)
{
	uint32_t sigset;
	uint32_t sigmsk;
	uint32_t sigact;

	sigset = thinkos_rt.monitor.events;
	sigmsk = thinkos_rt.monitor.mask;
	sigact = sigset & sigmsk;

	/* Process monitor events */
	if (sigact != 0) {
		if (sigact & (1 << MONITOR_RESET)) {
			__thinkos_monitor_on_reset();

			/* clear the RESET event */
			thinkos_rt.monitor.events = sigset & ~(1 << MONITOR_RESET);
		}

#if 0
		/* TODO: this stack check is very usefull... 
		   Some sort of error to the developer should be raised or
		 force a fault */
		if (thinkos_monitor_rt.ctx < thinkos_monitor_stack) {
			DCC_LOG(LOG_PANIC, "stack overflow!");
			DCC_LOG2(LOG_PANIC, "monitor.ctx=%08x monitor.stack=%08x", 
					 thinkos_monitor_rt.ctx, thinkos_monitor_stack);
			DCC_LOG2(LOG_PANIC, "sigset=%08x sigmsk=%08x", sigset, sigmsk);
		}
		DCC_LOG1(LOG_TRACE, "sigset %08x", sigset);
#endif
		__monitor_context_swap(&thinkos_monitor_rt.ctx); 
	}

	return sigact;
}


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

void thinkos_krn_monitor_reset(void)
{
#if (THINKOS_ENABLE_STACK_INIT)
	__thinkos_memset32(thinkos_monitor_stack, 0xdeadbeef, 
					   sizeof(thinkos_monitor_stack));
#endif

	/* set the clock in the past so it won't generate signals in 
	 the near future */
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	thinkos_rt.monitor_clock = thinkos_rt.ticks - 1;
#endif

	thinkos_monitor_rt.ret_thread_id = -1;
	thinkos_monitor_rt.ret_code = 0;
	thinkos_monitor_rt.err_thread_id = -1;
	thinkos_monitor_rt.err_code = THINKOS_NO_ERROR;
}

void thinkos_krn_monitor_init(const struct monitor_comm * comm, 
                     void (* task)(const struct monitor_comm *, void *),
					 void * param)
{
#if (THINKOS_ENABLE_STACK_INIT)
	__thinkos_memset32(thinkos_monitor_stack, 0xdeadbeef, 
					   sizeof(thinkos_monitor_stack));
#endif

	thinkos_monitor_rt.ret_thread_id = -1;
	thinkos_monitor_rt.ret_code = 0;
	thinkos_monitor_rt.err_thread_id = -1;
	thinkos_monitor_rt.err_code = THINKOS_NO_ERROR;

	/* Set the communication channel */
	thinkos_monitor_rt.comm = comm; 

	/* Set the new task to default NULL */
	if (task == NULL)
		/* Set the new task to NULL */
		thinkos_monitor_rt.task = monitor_null_task;
	else
		thinkos_monitor_rt.task = task;
	thinkos_monitor_rt.param = param;
	
	/* set the startup and reset signals */
	thinkos_rt.monitor.events = (1 << MONITOR_STARTUP) | (1 << MONITOR_RESET);
	thinkos_rt.monitor.mask = MONITOR_PERISTENT_MASK | (1 << MONITOR_STARTUP);
}

#if (THINKOS_ENABLE_MONITOR_SYSCALL)
void thinkos_monitor_svc(int32_t arg[], int self)
{
	void (* task)(const struct monitor_comm *, void *) = 
		(void (*)(const struct monitor_comm *, void *))arg[0];
	void * param = (void *)arg[1];
	struct cm3_scb * scb = CM3_SCB;

	/* Set the persistent signals */
	thinkos_rt.monitor.events |= (1 << MONITOR_RESET);
	thinkos_rt.monitor.mask = MONITOR_PERISTENT_MASK | (1 << MONITOR_STARTUP);

	arg[0] = (uint32_t)thinkos_monitor_rt.task;
	if (task == NULL)
		/* Set the new task to NULL */
		thinkos_monitor_rt.task = monitor_null_task;
	else
		thinkos_monitor_rt.task = task;

	thinkos_monitor_rt.param = param;
	thinkos_monitor_rt.ctx = 0;

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
}
#endif

#endif /* THINKOS_ENABLE_MONITOR */


