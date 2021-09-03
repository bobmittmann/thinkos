/* 
 * thinkos_svc.h
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


#define OFFSETOF_XCPT_CONTEXT    0

#define OFFSETOF_XCPT_SP         64

#define OFFSETOF_XCPT_CFSR       68
#define OFFSETOF_XCPT_MMFAR      72
#define OFFSETOF_XCPT_BFAR       76

#define OFFSETOF_XCPT_RET        80
#define OFFSETOF_XCPT_CONTROL    81
#define OFFSETOF_XCPT_ERRNO      82
#define OFFSETOF_XCPT_SEQ        83

#define OFFSETOF_XCPT_THREAD     84
#define OFFSETOF_XCPT_ACK        87


#if 0
#define OFFSETOF_XCPT_BASEPRI    5

#define OFFSETOF_XCPT_PRIMASK    7
#define OFFSETOF_XCPT_FAULTMASK  7
#define OFFSETOF_XCPT_MSP       72
#define OFFSETOF_XCPT_PSP       76

#define OFFSETOF_XCPT_SCHED     80
#define OFFSETOF_XCPT_READY     84


#define OFFSETOF_XCPT_ICSR     104
#define OFFSETOF_XCPT_SHCSR    108

#if (THINKOS_ENABLE_PROFILING)
  #define OFFSETOF_XCPT_CYCREF  112
  #define OFFSETOF_XCPT_CYCCNT  116
#else
#endif

#endif

/*
#define SIZEOF_THINKOS_EXCEPT (OFFSETOF_XCPT_CONTEXT + \
							   (SIZEOF_THINKOS_CONTEXT))

#define SIZEOF_THINKOS_EXCEPT (OFFSETOF_XCPT_CONTEXT + \
							   (SIZEOF_THINKOS_BASIC_CONTEXT))
*/


#ifndef __ASSEMBLER__
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
	struct thinkos_context ctx;

	uint32_t sp; /* SP */

	uint32_t cfsr;
	uint32_t mmfar;
	uint32_t bfar;

	uint8_t ret;       /* exception exit return code low byte  */
	uint8_t control;
	uint8_t errno;     /* exception error code */
	uint8_t seq;       /* number of exceptions since except_ack() */

	uint8_t thread;	
	uint8_t res1;	
	uint8_t res2;	
	uint8_t ack;
	
};

extern uint32_t thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];
extern const uint16_t thinkos_except_stack_size;

#ifdef __cplusplus
extern "C" {
#endif

static inline struct thinkos_except * __thinkos_except_buf(void) {
	uintptr_t xcpt= (uintptr_t)thinkos_except_stack;
	return (struct thinkos_except *)xcpt;
}

static inline int __xcpt_thread_get(struct thinkos_except * xcpt) {
	uint32_t thread = xcpt->thread; 
	return (xcpt->seq != xcpt->ack) ? (int32_t)thread : -1;
}

static inline bool __thinkos_xcpt_valid(struct thinkos_except * xcpt) {
	return (xcpt->seq == xcpt->ack) ? false : true;
}

static inline int32_t __thinkos_xcpt_cnt(struct thinkos_except * xcpt) {
	return (int32_t)xcpt->seq - (int32_t)xcpt->ack;
}

static inline bool __thinkos_xcpt_errno(struct thinkos_except * xcpt) {
	return (xcpt->seq == xcpt->ack) ? 0 : xcpt->errno;
}

/* -------------------------------------------------------------------------
 * Exception API 
 * ------------------------------------------------------------------------- */

void thinkos_krn_exception_init(void);

void thinkos_krn_exception_reset(void);

uint32_t * __thinkos_xcpt_stack_top(void);

struct thinkos_except * __thinkos_except_buf(void);

/* -------------------------------------------------------------------------
 * Exception handling utility functions
 * ------------------------------------------------------------------------- */

void __xinfo(struct thinkos_except * xcpt);

void __xdump(struct thinkos_rt * krn, 
			 struct thinkos_except * xcpt);

void __idump(const char * s, uint32_t ipsr);

void __tdump(struct thinkos_rt * krn);

void __kdump(struct thinkos_rt * krn);

void __mpudump(void);

void __odump(void);

void __pdump(void);


int __xcpt_next_active_irq(int this_irq);

void __xcpt_systick_int_disable(void);

void __xcpt_systick_int_enable(void);

const char * __retstr(uint32_t __ret);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_EXCEPT_H__ */

