/* 
 * thinkos.c
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

bool ready_resume(struct thinkos_rt * krn, 
						 unsigned int th, unsigned int wq, bool tmw) 
{
	DCC_LOG2(LOG_INFO, "th=%d PC=%08x +++++", th, __thread_pc_get(krn, th)); 
	__thread_ready_set(krn, th);
	return true;
}

#if (THINKOS_MUTEX_MAX) > 0
bool krn_mutex_resume(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int mtx, bool tmw); 
#endif

#if (THINKOS_IRQ_MAX) > 0
bool irq_resume(struct thinkos_rt * krn, unsigned int th, 
					   unsigned int wq, bool tmw);
#endif

#if (THINKOS_ENABLE_TIMESHARE)
bool tmshare_resume(struct thinkos_rt * krn, unsigned int th, 
						   unsigned int wq, bool tmw) 
{
	DCC_LOG2(LOG_INFO, "th=%d PC=%08x +++++", 
			 th, __thread_pc_get(krn, th)); 
	__thread_wq_set(krn, th, wq);

	return true;
}
#endif

bool clock_resume(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int wq, bool tmw);

#if (THINKOS_COND_MAX) > 0
bool krn_cond_resume(struct thinkos_rt * krn, unsigned int th, 
				 unsigned int wq, bool tmw);
#endif

#if (THINKOS_SEMAPHORE_MAX) > 0
bool semaphore_resume(struct thinkos_rt * krn, unsigned int th, 
							 unsigned int wq, bool tmw);
#endif

#if (THINKOS_EVENT_MAX) > 0
bool evset_resume(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int wq, bool tmw);
#endif

#if (THINKOS_FLAG_MAX) > 0
bool flag_resume(struct thinkos_rt * krn, unsigned int th, 
				 unsigned int wq, bool tmw);
#endif

#if (THINKOS_GATE_MAX > 0)
bool gate_resume(struct thinkos_rt * krn, unsigned int th, 
					unsigned int wq, bool tmw);
#endif

#if (THINKOS_ENABLE_JOIN)
bool join_resume(struct thinkos_rt * krn, unsigned int th, 
						unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 

	__thread_wq_set(krn, th, wq);
	return true;
}
#endif

#if (THINKOS_ENABLE_CONSOLE)
bool thinkos_console_rd_resume(struct thinkos_rt * krn, unsigned int th, 
							   unsigned int wq, bool tmw);
bool thinkos_console_wr_resume(struct thinkos_rt * krn, unsigned int th, 
							   unsigned int wq, bool tmw);
#endif

#if (THINKOS_ENABLE_COMM)
bool comm_recv_resume(struct thinkos_rt * krn, unsigned int th, unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 
	/* wakeup from the comm recv wait queue setting the return value to 0.
	   The calling thread should retry the operation. */
	__krn_wq_wakeup(krn, wq, th, 0);
	return true;
}

bool comm_send_resume(struct thinkos_rt * krn, unsigned int th, 
							 unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 
	/* wakeup from the comm send wait queue setting the return value to 0.
	   The calling thread should retry the operation. */
	__krn_wq_wakeup(krn, wq, th, 0);
	return true;
}
#endif

#if (THINKOS_ENABLE_JOIN)
bool canceled_resume(struct thinkos_rt * krn, unsigned int th, 
							unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 

	__thread_wq_set(krn, th, wq);
	return true;
}
#endif

#if (THINKOS_ENABLE_PAUSE)
bool paused_resume(struct thinkos_rt * krn, unsigned int th, 
						  unsigned int wq, bool tmw) 
{
	DCC_LOG2(LOG_INFO, "invalid state: th=%d PC=%08x !!!!!!",
			 th, __thread_pc_get(krn, th)); 
	return false;
}
#endif

#if (THINKOS_ENABLE_THREAD_FAULT)
bool fault_resume(struct thinkos_rt * krn, unsigned int th, 
						 unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th));

	/* Is the thread error condition clear ? */
	if (__thread_errno_get(krn, th) == 0)
		__krn_wq_wakeup(krn, wq, th);
	return true;
}
#endif

