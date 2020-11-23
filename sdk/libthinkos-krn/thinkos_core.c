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

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_CNT; ++i)
		krn->wq_lst[i] = 0x00000000;

	/* clear all threads excpet NULL */
	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		__thread_ctx_clr(krn, i);
		__thread_stat_clr(krn, i);
		__thread_inf_clr(krn, i);
		__thread_errno_clr(krn, i);
		__thread_cyccnt_clr(krn, i);
	}

#if (THINKOS_ENABLE_OBJ_ALLOC)
	__krn_alloc_init(krn);
#endif

#if (THINKOS_MUTEX_MAX) > 0
	/* initialize the mutex locks */
	for (i = 0; i < THINKOS_MUTEX_MAX; i++) 
		krn->lock[i] = -1;
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
	krn->step_id = -1;
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

void thinkos_krn_core_reset(struct thinkos_rt * krn)
{
	int active = __thread_active_get(krn);

	DCC_LOG(LOG_WARNING, VT_PSH VT_FYW "!! Kernel Reset !!" VT_POP);

	DCC_LOG(LOG_TRACE, "1. Initialize kernel datastructures ...");
	__thinkos_krn_core_init(krn);

#if (THINKOS_IRQ_MAX) > 0
	DCC_LOG(LOG_TRACE, "2. Disable all intrerrupts ...");
	__thinkos_irq_disable_all();
	DCC_LOG(LOG_TRACE, "3. Reset all intrerrupts ...");
	__krn_irq_reset_all(krn);
#endif

#if THINKOS_ENABLE_EXCEPTIONS
	DCC_LOG(LOG_TRACE, "4. exception reset...");
	thinkos_krn_exception_reset();
#endif

	if  (active != THINKOS_THREAD_IDLE) {
		__thread_active_set(krn, THINKOS_THREAD_VOID);
		__thinkos_defer_sched();
	}
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

/* -------------------------------------------------------------------------- 
 * thinkos runtime thread context access methods 
 * --------------------------------------------------------------------------*/

struct thinkos_context * __thinkos_thread_ctx_get(unsigned int id) 
{
	return __thread_ctx_get(&thinkos_rt, id);
}

uintptr_t __thinkos_thread_pc_get(unsigned int id) 
{
	return __thread_pc_get(&thinkos_rt, id); 
}

uint32_t __thinkos_thread_lr_get(unsigned int id) 
{
	return __thread_lr_get(&thinkos_rt, id); 
}

uint32_t __thinkos_thread_sp_get(unsigned int id) 
{
	return __thread_sp_get(&thinkos_rt, id); 
}

uint32_t __thinkos_thread_sl_get(unsigned int id) 
{
	return __thread_sl_get(&thinkos_rt, id); 
}

uint32_t __thinkos_thread_xpsr_get(unsigned int id) 
{
	return __thread_xpsr_get(&thinkos_rt, id); 
}

uint32_t __thinkos_thread_r0_get(unsigned int id) 
{
	return __thread_r0_get(&thinkos_rt, id);
}

void __thinkos_thread_r0_set(unsigned int id, uint32_t val) 
{
	__thread_r0_set(&thinkos_rt, id, val);
}

uint32_t __thinkos_thread_r1_get(unsigned int id) 
{
	return __thread_r1_get(&thinkos_rt, id);
}

void __thinkos_thread_r1_set(unsigned int id, uint32_t val) 
{
	__thread_r1_set(&thinkos_rt, id, val);
}

void __thinkos_thread_r2_set(unsigned int id, uint32_t val) 
{
	__thread_r2_set(&thinkos_rt, id, val);
}

void __thinkos_thread_r3_set(unsigned int id, uint32_t val) 
{
	__thread_r3_set(&thinkos_rt, id, val);
}

void __thinkos_thread_pc_set(unsigned int id, uintptr_t val) 
{
	__thread_pc_set(&thinkos_rt, id, val);
}

void __thinkos_thread_lr_set(unsigned int id, uintptr_t val) 
{
	__thread_lr_set(&thinkos_rt, id, val);
}

uint32_t * __thinkos_thread_frame_get(unsigned int id) 
{
	return __thread_frame_get(&thinkos_rt, id);
}

uint32_t __thinkos_thread_ctrl_get(unsigned int id) 
{
	return __thread_ctrl_get(&thinkos_rt, id); 
}

uint32_t __thinkos_thread_exec_ret_get(unsigned int id) 
{
	return __thread_exec_ret_get(&thinkos_rt, id); 
}

bool __thinkos_thread_ctx_is_valid(unsigned int id) 
{
	return __thread_ctx_is_valid(&thinkos_rt, id);
}

void  __thinkos_thread_ctx_set(unsigned int id, struct thinkos_context * ctx,
							   unsigned int ctrl) 
{
	__thread_ctx_set(&thinkos_rt, id, ctx, ctrl);
}

void  __thinkos_thread_ctx_flush(int32_t arg[], unsigned int id) 
{
	__thread_ctx_flush(&thinkos_rt, arg, id);
}

void  __thinkos_thread_ctx_clr(unsigned int id) 
{
	__thread_ctx_clr(&thinkos_rt, id);
}

const struct thinkos_thread_inf * __thinkos_thread_inf_get(unsigned int id) 
{
	return __thread_inf_get(&thinkos_rt, id);
}

void  __thinkos_thread_inf_set(unsigned int id, 
							   const struct thinkos_thread_inf * inf) 
{
	__thread_inf_set(&thinkos_rt, id, inf);
}

void __thinkos_thread_inf_clr(unsigned int id) 
{
	__thread_inf_set(&thinkos_rt, id, NULL);
}

int __thinkos_thread_errno_get(unsigned int id) 
{
	return __thread_errno_get(&thinkos_rt, id);
}

void __thinkos_thread_errno_set(unsigned int id, int errno) 
{
	__thread_errno_set(&thinkos_rt, id, errno);
}

void __thinkos_thread_errno_clr(unsigned int id) 
{
	__thread_errno_clr(&thinkos_rt, id);
}

void __thinkos_thread_cyccnt_clr(unsigned int id) 
{
	__thread_cyccnt_clr(&thinkos_rt, id);
}

unsigned int __thinkos_thread_stat_wq_get(unsigned int th) 
{
	return __thread_stat_wq_get(&thinkos_rt, th);
}

bool __thinkos_thread_stat_tmw_get(unsigned int th) 
{
	return __thread_stat_tmw_get(&thinkos_rt, th);
}

void __thinkos_thread_stat_clr(unsigned int th) 
{
	__thread_stat_clr(&thinkos_rt, th);
}

void __thinkos_thread_stat_set(unsigned int th, unsigned int wq, bool tmd) 
{
	__thread_stat_set(&thinkos_rt, th, wq, tmd);
}

/* -------------------------------------------------------------------------- 
 * thread stack limit access methods 
 * --------------------------------------------------------------------------*/

void __thinkos_thread_sl_clr(unsigned int idx) 
{
	__thread_sl_clr(&thinkos_rt, idx);
}

void __thinkos_thread_sl_set(unsigned int idx, uint32_t addr) 
{
	__thread_sl_set(&thinkos_rt, idx, addr);
}

void __thinkos_active_set(unsigned int th) 
{
	__thread_active_set(&thinkos_rt, th);
}

unsigned int __thinkos_active_get(void) 
{
	return __thread_active_get(&thinkos_rt);
}

uint32_t __thinkos_active_sl_get(void) 
{
	return __thread_active_sl_get(&thinkos_rt);
}

/* Set the active thread and stack limit */
void __thinkos_active_sl_set(unsigned int th, uint32_t sl) 
{
	__thread_active_sl_set(&thinkos_rt, th, sl);
}

bool __thinkos_thread_is_in_wq(unsigned int id, unsigned int wq) 
{
	return __thread_is_in_wq(&thinkos_rt, id, wq);
}

void  __thinkos_ready_clr(void) 
{
	__wq_ready_clr(&thinkos_rt); 
}

void __thinkos_suspend(unsigned int idx) 
{
	__thread_suspend(&thinkos_rt, idx);
}

int __thinkos_wq_idx(uint32_t * ptr) 
{
	return __wq_idx(&thinkos_rt, ptr);
}

int __thinkos_wq_head(unsigned int wq) 
{
	return __wq_head(&thinkos_rt, wq);
}

void __thinkos_wq_insert(unsigned int wq, unsigned int th)
{
	__wq_insert(&thinkos_rt, wq, th);
}

#if (THINKOS_ENABLE_TIMED_CALLS)
void __thinkos_tmdwq_insert(unsigned int wq, unsigned int th, unsigned int ms)
{
	__tmdwq_insert(&thinkos_rt, wq, th, ms);
}
#endif

#if (THINKOS_ENABLE_TIMED_CALLS)
void __thinkos_wq_clock_insert(unsigned int th, unsigned int ms) 
{
	__wq_clock_insert(&thinkos_rt, th, ms);
}
#endif

void __thinkos_wq_remove( unsigned int wq, unsigned int th) 
{
	__wq_remove(&thinkos_rt, wq, th);
}

void __thinkos_wakeup( unsigned int wq, unsigned int th) 
{
	__wq_wakeup(&thinkos_rt, wq, th);
}

void __thinkos_wakeup_return( unsigned int wq, unsigned int th, int ret) 
{
	__wq_wakeup_return(&thinkos_rt, wq, th, ret);
}


#if (THINKOS_ENABLE_CLOCK)
uint32_t  __thinkos_ticks(void) 
{
	return __krn_ticks(&thinkos_rt);
}
#endif

/* Set the fault flag */
void __thinkos_thread_fault_set(unsigned int th, int errno) 
{
	__thread_fault_set(&thinkos_rt, th, errno);
}

/* Clear the fault flag */
void __thinkos_thread_fault_clr(unsigned int th) 
{
	__thread_fault_clr(&thinkos_rt, th);
}

/* Get the fault flag */
bool __thinkos_thread_fault_get(unsigned int th) 
{
	return __thread_fault_get(&thinkos_rt, th);
}


/* Set the pause flag */
void __thinkos_thread_pause_set(unsigned int th) 
{
	__thread_pause_set(&thinkos_rt, th);
}

/* Clear the pause flag */
void __thinkos_thread_pause_clr(unsigned int th) 
{
	__thread_pause_clr(&thinkos_rt, th);
}

/* Get the pause flag */
bool __thinkos_thread_pause_get(unsigned int th) 
{
	return __thread_pause_get(&thinkos_rt, th);
}

bool __thinkos_thread_pause(unsigned int thread_id)
{
	return __krn_thread_pause(&thinkos_rt, thread_id);
}

bool __thinkos_thread_resume(unsigned int thread_id)
{
	return __krn_thread_resume(&thinkos_rt, thread_id);
}

void __thinkos_resume_all(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_resume_all(krn);
}

void __thinkos_pause_all(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_pause_all(krn);
}


/* -------------------------------------------------------------------------- 
 * kernel scheduler methods 
 * --------------------------------------------------------------------------*/

void __thinkos_cancel_sched(void)
{
	__krn_cancel_sched(&thinkos_rt);
}

/* flags a deferred execution of the scheduler */
void  __thinkos_defer_sched(void)
{
	__krn_defer_sched(&thinkos_rt);
}
/* flags a deferred execution of the scheduler */
void __thinkos_preempt(void)
{
	__krn_preempt(&thinkos_rt);
}

/* -------------------------------------------------------------------------- 
 * kernel structure public access methods 
 * --------------------------------------------------------------------------*/
#if 0
unsigned int thinkos_krn_active_get(struct thinkos_rt * krn) 
{
	idx = __thread_active_get(krn);

	return idx;
}
#endif

const char * __thinkos_thread_tag_get(unsigned int idx) 
{
	return __thread_tag_get(&thinkos_rt, idx);
}

int __thinkos_thread_wq_get(unsigned int thread_idx)
{
	return __thread_wq_get(&thinkos_rt, thread_idx);
}

/* -------------------------------------------------------------------------- 
 * Objects
 * --------------------------------------------------------------------------*/

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

int __thinkos_thread_tmw_get(unsigned int idx) 
{
	return __thread_tmw_get(&thinkos_rt, idx);
}

void __thinkos_suspend_all(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_suspend_all(krn);
}

