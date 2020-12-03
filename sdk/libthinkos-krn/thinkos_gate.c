/* 
 * thinkos_gate.c
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
 * but WITHOUT ANY WARRANTY; without gateen the implied warranty of
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

#if (THINKOS_GATE_MAX > 0)

static inline bool __attribute__((always_inline)) 
__krn_obj_is_gate(struct thinkos_rt * krn, unsigned int gate) {
	return __obj_is_valid(gate, THINKOS_GATE_BASE, THINKOS_GATE_MAX);
}

static inline bool __attribute__((always_inline)) 
__krn_gate_is_alloc(struct thinkos_rt * krn, unsigned int gate) {
	return __bit_mem_rd(krn->gate_alloc, gate - THINKOS_GATE_BASE) ? 
		true : false;
}

#if THINKOS_ENABLE_ARG_CHECK
int krn_gate_check(struct thinkos_rt * krn, int gate)
{
	if (!__krn_obj_is_gate(krn, gate)) {
		return THINKOS_ERR_GATE_INVALID;
	}
#if THINKOS_ENABLE_GATE_ALLOC
	if (__krn_gate_is_alloc(krn, gate) == 0) {
		return THINKOS_ERR_GATE_ALLOC;
	}
#endif
	return THINKOS_OK;
}
#endif

/* --------------------------------------------------------------------------
 * 
 * -------------------------------------------------------------------------- */

#define __GATE_CLOSED   0
#define __GATE_SIGNALED 1
#define __GATE_LOCKED   2

void thinkos_gate_wait_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int gate = arg[0];
	unsigned int idx = gate - THINKOS_GATE_BASE;
	uint32_t * gates_bmp;
	uint32_t gates;
	uint32_t queue;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_gate_check(krn, gate)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid gate %d!", self, gate);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_GATE_MAX < 16)
	gates_bmp = &krn->gate[0];
#else
	gates_bmp = &krn->gate[idx / 16];
	idx %= 16;
#endif
	idx *= 2;

	DCC_LOG1(LOG_INFO, "gate %d", gate);

again:
	/* check whether the gate is open or not */
	gates = __ldrex(gates_bmp);
	if (((gates >> idx) & 3) == __GATE_SIGNALED) {
		/* close and lock the gate */
		gates = (gates & ~(3 << idx)) | (__GATE_LOCKED << idx);
		if (__strex(gates_bmp, gates))
			goto again;
		DCC_LOG2(LOG_INFO, "<%d> enter gate %d.", self, gate);
		arg[0] = 0;
		return;
	}

	/* (1) - suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem to have a thread not contained in any waiting
	   queue inside a system call as long as we insert it into a 
	   waiting queue before leaving.
	 */
	__krn_thread_suspend(krn, self);
	/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
	__thread_stat_set(krn, self, gate, 0);
#endif
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
/* NEW: 2020-12-02 performed by the svc call entry stub 
	__thread_ctx_flush(krn, arg, self);
*/

	/* insert into the gate wait queue */
	queue = __ldrex(&krn->wq_lst[gate]);
	queue |= (1 << (self - 1));
	/* The gate may have been signaled while suspending (1).
	 If this is the case roll back and restart. */
	if ((((volatile uint32_t)*gates_bmp >> idx) & 3) == __GATE_SIGNALED ||
		__strex(&krn->wq_lst[gate], queue)) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting at gate %d...", self, gate);
	/* signal the scheduler ... */
	__krn_defer_sched(krn); 
}

#if (THINKOS_ENABLE_TIMED_CALLS)
void thinkos_gate_timedwait_svc(int32_t arg[], int self, 
								struct thinkos_rt * krn)
{
	unsigned int gate = arg[0];
	uint32_t ms = (uint32_t)arg[1];
	unsigned int idx = gate - THINKOS_GATE_BASE;
	uint32_t * gates_bmp;
	uint32_t gates;
	uint32_t queue;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_gate_check(krn, gate)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid gate %d!", self, gate);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_GATE_MAX < 16)
	gates_bmp = &krn->gate[0];
#else
	gates_bmp = &krn->gate[idx / 16];
	idx %= 16;
#endif
	idx *= 2;

	DCC_LOG1(LOG_MSG, "gate %d", gate);

again:
	/* check whether the gate is open or not */
	gates = __ldrex(gates_bmp);

	if (((gates >> idx) & 3) == 3) {
		DCC_LOG1(LOG_ERROR, "<%d> invalid state, open and locked.", self);
		arg[0] = 0;
		return;
	}

	if (((gates >> idx) & 3) == __GATE_SIGNALED) {
		/* close and lock the gate */
		gates = (gates & ~(3 << idx)) | (__GATE_LOCKED << idx);
		if (__strex(gates_bmp, gates))
			goto again;
		DCC_LOG2(LOG_INFO, "<%d> enter gate %d.", self, gate);
		arg[0] = 0;
		return;
	}


