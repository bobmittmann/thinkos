/* 
 * thinkos/kernel.h
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

#ifndef __THINKOS_KERNEL_H__
#define __THINKOS_KERNEL_H__

#ifndef __THINKOS_KERNEL__
#error "Never use <thinkos/kernel.h> directly; include <thinkos.h> instead."
#endif 

#include <arch/cortex-m3.h>

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>

#define __THINKOS_ERROR__
#include <thinkos/error.h>

#define __THINKOS_FLASH__
#include <thinkos/flash.h>

#define __THINKOS_IDLE__
#include <thinkos/idle.h>

#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>

#define __THINKOS_TIME__
#include <thinkos/time.h>

#define __THINKOS_CLOCK__
#include <thinkos/clock.h>

#define __THINKOS_IRQ__
#include <thinkos/irq.h>

/* Enable kernel support for real time trace. The kernel hold the trace
   ring in a protected memory and mediate its access by the application 
   and debug monitor services. */
#ifndef THINKOS_ENABLE_TRACE    
#define THINKOS_ENABLE_TRACE            0 
#endif

/* -------------------------------------------------------------------------- 
 * context register offsets
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_FPU)
  #define CONTROL_MSK 0x07
#else
  #define CONTROL_MSK 0x03
#endif

#if (THINKOS_ENABLE_STACK_ALIGN)
  #define STACK_ALIGN_MSK 0x0000003f
#else
  #define STACK_ALIGN_MSK 0x0000000f
#endif

/* Position of register R0 in the context */
#define CTX_R0 8
/* Position of register PC in the context */
#define CTX_PC 14
#define CTX_SIZE (16 * 4)

#define SIZEOF_THINKOS_BASIC_CONTEXT (16 * 4)
#define SIZEOF_THINKOS_EXTENDED_CONTEXT ((16 + 16 + 18) * 4)

#if (THINKOS_ENABLE_FPU) 
#define SIZEOF_THINKOS_CONTEXT SIZEOF_THINKOS_EXTENDED_CONTEXT
#else
#define SIZEOF_THINKOS_CONTEXT SIZEOF_THINKOS_BASIC_CONTEXT 
#endif


/* -------------------------------------------------------------------------- 
 * Static trhead references
 * --------------------------------------------------------------------------*/
#define TH_CTX_CNT (THINKOS_THREADS_MAX)
#define NRT_CTX_CNT (THINKOS_NRT_THREADS_MAX)

/* Total number of threads */
#define THINKOS_CTX_CNT (TH_CTX_CNT + NRT_CTX_CNT + 1)

#define THINKOS_THREAD_NULL (32)

#define THINKOS_THREAD_IDLE (TH_CTX_CNT + NRT_CTX_CNT)
/* Discard context signal */
#define THINKOS_THREAD_VOID (TH_CTX_CNT + NRT_CTX_CNT + 1)

/* -------------------------------------------------------------------------- 
 * Monitor structure offsets (used in assembler code)
  * --------------------------------------------------------------------------*/
#define MONITOR_CTX_OFFS    (0) 
#define MONITOR_EVS_OFFS    (MONITOR_CTX_OFFS  + 4) 
#define MONITOR_MSK_OFFS    (MONITOR_EVS_OFFS  + 4) 

/* -------------------------------------------------------------------------- 
 * ThinkOS RT structure offsets (used in assembler code)
  * --------------------------------------------------------------------------*/

#define SIZEOF_TH_CTX       (TH_CTX_CNT * 4)
#define SIZEOF_NRT_CTX      (NRT_CTX_CNT * 4)
#define SIZEOF_IDLE_CTX     4
#define SIZEOF_CTX          (SIZEOF_TH_CTX + SIZEOF_NRT_CTX + SIZEOF_IDLE_CTX)

#if (THINKOS_ENABLE_STACK_LIMIT)
  #define SIZEOF_TH_SL      (THINKOS_CTX_CNT * 4)
