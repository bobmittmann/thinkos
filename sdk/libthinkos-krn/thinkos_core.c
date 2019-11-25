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
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#include <sys/delay.h>
#include <thinkos.h>
#include <vt100.h>
#include <sys/dcclog.h>

/* -------------------------------------------------------------------------- 
 * Run Time ThinkOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt thinkos_rt;

uint32_t __attribute__((aligned(8))) 
	thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

const uint16_t thinkos_except_stack_size = sizeof(thinkos_except_stack);

void __thinkos_kill_all(void) 
{
	int i;

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_CNT; ++i)
		thinkos_rt.wq_lst[i] = 0x00000000;

#if THINKOS_ENABLE_THREAD_VOID 
	/* discard current thread context */
	if (thinkos_rt.active != THINKOS_THREAD_IDLE)
		thinkos_rt.active = THINKOS_THREAD_VOID;
#else
	DCC_LOG(LOG_PANIC, "can't set current thread to void!"); 
#endif
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}


void __thinkos_core_reset(void)
{
	int i;

	/* clear all wait queues */
	for (i = 0; i < THINKOS_WQ_CNT; ++i)
		thinkos_rt.wq_lst[i] = 0x00000000;

	/* clear all threads excpet NULL */
	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		thinkos_rt.ctx[i] = 0x00000000;
#if THINKOS_ENABLE_THREAD_STAT
		thinkos_rt.th_stat[i] = 0; 
#endif
#if THINKOS_ENABLE_THREAD_INFO
		thinkos_rt.th_inf[i] = NULL; 
#endif
	}


#if (THINKOS_ENABLE_PROFILING)
	/* Per thread cycle count */
	for (i = 0; i < THINKOS_THREADS_MAX ; ++i)
		thinkos_rt.cyccnt[i] = 0;
#endif

#if THINKOS_ENABLE_THREAD_ALLOC
	/* initialize the thread allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.th_alloc, THINKOS_THREADS_MAX); 
#endif

#if (THINKOS_MUTEX_MAX > 0)
#if THINKOS_ENABLE_MUTEX_ALLOC
	/* initialize the mutex locks */
	for (i = 0; i < THINKOS_MUTEX_MAX; i++) 
		thinkos_rt.lock[i] = -1;
	/* initialize the mutex allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.mutex_alloc, THINKOS_MUTEX_MAX); 
#endif
#endif /* THINKOS_MUTEX_MAX > 0 */

#if THINKOS_SEMAPHORE_MAX > 0
	for (i = 0; i < THINKOS_SEMAPHORE_MAX; i++) 
		thinkos_rt.sem_val[i] = 0;
#if THINKOS_ENABLE_SEM_ALLOC
	/* initialize the semaphore allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.sem_alloc, THINKOS_SEMAPHORE_MAX); 
#endif
#endif /* THINKOS_SEMAPHORE_MAX > 0 */

#if THINKOS_ENABLE_COND_ALLOC
	/* initialize the conditional variable allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.cond_alloc, THINKOS_COND_MAX); 
#endif

#if THINKOS_FLAG_MAX > 0
	for (i = 0; i < (THINKOS_FLAG_MAX + 31) / 32; i++) 
		thinkos_rt.flag[i] = 0;
#if THINKOS_ENABLE_FLAG_ALLOC
	/* initialize the flag allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.flag_alloc, THINKOS_FLAG_MAX); 
#endif
#endif /* THINKOS_FLAG_MAX > 0 */

#if THINKOS_EVENT_MAX > 0
	for (i = 0; i < THINKOS_EVENT_MAX ; i++) {
		thinkos_rt.ev[i].pend = 0;
		thinkos_rt.ev[i].mask = 0xffffffff;
	}
#if THINKOS_ENABLE_EVENT_ALLOC
	/* initialize the event set allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.ev_alloc, THINKOS_EVENT_MAX); 
#endif
#endif /* THINKOS_EVENT_MAX > 0 */

#if THINKOS_GATE_MAX > 0
	for (i = 0; i < ((THINKOS_GATE_MAX + 15) / 16); i++) 
		thinkos_rt.gate[i] = 0;
#if THINKOS_ENABLE_GATE_ALLOC
	/* initialize the gate allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.gate_alloc, THINKOS_GATE_MAX); 
#endif
#endif /* THINKOS_GATE_MAX > 0 */

#if (THINKOS_ENABLE_DEBUG_BKPT)
	thinkos_rt.step_id = -1;
#if THINKOS_ENABLE_DEBUG_STEP
	thinkos_rt.step_svc = 0;  /* step at service call bitmap */
	thinkos_rt.step_req = 0;  /* step request bitmap */
#endif
#endif

#if (THINKOS_ENABLE_CRITICAL)
	thinkos_rt.critical_cnt = 0;
#endif

#if THINKOS_IRQ_MAX > 0
	__thinkos_irq_reset_all();
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

#if THINKOS_ENABLE_MONITOR
	DCC_LOG(LOG_TRACE, "3. reset debug monitor...");
	thinkos_dbgmon_reset();
#endif

	/* Enable Interrupts */
	DCC_LOG(LOG_TRACE, "4. enablig interrupts...");
	cm3_cpsie_i();
}


bool __thinkos_mem_usr_rw_chk(uint32_t addr, uint32_t size)
{
	uint32_t sram_base = 0x20000000;
	uint32_t krn_base = sram_base + thinkos_mpu_kernel_mem.offs;
	uint32_t krn_size = thinkos_mpu_kernel_mem.size;

	DCC_LOG4(LOG_TRACE, "krn=%08x(%d) mem=%08x(%d)",
			 krn_base, krn_size, addr, size);
	
	/* FIXME: this is a minimum implementation just to avoid
	   invalid accesses to the kernel memory */

	if ((addr < (krn_base + krn_size)) && ((addr + size) > krn_base)) {
		return false;
	}

	return true;
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
	return (CM3_SCB->shcsr & (SCB_SHCSR_SYSTICKACT | SCB_SHCSR_PENDSVACT | 
							  SCB_SHCSR_SVCALLACT)) ? true : false;
}
