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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

#include <sys/delay.h>
#include <thinkos.h>
#include <vt100.h>
#include <sys/dcclog.h>

extern void * __rom_vectors[];
extern int __sizeof_rom_vectors;

#ifdef CM3_RAM_VECTORS
extern void * __ram_vectors[];
#endif

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

void __thinkos_krn_reset(struct thinkos_rt * krn)
{
	unsigned int i;

	DCC_LOG(LOG_WARNING, VT_PSH VT_FRD " /!\\ Kernel Reset /!\\ " VT_POP);

	/* clear all threads excpet NULL */
	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		__thread_ctx_clr(krn, i);
		__thread_stat_clr(krn, i);
		__thread_inf_clr(krn, i);
		__thread_fault_clr(krn, i);
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

void thinkos_krn_kill_all(struct thinkos_rt * krn)
{
	int active = __thread_active_get(krn);
	int i;

	DCC_LOG1(LOG_WARNING, VT_PSH VT_FYW
			 "<%2d> killing all threads ..." VT_POP, active + 1);

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_CNT; ++i)
		krn->wq_lst[i] = 0x00000000;

	if  (active != THINKOS_THREAD_IDLE) {
		__thread_active_set(krn, THINKOS_THREAD_VOID);
	}

	__thinkos_defer_sched();
}

void thinkos_krn_abort(struct thinkos_rt * krn)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_FGR " /!\\ Kernel Abort /!\\ " VT_POP);

	thinkos_krn_kill_all(krn); 

	__thinkos_krn_reset(krn);

#if (THINKOS_ENABLE_MONITOR)
	monitor_signal(MONITOR_KRN_ABORT);
#else

#endif
}

void __thinkos_core_reset(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__thinkos_krn_reset(krn);

#if (THINKOS_IRQ_MAX) > 0
	__thinkos_irq_reset_all();
#endif

#ifdef CM3_RAM_VECTORS
	__thinkos_memcpy(__ram_vectors, __rom_vectors, __sizeof_rom_vectors);
	/* Remap the Vector table to SRAM */
	CM3_SCB->vtor = (uintptr_t)__ram_vectors; /* Vector Table Offset */
#endif
}

void __thinkos_system_reset(void)
{
	DCC_LOG(LOG_WARNING, "/!\\ System reset in progress...");

	DCC_LOG(LOG_TRACE, "1. ThinkOS core reset...");
	__thinkos_core_reset();

#if THINKOS_ENABLE_EXCEPTIONS
	DCC_LOG(LOG_TRACE, "2. exception reset...");
	__exception_reset();
#endif

	/* Enable Interrupts */
	DCC_LOG(LOG_TRACE, "4. enablig interrupts...");
	cm3_cpsie_i();
}

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

