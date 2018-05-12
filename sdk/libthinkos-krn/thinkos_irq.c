/* 
 * thikos_irq.c
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

#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#include <thinkos.h>

#include <stdio.h>

#if THINKOS_IRQ_MAX > 0
void __thinkos_irq_reset_all(void)
{
	int irq;
	/* adjust IRQ priorities to regular (above SysTick and bellow SVC) */
	for (irq = 0; irq < THINKOS_IRQ_MAX; irq++) {
		cm3_irq_pri_set(irq, IRQ_DEF_PRIORITY);
		thinkos_rt.irq_th[irq] = THINKOS_THREAD_IDLE;
	}
}
#endif

#if THINKOS_IRQ_MAX > 0

void cm3_default_isr(unsigned int irq) 
{
	unsigned int thread_id;
#if THINKOS_ENABLE_IRQ_CYCCNT_RET
	/* set the thread's return value to cyle count */
	uint32_t cyccnt = CM3_DWT->cyccnt;
#endif

	/* disable this interrupt source */
	cm3_irq_disable(irq);

	thread_id = thinkos_rt.irq_th[irq];
	thinkos_rt.irq_th[irq] = THINKOS_THREAD_IDLE;
#if DEBUG
	/* TODO: create a wait queue for IRQ waiting. */
	if (thread_id >= THINKOS_THREAD_IDLE) {
		DCC_LOG2(LOG_ERROR, "<%d> IRQ %d invalid thread!", thread_id + 1, irq);
		return;
	}
#endif

	/* insert the thread into ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread_id, 1);  

/* For possible speed up, for vector on RAM case,
 * this function should be split into two variant, 2nd one with cycle count */
#if THINKOS_ENABLE_IRQ_CYCCNT_RET
	DCC_LOG3(LOG_MSG, "<%d> IRQ %d, cyccnt:%d", thread_id + 1, irq, cyccnt);
	/* set the thread's return value to cyle count */
	if(0 == thinkos_rt.irq_cyccnt[thread_id])
		thinkos_rt.irq_cyccnt[thread_id] = cyccnt;
#endif

	/* signal the scheduler ... */
	__thinkos_preempt();
}

#if THINKOS_IRQ_MAX > 0 && THINKOS_ENABLE_TIMED_CALLS
void thinkos_irq_timedwait_cleanup_svc(int32_t * arg, int self) {
	unsigned int irq = arg[0];

#if THINKOS_ENABLE_ARG_CHECK
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, self, 0);
	/* update status */
	thinkos_rt.th_stat[self] = 0;
	/* enable this interrupt source */
	cm3_irq_disable(irq);

	/* update the thread status if interrupt is received before timeout */
	if (thinkos_rt.irq_th[irq] != THINKOS_THREAD_IDLE) {
		/* assign idle thread to the interrupt */
		thinkos_rt.irq_th[irq] = THINKOS_THREAD_IDLE;
		arg[0] = THINKOS_ETIMEDOUT;      /* return value */
	} else {
		arg[0] = THINKOS_OK;             /* return value */
	}
}

void thinkos_irq_timedwait_svc(int32_t * arg, int self)
{
	unsigned int irq = arg[0];
	uint32_t ms = (uint32_t)arg[1];

#if THINKOS_ENABLE_ARG_CHECK
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG2(LOG_ERROR, "invalid IRQ %d! irq_th=%d", irq, thinkos_rt.irq_th[53]);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	/* remove from ready Q */
	__thinkos_suspend(self);

	__thinkos_tmdwq_insert(THINKOS_WQ_IRQ, self, ms);

	/* assign this thread to the interrupt */
	thinkos_rt.irq_th[irq] = self;

	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* enable this interrupt source */
	cm3_irq_enable(irq);
}

#endif

void thinkos_irq_wait_svc(int32_t * arg, int self)
{
	unsigned int irq = arg[0];

#if THINKOS_ENABLE_ARG_CHECK
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
	arg[0] = THINKOS_OK;

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	/* remove from ready Q */
	__thinkos_suspend(self);

	__thinkos_wq_insert(THINKOS_WQ_IRQ, self);  

	/* assign this thread to the interrupt */
	thinkos_rt.irq_th[irq] = self;

	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* enable this interrupt source */
	cm3_irq_enable(irq);
}

#endif

extern int __sizeof_rom_vectors;

#ifdef CM3_RAM_VECTORS
extern void * __ram_vectors[];
#endif

void thinkos_irq_ctl_svc(int32_t * arg, int self)
{
	unsigned int req = arg[0];
	unsigned int irq = arg[1];

#if THINKOS_ENABLE_ARG_CHECK
	int irq_max = ((uintptr_t)&__sizeof_rom_vectors / sizeof(void *)) - 16;

	if (irq >= irq_max) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
	arg[0] = 0;
	
	switch (req) {
/* This macro is here for backword compatibility, TODO should be depricated */
#ifdef THINKOS_ENABLE_IRQ_CTL
	case THINKOS_IRQ_ENABLE:
		DCC_LOG1(LOG_TRACE, "enabling IRQ %d", irq);
		/* clear pending interrupt */
		cm3_irq_enable(irq);
		break;

	case THINKOS_IRQ_DISABLE:
		cm3_irq_disable(irq);
		break;

	case THINKOS_IRQ_PRIORITY_SET:
		{
			int priority = arg[2];

			if (priority > IRQ_PRIORITY_VERY_LOW)
				priority = IRQ_PRIORITY_VERY_LOW;
#if !THINKOS_ENABLE_IRQ_PRIORITY_0
			else if (priority < IRQ_PRIORITY_VERY_HIGH)
				priority = IRQ_PRIORITY_VERY_HIGH;
#endif
			/* set the interrupt priority */
			cm3_irq_pri_set(irq, priority);
		}
		break;

#endif

#ifdef CM3_RAM_VECTORS
	case THINKOS_IRQ_REGISTER:
		{
			unsigned int priority = arg[2];
			void * isr = (void *)arg[3];

			/* disable this interrupt source */
			cm3_irq_disable(irq);

			if (priority > IRQ_PRIORITY_VERY_LOW)
				priority = IRQ_PRIORITY_VERY_LOW;
#if !THINKOS_ENABLE_IRQ_PRIORITY_0
			else if (priority < IRQ_PRIORITY_VERY_HIGH)
				priority = IRQ_PRIORITY_VERY_HIGH;
#endif

			/* set the interrupt priority */
			cm3_irq_pri_set(irq, priority);

			/* clear pending interrupt */
			cm3_irq_pend_clr(irq);

			/* set the vector */
			__ram_vectors[irq + 16] = isr;

			/* enable this interrupt source */
			cm3_irq_enable(irq);
		}
		break;
#endif

#if THINKOS_ENABLE_IRQ_CYCCNT_RET
	case THINKOS_IRQ_CYCCNT_GET:
		{
			arg[0] = thinkos_rt.irq_cyccnt[self];
			thinkos_rt.irq_cyccnt[1] = 0;
			DCC_LOG3(LOG_MSG, "get cyccnt:irq=%d, thread=%d, cyccnt=%d!",
					irq, self + 1, arg[0]);
		}
		break;
#endif

	default:
		DCC_LOG1(LOG_ERROR, "invalid IRQ ctl request %d!", req);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

const char thinkos_irq_nm[] = "IRQ";


