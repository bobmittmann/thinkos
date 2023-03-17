/* 
 * thinkos_event.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_EVENT_MAX) > 0

static inline bool __attribute__((always_inline)) 
__krn_obj_is_event(struct thinkos_rt * krn, unsigned int event) {
	return __obj_is_valid(event, THINKOS_EVENT_BASE, THINKOS_EVENT_MAX);
}

static inline bool __attribute__((always_inline)) 
__krn_event_is_alloc(struct thinkos_rt * krn, unsigned int event) {
	return __bit_mem_rd(krn->ev_alloc, event - THINKOS_EVENT_BASE) ? 
		true : false;
}

#if (THINKOS_ENABLE_ARG_CHECK)
int krn_event_check(struct thinkos_rt * krn, int event)
{
	if (!__krn_obj_is_event(krn, event)) {
		return THINKOS_ERR_EVSET_INVALID;
	}
#if (THINKOS_ENABLE_EVENT_ALLOC)
	if (__krn_event_is_alloc(krn, event) == 0) {
		return THINKOS_ERR_EVSET_ALLOC;
	}
#endif
	return THINKOS_OK;
}
#endif


#if 0
void __thinkos_ev_info(unsigned int evset)
{
	unsigned int no = evset - THINKOS_EVENT_BASE;
	uint32_t mask;
	uint32_t pend;

	mask = krn->ev[no].mask;
	pend = krn->ev[no].pend;
	(void)mask;
	(void)pend;
	DCC_LOG3(LOG_TRACE, "event evset=%d pend=%08x mask=%08x", evset, pend, mask);
}
#endif

void thinkos_ev_wait_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int evset = arg[0];
	unsigned int no = evset - THINKOS_EVENT_BASE;
	uint32_t mask;
	uint32_t pend;
	uint32_t queue;
	uint32_t ev;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_event_check(krn, evset)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, evset);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	mask = krn->ev[no].mask;
again:
	/* check for any pending unmasked event */
	pend = __ldrex(&krn->ev[no].pend);
	if ((ev = __thinkos_ffs(pend & mask)) < 32) {
		pend &= ~(1 << ev);
		DCC_LOG2(LOG_MSG, "set=0x%08x msk=0x%08x", 
				 krn->ev[no].pend, krn->ev[no].mask);
		if (__strex(&krn->ev[no].pend, pend))
			goto again;
		arg[0] = ev;
		return;
	}

	/* (1) - suspend the thread by removing it from the
	   ready wait queue. The __krn_thread_suspend(krn) call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem having a thread not contained in any waiting
	   queue inside a system call. 
	 */
	__krn_thread_suspend(krn, self);
	/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
	__thread_stat_set(krn, self, evset, 0);
#endif
	/* Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
/* NEW: 2020-12-02 performed by the svc call entry stub 
	__thread_ctx_flush(krn, arg, self);
*/

	/* insert into the event wait queue */
	queue = __ldrex(&krn->wq_lst[evset]);
	queue |= (1 << (self - 1));
	pend = (volatile uint32_t)krn->ev[no].pend;
	if (((ev = __thinkos_ffs(pend & mask)) < 32) || 
		(__strex(&krn->wq_lst[evset], queue))) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting for event on %d", self, evset);
	DCC_LOG3(LOG_INFO, "<%d> sp=%08x ctx=%p", 
			 self, cm3_psp_get(), __thread_ctx_get(krn, self));
	DCC_LOG3(LOG_INFO, "<%d> ctx=%p pc=%p", 
			 self, __thread_ctx_get(krn, self), arg[6]);
	/* signal the scheduler ... */
	__krn_sched_defer(krn); 
}

#if (THINKOS_ENABLE_TIMED_CALLS)
void thinkos_ev_timedwait_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int evset = arg[0];
	uint32_t ms = (uint32_t)arg[1];
	unsigned int no = evset - THINKOS_EVENT_BASE;
	unsigned int ev;
	uint32_t mask;
	uint32_t pend;
	uint32_t queue;

