/* 
 * thinkos_irq.h
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

#ifndef __THINKOS_IRQ_H__
#define __THINKOS_IRQ_H__

#ifndef __THINKOS_IRQ__
#error "Only use this file on privileged code"
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#ifndef __ASSEMBLER__

#define DEBUGGER_PRIORITY       (0 << 5)
#define EXCEPT_PRIORITY         (1 << 5)
#define IRQ_VERY_HIGH_PRIORITY  (2 << 5)
#define IRQ_HIGH_PRIORITY       (3 << 5)
#define IRQ_DEF_PRIORITY        (4 << 5)
#define IRQ_LOW_PRIORITY        (5 << 5)
#define IRQ_VERY_LOW_PRIORITY   (6 << 5)
#define CLOCK_PRIORITY          ((7 << 5) + 0x1d)
#define SYSCALL_PRIORITY        ((7 << 5) + 0x1e)
#define SCHED_PRIORITY          ((7 << 5) + 0x1f)


#if (THINKOS_ENABLE_DEBUG)
  #define MONITOR_PRIORITY      (DEBUGGER_PRIORITY)
#else
  #define MONITOR_PRIORITY      (CLOCK_PRIORITY)
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline void 
__attribute__((always_inline)) thinkos_krn_sched_off(void)  {
	/* rise the BASEPRI to stop the scheduler */
	cm3_basepri_set(SCHED_PRIORITY); 
}

static inline void 
__attribute__((always_inline)) thinkos_krn_sched_on(void)  {
	/* return the BASEPRI to the default to reenable the scheduler. */
	cm3_basepri_set(0x00);
}

#if (THINKOS_ENABLE_CLOCK)
static inline uint32_t __attribute__((always_inline)) 
	thinkos_clock_i(void)  {
	return (volatile uint32_t)thinkos_rt.ticks;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_IRQ_H__ */

