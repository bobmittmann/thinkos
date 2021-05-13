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

#include "thinkos_mon-i.h"

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_ENABLE_MONITOR) 

#ifndef THINKOS_ENABLE_MONITOR_NULL_TASK
#define THINKOS_ENABLE_MONITOR_NULL_TASK 0
#endif

#define MONITOR_PERISTENT_MASK ((1 << MONITOR_TASK_INIT) | \
								(1 << MONITOR_SOFTRST))

struct {
	/* task entry point */
	void (* task)(const struct monitor_comm *, void *);
	const struct monitor_comm * comm;
	/* user supplied parameter */
	void * param;             

#if (THINKOS_ENABLE_MONITOR_THREADS)
	/* entry/exit signal thread id */
	int8_t ret_thread_id;
	/* entry/exit signal thread exit code */
	int32_t ret_code;
#endif
} thinkos_monitor_rt;

uint32_t __attribute__((aligned(8))) 
	thinkos_monitor_stack[THINKOS_MONITOR_STACK_SIZE / 4];

const uint16_t thinkos_monitor_stack_size = sizeof(thinkos_monitor_stack);

#if (THINKOS_ENABLE_MONITOR_SCHED)
void __attribute__((noinline))__monitor_wait(struct thinkos_monitor * mon)
{
	struct cm3_scb * scb = CM3_SCB;

	/* set the state to WAITING */
	mon->ctl = 1;

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
}

#else
void __monitor_context_swap(uint32_t ** pctx); 
#endif

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

void monitor_signal_break(int32_t sig) 
{
	monitor_signal(MONITOR_SOFTRST); 
	monitor_signal(sig); 
}

#if 0
void monitor_signal_break(int32_t event) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t evset;
	uint32_t set;

	/* Disable systick interrupts */
	//__thinkos_systick_sleep();

	set = (1 << event) | (1 << MONITOR_SOFTRST); 

	do {
		/* avoid possible race condition on monitor.events */
		evset = __ldrex((uint32_t *)&krn->monitor.events);
		evset |= set;
	} while (__strex((uint32_t *)&krn->monitor.events, evset));

	DCC_LOG1(LOG_TRACE, "set=0x%08x", evset);

	/* Issue an idle hook request */
//	__idle_hook_req(IDLE_HOOK_MONITOR_WAKEUP);
}
#endif

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
	/* adjust the local mask */
	evmsk |= save | MONITOR_PERISTENT_MASK;
	DCC_LOG1(LOG_MSG, "evmask=%08x ........ ", evmsk);
	/* set the global mask */
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

		DCC_LOG2(LOG_INFO, "waiting evmsk=%08x, sp=%08x sleeping...", 
				 evmsk, cm3_sp_get());
#if (THINKOS_ENABLE_MONITOR_SCHED)
		__monitor_wait(&thinkos_rt.monitor); 
#else
		__monitor_context_swap(&thinkos_rt.monitor.ctx); 
#endif
		DCC_LOG1(LOG_INFO, "wakeup, sp=%08x ...", cm3_sp_get());
	} 
	/* restore the global mask */
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
#if (THINKOS_ENABLE_MONITOR_SCHED)
		__monitor_wait(&thinkos_rt.monitor); 
#else
		__monitor_context_swap(&thinkos_rt.monitor.ctx); 
#endif
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

#if (THINKOS_ENABLE_MONITOR_THREADS)
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
 
void monitor_signal_thread_terminate(unsigned int thread_id, int code) 
{
	thinkos_monitor_rt.ret_thread_id = thread_id;
	thinkos_monitor_rt.ret_code = code;
	monitor_signal(MONITOR_THREAD_TERMINATE);
}
#endif


void __thinkos_systick_sleep(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__systick_int_disable(krn);
	__systick_pend_clr(krn);
}

void __thinkos_systick_wakeup(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__systick_int_enable(krn);
	__systick_pend_set(krn);
}


void thinkos_monitor_sleep(void)
{
	/* Disable the systick interrupts */
	__thinkos_systick_sleep();
}

