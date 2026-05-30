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


#define OFFSETOF_FAULT_CONTEXT    0

#define OFFSETOF_FAULT_SP         64
#define OFFSETOF_FAULT_SHCSR      68
#define OFFSETOF_FAULT_CFSR       72
#define OFFSETOF_FAULT_MMFAR      76
#define OFFSETOF_FAULT_BFAR       80

#define OFFSETOF_FAULT_RET        84
#define OFFSETOF_FAULT_CONTROL    85
#define OFFSETOF_FAULT_ERRNO      86
#define OFFSETOF_FAULT_SEQ        87

#define OFFSETOF_FAULT_THREAD     88
#define OFFSETOF_FAULT_ACK        91


#if 0
#define OFFSETOF_FAULT_HFSR       68
#define OFFSETOF_FAULT_BASEPRI    5

#define OFFSETOF_FAULT_PRIMASK    7
#define OFFSETOF_FAULT_FAULTMASK  7
#define OFFSETOF_FAULT_MSP       72
#define OFFSETOF_FAULT_PSP       76

#define OFFSETOF_FAULT_SCHED     80
#define OFFSETOF_FAULT_READY     84


#define OFFSETOF_FAULT_ICSR     104
#define OFFSETOF_FAULT_SHCSR    108

#if (THINKOS_ENABLE_PROFILING)
  #define OFFSETOF_FAULT_CYCREF  112
  #define OFFSETOF_FAULT_CYCCNT  116
#else
#endif

#endif


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

struct thinkos_fault {
	struct thinkos_context ctx;

	uint32_t sp; /* SP */

	uint32_t shcsr;
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

extern uint32_t thinkos_except_stack[(THINKOS_EXCEPT_STACK_SIZE) / 4];
extern const uint16_t thinkos_except_stack_size;

extern struct thinkos_fault thinkos_fault_rt;

#ifdef __cplusplus
extern "C" {
#endif

static inline struct thinkos_fault * __thinkos_fault_rt(void) {
	return &thinkos_fault_rt;
}

static inline int __fault_thread_get(struct thinkos_fault * fault) {
	return fault->thread; 
}

static inline bool __thinkos_fault_valid(struct thinkos_fault * fault) {
	return (fault->seq == fault->ack) ? false : true;
}

static inline int32_t __thinkos_fault_cnt(struct thinkos_fault * fault) {
	return (int32_t)fault->seq - (int32_t)fault->ack;
}

static inline bool __thinkos_fault_errno(struct thinkos_fault * fault) {
	return (fault->seq == fault->ack) ? 0 : fault->errno;
}

/* -------------------------------------------------------------------------
 * Exception API 
 * ------------------------------------------------------------------------- */

void thinkos_krn_exception_init(void);

void thinkos_krn_fault_clr(void);

uint32_t *  thinkos_krn_xcpt_stack_top(void);

/* -------------------------------------------------------------------------
 * Exception handling utility functions
 * ------------------------------------------------------------------------- */

void __xinfo(struct thinkos_fault * fault);

void __xdump(struct thinkos_rt * krn, 
			 struct thinkos_fault * fault);

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

