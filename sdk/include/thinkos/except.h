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
#error "Never use <thinkos/except.h> directly; include <thinkos/kernel.h> instead."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

/* -------------------------------------------------------------------------- 
 * Exception state
 * --------------------------------------------------------------------------*/

struct armv7m_basic_frame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;
};

struct armv7m_extended_frame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;
	float    s[16];
	uint32_t fpscr;
	uint32_t res;
};

struct thinkos_except {
#if (THINKOS_ENABLE_FPU) 
	struct thinkos_fp_context ctx;
#else
	struct {
		struct thinkos_context core;
	} ctx;
#endif
#if !((THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_NULL_MSP))
	uint32_t ret;
#endif
	uint32_t msp;
	uint32_t psp;
	uint32_t icsr;
	uint32_t cfsr;
	uint32_t mmfar;
	uint32_t bfar;
	uint8_t  ipsr;   /* IPSR */
	uint8_t  ctrl;   /* CONTROL */
	uint8_t  type;   /* exception type */
	uint8_t  unroll; /* unroll count */

	uint32_t  active; /* active thread at the time of the exception */
	uint32_t  ready; /* ready bitmap */
	struct thinkos_context * idle_ctx;
#if (THINKOS_ENABLE_PROFILING)
	uint32_t  cycref; 
#endif
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

uint32_t * __thinkos_xcpt_stack_top(void);

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

const char * __retstr(uint32_t __ret);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_EXCEPT_H__ */