void thinkos_monitor_wakeup(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t sigset;
	uint32_t sigmsk;

	sigset = krn->monitor.events;
	sigmsk = krn->monitor.mask;
	(void)sigset;
	(void)sigmsk;

	if (sigset == 0) {
		DCC_LOG2(LOG_ERROR, "set=0x%08x msk=0x%08x", sigset, sigmsk);
	} else {
		DCC_LOG2(LOG_TRACE, "set=0x%08x msk=0x%08x", sigset, sigmsk);
	}

	/* Reenable systick interrupts and signal the monitor */
	__thinkos_systick_wakeup();
}


int monitor_thread_inf_get(unsigned int id, struct monitor_thread_inf * inf)
{
	unsigned int thread_id = id;

	if (!thinkos_dbg_thread_ctx_is_valid(id)) {
		return -1;
	}

	if (inf != NULL) {
		inf->thread_id = thread_id;
		inf->ctrl = thinkos_dbg_thread_ctrl_get(thread_id);
		inf->pc = thinkos_dbg_thread_pc_get(thread_id);
		inf->sp = thinkos_dbg_thread_sp_get(thread_id);
		inf->errno = thinkos_dbg_thread_errno_get(thread_id);
	}

	return 0;
}

int monitor_thread_break_get(int32_t * pcode)
{
	return thinkos_dbg_thread_break_get(pcode);
}

void monitor_thread_break_clr(void)
{
	monitor_signal(MONITOR_THREAD_FAULT);
}
 
static inline void __monitor_task_reset(void)
{
	monitor_signal(MONITOR_TASK_INIT);
#if (THINKOS_ENABLE_MONITOR_SCHED)
	__monitor_wait(&thinkos_rt.monitor); 
#else
	__monitor_context_swap(&thinkos_rt.monitor.ctx); 
#endif
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

/* -------------------------------------------------------------------------
 * ThinkOS Monitor Core
 * ------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_MONITOR_NULL_TASK)
void monitor_null_task(const struct monitor_comm * comm, void * param)
{
	for (;;) {
#if (THINKOS_ENABLE_MONITOR_SCHED)
		__monitor_wait(&thinkos_rt.monitor); 
#else
		__monitor_context_swap(&thinkos_rt.monitor.ctx); 
#endif
	}
}
#endif

#if !(THINKOS_ENABLE_MONITOR_SCHED)
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

#if (THINKOS_ENABLE_MONITOR_NULL_TASK)
	/* Set the new task to the default NULL in 
	   case the new task returns; */
	thinkos_monitor_rt.task = monitor_null_task;
#endif

	/* set the clock in the past so it won't generate signals in 
	 the near future */
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	thinkos_rt.monitor_clock = thinkos_rt.ticks - 1;
#endif

	DCC_LOG2(LOG_TRACE, "PC=%08x SP=0x%08x!", monitor_task, cm3_sp_get());
	monitor_task(comm, param);
	__monitor_task_reset();
}

/* Prepare the execution environment to invoke the new monitor task. */
void __thinkos_monitor_on_reset(void)
{
	struct monitor_swap * swap;
	uint32_t * sp;
	uint32_t idx;

	idx = (sizeof(thinkos_monitor_stack) - 
		   sizeof(struct monitor_swap)) / sizeof(uint32_t);

	sp = &thinkos_monitor_stack[idx];
	thinkos_rt.monitor.ctx = sp;
	
	swap = (struct monitor_swap *)sp;
	swap->xpsr = CM_EPSR_T + CM3_EXCEPT_SYSTICK; /* XPSR */
	swap->lr = ((uintptr_t)monitor_bootstrap) | 1; /* LR */
}

#if 0
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

		__monitor_context_swap(&thinkos_rt.monitor.ctx); 
	}

	return sigact;
}
#endif

#endif


/**
 * monitor_soft_reset:
 *
 * Reinitialize the plataform by reseting all ThinkOS subsystems.
 * 
 */

void monitor_soft_reset(void)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " Monitor Soft Reset " VT_POP);

	thinkos_dbg_reset();

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

#if (THINKOS_ENABLE_MONITOR_THREADS)
	thinkos_monitor_rt.ret_thread_id = -1;
	thinkos_monitor_rt.ret_code = 0;
