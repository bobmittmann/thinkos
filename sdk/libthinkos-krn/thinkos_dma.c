/* 
 * thinkos_dma.c
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
#define __THINKOS_DMA__
#include <thinkos/dma.h>
#include <thinkos.h>
#include <stdio.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if THINKOS_DMA_MAX > 0
void __thinkos_dma_reset_all(void)
{
	int dma;
	/* adjust DMA priorities to regular (above SysTick and bellow SVC) */
	for (dma = 0; dma < THINKOS_DMA_MAX; dma++) {
		cm3_dma_pri_set(dma, DMA_DEF_PRIORITY);
		thinkos_rt.dma_th[dma] = THINKOS_THREAD_IDLE;
	}
}
#endif

#if THINKOS_DMA_MAX > 0

void thinkos_dma_irq(unsigned int irq) 
{
	unsigned int thread_id;
#if THINKOS_ENABLE_DMA_CYCCNT
	/* set the thread's return value to cyle count */
	uint32_t cyccnt = CM3_DWT->cyccnt;
	uint32_t * usr_ptr;
#endif

	/* disable this interrupt source */
	cm3_irq_disable(irq);

	thread_id = thinkos_rt.dma_th[dma];
	thinkos_rt.dma_th[dma] = THINKOS_THREAD_IDLE;

#if DEBUG
	if (thread_id >= THINKOS_THREAD_IDLE) {
		DCC_LOG2(LOG_ERROR, "<%d> DMA %d invalid thread!", thread_id + 1, dma);
		return;
	} else {
		DCC_LOG2(LOG_TRACE, "<%d> DMA %d..", thread_id + 1, dma);
	}
#endif
	/* insert the thread into ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread_id, 1);

#if THINKOS_ENABLE_WQ_DMA
	/* remove from the wait queue */
	__thinkos_wq_remove(THINKOS_WQ_DMA, thread_id);  
#endif


#if THINKOS_ENABLE_DMA_CYCCNT
	/* set cyle count in the storage provided by the thread */
	usr_ptr = (uint32_t *)(thinkos_rt.dma_cyccnt[thread_id]);
	*usr_ptr = cyccnt;
#endif

	/* signal the scheduler ... */
	__thinkos_preempt();
}