	/* (1) - suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem having a thread not contained in any waiting
	   queue inside a system call. 
	 */
	__krn_thread_suspend(krn, self);
	/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	__thread_stat_set(krn, self, gate, 1);
#endif
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
/* NEW: 2020-12-02 performed by the svc call entry stub 
	__thread_ctx_flush(krn, arg, self);
*/
	/* insert into the gate wait queue */
	queue = __ldrex(&krn->wq_lst[gate]);
	queue |= (1 << (self -1));
	/* The gate may have been signaled while suspending (1).
	 If this is the case roll back and restart. */
	if ((((volatile uint32_t)*gates_bmp >> idx) & 3) == __GATE_SIGNALED ||
		__strex(&krn->wq_lst[gate], queue)) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);
		DCC_LOG1(LOG_INFO, "<%d> again.", self);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG3(LOG_INFO, "<%d> waiting at gate %d, state=%d...", 
			 self, gate, (gates >> idx) & 3);
	__krn_thread_clk_itv_wait(krn, self, ms);
}
#endif

void thinkos_gate_exit_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int gate = arg[0];
	unsigned int open = arg[1];
	unsigned int idx = gate - THINKOS_GATE_BASE;
	uint32_t * gates_bmp;
	uint32_t gates;
	uint32_t queue;
	int th;
	int j;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_gate_check(krn, gate)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid gate %d!", self, gate);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_ENABLE_SANITY_CHECK)
	if (!__bit_mem_rd(krn->gate, idx * 2 + 1)) {
		DCC_LOG2(LOG_ERROR, "<%d> gate %d is not locked!", self, gate);
		__THINKOS_ERROR(self, THINKOS_ERR_GATE_UNLOCKED);
		arg[0] = THINKOS_EPERM;
		return;
	}
#endif

#if (THINKOS_GATE_MAX < 16)
	gates_bmp = &krn->gate[0];
#else
	gates_bmp = &krn->gate[idx / 16];
	idx %= 16;
#endif
	idx *= 2;

	DCC_LOG1(LOG_MSG, "gate %d", gate);

	arg[0] = 0;

	/* At this point assume the gate is locked! */

	/* check whether the gate should be left oepn or not */
	if (open > 0) {
		/* user wants the gate to stay open, make sure it signaled.  */
		do {
			gates = __ldrex(gates_bmp);
			gates |= (__GATE_SIGNALED << idx);
		} while (__strex(gates_bmp, gates));
		DCC_LOG2(LOG_INFO, "<%d> exit gate %d, leave open.", self, gate);
	} else { /* (open == 0) */
again:
		gates = __ldrex(gates_bmp);
		if ((gates & (__GATE_SIGNALED << 3)) == 0) {
			/* not signaled, unlock the gate and leave. */
			gates &= ~(__GATE_LOCKED << idx);
			if (__strex(gates_bmp, gates))
				goto again;
			DCC_LOG2(LOG_INFO, "<%d> exit gate %d, leave closed.", 
					 self, gate);
			return;
		}
		DCC_LOG2(LOG_INFO, "<%d> exit gate %d, leave open due to signal.", 
				 self, gate);
	}

	/* At this point the gate is garanteed to be signaled and locked! */

	do {
		/* get a thread from the queue bitmap */
		queue = __ldrex(&krn->wq_lst[gate]);
		if ((j = __thinkos_ffs(queue)) == THINKOS_THREAD_NULL) {
			/* no threads waiting on the gate, unlock the gate */
			do {
				gates = __ldrex(gates_bmp);
				gates &= ~(__GATE_LOCKED << idx);
			} while (__strex(gates_bmp, gates));
			DCC_LOG2(LOG_MSG, "<%d> unlock gate %d.", self, gate);
			return;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << j);
	} while (__strex(&krn->wq_lst[gate], queue));
	th = j + 1;

	/* close the gate by removing the signal */
	do {
		gates = __ldrex(gates_bmp);
		gates &= ~(__GATE_SIGNALED << idx);
	} while (__strex(gates_bmp, gates));


	DCC_LOG2(LOG_INFO, "<%d> enter gate %d.", th, gate);

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);  
	/* set the thread's return value */
	__thread_r0_set(krn, th, 0);
#endif
	/* update status */
	__thread_stat_clr(krn, th);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