#else
  #define SIZEOF_TH_SL     0
#endif

#if (THINKOS_ENABLE_PROFILING)
  #define SIZEOF_CYCREF    4
  #define SIZEOF_CYCCNT    (THINKOS_CTX_CNT * 4)
#else
  #define SIZEOF_CYCREF    0
  #define SIZEOF_CYCCNT    0
#endif

#if (THINKOS_ENABLE_CRITICAL)
  #define SIZEOF_CRITCNT   4
#else
  #define SIZEOF_CRITCNT   0
#endif

#if (THINKOS_ENABLE_TIMESHARE)
  #define SIZEOF_SCHED_LM  4
#else
  #define SIZEOF_SCHED_LM  0
#endif

#if (THINKOS_ENABLE_CLOCK)
  #define SIZEOF_TICKS     4
  #if (THINKOS_ENABLE_MONITOR_CLOCK)
    #define SIZEOF_MONITOR_CLOCK 4
  #else
    #define SIZEOF_MONITOR_CLOCK 0
  #endif
#else
  #define SIZEOF_TICKS     0
  #define SIZEOF_MONITOR_CLOCK 0
#endif

#if (THINKOS_ENABLE_MONITOR)
  #define SIZEOF_MONITOR   (4 * 3)
#else
  #define SIZEOF_MONITOR   0
#endif

#if (THINKOS_ENABLE_DEBUG_BKPT)
  #if (THINKOS_ENABLE_DEBUG_STEP)
    #define SIZEOF_STEP_REQ  4
    #define SIZEOF_STEP_SVC  4
  #else
    #define SIZEOF_STEP_REQ  0
    #define SIZEOF_STEP_SVC  0
  #endif
  #define SIZEOF_XCPT_IPSR 2
  #define SIZEOF_STEP_ID   1
  #define SIZEOF_BREAK_ID  1
#else
  #define SIZEOF_STEP_REQ  0
  #define SIZEOF_STEP_SVC  0
  #define SIZEOF_XCPT_IPSR 0
  #define SIZEOF_STEP_ID   0
  #define SIZEOF_BREAK_ID  0
#endif

#if (THINKOS_ENABLE_RUNMASK)
  #define SIZEOF_RUNMASK 4
#else
  #define SIZEOF_RUNMASK 0
#endif

#define SIZEOF_ACTIVE 4

#if (THINKOS_ENABLE_TIMESHARE)
  #define SIZEOF_WQTMSHARE 4
#else
  #define SIZEOF_WQTMSHARE 0
#endif

#if (THINKOS_ENABLE_CLOCK)
  #define SIZEOF_WQCLK 4
#else
  #define SIZEOF_WQCLK 0
#endif

#define THINKOS_RT_TH_CTX_OFFS         0
#define THINKOS_RT_NRT_CTX_OFFS    (THINKOS_RT_TH_CTX_OFFS + SIZEOF_TH_CTX)
#define THINKOS_RT_IDLE_CTX_OFFS   (THINKOS_RT_NRT_CTX_OFFS + SIZEOF_NRT_CTX)
#define THINKOS_RT_TH_SL_OFFS      (THINKOS_RT_IDLE_CTX_OFFS + SIZEOF_IDLE_CTX)
#define THINKOS_RT_CYCCNT_OFFS     (THINKOS_RT_TH_SL_OFFS + SIZEOF_TH_SL)
#define THINKOS_RT_CYCREF_OFFS     (THINKOS_RT_CYCCNT_OFFS + SIZEOF_CYCCNT)
#define THINKOS_RT_STEP_REQ_OFFS   (THINKOS_RT_CYCREF_OFFS + SIZEOF_CYCREF)
#define THINKOS_RT_STEP_SVC_OFFS   (THINKOS_RT_STEP_REQ_OFFS + SIZEOF_STEP_REQ)
#define THINKOS_RT_XCPT_IPSR_OFFS  (THINKOS_RT_STEP_SVC_OFFS + SIZEOF_STEP_SVC)
#define THINKOS_RT_STEP_ID_OFFS    (THINKOS_RT_XCPT_IPSR_OFFS + \
									SIZEOF_XCPT_IPSR)
