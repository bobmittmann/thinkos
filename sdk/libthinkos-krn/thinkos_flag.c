/* 
 * thinkos_flag.c
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
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

#if (THINKOS_FLAG_MAX) > 0

static inline bool __attribute__((always_inline)) 
__krn_obj_is_flag(struct thinkos_rt * krn, unsigned int flag) {
	return __obj_is_valid(flag, THINKOS_FLAG_BASE, THINKOS_FLAG_MAX);
}

#if (THINKOS_ENABLE_FLAG_ALLOC)
static inline bool __attribute__((always_inline)) 
__krn_flag_is_alloc(struct thinkos_rt * krn, unsigned int flag) {
	return __bit_mem_rd(krn->flag_alloc, flag - THINKOS_FLAG_BASE) ? 
		true : false;
}
#endif

#if (THINKOS_ENABLE_ARG_CHECK)
int krn_flag_check(struct thinkos_rt * krn, int flag)
{
	if (!__krn_obj_is_flag(krn, flag)) {
		return THINKOS_ERR_FLAG_INVALID;
	}
#if (THINKOS_ENABLE_FLAG_ALLOC)
	if (__krn_flag_is_alloc(krn, flag) == 0) {
		return THINKOS_ERR_FLAG_ALLOC;
	}
#endif
	return THINKOS_OK;
}
#endif

/* --------------------------------------------------------------------------
 * Flag give/take family 
 * -------------------------------------------------------------------------- */

/* wait for the flag */
void thinkos_flag_take_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
	unsigned int idx = flag - THINKOS_FLAG_BASE;
	uint32_t * flags_bmp;
	uint32_t flags;
	uint32_t queue;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_FLAG_MAX) < 32
	flags_bmp = &krn->flag[0];
#else
	flags_bmp = &krn->flag[idx / 32];
	idx %= 32;
#endif

again:
	flags = __ldrex(flags_bmp);
	if (flags & (1 << idx)) {
		DCC_LOG2(LOG_TRACE, "<%d> signaled %d...", self, flag);
		/* clear the flag */
		flags &= ~(1 << idx);
		if (__strex(flags_bmp, flags))
			goto again;
		arg[0] = THINKOS_OK;
		return;
	}

	/* (1) - suspend the thread by removing it from the
	   ready wait queue. The __thread_suspend(krn, ) call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem having a thread not contained in any waiting
	   queue inside a system call. 
	 */
	__krn_thread_suspend(krn, self);
	/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
	__thread_stat_set(krn, self, flag, 0);
#endif
	/* insert into the flag wait queue */
	queue = __ldrex(&krn->wq_lst[flag]);
	queue |= (1 << (self - 1));
	/* The flag may have been signaled while suspending (1).
	 If this is the case roll back and restart. */
	if (((volatile uint32_t)*flags_bmp & (1 << idx)) ||
		__strex(&krn->wq_lst[flag], queue)) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_TRACE, "<%d> waiting for flag %d...", self, flag);
	/* signal the scheduler ... */
	__krn_sched_defer(krn); 
}


#if (THINKOS_ENABLE_TIMED_CALLS)
void thinkos_flag_timedtake_svc(int32_t arg[], int self, 
								struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
	uint32_t ms = (uint32_t)arg[1];
	unsigned int idx = flag - THINKOS_FLAG_BASE;
	uint32_t * flags_bmp;
	uint32_t bits;
	uint32_t queue;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_FLAG_MAX) < 32
	flags_bmp = &krn->flag[0];
#else
	flags_bmp = &krn->flag[idx / 32];
	idx %= 32;
#endif

again:
	bits = __ldrex(flags_bmp);
	if (bits & (1 << idx)) {
		DCC_LOG2(LOG_TRACE, "<%d> signaled %d...", self, flag);
		/* clear the flag */
		bits &= ~(1 << idx);
		if (__strex(flags_bmp, bits))
			goto again;
		arg[0] = THINKOS_OK;
		return;
	}

	/* (1) - suspend the thread by removing it from the
	   ready wait queue. The __thread_suspend(krn) call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem having a thread not contained in any waiting
	   queue inside a system call. 
	 */
	__krn_thread_suspend(krn, self);
	/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	__thread_stat_set(krn, self, flag, 0);
#endif
	/* insert into the flag wait queue */
	queue = __ldrex(&krn->wq_lst[flag]);
	queue |= (1 << (self - 1));
	/* The flag may have been signaled while suspending (1).
	 If this is the case roll back and restart. */
	if (((volatile uint32_t)*flags_bmp & (1 << idx)) ||
		__strex(&krn->wq_lst[flag], queue)) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_TRACE, "<%d> waiting for flag %d...", self, flag);
	__krn_thread_clk_itv_wait(krn, self, ms);
}
#endif

