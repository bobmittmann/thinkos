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


#define MONITOR_PERISTENT_MASK ((1 << MONITOR_TASK_INIT) | \
								(1 << MONITOR_ON_CORE_RST))


uint32_t __attribute__((aligned(8), section(".krn.stack"))) 
	thinkos_monitor_stack[THINKOS_MONITOR_STACK_SIZE / 4];

const uint16_t thinkos_monitor_stack_size = sizeof(thinkos_monitor_stack);

void __monitor_context_swap(uint32_t ** pctx); 

void __attribute__((noreturn)) __monitor_context_exec(uintptr_t task, 
													  uintptr_t comm,
													  void * env, 
													  uintptr_t atexit);
void __attribute__((noreturn)) __monitor_bootstrap(void);

/* -------------------------------------------------------------------------
 * Debug Monitor API
 * ------------------------------------------------------------------------- */

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
}

void monitor_req_core_rst(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	thinkos_krn_req_core_rst(krn);
}

void monitor_core_rst_and_signal(unsigned int sig)
{
	struct thinkos_rt * krn = &thinkos_rt;

	thinkos_krn_sched_brk(krn, sig);
}

#if 0
void monitor_signal_break(int32_t sig) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct cm3_scb * scb = CM3_SCB;

	__monitor_event_set(krn, (1 << sig) | (1 << MONITOR_SOFTRST)); 

	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
}

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
		sig = __clz(__rbit(evset));

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
	/* set the timer */
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
	/* set the timer */
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

#if (THINKOS_ENABLE_ERROR_TRAP)

int monitor_thread_break_get(int32_t * perrno)
{
	struct thinkos_rt * krn = &thinkos_rt;
	int errno;
	int brkid;
	int thread;

	thread = __krn_sched_act_get(krn);
	(void)thread;
	brkid = __krn_sched_brk_get(krn);
	errno = __krn_sched_err_get(krn);
	
	DCC_LOG3(LOG_TRACE, "act=%d brk=%d err=%d",  
			 thread, brkid, errno);

	if (perrno) {
		*perrno = errno;
	}

	return brkid;
}

struct thinkos_context * monitor_thread_ctx_get(unsigned int th)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __thread_ctx_get(krn, th);
}

int monitor_thread_err_get(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __krn_sched_err_get(krn);
}

void monitor_thread_err_clr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_sched_err_clr(krn);
}

int monitor_krn_except_get(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return  __krn_sched_xcp_get(krn);
}

void monitor_krn_except_clr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_sched_xcp_clr(krn);

	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

uint32_t monitor_sched_ctrl_get(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return  __krn_sched_ctrl_get(krn);
}

void monitor_thread_break_clr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	thinkos_krn_brk_clr(krn);
}

#endif

/* -------------------------------------------------------------------------
 * ThinkOS Monitor Core
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * ThinkOS kernel level API
 * ------------------------------------------------------------------------- */

static void thinkos_krn_monitor_reset(struct thinkos_rt * krn)
{
#if (THINKOS_ENABLE_STACK_INIT)
	__thinkos_memset32(thinkos_monitor_stack, 0xdeadbeef, 
					   sizeof(thinkos_monitor_stack));
#elif (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(thinkos_monitor_stack, 0, 
					   sizeof(thinkos_monitor_stack));
#endif
}

void monitor_exec(int (* task)(const struct monitor_comm *, 
							   void *, struct thinkos_rt *), 
				  const struct monitor_comm * comm, void * env,
				  void (* atexit)(int))
{
	DCC_LOG1(LOG_TRACE, "task=%p", task);
	__monitor_context_exec((uintptr_t)task, (uintptr_t)comm, env, 
						   (uintptr_t)atexit); 
}

/* Prepare the execution environment to invoke the new monitor task. */
static uint32_t * __monitor_ctx_init(uintptr_t task, uintptr_t comm, void * env,
									 uintptr_t atexit)
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
	swap->r7 = (uintptr_t)atexit;

	return sp;
}

void thinkos_krn_monitor_init(struct thinkos_rt * krn,
							  const struct monitor_comm * comm, 
							  void (* task)(const struct monitor_comm *, void *,
											struct thinkos_rt *),
							  void * env)
{
	uint32_t * sp;

	thinkos_krn_monitor_reset(krn);

	sp = __monitor_ctx_init((uintptr_t)task, (uintptr_t)comm, 
							env, (uintptr_t)thinkos_krn_halt);
	krn->monitor.ctx = sp;
	/* Set the communication channel */
	krn->monitor.comm = comm; 
	/* set the task init signal */
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
	//	const struct monitor_comm * comm (const struct monitor_comm *)arg[2];
		void * env = (void *)arg[3];

		/* disable interrupts */
		cm3_cpsid_i();

		/* Set the task init signal */
		krn->monitor.events |= (1 << MONITOR_TASK_INIT);
		/* Set the persistent mmask */
		krn->monitor.mask = MONITOR_PERISTENT_MASK;

		arg[SVC_RETURN] = THINKOS_OK;

		thinkos_monitor_rt.task = task;

		thinkos_monitor_rt.param = env;

		cm3_cpsie_i();

		/* rise a pending systick interrupt */
		scb->icsr = SCB_ICSR_PENDSTSET;
	}
	break;

	case MONITOR_CTL_SIGNAL: {
		unsigned int signo = arg[1];

		arg[SVC_RETURN] = THINKOS_OK;
		thinkos_signal(signo);
	}
	break;

	default:
		DCC_LOG1(LOG_ERROR, "invalid CTL request %d!", req);
		__THINKOS_ERROR(self, THINKOS_ERR_CTL_REQINV);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		break;

	}
}
#endif

#endif /* THINKOS_ENABLE_MONITOR */