#if (THINKOS_ENABLE_ARG_CHECK)
	if (no >= THINKOS_EVENT_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not an event set!", evset);
		__THINKOS_ERROR(self, THINKOS_ERR_EVSET_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if (THINKOS_ENABLE_EVENT_ALLOC)
	if (__bit_mem_rd(&krn->ev_alloc, no) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid event set %d!", evset);
		__THINKOS_ERROR(self, THINKOS_ERR_EVSET_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	mask = krn->ev[no].mask;
again:
	/* check for any pending unmasked event */
	pend = __ldrex(&krn->ev[no].pend);
	if ((ev = __thinkos_ffs(pend & mask)) < 32) {
		pend &= ~(1 << ev);
		DCC_LOG2(LOG_MSG, "set=0x%08x msk=0x%08x", 
				 krn->ev[no].pend, krn->ev[no].mask);
		DCC_LOG2(LOG_INFO, "pending event %d.%d!", evset, ev);
		if (__strex(&krn->ev[no].pend, pend))
			goto again;
		arg[0] = ev;
		return;
	}

	__krn_thread_suspend(krn, self);
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	__thread_stat_set(krn, self, evset, 1);
#endif
	/* Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
	__thread_ctx_set(krn, self, (struct thinkos_context *)&arg[-CTX_R0],
							 CONTROL_SPSEL | CONTROL_nPRIV);
	queue = __ldrex(&krn->wq_lst[evset]);
	queue |= (1 << (self - 1));
	pend = (volatile uint32_t)krn->ev[no].pend;
	if (((ev = __thinkos_ffs(pend & mask)) < 32) || 
		(__strex(&krn->wq_lst[evset], queue))) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting for event on %d", self, evset);
	__krn_thread_clk_itv_wait(krn, self, ms);
}
#endif

void __krn_ev_raise(struct thinkos_rt * krn, uint32_t evset, unsigned int ev)
{
	unsigned int no = evset - THINKOS_EVENT_BASE;
	uint32_t queue;
	int th;
	int j;

	if (__bit_mem_rd(&krn->ev[no].mask, ev) == 0) {
		DCC_LOG2(LOG_INFO, "pending event %d.%d", evset, ev);
		/* event is masked, set the event as pending */
		__bit_mem_wr(&krn->ev[no].pend, ev, 1);  
		return;
	}

	do {
		/* insert into the event wait queue */
		queue = __ldrex(&krn->wq_lst[evset]);
		/* get a thread from the queue bitmap */
		if ((j = __thinkos_ffs(queue)) == 32) {
			/* no threads waiting on the event set, mark the event as pending */
			DCC_LOG2(LOG_INFO, "pending event %d.%d", evset, ev);
			__bit_mem_wr(&krn->ev[no].pend, ev, 1);  
			return;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << j);
	} while (__strex(&krn->wq_lst[evset], queue));
	th = j + 1;

	DCC_LOG3(LOG_INFO, "<%d> wakeup on event %d.%d", th, evset, ev);

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);  
#endif
	/* set the thread's return value */
	__thread_r0_set(krn, th, ev);
	/* update status */
	__thread_stat_clr(krn, th);
}

#if (THINKOS_ENABLE_I_CALLS)
void cm3_except9_isr(uint32_t evset, int ev)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_ev_raise(krn, evset, ev);
	/* signal the scheduler ... */
	__krn_preempt(krn);
}

void __thinkos_ev_raise_i(uint32_t evset, unsigned int ev)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_ev_raise(krn, evset, ev);
}

#endif /* THINKOS_ENABLE_I_CALLS */

void thinkos_ev_raise_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int evset = arg[0];
	unsigned int ev = arg[1];

