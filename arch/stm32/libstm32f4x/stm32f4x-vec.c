/* 
 * stm32f-crt0.S
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

#include <sys/stm32f.h>

void nvic_irq0_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq1_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq2_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq3_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq4_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq5_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq6_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq7_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));

typedef void (* nvic_isr_t)(void);

void _reset(void);
extern uintptr_t _stack;
extern uintptr_t ice_comm_blk;

void nmi_isr(void);
void hard_fault_isr(void);
void mem_manage_isr(void);
void bus_fault_isr(void);
void usage_fault_isr(void);
void except7_isr(void);
void except9_isr(void);
void except10_isr(void);
void svc_isr(void);
void debug_mon_isr(void);
void except13_isr(void);
void pendsv_isr(void);
void systick_isr(void);


typedef void (* nvic_isr_t)(void);

uintptr_t fault_vector[] = {
	(uintptr_t)&_stack,
	(uintptr_t)_reset,
	(uintptr_t)cm3_nmi_isr,
	(uintptr_t)cm3_hard_fault_isr,
	(uintptr_t)cm3_mem_manage_isr,
	(uintptr_t)cm3_bus_fault_isr,
	(uintptr_t)cm3_usage_fault_isr,
	(uintptr_t)cm3_except7_isr,
	(uintptr_t)ice_comm_blk,
	(uintptr_t)cm3_except9_isr,
	(uintptr_t)cm3_except10_isr,
	(uintptr_t)cm3_svc_isr,
	(uintptr_t)cm3_debug_mon_isr,
	(uintptr_t)cm3_except13_isr,
	(uintptr_t)cm3_pendsv_isr,
	(uintptr_t)cm3_systick_isr
} __attribute__((section(".rom_vectors")))

uintptr_t fault_vector[] = {
};

void nvic_isr(int irqno)
{

	nvic_vector[irqno]();
}
