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

#include "thinkos_krn-i.h"
#include <sys/delay.h>
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_ENABLE_MONITOR) 


#ifndef THINKOS_ENABLE_MONITOR_NULL_TASK
#define THINKOS_ENABLE_MONITOR_NULL_TASK 1
#endif

#define MONITOR_PERISTENT_MASK ((1 << MONITOR_SOFTRST) | (1 << MONITOR_TASK_INIT))

struct {
	/* task entry point */
	void (* task)(const struct monitor_comm *, void *);
	const struct monitor_comm * comm;
	/* user supplied parameter */
	void * param;             
	struct comm_tx_req * tx_req;

#if (THINKOS_ENABLE_MONITOR_THREADS)
	/* entry/exit signal thread id */
	int8_t ret_thread_id;
	/* entry/exit signal thread exit code */
	int32_t ret_code;
#endif
} thinkos_monitor_rt;

uint32_t __attribute__((aligned(8), section(".krn.stack"))) 
	thinkos_monitor_stack[THINKOS_MONITOR_STACK_SIZE / 4];

const uint16_t thinkos_monitor_stack_size = sizeof(thinkos_monitor_stack);

void __monitor_context_swap(uint32_t ** pctx); 

void __attribute__((noreturn)) __monitor_context_exec(uintptr_t task, 
													  uintptr_t comm,
													  void * env, 
													  uintptr_t sta);
void __attribute__((noreturn)) __monitor_bootstrap(void);

/* -------------------------------------------------------------------------
 * Debug Monitor API
 * ------------------------------------------------------------------------- */

static void __monitor_event_set(struct thinkos_rt * krn, uint32_t ev) 
{
	uint32_t evset;
	
	do {
		/* avoid possible race condition on monitor.events */
		evset = __ldrex((uint32_t *)&krn->monitor.events);
		evset |= ev;
	} while (__strex((uint32_t *)&krn->monitor.events, evset));
}

void krn_monitor_signal(struct thinkos_rt * krn, int sig) 
{
	__monitor_event_set(krn, (1 << sig)); 

	/* rise a pending systick interrupt */
	CM3_SCB->icsr = SCB_ICSR_PENDSTSET;
}


void monitor_signal(int sig) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct cm3_scb * scb = CM3_SCB;
	
	__monitor_event_set(krn, (1 << sig)); 

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;

//	asm volatile ("isb\n" :  :  : );
}

void monitor_signal_break(int32_t sig) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct cm3_scb * scb = CM3_SCB;

	__monitor_event_set(krn, (1 << sig) | (1 << MONITOR_SOFTRST)); 

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
}

#if 0
bool monitor_is_set(int sig) 
{
	return krn->monitor.events &  (1 << sig) ? true : false;
}
#endif

void monitor_unmask(int sig)
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&krn->monitor.mask);
		mask |= (1 << sig);
	} while (__strex((uint32_t *)&krn->monitor.mask, mask));
}

void monitor_mask(int sig)
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t mask;

	do {
		mask = __ldrex((uint32_t *)&krn->monitor.mask);
		mask &= ~(1 << sig);
	} while (__strex((uint32_t *)&krn->monitor.mask, mask));
}

void monitor_clear(int sig)
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t evact;
	uint32_t evset;

	do {
		/* avoid possible race condition on monitor.events */
		evset = __ldrex((uint32_t *)&krn->monitor.events);
		evact = evset & (1 << sig);
		evset ^= evact;
	} while (__strex((uint32_t *)&krn->monitor.events, evset));
}

/* wait for multiple events, return the highest priority (smaller number)
   don't clear the event upon exiting. */