#if (THINKOS_ENABLE_ARG_CHECK)
	unsigned int no = evset - THINKOS_EVENT_BASE;

	if (ev > 31) {
		DCC_LOG1(LOG_ERROR, "event %d is invalid!", ev);
		__THINKOS_ERROR(self, THINKOS_ERR_EVENT_OUTOFRANGE);
		arg[0] = THINKOS_EINVAL;
		return;
	}
	if (no >= THINKOS_EVENT_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not an event set!", evset);
		__THINKOS_ERROR(self, THINKOS_ERR_EVSET_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if (THINKOS_ENABLE_EVENT_ALLOC)
	if (__bit_mem_rd(&krn->ev_alloc, no) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid event set %d!", evset);
		__THINKOS_ERROR(self, THINKOS_ERR_EVSET_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	arg[0] = THINKOS_OK;
	__krn_ev_raise(krn, evset, ev);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

void thinkos_ev_mask_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int evset = arg[0];
	unsigned int ev = arg[1];
	unsigned int val = arg[2];
	unsigned int no = evset - THINKOS_EVENT_BASE;
	uint32_t queue;
	uint32_t mask;
	int j;
	int th;

#if (THINKOS_ENABLE_ARG_CHECK)
	if (no >= THINKOS_EVENT_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not an event set!", evset);
		__THINKOS_ERROR(self, THINKOS_ERR_EVSET_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if (THINKOS_ENABLE_EVENT_ALLOC)
	if (__bit_mem_rd(&krn->ev_alloc, no) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid event set %d!", evset);
		__THINKOS_ERROR(self, THINKOS_ERR_EVSET_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	arg[0] = THINKOS_OK;
	if (val == 0) {
		/* mask the event on the mask bitmap */
		__bit_mem_wr(&krn->ev[no].mask, ev, 0);  
		return;
	}

again:
	/* check for pending masked event */
	mask = __ldrex(&krn->ev[no].mask);

	if (mask & (1 << ev)) {
		/* mask bit already set */
		__clrex();
		return;
	}

	if (!__bit_mem_rd(&krn->ev[no].pend, ev)) {
		/* no pending event, set the mask bit and return */
		mask |= (1 << ev);
		if (__strex(&krn->ev[no].mask, mask))
			goto again;
		return;
	}

	__clrex();

	do {
		/* get the event wait queue bitmap */
		queue = __ldrex(&krn->wq_lst[evset]);
		/* get a thread from the queue bitmap */
		if ((j = __thinkos_ffs(queue)) == 32) {
			/* no threads waiting */
			__clrex();
			/* set the mask bit on the mask bitmap */
			__bit_mem_wr(&krn->ev[no].mask, ev, 1);  
			return;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << j);
	} while (__strex(&krn->wq_lst[evset], queue));
	th = j + 1;

	/* clear the event */
	__bit_mem_wr(&krn->ev[no].pend, ev, 0);  

	/* set the mask bit on the mask bitmap */
	__bit_mem_wr(&krn->ev[no].mask, ev, 1);  

	/* set the thread's return value */
	__thread_r0_set(krn, th, ev);

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);  
#endif
	/* update status */
	__thread_stat_clr(krn, th);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

void thinkos_ev_clear_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int evset = arg[0];
	unsigned int ev = arg[1];
	unsigned int no = evset - THINKOS_EVENT_BASE;

#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_event_check(krn, evset)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event set %d!", self, evset);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = THINKOS_OK;
	/* clear the event bit on the pending bitmap */
	__bit_mem_wr(&krn->ev[no].pend, ev, 0);  
}

#if (THINKOS_ENABLE_PAUSE)
bool evset_resume(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int evset, bool tmw) 
{
	unsigned int no = evset - THINKOS_EVENT_BASE;
	unsigned int ev;

	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 

	/* check for any pending unmasked event */
	if ((ev = __thinkos_ffs(krn->ev[no].pend & 
						   krn->ev[no].mask)) < 32) {
		__bit_mem_wr(&krn->ev[no].pend, ev, 0);  
		/* insert the thread into ready queue */
		__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
		/* set the thread's return value */
		__thread_r0_set(krn, th, 0);
#endif
		/* update status */
		__thread_stat_clr(krn, th);
	} else {
		__thread_wq_set(krn, th, evset);
#if (THINKOS_ENABLE_TIMED_CALLS)
		if (tmw)
			__thread_clk_enable(krn, th);
#endif
	}
	return true;
}
#endif

#endif /* THINKOS_EVENT_MAX > 0 */

