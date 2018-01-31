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
	DCC_LOG2(LOG_MSG, "<%d> IRQ %d", thread_id + 1, irq);
	/* TODO: create a wait queue for IRQ waiting. */
	if (thread_id >= THINKOS_THREAD_IDLE) {
		DCC_LOG2(LOG_ERROR, "<%d> IRQ %d invalid thread!", thread_id + 1, irq);
		return;
	}
#endif

	/* insert the thread into ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread_id, 1);  

#if THINKOS_ENABLE_IRQ_CYCCNT_RET
	/* set the thread's return value to cyle count */
	thinkos_rt.ctx[thread_id]->r0 = cyccnt;
#endif

	/* signal the scheduler ... */
	__thinkos_preempt();
}

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

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	/* wait for event */
	__thinkos_suspend(self);

#if THINKOS_ENABLE_IRQ_RESTORE
	/* Get the currently assigned thread */
	arg[0] = thinkos_rt.irq_th[irq] + 1;
#endif 

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

void thinkos_irq_register_svc(int32_t * arg)
{
	unsigned int irq = arg[0];
	unsigned int priority = arg[1];
	void * isr = (void *)arg[2];

#if THINKOS_ENABLE_ARG_CHECK
	int irq_max = ((uintptr_t)&__sizeof_rom_vectors / sizeof(void *)) - 16;

	if (irq >= irq_max) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

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

#endif

#ifdef THINKOS_ENABLE_IRQ_CTL
void thinkos_irq_ctl_svc(int32_t * arg)
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
	case THINKOS_IRQ_ENABLE:
		DCC_LOG1(LOG_TRACE, "enabling IRQ %d", irq);
		/* clear pending interrupt */
		cm3_irq_enable(irq);
		break;

	case THINKOS_IRQ_DISABLE:
		cm3_irq_disable(irq);
		break;

#if THINKOS_ENABLE_IRQ_RESTORE
	case THINKOS_IRQ_RESTORE:
		{
			unsigned int thread_id = arg[2] - 1;

#if THINKOS_ENABLE_ARG_CHECK
			if (thread_id >= THINKOS_THREADS_MAX) {
				DCC_LOG1(LOG_INFO, "invalid thread %d!", thread_id);
				__THINKOS_ERROR(THINKOS_ERR_THREAD_INVALID);
				arg[0] = THINKOS_EINVAL;
				return;
			}
#if THINKOS_ENABLE_THREAD_ALLOC
			if (__bit_mem_rd(thinkos_rt.th_alloc, thread_id) == 0) {
				DCC_LOG1(LOG_INFO, "invalid thread %d!", thread_id);
				__THINKOS_ERROR(THINKOS_ERR_THREAD_ALLOC);
				arg[0] = THINKOS_EINVAL;
				return;
			}
#endif
#endif

#if THINKOS_ENABLE_SANITY_CHECK
			if (thinkos_rt.ctx[thread_id] == NULL) {
				DCC_LOG1(LOG_INFO, "invalid thread %d!", thread_id);
				arg[0] = THINKOS_EINVAL;
				return;
			}
#endif
			/* Get the currently assigned thread */
			arg[0] = thread_id;
			cm3_irq_enable(irq);
		}
		break;
#endif 

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

//	case THINKOS_IRQ_SVC_SET:
//		break;

	default:
		DCC_LOG1(LOG_ERROR, "invalid IRQ ctl request %d!", req);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

const char thinkos_irq_nm[] = "IRQ";

#endif