int monitor_select(uint32_t evmsk)
{
	struct thinkos_rt * krn = &thinkos_rt;
	int evset;
	uint32_t save;
	int sig;

	save = krn->monitor.mask;
	/* adjust the local mask */
	evmsk |= save | MONITOR_PERISTENT_MASK;
	/* set the global mask */
	krn->monitor.mask = evmsk;
	DCC_LOG1(LOG_MSG, "evmask=%08x ........ ", evmsk);

	for(;;) {
		evset = krn->monitor.events;
		/* apply local and global masks, 
		   making sure not to mask non maskable events */
		evset &= evmsk;
		/* select the event with highest priority */
		sig = __clz(__rbit(evset & evmsk));

		if (sig < 32)
			break;

		DCC_LOG2(LOG_MSG, "waiting evmsk=%08x, sp=%08x sleeping...", 
				 evmsk, cm3_sp_get());
		__monitor_context_swap(&krn->monitor.ctx); 
		DCC_LOG1(LOG_MSG, "wakeup, sp=%08x ...", cm3_sp_get());
	} 
	/* restore the global mask */
	krn->monitor.mask = save;

	DCC_LOG1(LOG_MSG, "event=%d", sig);

	return sig;
}


/* wait for a single event and clear the event */
int monitor_expect(int sig)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct cm3_scb * scb = CM3_SCB;
	uint32_t save;
	uint32_t evset;
	uint32_t evmsk;
	uint32_t result;

	save = krn->monitor.mask;
	/* set the local mask */
	evmsk = save | (1 << sig) | MONITOR_PERISTENT_MASK;
	krn->monitor.mask = evmsk;
	DCC_LOG2(LOG_MSG, "evmask=%08x save=%08x", evmsk, save);

	for (;;) {
		/* avoid possible race condition on monitor.events */
		do {
			evset = __ldrex((uint32_t *)&krn->monitor.events);
			/* apply local and global masks, 
			   making sure not to mask non maskable events */
			result = evset;
			result &= evmsk;
			if (result & (1 << sig))
				evset &= ~(1 << sig); /* clear the event */
		} while (__strex((uint32_t *)&krn->monitor.events, evset));

		if (result != 0)
			break;

		if (evset != 0) {
			DCC_LOG3(LOG_MSG, "expected %08x got %08x/%08x, sleeping...", 
					 (1 << sig), evset, evmsk);
		}
		__monitor_context_swap(&krn->monitor.ctx); 
		if (evset != 0) {
			DCC_LOG(LOG_MSG, "wakeup...");
		}
	}

	krn->monitor.mask = save;

	if ((result & (1 << sig)) == 0) {
		DCC_LOG3(LOG_MSG, "expected %08x got %08x/%08x.", 
				 (1 << sig), result, evmsk);
		/* unexpected event received */
		sig = -1;
		/* rise a pending systick interrupt */
		scb->icsr = SCB_ICSR_PENDSTSET;
	} else {
		DCC_LOG1(LOG_MSG, "sig=%d", sig);
	}

	return sig;
}

int monitor_sleep(unsigned int ms)
{
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	struct thinkos_rt * krn = &thinkos_rt;
	
	monitor_clear(MONITOR_ALARM);
	/* set the clock */
	krn->clk.th_tmr[0] = krn->clk.time + ms;
	/* wait for signal */
	return monitor_expect(MONITOR_ALARM);
#else
	mdelay(ms);
	return 0;
#endif
}

void monitor_alarm(unsigned int ms)
{
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	struct thinkos_rt * krn = &thinkos_rt;
	
	DCC_LOG1(LOG_MSG, "alarm at %d ms!", ms);
	monitor_clear(MONITOR_ALARM);
	monitor_unmask(MONITOR_ALARM);
	/* set the clock */
	krn->clk.th_tmr[0] = krn->clk.time + ms;
#endif
}