#endif
}


#if (THINKOS_ENABLE_MONITOR_SCHED)
struct monitor_context * __monitor_base_ctx(void)
{
	struct monitor_context * ctx;
	uintptr_t sp;

	sp = (uintptr_t)&thinkos_monitor_stack[(sizeof(thinkos_monitor_stack) / 4)];
	sp &= 0xfffffff0; /* 64bits alignemnt */

	sp -= sizeof(struct thinkos_context);
	ctx = (struct monitor_context *)sp;

	return ctx;
}

void __attribute__((noreturn)) __monitor_exit_stub(int code)
{
	DCC_LOG(LOG_WARNING, _ATTR_PUSH_ _FG_YELLOW_ _REVERSE_
			"monitor task return!!!"  _ATTR_POP_);
	for(;;);
}

struct monitor_context * __monitor_ctx_init(uintptr_t task,
											uintptr_t comm,
											uintptr_t arg)
{
	struct monitor_context * ctx;

	ctx = __monitor_base_ctx();

#if (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(ctx, 0, sizeof(struct monitor_context));
#endif

	ctx->r0 = (uint32_t)comm;
	ctx->r1 = (uint32_t)arg;
	ctx->pc = task;
	ctx->lr = (uint32_t)__monitor_exit_stub;
	ctx->xpsr = CM_EPSR_T; /* set the thumb bit */

#if 1
	DCC_LOG4(LOG_TRACE, "ctx=%08x r0=%08x lr=%08x pc=%08x", 
			 ctx, ctx->r0,  ctx->lr, ctx->pc);
	DCC_LOG3(LOG_TRACE, "msp=%08x psp=%08x ctrl=%02x", 
			 cm3_msp_get(), cm3_psp_get(), cm3_control_get());
#endif
	return ctx;
}
#endif

void thinkos_krn_monitor_init(const struct monitor_comm * comm, 
                     void (* task)(const struct monitor_comm *, void *),
					 void * param)
{
#if (THINKOS_ENABLE_STACK_INIT)
	__thinkos_memset32(thinkos_monitor_stack, 0xdeadbeef, 
					   sizeof(thinkos_monitor_stack));
#elif (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(thinkos_monitor_stack, 0, 
					   sizeof(thinkos_monitor_stack));
#endif

#if (THINKOS_ENABLE_MONITOR_THREADS)
	thinkos_monitor_rt.ret_thread_id = -1;
	thinkos_monitor_rt.ret_code = 0;
#endif

	/* Set the communication channel */
	thinkos_monitor_rt.comm = comm; 

#if (THINKOS_ENABLE_MONITOR_NULL_TASK)
	/* Set the new task to default NULL */
	if (task == NULL)
		/* Set the new task to NULL */
		thinkos_monitor_rt.task = monitor_null_task;
	else
#endif
		thinkos_monitor_rt.task = task;
	thinkos_monitor_rt.param = param;
	
	/* set the task init and software reset signals */
	thinkos_rt.monitor.events = (1 << MONITOR_TASK_INIT) | 
		(1 << MONITOR_SOFTRST);
	thinkos_rt.monitor.mask = MONITOR_PERISTENT_MASK;
#if (THINKOS_ENABLE_MONITOR_SCHED)
	{
		struct monitor_context * ctx;

		ctx = __monitor_ctx_init((uintptr_t)task, (uintptr_t)comm,
								 (uintptr_t)param);
		thinkos_rt.monitor.ctl = (uintptr_t)ctx;
	}
#endif
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
	thinkos_rt.monitor.mask = MONITOR_PERISTENT_MASK;

	arg[0] = (uint32_t)thinkos_monitor_rt.task;
#if (THINKOS_ENABLE_MONITOR_NULL_TASK)
	if (task == NULL)
		/* Set the new task to NULL */
		thinkos_monitor_rt.task = monitor_null_task;
	else
#endif
		thinkos_monitor_rt.task = task;

	thinkos_monitor_rt.param = param;
	thinkos_monitor_rt.ctx = 0;

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
}
#endif

#endif /* THINKOS_ENABLE_MONITOR */