#if ((THINKOS_FLASH_MEM_MAX) > 0)
bool flash_mem_resume(struct thinkos_rt * krn, 
							 unsigned int th, unsigned int wq, bool tmw) 
{
	return true;
}
#endif


#if (THINKOS_ENABLE_PAUSE) && (THINKOS_ENABLE_THREAD_STAT) 
typedef  bool (* thread_resume_t)(struct thinkos_rt *, 
								  unsigned int, unsigned int, bool);

static const thread_resume_t thread_resume_lut[] = {
	[THINKOS_OBJ_READY] = ready_resume,
#if (THINKOS_ENABLE_JOIN)
	[THINKOS_OBJ_THREAD] = join_resume,
#endif
	[THINKOS_OBJ_CLOCK] = clock_resume,
#if THINKOS_MUTEX_MAX > 0
	[THINKOS_OBJ_MUTEX] = krn_mutex_resume,
#endif
#if THINKOS_COND_MAX > 0
	[THINKOS_OBJ_COND] = krn_cond_resume,
#endif
#if THINKOS_SEMAPHORE_MAX > 0
	[THINKOS_OBJ_SEMAPHORE] = semaphore_resume,
#endif
#if THINKOS_EVENT_MAX > 0
	[THINKOS_OBJ_EVENT] = evset_resume,
#endif
#if THINKOS_FLAG_MAX > 0
	[THINKOS_OBJ_FLAG] = flag_resume,
#endif
#if (THINKOS_GATE_MAX) > 0
	[THINKOS_OBJ_GATE] = gate_resume,
#endif
#if (THINKOS_ENABLE_CONSOLE)
	[THINKOS_OBJ_CONREAD] = thinkos_console_rd_resume,
	[THINKOS_OBJ_CONWRITE] = thinkos_console_wr_resume,
#endif
#if (THINKOS_ENABLE_PAUSE)
	[THINKOS_OBJ_PAUSED] = paused_resume,
#endif
#if (THINKOS_ENABLE_JOIN)
	[THINKOS_OBJ_CANCELED] = canceled_resume,
#endif
#if (THINKOS_ENABLE_TIMESHARE)
	[THINKOS_OBJ_TMSHARE] = tmshare_resume,
#endif
#if (THINKOS_ENABLE_COMM)
	[THINKOS_OBJ_COMMSEND] = comm_send_resume,
	[THINKOS_OBJ_COMMRECV] = comm_recv_resume
#endif
#if (THINKOS_IRQ_MAX) > 0
	[THINKOS_OBJ_IRQ]      = irq_resume,
#endif
#if (THINKOS_ENABLE_THREAD_FAULT)
	[THINKOS_OBJ_FAULT] = fault_resume,
#endif
#if (THINKOS_FLASH_MEM_MAX) > 0
	[THINKOS_OBJ_FLASH_MEM] = flash_mem_resume
#endif
};
#endif /* (THINKOS_ENABLE_PAUSE) && (THINKOS_ENABLE_THREAD_STAT) */

bool __krn_thread_pause(struct thinkos_rt * krn, unsigned int th)
{
	unsigned int wq;

#if (THINKOS_ENABLE_PAUSE)
	if (__thread_pause_get(krn, th)) {
		DCC_LOG1(LOG_INFO, "thread=%d is paused already!", th);
		/* paused */
		return false;
	}

	/* insert into the paused queue */
	__thread_pause_set(krn, th);  
#endif

#if (THINKOS_ENABLE_THREAD_STAT)
	{
		int stat;

		/* remove the thread from a waiting queue, including ready  */
		stat = krn->th_stat[th];
		wq = stat >> 1;
		DCC_LOG4(LOG_TRACE, "thread=%d stat=0x%02x wq=%d clk=%d", 
				 th, stat, wq, (stat & 1));
		__krn_wq_thread_del(krn, wq, th);
		__krn_wq_tmshare_thread_del(krn, wq);
	}
#else
	/* remove from all queues */
	for (wq = 0; wq < THINKOS_WQ_CNT; ++wq) 
		__krn_wq_remove(krn, wq, th);
#endif /* THINKOS_ENABLE_THREAD_STAT */

#if (THINKOS_IRQ_MAX) > 0
	{
		int irq;
		for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
			if (krn->irq_th[irq] == (int)th) {
				DCC_LOG2(LOG_INFO, "thread=%d IRQ=%d", th, irq);
				/* disable this interrupt source */
				cm3_irq_disable(irq);
				break;
			}
		}
	}