void monitor_alarm_stop(void)
{
#if (THINKOS_ENABLE_MONITOR_CLOCK)
	struct thinkos_rt * krn = &thinkos_rt;

	/* set the clock in the past so it won't generate a signal */
	krn->clk.th_tmr[0] = krn->clk.time - 1;
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

int monitor_thread_break_clr(void)
{
	return thinkos_dbg_thread_break_clr();
}

 
/* -------------------------------------------------------------------------
 * ThinkOS Monitor Core
 * ------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_MONITOR_NULL_TASK)
void monitor_null_task(struct thinkos_rt * krn, 
					   const struct monitor_comm * comm, 
					   void * env)
{
	for (;;) {
		__monitor_context_swap(&krn->monitor.ctx); 
	}
}
#endif

void __monitor_at_exit(int retcode)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " Monitor Exit " VT_POP);
	for (;;) {
	}
}

/**
 * monitor_soft_reset:
 *
 * Reinitialize the platform by resetting ThinkOS subsystems.
 * 
 */

void monitor_soft_reset(void)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " Monitor Soft Reset " VT_POP);

#if (THINKOS_ENABLE_DEBUG_BASE)
	thinkos_dbg_reset();
#endif

	monitor_signal(MONITOR_SOFTRST); 
}

/* -------------------------------------------------------------------------
 * ThinkOS kernel level API
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * Deferred ISR
 * ------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_DEFERRED_ISR)
void def_on_comm_brk(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

void def_on_comm_rcv(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

void def_on_comm_eot(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

void def_on_comm_ctl(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

void def_on_console_rx(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

void def_on_console_tx(struct thinkos_rt * krn, void * env)
{
	const struct thinkos_comm * comm;
	unsigned int wq = THINKOS_WQ_CONSOLE_WR;
	struct comm_tx_req * req;
	int th;

	/* check if there is a pending monitor request */
	if ((req = krn->monitor.tx_req) != NULL) {
		th = 0;
	} else if ((th = __krn_wq_head(krn, wq)) != THINKOS_THREAD_NULL) {
		req = (struct comm_tx_req *)__thread_frame_get(krn, th);
	} else {
		DCC_LOG(LOG_WARNING, "no thread waiting.");
		return;
	}

	if ((comm = krn->monitor.comm) == NULL) {
		DCC_LOG(LOG_ERROR, "no communication port.");
		return;
	}
}

void def_on_console_ctl(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

void def_on_console_tmr(struct thinkos_rt * krn, void * env)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_REV VT_FYW " ... " VT_POP);
}

const struct deferred_svc_map thinkos_def_svc = {
	.on_comm_brk = def_on_comm_brk,
	.on_comm_rcv = def_on_comm_rcv,
	.on_comm_eot = def_on_comm_eot,
	.on_comm_ctl = def_on_comm_ctl,
	.on_console_rx = def_on_console_rx,
	.on_console_tx = def_on_console_tx,
	.on_console_ctl = def_on_console_ctl,
	.on_console_tmr = def_on_console_tmr
};
#endif /* (THINKOS_ENABLE_DEFERRED_ISR) */

static void thinkos_krn_monitor_reset(struct thinkos_rt * krn)
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
}

/* Prepare the execution environment to invoke the new monitor task. */
uint32_t * __monitor_ctx_init(uintptr_t task, uintptr_t comm,
								  void * env, uintptr_t sta)
{
	uint32_t idx = (sizeof(thinkos_monitor_stack) - 
		   sizeof(struct monitor_swap)) / sizeof(uint32_t);
	struct monitor_swap * swap;
	uint32_t * sp;

	sp = &thinkos_monitor_stack[idx];
	swap = (struct monitor_swap *)sp;
	swap->xpsr = CM_EPSR_T + CM3_EXCEPT_SYSTICK; /* XPSR */
	swap->lr = ((uintptr_t)__monitor_bootstrap); /* LR */
	swap->r4 = (uintptr_t)task;
	swap->r5 = (uintptr_t)comm;
	swap->r6 = (uintptr_t)env;
	swap->r7 = (uintptr_t)sta;
	swap->r8 = (uintptr_t)__monitor_at_exit;

	return sp;
}