#define THINKOS_RT_BREAK_ID_OFFS   (THINKOS_RT_STEP_ID_OFFS + SIZEOF_STEP_ID)
#define THINKOS_RT_CRITCNT_OFFS    (THINKOS_RT_BREAK_ID_OFFS + SIZEOF_BREAK_ID)
#define THINKOS_RT_RUNMASK_OFFS    (THINKOS_RT_CRITCNT_OFFS + SIZEOF_CRITCNT)
#define THINKOS_RT_ACTIVE_OFFS     ((THINKOS_RT_RUNMASK_OFFS) + SIZEOF_RUNMASK)
#define THINKOS_RT_READY_OFFS      (THINKOS_RT_ACTIVE_OFFS + SIZEOF_ACTIVE)
#define THINKOS_RT_WQTMSAHRE_OFFS  ((THINKOS_RT_READY_OFFS) + SIZEOF_WQTMSHARE)
#define THINKOS_RT_WQCLK_OFFS      ((THINKOS_RT_WQTMSAHRE_OFFS) + SIZEOF_WQCLK)


/* Mark for kernel breakpoint numbers. Breakpoints above this
   number are considered errors. */
#define THINKOS_BKPT_EXCEPT_OFF 128

#define THINKOS_ERROR_BKPT(_CODE_) ((THINKOS_BKPT_EXCEPT_OFF) + (_CODE_))

#ifndef __ASSEMBLER__

#include <thinkos.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* -------------------------------------------------------------------------- 
 * Opaque declarations
 * --------------------------------------------------------------------------*/

struct thinkos_mem_map;

/* -------------------------------------------------------------------------- 
 * Monitor control structure
 * --------------------------------------------------------------------------*/

struct thinkos_monitor { 
	union{
		volatile uintptr_t ctl; /* control: semaphore/context pointer [PSP] */
		uint32_t * ctx;
	};
	volatile uint32_t events;  /* event set bitmap */
	volatile uint32_t mask;  /* events mask */
};

/* -------------------------------------------------------------------------- 
 * Thread context layout
 * --------------------------------------------------------------------------*/

struct thinkos_context {
	/* scheduler saved context */
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;

	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;

	/* automatic saved context - exception frame */
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;

	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;

};

#if (THINKOS_ENABLE_FPU) 
struct thinkos_fp_context {
	/* FP context { S16..S31 }*/
	uint32_t s1[16];

	/* baisc frame only core registers */
	struct thinkos_context core;

	/* FP context { S0..S15 }*/
	uint32_t s0[16];
	uint32_t fpscr;
	uint32_t res;
};
#endif

/* -------------------------------------------------------------------------- 
 * ThinkOS kernel data block
 * --------------------------------------------------------------------------*/

