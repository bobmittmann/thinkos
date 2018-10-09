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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#include <thinkos.h>
#include <stdio.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

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
#if THINKOS_ENABLE_IRQ_CYCCNT
	/* set the thread's return value to cyle count */
	uint32_t cyccnt = CM3_DWT->cyccnt;
	uint32_t * usr_ptr;
#endif

	/* disable this interrupt source */
	cm3_irq_disable(irq);

	thread_id = thinkos_rt.irq_th[irq];
	thinkos_rt.irq_th[irq] = THINKOS_THREAD_IDLE;

#if DEBUG
	if (thread_id >= THINKOS_THREAD_IDLE) {
		DCC_LOG2(LOG_ERROR, "<%d> IRQ %d invalid thread!", thread_id + 1, irq);
		return;
	} else {
		DCC_LOG2(LOG_TRACE, "<%d> IRQ %d..", thread_id + 1, irq);
	}
#endif
	/* insert the thread into ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread_id, 1);

#if THINKOS_ENABLE_WQ_IRQ
	/* remove from the wait queue */
	__thinkos_wq_remove(THINKOS_WQ_IRQ, thread_id);  
#endif



#if THINKOS_ENABLE_IRQ_CYCCNT
	/* set cyle count in the storage provided by the thread */
	usr_ptr = (uint32_t *)(thinkos_rt.irq_cyccnt[thread_id]);
	*usr_ptr = cyccnt;
#endif

	/* signal the scheduler ... */
	__thinkos_preempt();
}


#if THINKOS_ENABLE_IRQ_TIMEDWAIT 
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

	/* update the thread status if interrupt is received before timeout */
	if (thinkos_rt.irq_th[irq] != THINKOS_THREAD_IDLE) {
		/* assign idle thread to the interrupt */
		thinkos_rt.irq_th[irq] = THINKOS_THREAD_IDLE;
		/* disable this interrupt source */
		cm3_irq_disable(irq);
		arg[0] = THINKOS_ETIMEDOUT;      /* return value */
	} else {
		arg[0] = THINKOS_OK;             /* return value */
	}
}
void thinkos_irq_timedwait_svc(int32_t * arg, int self)
{
	unsigned int irq = arg[0];
	uint32_t ms = (uint32_t)arg[1];
#if THINKOS_ENABLE_IRQ_CYCCNT
	uint32_t * cyccnt_ptr;
#endif

#if THINKOS_ENABLE_ARG_CHECK
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG2(LOG_ERROR, "invalid IRQ %d! irq_th=%d", irq, thinkos_rt.irq_th[53]);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_IRQ_CYCCNT
	/* The cycle count is returned on the location pointed by irq_cyccnt. Make
	   sure it is a valid reference. */
	cyccnt_ptr = (uint32_t *)&thinkos_rt.irq_cyccnt[self];
	thinkos_rt.irq_cyccnt[self] = cyccnt_ptr;
#endif

	/* remove from ready Q */
	__thinkos_suspend(self);

#if THINKOS_ENABLE_WQ_IRQ
	__thinkos_tmdwq_insert(THINKOS_WQ_IRQ, self, ms);
#endif 

	/* assign this thread to the interrupt */
	thinkos_rt.irq_th[irq] = self;

	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	/* enable this interrupt source */
	cm3_irq_enable(irq);
}
#endif

void thinkos_irq_wait_svc(int32_t * arg, int self)
{
	unsigned int irq = arg[0];
#if THINKOS_ENABLE_IRQ_CYCCNT
	uint32_t * cyccnt_ptr;
#endif

#if THINKOS_ENABLE_ARG_CHECK
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(THINKOS_ERR_IRQ_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

#if THINKOS_ENABLE_IRQ_CYCCNT
	/* The cycle counter is returned in the memorty pointed by
	   R1. Chek if it's a valid memory area. */
	if (NULL == (void *)arg[1]) {
		/* If the user pointer is null then ignore the cycle count, but
		   make sure irq_cyccnt is a valid reference. */
		cyccnt_ptr = (uint32_t *)&thinkos_rt.irq_cyccnt[self];
	} 
#if THINKOS_ENABLE_MPU
	/* There is a potential security brech as the pointer is written by 
	   an priviledged interrupt handler. For this reason the whole kernel 
	   memory space is checked.  */
	else if ((arg[1] >= thinkos_rt.mpu_kernel_mem.offs) && 
		(arg[1] <= thinkos_rt.mpu_kernel_mem.offs +
		 thinkos_rt.mpu_kernel_mem.size)) {
		DCC_LOG(LOG_ERROR, "invalid pointer!");
		__THINKOS_ERROR(THINKOS_ERR_INVALID_POINTER);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif /* THINKOS_ENABLE_MPU */
	else {
		cyccnt_ptr = (uint32_t *)arg[1];
	}

	thinkos_rt.irq_cyccnt[self] = cyccnt_ptr;
#endif /* THINKOS_ENABLE_IRQ_CYCCNT */

#endif /* THINKOS_ENABLE_ARG_CHECK */

	DCC_LOG2(LOG_MSG, "<%d> IRQ %d!", self, irq);
	arg[0] = THINKOS_OK;

#if THINKOS_ENABLE_IRQ_CYCCNT
	/* Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the cycle counter (r1) address. */
	thinkos_rt.ctx[self] = (struct thinkos_context *)&arg[-CTX_R0];
#endif

	/* remove from ready Q */
	__thinkos_suspend(self);

#if THINKOS_ENABLE_WQ_IRQ
	__thinkos_wq_insert(THINKOS_WQ_IRQ, self);  
#endif

	/* assign this thread to the interrupt */
	thinkos_rt.irq_th[irq] = self;

	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

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
/* This macro is here for backword compatibility, TODO should be deprecated */
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

	default:
		DCC_LOG1(LOG_ERROR, "invalid IRQ ctl request %d!", req);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

const char thinkos_irq_nm[] = "IRQ";