void monitor_exec(void (* task)(const struct monitor_comm *, void *, uintptr_t, 
								struct thinkos_rt *), 
				  const struct monitor_comm * comm, void * env, uintptr_t sta)
{
//	monitor_signal(MONITOR_TASK_INIT);
	DCC_LOG1(LOG_TRACE, "task=%p", task);
	__monitor_context_exec((uintptr_t)task, (uintptr_t)comm, env, sta); 
}

void thinkos_krn_monitor_init(struct thinkos_rt * krn,
							  const struct monitor_comm * comm, 
							  void (* task)(const struct monitor_comm *, void *,
											uintptr_t, struct thinkos_rt *),
							  void * env)
{
	uint32_t * sp;

	thinkos_krn_monitor_reset(krn);

	/* Set the communication channel */
	thinkos_monitor_rt.comm = comm; 

#if (THINKOS_ENABLE_DEFERRED_ISR)
	krn->monitor.svc = &thinkos_def_svc;
	krn->monitor.env = (void *)env;
#endif
	
	sp = __monitor_ctx_init((uintptr_t)task, (uintptr_t)comm, env, (uintptr_t)0);

	krn->monitor.ctx = sp;
	/* set the task init and software reset signals */
	krn->monitor.events = (1 << MONITOR_TASK_INIT);
	krn->monitor.mask = MONITOR_PERISTENT_MASK;

	DCC_LOG1(LOG_TRACE, "mask=%08x", krn->monitor.mask);
}

#if (THINKOS_ENABLE_MONITOR_SYSCALL)
void thinkos_monitor_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int oper = arg[0];
	struct cm3_scb * scb = CM3_SCB;

	switch (oper) {
	case MONITOR_CTL_TASK_INIT: {
		void (* task)(const struct monitor_comm *, void *) = 
			(void (*)(const struct monitor_comm *, void *))arg[1];
		const struct monitor_comm * comm (const struct monitor_comm *)arg[2];
		void * env = (void *)arg[3];

		/* disable interrupts */
		cm3_cpsid_i();

		/* Set the task init signal */
		krn->monitor.events |= (1 << MONITOR_TASK_INIT);
		/* Set the persistent mmask */
		krn->monitor.mask = MONITOR_PERISTENT_MASK;

//		arg[4] = (uint32_t)thinkos_monitor_rt.task;
		arg[4] = THINKOS_OK;

		thinkos_monitor_rt.task = task;

		thinkos_monitor_rt.param = env;

		cm3_cpsie_i();

		/* rise a pending systick interrupt */
		scb->icsr = SCB_ICSR_PENDSTSET;
	}
	break;

	case MONITOR_CTL_SIGNAL: {
		unsigned int signo = arg[1];

		arg[4] = THINKOS_OK;
		thinkos_signal(signo);
	}
	break;

	default:
		DCC_LOG1(LOG_ERROR, "invalid CTL request %d!", req);
		__THINKOS_ERROR(self, THINKOS_ERR_CTL_REQINV);
		arg[4] = THINKOS_EINVAL;
		break;

	}
}
#endif

#endif /* THINKOS_ENABLE_MONITOR */

#if 0

/*
 * This is a wrapper for the monitor entry function (task).
 * It's pourpose is to get the arguments to the monitor entry function
 * as well as to set a fallback function if the task returns.
 */
void __attribute__((naked, noreturn)) monitor_bootstrap(void)
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
	struct thinkos_rt * krn = &thinkos_rt;

	krn->clk.th_tmr[0] = krn->clk.time - 1;
#endif

	DCC_LOG2(LOG_TRACE, "PC=%08x SP=0x%08x!", monitor_task, cm3_sp_get());
	monitor_task(comm, param);
	__monitor_task_reset();
}
static inline void __monitor_task_reset(void)
{
	monitor_signal(MONITOR_TASK_INIT);
	__monitor_context_swap(&thinkos_rt.monitor.ctx); 
}
#endif