#endif

#if (THINKOS_ENABLE_DEBUG_STEP)
	/* possibly clear the step request */
	__bit_mem_wr(&krn->debug.step_req, th - 1, 0);
	/* possibly clear the step on service */
	__bit_mem_wr(&krn->debug.step_svc, th - 1, 0);
#endif

	/* disable the clock */
	__krn_wq_clock_thread_del(krn, th);

	return true;
}

bool __krn_thread_resume(struct thinkos_rt * krn, unsigned int th)
{
#if (THINKOS_ENABLE_THREAD_STAT) 
	unsigned int wq;
	bool tmw;
#endif

#if (THINKOS_ENABLE_PAUSE) && (THINKOS_ENABLE_THREAD_STAT) 
	bool (* resume)(struct thinkos_rt *, unsigned int, unsigned int, bool);
	int type;

	if (__thread_pause_get(krn, th)) {
		/* remove from the paused queue */
		__thread_pause_clr(krn, th);  
	} 

	/* reinsert the thread into a waiting queue, including ready  */
	wq = __thread_wq_get(&thinkos_rt, th);
	tmw = __thread_tmw_get(&thinkos_rt, th);
	type = __thinkos_obj_kind(wq);

	DCC_LOG4(LOG_TRACE, "thread=%d wq=%d clk=%d type=%d", 
			 th, wq, tmw, type);
	resume = thread_resume_lut[type];
	return resume(krn, th, wq, tmw);
#else
  #if (THINKOS_ENABLE_PAUSE)
	if (__thread_pause_get(krn, th)) {
		/* remove from the paused queue */
		__thread_pause_clr(krn, th);  
	} 
  #endif
  #if (THINKOS_ENABLE_THREAD_FAULT)
	if (__thread_fault_get(krn, th)) {
		/* clear the fault flag queue */
		__thread_fault_clr(krn, th);
	}
  #endif
	DCC_LOG1(LOG_INFO, "thread=%d [ready]", th);

#if (THINKOS_ENABLE_THREAD_STAT) 
	wq = (krn->th_stat[th] >> 1) & 0x1ff;
	tmw = krn->th_stat[th] & 1;
	/* reinsert the thread into a waiting queue, including ready  */
	__krn_wq_thread_ins(krn, wq, th);
	/* conditionally re-enable the clock ? */
	if (tmw)
		__krn_wq_clock_thread_ins(krn, th);
#else
	__krn_wq_ready_thread_ins(krn, th);
#endif

  #if (THINKOS_IRQ_MAX) > 0
	{
		int irq;
		for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
			if (krn->irq_th[irq] == (int)th) {
				DCC_LOG2(LOG_INFO, "thread=%d IRQ=%d", th, irq);
				/* enable this interrupt source */
				cm3_irq_enable(irq);
				break;
			}
		}
	}
  #endif

#endif /* (THINKOS_ENABLE_THREAD_STAT) */
	return true;
}

void thinkos_resume_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int th = (unsigned int)arg[0];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = __krn_thread_check(krn, th)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid thread %d!", self, th);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_SANITY_CHECK
	if (!__thread_ctx_is_valid(krn, th)) {
		DCC_LOG1(LOG_INFO, "invalid thread %d!", th);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_CTX_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = THINKOS_OK;

	if (__krn_thread_resume(krn, th))
		__krn_sched_defer(krn);
}

void thinkos_pause_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int th = (unsigned int)(arg[0] == 0 ? self : arg[0]);
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = __krn_thread_check(krn, th)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid thread %d!", self, th);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_SANITY_CHECK
	if (!__thread_ctx_is_valid(krn, th)) {
		DCC_LOG1(LOG_INFO, "invalid thread %d!", th);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_CTX_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	arg[0] = THINKOS_OK;

	if (__krn_thread_pause(krn, th))
		__krn_sched_defer(krn);
}

