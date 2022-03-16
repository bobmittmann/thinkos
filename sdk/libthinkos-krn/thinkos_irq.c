/* 
 * thinkos_irq.c
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

#ifndef IRQ_DEBUG
#define IRQ_DEBUG 0
#endif

#ifndef THINKOS_ENABLE_ERR_IRQ_TAKEN
#define THINKOS_ENABLE_ERR_IRQ_TAKEN 0
#endif

#ifndef THINKOS_ENABLE_IRQ_SANITY_CHECK
#define THINKOS_ENABLE_IRQ_SANITY_CHECK 0
#endif


void thinkos_krn_irq_on(void) 
{
	__krn_irq_on();
}

/* disable interrupts */
static inline void __attribute__((always_inline)) __krn_irq_off(void)  {
	asm volatile ("cpsid i\n");
}

void thinkos_krn_irq_off(void) 
{
	DCC_LOG(LOG_TRACE, "disabling interrupts...");

	__krn_irq_off();
}


#if (THINKOS_IRQ_MAX) > 0
void __krn_irq_reset_all(struct thinkos_rt* krn)
{
	int irq;

	/* adjust IRQ priorities to regular (above SysTick and bellow SVC) */
	for (irq = 0; irq < THINKOS_IRQ_MAX; irq++) {
		cm3_irq_pri_set(irq, IRQ_DEF_PRIORITY);
		krn->irq_th[irq] = THINKOS_THREAD_VOID;
	}

}

#if (THINKOS_IRQ_MAX) > 0
/* Cancel an interrupt ... */
void __krn_irq_thread_del(struct thinkos_rt* krn, unsigned int th)
{
	int irq;

	for (irq = 0; irq < THINKOS_IRQ_MAX; irq++) {
		if (th == krn->irq_th[irq]) {
			/* disable this interrupt source */
			cm3_irq_disable(irq);
			/* remove */
			krn->irq_th[irq] = THINKOS_THREAD_VOID;
			DCC_LOG2(LOG_TRACE, "thread=%d irq=%d", th, irq);
			return;
		}
	}
}
#endif 


#define NVIC_IRQ_REGS ((THINKOS_IRQ_MAX + 31) / 32)

#ifndef NVIC_ENABLE_CLR_PEND
#define NVIC_ENABLE_CLR_PEND 0
#endif

/**
  * __nvic_irq_disable_all:
  *
  * Disable all interrupts by clearing the interrupt enable bit
  * of all interrupts on the Nested Vector Interrupt Controller (NVIC).
  *
  * Also the interrupt enable backup is cleared to avoid 
  * interrupts being reenabled by calling __monitor_irq_restore_all().
  *
  * The systick interrupt is not disabled.
  */
void __nvic_irq_disable_all(void)
{
	int i;

	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		/* FIXME: clearing the pending interrupt may have a side effect 
		   on the comms irq used by the debug monitor. An alternative 
		   would be to use the force enable list to avoid clearing those
		   in the list. */
#if (NVIC_ENABLE_CLR_PEND)
		CM3_NVIC->icpr[i] = 0xffffffff; /* clear pending interrupts */
#endif
		CM3_NVIC->icer[i] = 0xffffffff; /* disable interrupts */
	}

}

void __nvic_irq_clrpend_all(void)
{
	int i;

	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		CM3_NVIC->icpr[i] = 0xffffffff; /* clear pending interrupts */
	}

}
#endif

void __thinkos_krn_irq_init(struct thinkos_rt * krn)
{

#if (THINKOS_IRQ_MAX) > 0
	DCC_LOG(LOG_MSG, "initializing interrupts");
	__krn_irq_reset_all(krn);
#endif

#ifdef CM3_RAM_VECTORS
	{
		uint32_t * tab_ptr = (uint32_t *)&__vcts_start;
		unsigned int tab_size = (uintptr_t)&__vcts_end - 
			(uintptr_t)&__vcts_start;

		DCC_LOG2(LOG_MSG, "copying RAM vectors 0x%08x, %d", 
				 tab_ptr, tab_size);

		__thinkos_memcpy(__ram_vectors, tab_ptr, tab_size);
	}
#endif

#ifdef CM3_RAM_VECTORS
	/* Remap the Vector table to SRAM */
	CM3_SCB->vtor = (uintptr_t)__ram_vectors; /* Vector Table Offset */

	DCC_LOG1(LOG_MSG, "remaping vectors to 0x%08x", CM3_SCB->vtor);

	if (CM3_SCB->vtor != (uintptr_t)__ram_vectors) {
		DCC_LOG1(LOG_PANIC, "SCB->VTOR(0x%08x) != __ram_vectors!", 
				 CM3_SCB->vtor);
	}
#endif
}


#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif


