/* 
 * thinkos_core.c
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

/* -------------------------------------------------------------------------- 
 * Run Time ThinkOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt thinkos_rt;

#if ((THINKOS_EXCEPT_STACK_SIZE & 0x0000003f) != 0)
#error "THINKOS_EXCEPT_STACK_SIZE must be a multiple 0f 64"
#endif 

uint32_t __attribute__((aligned(64))) 
	thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

const uint16_t thinkos_except_stack_size = sizeof(thinkos_except_stack);

void __thinkos_krn_core_init(struct thinkos_rt * krn)
{
	unsigned int i;

	krn->sched.state = 0x00000000;

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_CNT; ++i)
		krn->wq_lst[i] = 0x00000000;

	/* clear all threads but IDLE */
	for (i = 0; i <= (THINKOS_THREADS_MAX); ++i) {
		__thread_ctx_clr(krn, i);
		__thread_stat_clr(krn, i);
		__thread_inf_clr(krn, i);
		__thread_cyccnt_clr(krn, i);
#if (THINKOS_ENABLE_THREAD_FAULT)
		__thread_errno_clr(krn, i);
#endif
	}

#if (THINKOS_ENABLE_OBJ_ALLOC)
	__krn_alloc_init(krn);
#endif

#if (THINKOS_MUTEX_MAX) > 0
	/* initialize the mutex locks */
	for (i = 0; i < THINKOS_MUTEX_MAX; i++) 
		krn->mtx_lock[i] = 0;
#endif /* THINKOS_MUTEX_MAX > 0 */

#if (THINKOS_SEMAPHORE_MAX) > 0
	for (i = 0; i < THINKOS_SEMAPHORE_MAX; i++) 
		krn->sem_val[i] = 0;
#endif /* THINKOS_SEMAPHORE_MAX > 0 */


#if (THINKOS_FLAG_MAX) > 0
	for (i = 0; i < ((THINKOS_FLAG_MAX) + 31) / 32; i++) 
		krn->flag[i] = 0;
#endif /* THINKOS_FLAG_MAX > 0 */

#if (THINKOS_EVENT_MAX) > 0
	for (i = 0; i < (THINKOS_EVENT_MAX) ; i++) {
		krn->ev[i].pend = 0;
		krn->ev[i].mask = 0xffffffff;
	}
#endif /* THINKOS_EVENT_MAX > 0 */

#if (THINKOS_GATE_MAX) > 0
	for (i = 0; i < (((THINKOS_GATE_MAX) + 15) / 16); i++) 
		krn->gate[i] = 0;
#endif /* THINKOS_GATE_MAX > 0 */

#if (THINKOS_ENABLE_DEBUG_BKPT)
	krn->brk_idx = 0;
	krn->step_id = 0;
#if (THINKOS_ENABLE_DEBUG_STEP)
	krn->step_svc = 0;  /* step at service call bitmap */
	krn->step_req = 0;  /* step request bitmap */
#endif
#endif

#if (THINKOS_ENABLE_CRITICAL)
	krn->critical_cnt = 0;
#endif
}

#if 0
void __thinkos_krn_kill_all(struct thinkos_rt * krn)
{
	int active = __thread_active_get(krn);

	DCC_LOG1(LOG_WARNING, VT_PSH VT_FYW
			 "<%2d> killing all threads ..." VT_POP, active + 1);

	if  (active != THINKOS_THREAD_IDLE) {
		__thread_active_set(krn, THINKOS_THREAD_VOID);
	}

	__thinkos_defer_sched();
}
#endif

void __thinkos_krn_core_reset(struct thinkos_rt * krn)
{
#if DEBUG
	DCC_LOG(LOG_WARNING, VT_PSH VT_FYW "!! Core Reset !!" VT_POP);
#endif

#if (THINKOS_IRQ_MAX) > 0
	DCC_LOG(LOG_TRACE, "1. Disable all intrerrupts ...");
	__nvic_irq_disable_all();
	DCC_LOG(LOG_TRACE, "2. Reset all intrerrupts ...");
	__krn_irq_reset_all(krn);
#endif
	DCC_LOG(LOG_TRACE, "3. Initialize kernel datastructures ...");
	__thinkos_krn_core_init(krn);
#if THINKOS_ENABLE_EXCEPTIONS
	DCC_LOG(LOG_TRACE, "4. exception reset...");
	thinkos_krn_exception_reset();
#endif

#if DEBUG
//	mdelay(500);
//	__kdump(krn);
#endif
}

