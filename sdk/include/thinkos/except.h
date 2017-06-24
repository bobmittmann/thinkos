/* 
 * thikos_svc.h
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


#ifndef __THINKOS_EXCEPT_H__
#define __THINKOS_EXCEPT_H__

#ifndef __THINKOS_EXCEPT__
#error "Never use <thinkos/except.h> directly; include <thinkos.h> instead."
#endif 

#ifndef __THINKOS_KERNEL_H__
#error "Need <thinkos/kernel.h>."
#endif 

/* -------------------------------------------------------------------------- 
 * Exception state
 * --------------------------------------------------------------------------*/

struct thinkos_except {
	struct thinkos_context ctx;
	uint32_t ret;
	uint32_t msp;
	uint32_t psp;
	uint32_t icsr;
	uint8_t  ipsr;   /* IPSR */
	int8_t   active; /* active thread at the time of the exception */
	uint8_t  type;   /* exception type */
	uint8_t  unroll; /* unroll count */
};

extern struct thinkos_except thinkos_except_buf;

extern uint32_t thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

extern const uint16_t thinkos_except_stack_size;

extern const char thinkos_except_nm[];

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Exception API 
 * ------------------------------------------------------------------------- */

void thinkos_exception_init(void);

void thinkos_exception_dsr(struct thinkos_except * xcpt);

/* -------------------------------------------------------------------------
 * Exception handling utility functions
 * ------------------------------------------------------------------------- */

void __xdump(struct thinkos_except * xcpt);

void __idump(const char * s, uint32_t ipsr);

void __tdump(void);

void __mpudump(void);

int __xcpt_next_active_irq(int this_irq);

void __xcpt_systick_int_disable(void);

void __xcpt_systick_int_enable(void);

void __exception_reset(void);

struct thinkos_except * __thinkos_except_buf(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_EXCEPT_H__ */