#if (THINKOS_IRQ_MAX) > 0
void __attribute__ ((aligned(16))) cm3_default_isr(unsigned int irq)
{
	struct thinkos_rt * krn = &thinkos_rt;
#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* set the thread's return value to cyle count */
	uint32_t cyccnt = CM3_DWT->cyccnt;
#endif
#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	uint8_t * ptr;
#endif
	unsigned int th;

	/* disable this interrupt source */
	cm3_irq_disable(irq);

#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	ptr = (uint8_t *)&krn->irq_th[irq];
	do {
		th = __ldrexb(ptr);
	} while (__strexb(ptr, THINKOS_THREAD_VOID));

	if (th >= THINKOS_THREAD_IDLE) {
		__krn_sched_err_set(krn, THINKOS_ERR_THREAD_INVALID);	
		return;
	} else if (th == THINKOS_THREAD_VOID) {
		__krn_sched_err_set(krn, THINKOS_ERR_THREAD_INVALID);	
		return;
	}
#else
	th = krn->irq_th[irq];
#endif 

#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* set the thread's cyccnt value */
	__thread_r1_set(krn, th, cyccnt);
#endif

#if (THINKOS_ENABLE_WQ_IRQ)
	__krn_wq_thread_del(krn, THINKOS_WQ_IRQ, th);  
#endif 

	/* insert the thread into ready queue */
	__krn_wq_ready_thread_ins(krn, th);

	/* signal the scheduler ... */
	__krn_preempt(krn);
}

/* 
   This syscall returns the irq number upon success. 
 */

#if (THINKOS_ENABLE_IRQ_TIMEDWAIT)
void thinkos_irq_timedwait_fixup_svc(int32_t * arg, int self, 
									 struct thinkos_rt * krn) 
{
	unsigned int irq = arg[0];

#if (IRQ_DEBUG)
#if (THINKOS_ENABLE_ARG_CHECK)
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(self, THINKOS_ERR_IRQ_INVALID);
		arg[4] = THINKOS_EINVAL;
		return;
	}
#endif
#endif
	/* disable this interrupt source */
	cm3_irq_disable(irq);

	/* if the timer is no longer active declare a timeout */
	if (!__thread_clk_is_enabled(krn, self)) {
		arg[4] = THINKOS_ETIMEDOUT;      
	} else {
		arg[4] = THINKOS_OK;
	}
	
#if (THINKOS_ENABLE_WQ_IRQ)
	/* remove from the wait queue */
	__krn_wq_remove(krn, THINKOS_WQ_IRQ, self);  
#else
	__wq_clock_remove(krn, self);
#endif
#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	/* assign VOID thread to the interrupt */
	krn->irq_th[irq] = THINKOS_THREAD_VOID;
#endif
}

/* 
   This syscall returns the irq number upon success. 
 */
void thinkos_irq_timedwait_svc(int32_t * arg, unsigned int self,
							   struct thinkos_rt * krn) 
{
	unsigned int irq = arg[0];
	uint32_t ms = (uint32_t)arg[1];

#if (THINKOS_ENABLE_ARG_CHECK)
	if (irq >= THINKOS_IRQ_MAX) {
		return;
	}
#endif

#if (THINKOS_ENABLE_WQ_IRQ)
	__krn_tmdwq_insert(krn, THINKOS_WQ_IRQ, self, ms);
#else
	__wq_clock_insert(krn, self, ms);
#endif 

	/* assign this thread to the interrupt */
	krn->irq_th[irq] = self;

	/* remove from ready Q */
	__krn_thread_suspend(krn, self);

	/* signal the scheduler ... */
	__krn_sched_defer(krn);

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	/* enable this interrupt source */
	cm3_irq_enable(irq);
}
#endif

/* 
   This syscall returns the IRQ number upon success. 
 */
void thinkos_irq_wait_svc(int32_t * arg, unsigned int self,
						  struct thinkos_rt * krn)
{
	unsigned int irq = arg[0];
#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	uint8_t * ptr;
	uint32_t old;
#endif

#if (THINKOS_ENABLE_ARG_CHECK)
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(self, THINKOS_ERR_IRQ_INVALID);
		arg[4] = THINKOS_EINVAL;
		return;
	}
#endif /* THINKOS_ENABLE_ARG_CHECK */

	DCC_LOG2(LOG_INFO, "<%2d> IRQ %d!", self, irq);

#if (THINKOS_ENABLE_WQ_IRQ)
	__krn_wq_insert(krn, THINKOS_WQ_IRQ, self);  
#endif

#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	ptr = (uint8_t *)&krn->irq_th[irq];
	do {
		old = __ldrexb(ptr);
	} while (__strexb(ptr, self));

	if (old != THINKOS_THREAD_VOID) {
		DCC_LOG1(LOG_ERROR, "irq IRQ %d is busy!", irq);
		__THINKOS_ERROR(self, THINKOS_ERR_IRQ_TAKEN);
		arg[4] = THINKOS_EFAULT;
		return;
	}
#else
	/* assign this thread to the interrupt */
	krn->irq_th[irq] = self;
#endif 

	/* remove from ready queue */
	__krn_thread_suspend(krn, self);

	/* signal the scheduler ... */
	__krn_sched_defer(krn);

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	/* enable this interrupt source */
	cm3_irq_enable(irq);
}


