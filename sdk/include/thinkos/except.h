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

#define THINKOS_EXCEPT_ERRNO_OFFS      0
#define THINKOS_EXCEPT_SEQ_OFFS        1
#define THINKOS_EXCEPT_RET_OFFS        2
#define THINKOS_EXCEPT_PFMASK_OFFS     3

#define THINKOS_EXCEPT_BASEPRI_OFFS    4
#define THINKOS_EXCEPT_ACK_OFFS        5
#define THINKOS_EXCEPT_IPSR_OFFS       6
#define THINKOS_EXCEPT_CTRL_OFFS       7

#define THINKOS_EXCEPT_MSP_OFFS        8
#define THINKOS_EXCEPT_PSP_OFFS       12

#define THINKOS_EXCEPT_ICSR_OFFS      16
#define THINKOS_EXCEPT_SHCSR_OFFS     20

#define THINKOS_EXCEPT_CFSR_OFFS      24
#define THINKOS_EXCEPT_HFSR_OFFS      28

#define THINKOS_EXCEPT_MMFAR_OFFS     32
#define THINKOS_EXCEPT_BFAR_OFFS      36

#define THINKOS_EXCEPT_ACTIVE_OFFS    40
#define THINKOS_EXCEPT_READY_OFFS     44

#if (THINKOS_ENABLE_PROFILING)
  #define THINKOS_EXCEPT_CYCREF_OFFS  48
  #define THINKOS_EXCEPT_CYCCNT_OFFS  52
  #define THINKOS_EXCEPT_CONTEXT_OFFS 56
#else
  #define THINKOS_EXCEPT_CONTEXT_OFFS 48
#endif

/*
#define SIZEOF_THINKOS_EXCEPT (THINKOS_EXCEPT_CONTEXT_OFFS + \
							   (SIZEOF_THINKOS_CONTEXT))
*/

#define SIZEOF_THINKOS_EXCEPT (THINKOS_EXCEPT_CONTEXT_OFFS + \
							   (SIZEOF_THINKOS_BASIC_CONTEXT))


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
	uint8_t  errno;     /* exception error code */
	int8_t   seq;       /* number of exceptions since except_ack() */
	uint8_t  ret;       /* exception exit return code low byte  */
	uint8_t  pfmask;    /* PRIMASK / FAULTMASK */

	uint8_t  basepri;   /* BASEPRI */
	int8_t   ack;       /* */
	uint8_t  ipsr;      /* IPSR */
	uint8_t  ctrl;      /* CONTROL */

	uint32_t msp;       /* MSP */
	uint32_t psp;       /* PSP */

    /* SCB (System Control Block) */
	uint32_t icsr;      /* Interrupt Control State */
	uint32_t shcsr;     /* System Handler Control and State */

	uint32_t cfsr;      /* Configurable Fault Status */
	uint32_t hfsr;      /* Hard Fault Status */

	uint32_t mmfar;     /* Mem Manage Address */
	uint32_t bfar;      /* Bus Fault Address */

	uint32_t  active;   /* active thread at the time of the exception */
	uint32_t  ready;    /* ready bitmap */

#if (THINKOS_ENABLE_PROFILING)
	uint32_t  cycref;    
	uint32_t  cyccnt;    
#endif

	struct {
		struct thinkos_context core;
	} ctx;

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

static inline int __xcpt_active_get(struct thinkos_except * xcpt) {
	uint32_t active = xcpt->active; 
#if (THINKOS_ENABLE_STACK_LIMIT)
	active &= 0x3f;
#endif
	return (xcpt->seq != xcpt->ack) ? (int32_t)active : -1;
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

void __xdump(struct thinkos_except * xcpt);

void __idump(const char * s, uint32_t ipsr);

void __tdump(struct thinkos_rt * krn);

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