#if 0
void __thinkos_system_reset(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
	DCC_LOG(LOG_WARNING, VT_PSH VT_FRD "!! System Reset !!" VT_POP);

	DCC_LOG(LOG_TRACE, "1. ThinkOS core reset...");
	__thinkos_krn_core_reset(krn);

	
#if (THINKOS_IRQ_MAX) > 0
	DCC_LOG(LOG_TRACE, "2. Disable all intrerrupts ...");
	__thinkos_irq_disable_all();
	__thinkos_irq_reset_all();
#endif

#if THINKOS_ENABLE_EXCEPTIONS
	DCC_LOG(LOG_TRACE, "3. exception reset...");
	thinkos_krn_exception_reset();
#endif

	if  (active != THINKOS_THREAD_IDLE) {
		__thread_active_set(krn, THINKOS_THREAD_VOID);
		__thinkos_defer_sched();
	}

	/* Enable Interrupts */
	DCC_LOG(LOG_TRACE, "4. enablig interrupts...");
	cm3_cpsie_i();
}
#endif

bool thinkos_sched_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_PENDSVACT) ? true : false;
}
	
bool thinkos_syscall_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_SVCALLACT) ? true : false;
}

bool thinkos_clock_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_SYSTICKACT) ? true : false;
}

bool thinkos_dbgmon_active(void)
{
	return (CM3_SCB->shcsr & SCB_SHCSR_MONITORACT) ? true : false;
}

bool thinkos_kernel_active(void)
{
#if 0
	return (CM3_SCB->shcsr & (SCB_SHCSR_SYSTICKACT | SCB_SHCSR_PENDSVACT | 
							  SCB_SHCSR_SVCALLACT)) ? true : false;
#endif
	return (CM3_SCB->shcsr & (SCB_SHCSR_PENDSVACT | 
							  SCB_SHCSR_SVCALLACT)) ? true : false;
}

void thinkos_krn_userland(void)
{
	cm3_control_set(CONTROL_SPSEL | CONTROL_nPRIV);
}

unsigned int __thinkos_obj_kind(unsigned int oid) 
{
	return __obj_kind(oid);
}

int __thinkos_kind_prefix(unsigned int kind)
{ 
	return __kind_prefix(kind);
}

const char * __thinkos_kind_name(unsigned int kind)
{ 
	return __kind_name(kind);
}

void __krn_cyccnt_flush(struct thinkos_rt * krn, unsigned int th)
{
#if (THINKOS_ENABLE_PROFILING)
	uint32_t ref;
	uint32_t cnt;

	cnt = CM3_DWT->cyccnt;
	ref = krn->cycref;
	krn->cycref = cnt;

	krn->th_cyc[th] += cnt - ref;
#endif
}

int __krn_threads_cyc_get(struct thinkos_rt * krn, uint32_t cyc[], 
						  unsigned int from, unsigned int cnt)
{
#if (THINKOS_ENABLE_PROFILING)
	if (from >= __KRN_THREAD_LST_SIZ)
		return -THINKOS_EINVAL;

	if (cnt > (__KRN_THREAD_LST_SIZ - from))
		cnt = (__KRN_THREAD_LST_SIZ - from);

	__krn_cyccnt_flush(krn, __krn_sched_active_get(krn));
	__thinkos_memcpy32(cyc, &krn->th_cyc[from], cnt * sizeof(uint32_t)); 

	return cnt;
#else
	return -THINKOS_ENOSYS;
#endif
}

int __krn_threads_inf_get(struct thinkos_rt * krn, 
						  const struct thinkos_thread_inf * inf[],
						  unsigned int from, unsigned int cnt)
{
#if (THINKOS_ENABLE_PROFILING)
	if (from >= __KRN_THREAD_LST_SIZ)
		return -THINKOS_EINVAL;

	if (cnt > (__KRN_THREAD_LST_SIZ - from))
		cnt = (__KRN_THREAD_LST_SIZ - from);

	__thinkos_memcpy32((void *)inf, &krn->th_inf[from], cnt * sizeof(void *)); 

	return cnt;
#else
	return -THINKOS_ENOSYS;
#endif
}

