/* 
 * thikos_core.c
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

void thinkos_idle_task(void);

/* -------------------------------------------------------------------------- 
 * Run Time ThinkOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt thinkos_rt;

uint32_t __attribute__((aligned(8))) thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

const uint16_t thinkos_except_stack_size = sizeof(thinkos_except_stack);


void __thinkos_idle_error(void * arg)
{
	DCC_LOG(LOG_ERROR, "ThinkOS Idle error!!"); 
}

#if (THINKOS_ENABLE_SANITY_CHECK)
uint32_t thinkos_sched_error(struct thinkos_context * ctx, uint32_t thread_id)
{
	DCC_LOG(LOG_ERROR, "/!\\ scheduler error!");
	__context(ctx, thread_id); 
	__thinkos(&thinkos_rt);

	if (thread_id == THINKOS_THREAD_IDLE) {
		udelay(500000);
		udelay(500000);
 		return __thinkos_idle_reset(__thinkos_idle_error, NULL);
	}

	for(;;);
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
	return (CM3_SCB->shcsr & (SCB_SHCSR_SYSTICKACT | SCB_SHCSR_PENDSVACT | 
							  SCB_SHCSR_SVCALLACT)) ? true : false;
}

void __thinkos_core_reset(void)
{
	struct cm3_systick * systick = CM3_SYSTICK;

	/* adjust exception priorities */
	/*
	 *  0x00 - low latency interrupts
     *
	 *  0x20 - high priority interrupts
	 *  0x40   .
	 *
	 *  0x60 - SVC
	 *  0x80 - regular priority interrupts
	 * 
	 *  0xa0 - SysTick
	 *  0xc0 - PendSV (scheduler)
	 *
	 *  0xe0 - very low priority interrupts
	 */

	/* SVC should not be preempted by the scheduler, thus it runs 
	   at higher priority. In order for the regular priority
	   interrupts to call SVC, they should run at a lower priority
	   then SVC.*/
	cm3_except_pri_set(CM3_EXCEPT_SVC, SYSCALL_PRIORITY);
	/* SysTick interrupt has to have a lower priority then SVC,
	 to not preempt SVC */
	cm3_except_pri_set(CM3_EXCEPT_SYSTICK, CLOCK_PRIORITY);
	/* PendSV interrupt has to have the lowest priority among
	   regular interrupts (higher number) */
	cm3_except_pri_set(CM3_EXCEPT_PENDSV, SCHED_PRIORITY);

#if	THINKOS_ENABLE_USAGEFAULT 
	cm3_except_pri_set(CM3_EXCEPT_USAGE_FAULT, EXCEPT_PRIORITY);
#endif
#if	THINKOS_ENABLE_BUSFAULT 
	cm3_except_pri_set(CM3_EXCEPT_BUS_FAULT, EXCEPT_PRIORITY);
#endif
#if THINKOS_ENABLE_MPU
	cm3_except_pri_set(CM3_EXCEPT_MEM_MANAGE, EXCEPT_PRIORITY);
#endif
#if THINKOS_ENABLE_MONITOR 
	cm3_except_pri_set(CM3_EXCEPT_DEBUG_MONITOR, MONITOR_PRIORITY);
#endif

	/* clear the ThinkOS runtime structure */
	__thinkos_memset32(&thinkos_rt, 0, sizeof(struct thinkos_rt));  

#if THINKOS_IRQ_MAX > 0
	__thinkos_irq_reset_all();
#endif

#if (THINKOS_MUTEX_MAX > 0)
	{
		int i;
		/* initialize the mutex locks */
		for (i = 0; i < THINKOS_MUTEX_MAX; i++) 
			thinkos_rt.lock[i] = -1;
	}
#endif

#if THINKOS_EVENT_MAX > 0
	{
		int i;
		/* initialize the event set mask */
		for (i = 0; i < THINKOS_EVENT_MAX; i++) 
			thinkos_rt.ev[i].mask = 0xffffffff;
	}
#endif

#if THINKOS_ENABLE_THREAD_ALLOC
	/* initialize the thread allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.th_alloc, THINKOS_THREADS_MAX); 
#endif

#if THINKOS_ENABLE_MUTEX_ALLOC
	/* initialize the mutex allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.mutex_alloc, THINKOS_MUTEX_MAX); 
#endif

#if THINKOS_ENABLE_SEM_ALLOC
	/* initialize the semaphore allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.sem_alloc, THINKOS_SEMAPHORE_MAX); 
#endif

#if THINKOS_ENABLE_COND_ALLOC
	/* initialize the conditional variable allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.cond_alloc, THINKOS_COND_MAX); 
#endif

#if THINKOS_ENABLE_FLAG_ALLOC
	/* initialize the flag allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.flag_alloc, THINKOS_FLAG_MAX); 
#endif

#if THINKOS_ENABLE_EVENT_ALLOC
	/* initialize the event set allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.ev_alloc, THINKOS_EVENT_MAX); 
#endif

#if THINKOS_ENABLE_GATE_ALLOC
	/* initialize the gate allocation bitmap */ 
	__thinkos_bmp_init(thinkos_rt.gate_alloc, THINKOS_GATE_MAX); 
#endif

#if THINKOS_ENABLE_DEBUG_STEP
	thinkos_rt.step_id = -1;
	thinkos_rt.break_id = -1;
#endif

#if THINKOS_ENABLE_PROFILING
	/* Enable trace */
	CM3_DCB->demcr |= DCB_DEMCR_TRCENA;
	/* Enable cycle counter */
	CM3_DWT->ctrl |= DWT_CTRL_CYCCNTENA;

	/* set the reference to now */
	thinkos_rt.cycref = CM3_DWT->cyccnt;
#endif

	/* initialize the SysTick module */
	systick->rvr = cm3_systick_load_1ms; /* 1ms tick period */
	systick->cvr = 0;
#if THINKOS_ENABLE_CLOCK || THINKOS_ENABLE_TIMESHARE
	systick->csr = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT;
#else
	systick->csr = SYSTICK_CSR_ENABLE;
#endif

	/* Set the initial thread as idle. */
	thinkos_rt.active = THINKOS_THREAD_IDLE;
}


void __thinkos_system_reset(void)
{
	DCC_LOG(LOG_WARNING, "/!\\ System reset inprogress...");
	/* Return to the Monitor applet with the SOFTRST signal set.
	   The applet should clear the hardware and restore the core interrups.
	   Next context_swap() will continue in __do_soft_reset() ... */

	DCC_LOG(LOG_TRACE, "1. ThinkOS core reset...");
	__thinkos_core_reset();

#if THINKOS_ENABLE_EXCEPTIONS
	DCC_LOG(LOG_TRACE, "2. exception reset...");
	__exception_reset();
#endif

#if THINKOS_ENABLE_DEBUG_BKPT
	DCC_LOG(LOG_TRACE, "3. clear all breakpoints...");
	dmon_breakpoint_clear_all();
#endif

#if THINKOS_DBGMON_ENABLE_RST_VEC
	DCC_LOG(LOG_TRACE, "4. reset RAM vectors...");
	__reset_ram_vectors();
#endif

	/* Enable Interrupts */
	DCC_LOG(LOG_TRACE, "5. enablig interrupts...");
	cm3_cpsid_i();
}