struct thinkos_rt {
	/* This fields must be at the beginning of this structure 
	   and their order and sizes must not be changed.
	   This is critical for the scheduler operation. */
	/* Thread context pointers */
	union {
		uintptr_t ctx[THINKOS_CTX_CNT]; 
		struct {
			uintptr_t th_ctx[TH_CTX_CNT]; 
#if (NRT_CTX_CNT) > 0
			uintptr_t nrt_ctx[NRT_CTX_CNT]; 
#endif
			uintptr_t idle_ctx;
		};
	};

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* Per thread stack limit */
	uint32_t th_sl[THINKOS_CTX_CNT]; 
#endif
	
#if (THINKOS_ENABLE_PROFILING)
	/* Per thread cycle count */
	uint32_t cyccnt[THINKOS_CTX_CNT]; 
#endif

#if (THINKOS_ENABLE_PROFILING)
	/* Reference cycle state ... */
	uint32_t cycref;
#endif

#if (THINKOS_ENABLE_MONITOR)
#if (THINKOS_ENABLE_DEBUG_BKPT)
  #if (THINKOS_ENABLE_DEBUG_STEP)
	uint32_t step_req;  /* step request bitmap */
	uint32_t step_svc;  /* step at service call bitmap */
  #endif
#endif
	uint16_t xcpt_ipsr; /* Exception IPSR */
	int8_t   step_id;   /* current stepping thread id */
	int8_t   brk_idx;  /* break thread index */
#endif

#if (THINKOS_ENABLE_CRITICAL)
	uint32_t critical_cnt; /* critical section entry counter, if not zero,
							 thread preemption is disabled */
#endif

#if (THINKOS_ENABLE_RUNMASK)
	uint32_t runmask;
#endif

	uint32_t active; /* current active thread */