void __krn_gate_open(struct thinkos_rt * krn, uint32_t gate)
{
	unsigned int idx = gate - THINKOS_GATE_BASE;
	uint32_t * gates_bmp;
	uint32_t bits;
	uint32_t queue;
	int j;
	int th;

	DCC_LOG1(LOG_INFO, "gate %d", gate);

#if (THINKOS_GATE_MAX < 16)
	gates_bmp = &krn->gate[0];
#else
	gates_bmp = &krn->gate[idx / 16];
	idx %= 16;
#endif
	idx *= 2;

again:
	/* check whether the gate is locked or not */
	bits = __ldrex(gates_bmp);
	if (bits & (__GATE_LOCKED << idx)) {
		/* gate is locked, signal for open */
		bits |= __GATE_SIGNALED << idx;
		if (__strex(gates_bmp, bits))
			goto again;
		DCC_LOG1(LOG_INFO, "gate %d is locked, signaling!", gate);
		return;
	}
	/* lock the gate to avoid race conditions */
	bits |= (__GATE_LOCKED | __GATE_SIGNALED) << idx;
	if (__strex(gates_bmp, bits))
		goto again;

	do {
		/* get a thread from the queue bitmap */
		queue = __ldrex(&krn->wq_lst[gate]);
		if ((j = __thinkos_ffs(queue)) == THINKOS_THREAD_NULL) {
			/* no threads waiting on the gate, unlock the gate */
			do {
				bits = __ldrex(gates_bmp);
				bits &= ~(__GATE_LOCKED << idx);
			} while (__strex(gates_bmp, bits));
			DCC_LOG2(LOG_MSG, "<%d> unlock gate %d.", j + 1, gate);
			return;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << j);
	} while (__strex(&krn->wq_lst[gate], queue));
	th = j + 1;

	/* close the gate by removing the signal */
	do {
		bits = __ldrex(gates_bmp);
		bits &= ~(__GATE_SIGNALED << idx);
	} while (__strex(gates_bmp, bits));


	DCC_LOG2(LOG_INFO, "<%d> enter gate %d.", th, gate);

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);  
#endif
	/* set the thread's return value */
	__thread_r0_set(krn, th, 0);
	/* update status */
	__thread_stat_clr(krn, th);
}

#if (THINKOS_ENABLE_I_CALLS)
void cm3_except13_isr(uint32_t gate)
{
	struct thinkos_rt * krn = &thinkos_rt;

	/* open the gate */
	__krn_gate_open(krn, gate);
	/* signal the scheduler ... */
	__krn_preempt(krn);
}

void __thinkos_gate_open_i(uint32_t gate)
{
	struct thinkos_rt * krn = &thinkos_rt;

	/* open the gate */
	__krn_gate_open(krn, gate);
}
#endif /* THINKOS_ENABLE_I_CALLS */

void thinkos_gate_open_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int gate = arg[0];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_gate_check(krn, gate)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid gate %d!", self, gate);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
	arg[0] = 0;

	DCC_LOG1(LOG_INFO, "gate %d", gate);

	/* open the gate */
	__krn_gate_open(krn, gate);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);;
}

void thinkos_gate_close_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int gate = arg[0];
	unsigned int idx = gate - THINKOS_GATE_BASE;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_gate_check(krn, gate)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid gate %d!", self, gate);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
	arg[0] = 0;

	DCC_LOG1(LOG_INFO, "gate %d", gate);

	/* close the gate */
	__bit_mem_wr(krn->gate, idx * 2, 0);
}

#if (THINKOS_ENABLE_PAUSE)
bool gate_resume(struct thinkos_rt * krn, unsigned int th, 
				 unsigned int gate, bool tmw) 
{
	unsigned int idx = gate - THINKOS_GATE_BASE;
	int open;
	int lock;
	
	open = __bit_mem_rd(krn->gate, idx * 2);
	lock = __bit_mem_rd(krn->gate, idx * 2 + 1); 

	DCC_LOG3(LOG_INFO, "PC=%08x open=%d lock=%d...........", 
			 __thread_pc_get(krn, th), open, lock);

	/* THINKOS_GATE_SIGNALED */
	if (open && !lock) {
		/* close the gate */
		__bit_mem_wr(krn->gate, idx * 2, 0);
		/* lock the gate */
		__bit_mem_wr(krn->gate, idx * 2 + 1, 1);

		/* insert the thread into ready queue */
		__thread_ready_set(krn, th);
#if THINKOS_ENABLE_TIMED_CALLS
		/* set the thread's return value */
		__thread_r0_set(krn, th, 0);
#endif
		/* update status */
		__thread_stat_clr(krn, th);
	} else { 
		__thread_wq_set(krn, th, gate);
#if THINKOS_ENABLE_TIMED_CALLS
		if (tmw)
			__thread_clk_enable(krn, th);
#endif
	}
	return true;
}
#endif



#endif /* THINKOS_GATE_MAX > 0 */