void __krn_flag_give(struct thinkos_rt * krn, int flag)
{
	unsigned int idx = flag - THINKOS_FLAG_BASE;
	uint32_t * flags_bmp;
	uint32_t bits;
	uint32_t queue;
	int th;
	int j;

#if (THINKOS_FLAG_MAX) < 32
	flags_bmp = &krn->flag[0];
#else
	flags_bmp = &krn->flag[idx / 32];
	idx %= 32;
#endif

	do {
		/* insert into the event wait queue */
		queue = __ldrex(&krn->wq_lst[flag]);
		/* get a thread from the queue bitmap */
		if ((j = __thinkos_ffs(queue)) == 32) {
			/* no threads waiting on the flag, . */ 
			do {
				bits = __ldrex(flags_bmp);
				/* set the flag */
				bits |= (1 << idx);
			} while (__strex(flags_bmp, bits));

			return;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << j);
	} while (__strex(&krn->wq_lst[flag], queue));
	th = j + 1;

	DCC_LOG1(LOG_TRACE, "<%2d> wakeup...", th);

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);  
	/* set the thread's return value */
	__thread_r0_set(krn, th, 0);
#endif
	__thread_stat_clr(krn, th);
}

#if (THINKOS_ENABLE_I_CALLS)
void cm3_except10_isr(uint32_t flag)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_flag_give(krn, flag);
	/* signal the scheduler ... */
	__krn_preempt(krn);
}

void __thinkos_flag_give_i(uint32_t flag)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_flag_give(krn, flag);
	__krn_preempt(krn);
}
#endif /* THINKOS_ENABLE_I_CALLS */

/* wakeup a single thread waiting on the flag OR set the flag */
void thinkos_flag_give_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = THINKOS_OK;

	__krn_flag_give(krn, flag);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

/* --------------------------------------------------------------------------
 * Flag watch family 
 * -------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_FLAG_WATCH)

void thinkos_flag_val_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
	unsigned int idx = flag - THINKOS_FLAG_BASE;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = __bit_mem_rd(krn->flag, idx);
}

static void __thinkos_flag_clr(struct thinkos_rt * krn, uint32_t flag)
{
	unsigned int idx = flag - THINKOS_FLAG_BASE;

	/* clear the flag signal bit */
	__bit_mem_wr(krn->flag, idx, 0);  
}

void thinkos_flag_clr_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = THINKOS_OK;
	__thinkos_flag_clr(krn, flag);
}

void thinkos_flag_set_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
	unsigned int idx = flag - THINKOS_FLAG_BASE;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	DCC_LOG1(LOG_INFO, "flag=%d...", flag);
	arg[0] = THINKOS_OK;
	/* set the flag and wakeup all threads waiting on the flag */
	__bit_mem_wr(krn->flag, idx, 1);  
	__krn_wq_wakeup_all(krn, flag);
}

void thinkos_flag_watch_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
	unsigned int idx = flag - THINKOS_FLAG_BASE;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* set the return value */
	arg[0] = THINKOS_OK;

	/* flag is set just return */
	if (__bit_mem_rd(krn->flag, idx))
		return;

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting for flag %d...", self, flag);
	__krn_thread_wait(krn, self, flag);
}

#if (THINKOS_ENABLE_TIMED_CALLS)
void thinkos_flag_timedwatch_svc(int32_t arg[], int self, 
								 struct thinkos_rt * krn)
{
	unsigned int flag = arg[0];
	uint32_t ms = (uint32_t)arg[1];
	unsigned int idx = flag - THINKOS_FLAG_BASE;
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_flag_check(krn, flag)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid event %d!", self, flag);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = THINKOS_OK;
	if (__bit_mem_rd(krn->flag, idx))
		return;

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%2d> waiting for flag %d...", self, flag);
	/* Set the default return value to timeout. The
	   mutex_unlock() call will change this to 0 */
	arg[0] = THINKOS_ETIMEDOUT;
	__krn_thread_timedwait(krn, self, flag, ms);
}
#endif

#endif /* THINKOS_ENABLE_FLAG_WATCH */

#if (THINKOS_ENABLE_PAUSE)
bool flag_resume(struct thinkos_rt * krn, unsigned int th, 
				 unsigned int flag, bool tmw) 
{
	unsigned int idx = flag - THINKOS_FLAG_BASE;

	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 

	if (__bit_mem_rd(krn->flag, idx)) {
		/* clear the signal */
		__bit_mem_wr(krn->flag, idx, 0);
		/* insert the thread into ready queue */
		__thread_ready_set(krn, th);
#if (THINKOS_ENABLE_TIMED_CALLS)
		/* set the thread's return value */
		__thread_r0_set(krn, th, 0);
#endif
		/* update status */
		__thread_stat_clr(krn, th);
	} else { 
		__thread_wq_set(krn, th, flag);
#if (THINKOS_ENABLE_TIMED_CALLS)
		if (tmw)
			__thread_clk_enable(krn, th);
#endif
	}
	return true;
}

#endif

#endif /* THINKOS_FLAG_MAX > 0 */