	union {
		uint32_t wq_lst[THINKOS_WQ_CNT]; /* queue list */
		struct {
			uint32_t wq_ready; /* ready threads queue */


#if (THINKOS_ENABLE_TIMESHARE)
			uint32_t wq_tmshare; /* Threads waiting for time share cycle */
#endif

#if (THINKOS_ENABLE_CLOCK)
			uint32_t wq_clock;
#endif

#if ((THINKOS_MUTEX_MAX) > 0)
			uint32_t wq_mutex[THINKOS_WQ_MUTEX_CNT];
#endif

#if ((THINKOS_COND_MAX) > 0)
			uint32_t wq_cond[THINKOS_WQ_COND_CNT];
#endif

#if ((THINKOS_SEMAPHORE_MAX) > 0)
			uint32_t wq_sem[THINKOS_WQ_SEMAPHORE_CNT];
#endif

#if ((THINKOS_EVENT_MAX) > 0)
			uint32_t wq_event[THINKOS_WQ_EVENT_CNT]; 
#endif

#if ((THINKOS_FLAG_MAX) > 0)
			uint32_t wq_flag[THINKOS_WQ_FLAG_CNT]; 
#endif

#if ((THINKOS_GATE_MAX) > 0)
			uint32_t wq_gate[THINKOS_WQ_GATE_CNT]; 
#endif

#if (THINKOS_ENABLE_JOIN)
			uint32_t wq_join[THINKOS_WQ_JOIN_CNT];
#endif

#if (THINKOS_ENABLE_CONSOLE)
			uint32_t wq_console_wr;
			uint32_t wq_console_rd;
#endif

#if (THINKOS_ENABLE_PAUSE)
			uint32_t wq_paused;
#endif

#if (THINKOS_ENABLE_JOIN)
			uint32_t wq_canceled;
#endif

#if (THINKOS_ENABLE_COMM)
			uint32_t wq_comm_send;
			uint32_t wq_comm_recv;
#endif

#if (THINKOS_ENABLE_WQ_IRQ)
			uint32_t wq_irq;
#endif

#if (THINKOS_ENABLE_WQ_DMA)
			uint32_t wq_dma;
#endif

#if ((THINKOS_FLASH_MEM_MAX) > 0) 
			uint32_t wq_flash_mem[THINKOS_WQ_FLASH_MEM_CNT];
#endif

#if (THINKOS_ENABLE_THREAD_FAULT)
			uint32_t wq_fault; /* fault threads wait queue */
#endif
		};
	};

#if (THINKOS_ENABLE_THREAD_STAT)
	uint16_t th_stat[THINKOS_THREADS_MAX]; /* Per thread status */
#endif

#if (THINKOS_ENABLE_THREAD_FAULT)
	int8_t th_errno[THINKOS_THREADS_MAX]; /* Per thread error code */
#endif

#if (THINKOS_ENABLE_DATE_AND_TIME)
	struct krn_clock time_clk;
#endif
#if (THINKOS_ENABLE_CLOCK)
		uint32_t ticks;
		/* Per thread clock. Used for time wait (e.g. sleep()) */
		uint32_t th_clk[THINKOS_THREADS_MAX];
  #if (THINKOS_ENABLE_MONITOR_CLOCK)
		/* kernel monitor timer */
		uint32_t monitor_clock;
  #endif
#endif
#if (THINKOS_ENABLE_MONITOR)
		/* kernel monitor control structure */
		struct thinkos_monitor monitor;
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
	struct thinkos_idle_rt idle_hooks;
#endif

#if (THINKOS_ENABLE_TIMESHARE)
	/* This fields are used for time sharing (round robin) schedule only */
	struct {
		int8_t sched_val[THINKOS_THREADS_MAX]; /* Thread schedule value */
		int8_t sched_idle_val;
		uint8_t sched_pri[THINKOS_THREADS_MAX]; /* Thread schedule priority */
		int8_t sched_idle_pri;
		uint32_t sched_limit; /* Limiting time for an active thread */
	};
#endif


#if ((THINKOS_SEMAPHORE_MAX) > 0)
	uint32_t sem_val[THINKOS_SEMAPHORE_MAX];
#endif /* THINKOS_SEMAPHORE_MAX > 0 */

#if ((THINKOS_EVENT_MAX) > 0)
	struct {
		uint32_t pend; /* event set pending bitmap */
		uint32_t mask; /* event set mask */
	} ev[THINKOS_EVENT_MAX];
#endif /* THINKOS_EVENT_MAX > 0 */

#if ((THINKOS_FLAG_MAX) > 0)
	uint32_t flag[(THINKOS_FLAG_MAX + 31) / 32]; /* flag signal */
#endif /* THINKOS_FLAG_MAX > 0 */

#if ((THINKOS_GATE_MAX) > 0)
	/* gate bitmap, each gate takes two bits: 
	   1 - signal the gate is open or signaled to be open, 
	   2 - the gate is locked and can't be oepn. */
	uint32_t gate[(THINKOS_GATE_MAX + 15) / 16]; /* gates states */
#endif /* THINKOS_GATE_MAX > 0 */

#if ((THINKOS_MUTEX_MAX) > 0)
	int8_t lock[THINKOS_MUTEX_MAX];
#endif /* THINKOS_MUTEX_MAX > 0 */

#if ((THINKOS_IRQ_MAX) > 0)
	int8_t irq_th[THINKOS_IRQ_MAX];
#endif /* THINKOS_IRQ_MAX */

#if (THINKOS_ENABLE_THREAD_ALLOC)
	uint32_t th_alloc[1];
#endif

#if (THINKOS_ENABLE_MUTEX_ALLOC)
	uint32_t mutex_alloc[(THINKOS_MUTEX_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_COND_ALLOC)
	uint32_t cond_alloc[(THINKOS_COND_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_SEM_ALLOC)
	uint32_t sem_alloc[(THINKOS_SEMAPHORE_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_EVENT_ALLOC)
	uint32_t ev_alloc[(THINKOS_EVENT_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_FLAG_ALLOC)
	uint32_t flag_alloc[(THINKOS_FLAG_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_GATE_ALLOC)
	uint32_t gate_alloc[(THINKOS_GATE_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * th_inf[THINKOS_CTX_CNT];
#endif
#if ((THINKOS_FLASH_MEM_MAX) > 0)
	struct thinkos_flash_drv flash_drv[THINKOS_FLASH_MEM_MAX];
#endif
#if (THINKOS_ENABLE_MEMORY_MAP)
	const struct thinkos_mem_map * mem_map;
#endif
};

/* -------------------------------------------------------------------------- 
 * Base indexes for the wait queue list (wq_lst[])
 * --------------------------------------------------------------------------*/

#define THINKOS_WQ_READY 0

#define THINKOS_WQ_TMSHARE ((offsetof(struct thinkos_rt, wq_tmshare) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_WQ_CLOCK ((offsetof(struct thinkos_rt, wq_clock) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_MUTEX_BASE ((offsetof(struct thinkos_rt, wq_mutex) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_COND_BASE ((offsetof(struct thinkos_rt, wq_cond) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_SEM_BASE ((offsetof(struct thinkos_rt, wq_sem) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_EVENT_BASE ((offsetof(struct thinkos_rt, wq_event) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_FLAG_BASE ((offsetof(struct thinkos_rt, wq_flag) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_GATE_BASE ((offsetof(struct thinkos_rt, wq_gate) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_JOIN_BASE ((offsetof(struct thinkos_rt, wq_join) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_WQ_CONSOLE_WR ((offsetof(struct thinkos_rt, wq_console_wr) \
								- offsetof(struct thinkos_rt, wq_lst)) \
							   / sizeof(uint32_t))

#define THINKOS_WQ_CONSOLE_RD ((offsetof(struct thinkos_rt, wq_console_rd) \
								- offsetof(struct thinkos_rt, wq_lst)) \
							   / sizeof(uint32_t))

#define THINKOS_WQ_PAUSED ((offsetof(struct thinkos_rt, wq_paused) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_WQ_CANCELED ((offsetof(struct thinkos_rt, wq_canceled) \
							 - offsetof(struct thinkos_rt, wq_lst)) \
							/ sizeof(uint32_t))

#define THINKOS_WQ_COMM_SEND ((offsetof(struct thinkos_rt, wq_comm_send) \
								- offsetof(struct thinkos_rt, wq_lst)) \
							   / sizeof(uint32_t))

