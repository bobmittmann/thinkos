/* 
 * /thinkos/debug.h
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


#ifndef __THINKOS_DEBUG_H__
#define __THINKOS_DEBUG_H__

#ifndef __THINKOS_DEBUG__
#error "Never use <thinkos/debug.h> on user code, this is for kenel debug."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

struct thread_reg_core {
	union {
		uint32_t r[16];
		struct {
			uint32_t r0;
			uint32_t r1;
			uint32_t r2;
			uint32_t r3;
			uint32_t r4;
			uint32_t r5;
			uint32_t r6;
			uint32_t r7;
			uint32_t r8;
			uint32_t r9;
			uint32_t r10;
			uint32_t r11;
			uint32_t r12;
			uint32_t sp;
			uint32_t lr;
			uint32_t pc;
		};
	};
	uint32_t xpsr;
};

struct thread_reg_fp {
	uint32_t fpscr;
	union {
		float  s[32];
		double d[16];
	};		
};

struct thread_rec {
	uint8_t thread_id;
	uint8_t errno;
	uint8_t ctrl;
	uint8_t priority;
	uint32_t clock;
	char tag[8];
	uint32_t stack_base;
	uint32_t stack_top;
	struct {
		uint32_t wq: 8;
		uint32_t tmw: 1;
		uint32_t fp_regs: 1;
		uint32_t privileged: 1;
	};
};


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Debug API 
 * ------------------------------------------------------------------------- */


static inline void __attribute__((always_inline)) __thinkos_dbg_halt(void) 
{
	register uint32_t halt = 1;

	asm volatile ("1:\n" 
				  "cmp %0, #0\n" 
				  "bne 1b\n" : "=r"(halt) : "r"(halt));
}

struct thinkos_context * thinkos_dbg_thread_ctx_get(unsigned int id);

uintptr_t thinkos_dbg_thread_ctrl_get(unsigned int id);

uintptr_t thinkos_dbg_thread_pc_get(unsigned int id);

uint32_t thinkos_dbg_thread_lr_get(unsigned int id);

uint32_t thinkos_dbg_thread_sp_get(unsigned int id);

uint32_t thinkos_dbg_thread_stack_size_get(unsigned int th);

uint32_t thinkos_dbg_thread_stack_base_get(unsigned int th);

bool thinkos_dbg_thread_ctx_is_valid(unsigned int id);

uint32_t thinkos_dbg_thread_sl_get(unsigned int id);

const char * thinkos_dbg_thread_tag_get(unsigned int id);

int thinkos_dbg_thread_wq_get(unsigned int id);

int thinkos_dbg_thread_tmw_get(unsigned int id);

uint32_t thinkos_dbg_thread_cyccnt_get(unsigned int id);

int32_t thinkos_dbg_thread_clk_itv_get(unsigned int id);

int thinkos_dbg_thread_errno_get(unsigned int idx);

bool thinkos_dbg_thread_rec_get(unsigned int thread,
                           struct thread_rec * rec,
                           struct thread_reg_core * core,
                           struct thread_reg_fp * fp);

int thinkos_dbg_thread_brk_get(unsigned int id);

int thinkos_dbg_thread_break_get(int32_t * pcode);

int thinkos_dbg_thread_break_clr(void);

uint32_t thinkos_dbg_sched_state_get(void);

void thinkos_dbg_reset(void);

int thinkos_dbg_thread_create(int (* entry)(void *, unsigned int), void * arg,
						  void (* on_exit)(unsigned int), bool privileged);

void thinkos_dbg_resume_all(void);

void thinkos_dbg_pause_all(void);

int thinkos_dbg_thread_irq_get(unsigned int th);

bool thinkos_dbg_thread_is_ready(unsigned int th);

int thinkos_dbg_active_get(void);

/* -------------------------------------------------------------------------
 * Cycle counter
 * ------------------------------------------------------------------------- */

int thinkos_dbg_threads_cyc_get(uint32_t cyc[], uint32_t from, 
                                unsigned int max);

int thinkos_dbg_mutex_lock_get(unsigned int mtx);

struct thread_waitqueue;

struct thread_waitqueue * thinkos_dbg_wq_from_oid(unsigned int oid);

bool thinkos_dbg_wq_contains(struct thread_waitqueue * wq, 
							 unsigned int thread);

bool thinkos_dbg_wq_is_empty(struct thread_waitqueue * wq);


void thinkos_dbg_ack(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_DEBUG_H__ */