void thinkos_irq_dbg_svc(int32_t * arg, unsigned int self,
						 struct thinkos_rt * krn)
{
	unsigned int irq = arg[0];
#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	uint8_t * ptr;
	uint32_t old;
#endif

#if (THINKOS_ENABLE_ARG_CHECK)
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(self, THINKOS_ERR_IRQ_INVALID);
		arg[4] = THINKOS_EINVAL;
		return;
	}
#endif /* THINKOS_ENABLE_ARG_CHECK */

	DCC_LOG2(LOG_INFO, "<%2d> IRQ %d!", self, irq);

	/* remove from ready queue */
	__krn_thread_suspend(krn, self);

	/* signal the scheduler ... */
	__krn_sched_defer(krn);

#if (THINKOS_ENABLE_IRQ_SANITY_CHECK)
	ptr = (uint8_t *)&krn->irq_th[irq];
	do {
		old = __ldrexb(ptr);
	} while (__strexb(ptr, self));

	if (old != THINKOS_THREAD_VOID) {
		DCC_LOG1(LOG_ERROR, "irq IRQ %d is busy!", irq);
		__THINKOS_ERROR(self, THINKOS_ERR_IRQ_TAKEN);
		arg[4] = THINKOS_EFAULT;
		return;
	}
	arg[4] = old;
#else
	/* assign this thread to the interrupt */
	krn->irq_th[irq] = self;
	arg[4] = THINKOS_OK;
#endif 

	__dsb();

	/* clear pending interrupt */
	cm3_irq_pend_clr(irq);

	__dsb();

	/* enable this interrupt source */
	cm3_irq_enable(irq);
}
#endif


/* This macro is here for backword compatibility, TODO should be deprecated */
#if (THINKOS_ENABLE_IRQ_CTL)
void thinkos_irq_ctl_svc(int32_t * arg, unsigned int self, 
						 struct thinkos_rt * krn)
{
	unsigned int req = arg[0];
	unsigned int irq = arg[1];

#if (THINKOS_ENABLE_ARG_CHECK)
	if (irq >= THINKOS_IRQ_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid IRQ %d!", irq);
		__THINKOS_ERROR(self, THINKOS_ERR_IRQ_INVALID);
		arg[4] = THINKOS_EINVAL;
		return;
	}
#endif
	arg[4] = THINKOS_OK;
	
	switch (req) {
	case THINKOS_IRQ_ENABLE:
		DCC_LOG1(LOG_MSG, "enabling IRQ %d", irq);
		/* clear pending interrupt */
		cm3_irq_enable(irq);
		break;

	case THINKOS_IRQ_DISABLE:
		cm3_irq_disable(irq);
		cm3_irq_pend_clr(irq);
		break;

	case THINKOS_IRQ_PRIORITY_SET:
		{
			int priority = arg[2];

			if (priority > IRQ_PRIORITY_VERY_LOW)
				priority = IRQ_PRIORITY_VERY_LOW;
#if !(THINKOS_ENABLE_IRQ_PRIORITY_0)
			else if (priority < IRQ_PRIORITY_VERY_HIGH)
				priority = IRQ_PRIORITY_VERY_HIGH;
#endif
			/* set the interrupt priority */
			cm3_irq_pri_set(irq, priority);
		}
		break;

#ifdef CM3_RAM_VECTORS
	case THINKOS_IRQ_REGISTER:
		{
			unsigned int priority = arg[2];
			void * isr = (void *)arg[3];

			/* disable this interrupt source */
			cm3_irq_disable(irq);

			if (priority > IRQ_PRIORITY_VERY_LOW)
				priority = IRQ_PRIORITY_VERY_LOW;
#if !(THINKOS_ENABLE_IRQ_PRIORITY_0)
			else if (priority < IRQ_PRIORITY_VERY_HIGH)
				priority = IRQ_PRIORITY_VERY_HIGH;
#endif

			/* set the interrupt priority */
			cm3_irq_pri_set(irq, priority);

			/* clear pending interrupt */
			cm3_irq_pend_clr(irq);

			/* set the vector */
			__ram_vectors[irq + 16] = isr;

			DCC_LOG2(LOG_MSG, "irq_register(irq=%d isr=0x%08x)", irq, isr);

			/* enable this interrupt source */
			cm3_irq_enable(irq);
		}
		break;
#endif

	default:
		DCC_LOG1(LOG_ERROR, "invalid IRQ ctl request %d!", req);
		arg[4] = THINKOS_EINVAL;
		break;
	}
}
#endif


#if (THINKOS_ENABLE_PAUSE)
bool irq_resume(struct thinkos_rt * krn, unsigned int th, 
				unsigned int wq, bool tmw) 
{
	{
		int irq;
		for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
			if (krn->irq_th[irq] == (int)th) {
				DCC_LOG2(LOG_INFO, "PC=%08x IRQ=%d ......", 
						 __thread_pc_get(krn, th), irq); 
				/* disable this interrupt source */
				cm3_irq_enable(irq);
				break;
			}
		}
	}
	DCC_LOG2(LOG_INFO, "th=%d PC=%08x +++++", 
			 th, __thread_pc_get(krn, th)); 
	__bit_mem_wr(&krn->wq_lst[wq], th, 1);
	__bit_mem_wr(&krn->wq_clock, th, tmw);
	return true;
}
#endif