#define THINKOS_WQ_COMM_RECV ((offsetof(struct thinkos_rt, wq_comm_recv) \
								- offsetof(struct thinkos_rt, wq_lst)) \
							   / sizeof(uint32_t))

#define THINKOS_WQ_IRQ ((offsetof(struct thinkos_rt, wq_irq) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_WQ_DMA ((offsetof(struct thinkos_rt, wq_dma) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_FLASH_MEM_BASE ((offsetof(struct thinkos_rt, wq_flash_mem) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_WQ_FAULT ((offsetof(struct thinkos_rt, wq_fault) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

/* -------------------------------------------------------------------------- 
 * Static initialized kernel descriptors
 * --------------------------------------------------------------------------*/

#define THINKOS_MUTEX_DESC(_ID) (THINKOS_MUTEX_BASE + (_ID))
#define THINKOS_COND_DESC(_ID)  (THINKOS_COND_BASE + (_ID))
#define THINKOS_SEM_DESC(_ID)   (THINKOS_SEM_BASE + (_ID))
#define THINKOS_EVENT_DESC(_ID) (THINKOS_EVENT_BASE + (_ID))
#define THINKOS_FLAG_DESC(_ID)  (THINKOS_FLAG_BASE + (_ID))
#define THINKOS_GATE_DESC(_ID)  (THINKOS_GATE_BASE + (_ID))
#define THINKOS_FLASH_MEM_DESC(_ID)  (THINKOS_FLASH_MEM_BASE + (_ID))

/* -------------------------------------------------------------------------- 
 * Thread initialization 
 * --------------------------------------------------------------------------*/
struct thinkos_thread_opt {
	uint16_t stack_size;
	uint8_t priority;
	uint8_t id: 6;
	uint8_t privileged : 1;
	uint8_t paused: 1;
};


struct thinkos_thread_create_args {
	int (* task)(void *);            /* R0 */
	void * arg;                      /* R1 */
	void * stack_ptr;                /* R2 */
	struct thinkos_thread_opt opt;   /* R3 */
	struct thinkos_thread_inf * inf; /* R4 */
};

#define __THINKOS_MEMORY__
#include <thinkos/memory.h>

extern struct thinkos_rt thinkos_rt;