#if THINKOS_ENABLE_DMA_TIMEDWAIT 
void thinkos_dma_timedwait_cleanup_svc(int32_t * arg, int self) {
	unsigned int dma = arg[0];

#if THINKOS_ENABLE_ARG_CHECK
	if (dma >= THINKOS_DMA_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid DMA %d!", dma);
		__THINKOS_ERROR(THINKOS_ERR_DMA_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* update the thread status if interrupt is received before timeout */
	if (thinkos_rt.dma_th[dma] != THINKOS_THREAD_IDLE) {
		/* assign idle thread to the interrupt */
		thinkos_rt.dma_th[dma] = THINKOS_THREAD_IDLE;
		/* disable this interrupt source */
		cm3_dma_disable(dma);
		arg[0] = THINKOS_ETIMEDOUT;      /* return value */
	} else {
		arg[0] = THINKOS_OK;             /* return value */
	}
}

void thinkos_dma_timedwait_svc(int32_t * arg, int self)
{
	unsigned int dma = arg[0];
	uint32_t ms = (uint32_t)arg[1];
#if THINKOS_ENABLE_DMA_CYCCNT
	uint32_t * cyccnt_ptr;
#endif

#if THINKOS_ENABLE_ARG_CHECK
	if (dma >= THINKOS_DMA_MAX) {
		DCC_LOG2(LOG_ERROR, "invalid DMA %d! dma_th=%d", dma, thinkos_rt.dma_th[53]);
		__THINKOS_ERROR(THINKOS_ERR_DMA_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_DMA_CYCCNT
	/* The cycle count is returned on the location pointed by dma_cyccnt. Make
	   sure it is a valid reference. */
	cyccnt_ptr = (uint32_t *)&thinkos_rt.dma_cyccnt[self];
	thinkos_rt.dma_cyccnt[self] = cyccnt_ptr;
#endif

	/* remove from ready Q */
	__thinkos_suspend(self);

#if THINKOS_ENABLE_WQ_DMA
	__thinkos_tmdwq_insert(THINKOS_WQ_DMA, self, ms);
#endif 

	/* assign this thread to the interrupt */
	thinkos_rt.dma_th[dma] = self;

	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* clear pending interrupt */
	cm3_dma_pend_clr(dma);

	/* enable this interrupt source */
	cm3_dma_enable(dma);
}
#endif

void thinkos_dma_wait_svc(int32_t * arg, int self)
{
	unsigned int dma = arg[0];
#if THINKOS_ENABLE_DMA_CYCCNT
	uint32_t * cyccnt_ptr;
#endif

#if THINKOS_ENABLE_ARG_CHECK
	if (dma >= THINKOS_DMA_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid DMA %d!", dma);
		__THINKOS_ERROR(THINKOS_ERR_DMA_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

#if THINKOS_ENABLE_DMA_CYCCNT
	/* The cycle counter is returned in the memorty pointed by
	   R1. Chek if it's a valid memory area. */
	if (NULL == (void *)arg[1]) {
		/* If the user pointer is null then ignore the cycle count, but
		   make sure dma_cyccnt is a valid reference. */
		cyccnt_ptr = (uint32_t *)&thinkos_rt.dma_cyccnt[self];
	} 
#if THINKOS_ENABLE_MPU
	/* There is a potential security brech as the pointer is written by 
	   an priviledged interrupt handler. For this reason the whole kernel 
	   memory space is checked.  */
	else if ((arg[1] >= thinkos_mpu_kernel_mem.offs) && 
		(arg[1] <= thinkos_mpu_kernel_mem.offs +
		 thinkos_mpu_kernel_mem.size)) {
		DCC_LOG(LOG_ERROR, "invalid pointer!");
		__THINKOS_ERROR(THINKOS_ERR_INVALID_POINTER);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif /* THINKOS_ENABLE_MPU */
	else {
		cyccnt_ptr = (uint32_t *)arg[1];
	}

	thinkos_rt.dma_cyccnt[self] = cyccnt_ptr;
#endif /* THINKOS_ENABLE_DMA_CYCCNT */

#endif /* THINKOS_ENABLE_ARG_CHECK */

	DCC_LOG2(LOG_MSG, "<%d> DMA %d!", self, dma);
	arg[0] = THINKOS_OK;

#if THINKOS_ENABLE_DMA_CYCCNT
	/* Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the cycle counter (r1) address. */
	__thinkos_thread_ctx_set(self, (struct thinkos_context *)&arg[-CTX_R0],
							 CONTROL_SPSEL | CONTROL_nPRIV);
#endif

	/* remove from ready Q */
	__thinkos_suspend(self);

#if THINKOS_ENABLE_WQ_DMA
	__thinkos_wq_insert(THINKOS_WQ_DMA, self);  
#endif

	/* assign this thread to the interrupt */
	thinkos_rt.dma_th[dma] = self;

	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* clear pending interrupt */
	cm3_dma_pend_clr(dma);

	/* enable this interrupt source */
	cm3_dma_enable(dma);
}

void thinkos_dma_ctl_svc(int32_t * arg, int self)
{
	unsigned int req = arg[0];
	unsigned int dma = arg[1];

#if THINKOS_ENABLE_ARG_CHECK
	unsigned int dma_max = ((uintptr_t)&__sizeof_rom_vectors / 
							sizeof(void *)) - 16;

	if (dma >= dma_max) {
		DCC_LOG1(LOG_ERROR, "invalid DMA %d!", dma);
		__THINKOS_ERROR(THINKOS_ERR_DMA_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
	arg[0] = 0;
	
	switch (req) {
/* This macro is here for backword compatibility, TODO should be deprecated */
#ifdef THINKOS_ENABLE_DMA_CTL
	case THINKOS_DMA_ENABLE:
		DCC_LOG1(LOG_TRACE, "enabling DMA %d", dma);
		/* clear pending interrupt */
		cm3_dma_enable(dma);
		break;

	case THINKOS_DMA_DISABLE:
		cm3_dma_disable(dma);
		break;

	case THINKOS_DMA_PRIORITY_SET:
		{
			int priority = arg[2];

			if (priority > DMA_PRIORITY_VERY_LOW)
				priority = DMA_PRIORITY_VERY_LOW;
#if !THINKOS_ENABLE_DMA_PRIORITY_0
			else if (priority < DMA_PRIORITY_VERY_HIGH)
				priority = DMA_PRIORITY_VERY_HIGH;
#endif
			/* set the interrupt priority */
			cm3_dma_pri_set(dma, priority);
		}
		break;

#endif

#ifdef CM3_RAM_VECTORS
	case THINKOS_DMA_REGISTER:
		{
			unsigned int priority = arg[2];
			void * isr = (void *)arg[3];

			/* disable this interrupt source */
			cm3_dma_disable(dma);

			if (priority > DMA_PRIORITY_VERY_LOW)
				priority = DMA_PRIORITY_VERY_LOW;
#if !THINKOS_ENABLE_DMA_PRIORITY_0
			else if (priority < DMA_PRIORITY_VERY_HIGH)
				priority = DMA_PRIORITY_VERY_HIGH;
#endif

			/* set the interrupt priority */
			cm3_dma_pri_set(dma, priority);

			/* clear pending interrupt */
			cm3_dma_pend_clr(dma);

			/* set the vector */
			__ram_vectors[dma + 16] = isr;

			/* enable this interrupt source */
			cm3_dma_enable(dma);
		}
		break;
#endif

	default:
		DCC_LOG1(LOG_ERROR, "invalid DMA ctl request %d!", req);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

const char thinkos_dma_nm[] = "DMA";


#endif