#if (THINKOS_ENABLE_THREAD_INFO)
extern const struct thinkos_thread_inf thinkos_main_inf;
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct thinkos_context * __thinkos_thread_ctx_get(unsigned int idx);

uintptr_t __thinkos_thread_pc_get(unsigned int idx);


uint32_t __thinkos_thread_lr_get(unsigned int id);

uint32_t __thinkos_thread_sp_get(unsigned int id);

uint32_t __thinkos_thread_sl_get(unsigned int id);

uint32_t __thinkos_thread_xpsr_get(unsigned int id);

uint32_t __thinkos_thread_r0_get(unsigned int id);

void __thinkos_thread_r0_set(unsigned int id, uint32_t val);

uint32_t __thinkos_thread_r1_get(unsigned int id);

void __thinkos_thread_r1_set(unsigned int id, uint32_t val);

void __thinkos_thread_r2_set(unsigned int id, uint32_t val);

void __thinkos_thread_r3_set(unsigned int id, uint32_t val);

void __thinkos_thread_pc_set(unsigned int id, uintptr_t val);

void __thinkos_thread_lr_set(unsigned int id, uintptr_t val);

uint32_t * __thinkos_thread_frame_get(unsigned int id);

uint32_t __thinkos_thread_ctrl_get(unsigned int id);

uint32_t __thinkos_thread_exec_ret_get(unsigned int id);

bool __thinkos_thread_ctx_is_valid(unsigned int id);

void  __thinkos_thread_ctx_set(unsigned int id, struct thinkos_context * ctx,
							   unsigned int ctrl);

void  __thinkos_thread_ctx_flush(int32_t arg[], unsigned int id);

void  __thinkos_thread_ctx_clr(unsigned int id);

const struct thinkos_thread_inf * __thinkos_thread_inf_get(unsigned int id);

void  __thinkos_thread_inf_set(unsigned int id, 
							   const struct thinkos_thread_inf * inf);

void __thinkos_thread_inf_clr(unsigned int id);

int __thinkos_thread_errno_get(unsigned int id);

void __thinkos_thread_errno_set(unsigned int id, int errno);

void __thinkos_thread_errno_clr(unsigned int id);

void __thinkos_thread_cyccnt_clr(unsigned int id);

unsigned int __thinkos_thread_stat_wq_get(unsigned int th);

bool __thinkos_thread_stat_tmw_get(unsigned int th);

void __thinkos_thread_stat_clr(unsigned int th);

void __thinkos_thread_stat_set(unsigned int th, unsigned int wq, bool tmd);

/* -------------------------------------------------------------------------- 
 * thread stack limit access methods 
 * --------------------------------------------------------------------------*/

void __thinkos_thread_sl_clr(unsigned int idx);

void __thinkos_thread_sl_set(unsigned int idx, uint32_t addr);

void __thinkos_active_set(unsigned int th);

unsigned int __thinkos_active_get(void);

uint32_t __thinkos_active_sl_get(void);

/* Set the active thread and stack limit */
void __thinkos_active_sl_set(unsigned int th, uint32_t sl);

bool __thinkos_thread_is_in_wq(unsigned int id, unsigned int wq);

void  __thinkos_ready_clr(void);

void __thinkos_suspend(unsigned int idx);

int __thinkos_wq_idx(uint32_t * ptr);

int __thinkos_wq_head(unsigned int wq);

void __thinkos_wq_insert(unsigned int wq, unsigned int th);

#if (THINKOS_ENABLE_TIMED_CALLS)
void __thinkos_tmdwq_insert(unsigned int wq, unsigned int th, unsigned int ms);
#endif

#if (THINKOS_ENABLE_TIMED_CALLS)
void __thinkos_wq_clock_insert(unsigned int th, unsigned int ms);
#endif

void __thinkos_wq_remove( unsigned int wq, unsigned int th);

void __thinkos_wakeup( unsigned int wq, unsigned int th);

void __thinkos_wakeup_return( unsigned int wq, unsigned int th, int ret);


#if (THINKOS_ENABLE_CLOCK)
uint32_t  __thinkos_ticks(void);
#endif

/* -------------------------------------------------------------------------- 
 * kernel error and debug
 * --------------------------------------------------------------------------*/

/* Set the fault flag */
void __thinkos_thread_fault_set(unsigned int th, int errno);

/* Clear the fault flag */
void __thinkos_thread_fault_clr(unsigned int th);

/* Get the fault flag */
bool __thinkos_thread_fault_get(unsigned int th);

void __thinkos_pause_all(void);

void __thinkos_resume_all(void);

bool __thinkos_thread_pause(unsigned int thread_id);

bool __thinkos_thread_resume(unsigned int thread_id);

/* Set the pause flag */
void __thinkos_thread_pause_set(unsigned int th);

/* Clear the pause flag */
void __thinkos_thread_pause_clr(unsigned int th);

/* Get the pause flag */
bool __thinkos_thread_pause_get(unsigned int th);


bool __thinkos_thread_isalive(unsigned int thread_id);

bool __thinkos_thread_ispaused(unsigned int thread_id);

bool __thinkos_thread_isfaulty(unsigned int thread_id);

/* -------------------------------------------------------------------------- 
 * kernel utility library 
 * --------------------------------------------------------------------------*/

void __thinkos_memcpy(void * __dst, const void * __src,  
					  unsigned int __len);

void __thinkos_memcpy32(void * __dst, const void * __src,  
						unsigned int __len);

void __thinkos_memset32(void * __dst, uint32_t __val, unsigned int __len);

unsigned int __thinkos_strlen(const char * __s, unsigned int __max);

void __thinkos_system_reset(void);

uint32_t __thinkos_crc32_u32(uint32_t __buf[], unsigned int __len);

uint32_t __thinkos_crc32_u8(const void * __buf, unsigned int __len); 

/* -------------------------------------------------------------------------- 
 * kernel core functions 
 * --------------------------------------------------------------------------*/

void thinkos_krn_core_reset(struct thinkos_rt * krn);

int thinkos_krn_thread_init(struct thinkos_rt * krn,
							unsigned int thread_idx,
							struct thinkos_thread_initializer * init);

unsigned int thinkos_krn_active_get(struct thinkos_rt * krn);

void __attribute__((noreturn)) thinkos_krn_sysrst(void);

void thinkos_krn_udelay_calibrate(void);

/* -------------------------------------------------------------------------- 
 * kernel thread functions 
 * --------------------------------------------------------------------------*/

bool __thinkos_thread_resume(unsigned int thread_id);

int __thinkos_thread_wq_get(unsigned int thread_idx);

int __thinkos_thread_tmw_get(unsigned int thread_idx);

/* -------------------------------------------------------------------------- 
 * kernel scheduler methods 
 * --------------------------------------------------------------------------*/

void __thinkos_cancel_sched(void);

/* flags a deferred execution of the scheduler */
void  __thinkos_defer_sched(void);

/* flags a deferred execution of the scheduler */
void __thinkos_preempt(void);

const char * __thinkos_thread_tag_get(unsigned int idx);

unsigned int __thinkos_obj_kind(unsigned int oid);

int __thinkos_kind_prefix(unsigned int kind);

const char * __thinkos_kind_name(unsigned int kind);

/* -------------------------------------------------------------------------
 * Kernel Debug
 * ------------------------------------------------------------------------- */

int __thinkos_scan_stack(void * stack, unsigned int size);

int __thinkos_thread_getnext(int th);


void __thinkos_systick_sleep(void);

void __thinkos_systick_wakeup(void);

void __thinkos_krn_time_init(struct thinkos_rt * krn);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

#endif /* __THINKOS_KERNEL_H__ */

