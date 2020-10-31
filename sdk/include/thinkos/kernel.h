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


/* Enable kernel support for real time trace. The kernel hold the trace
   ring in a protected memory and mediate its access by the application 
   and debug monitor services. */
#ifndef THINKOS_ENABLE_TRACE    
#define THINKOS_ENABLE_TRACE            0 
#endif

/* -------------------------------------------------------------------------- 
 * context register offsets
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
/* Position of register R0 in the context */
  #define CTX_R0 10
  #define CTX_PC 16
  #define CTX_SIZE (18 * 4)
#else
/* Position of register R0 in the context */
  #define CTX_R0 8
/* Position of register PC in the context */
  #define CTX_PC 14
  #define CTX_SIZE (16 * 4)
#endif

/* -------------------------------------------------------------------------- 
 * Static trhead references
 * --------------------------------------------------------------------------*/

#define THINKOS_THREAD_NULL (32)
#define THINKOS_THREAD_IDLE (THINKOS_THREADS_MAX)

#if (THINKOS_ENABLE_THREAD_VOID)
  #define THINKOS_THREAD_VOID ((THINKOS_THREADS_MAX) + 1)
#endif

#if (THINKOS_ENABLE_THREAD_VOID)
#define THINKOS_NRT_THREAD0 (THINKOS_THREADS_MAX + 2)
#else
#define THINKOS_NRT_THREAD0 (THINKOS_THREADS_MAX + 1)
#endif

#define THINKOS_CYCCNT_IDLE (THINKOS_THREADS_MAX)

/* -------------------------------------------------------------------------- 
 * Monitor structure offsets (used in assembler code)
  * --------------------------------------------------------------------------*/
#define MONITOR_CTX_OFFS  (0) 
#define MONITOR_EVS_OFFS  (MONITOR_CTX_OFFS  + 4) 
#define MONITOR_MSK_OFFS  (MONITOR_EVS_OFFS  + 4) 

/* -------------------------------------------------------------------------- 
 * ThinkOS RT structure offsets (used in assembler code)
  * --------------------------------------------------------------------------*/
#if (THINKOS_ENABLE_THREAD_VOID)
  #define THINKOS_CTX_CNT ((THINKOS_THREADS_MAX) + 2)
  #define SIZEOF_VOID_CTX  4
  #define SIZEOF_CTX (((THINKOS_THREADS_MAX) + 2) * 4)
#else
  #define THINKOS_CTX_CNT ((THINKOS_THREADS_MAX) + 1)
  #define SIZEOF_VOID_CTX  0
  #define SIZEOF_CTX       (((THINKOS_THREADS_MAX) + 1) * 4)
#endif

#define SIZEOF_NRT_CTX     ((THINKOS_NRT_THREADS_MAX) * 4)

#if (THINKOS_ENABLE_STACK_LIMIT)
  #define SIZEOF_TH_SL     (THINKOS_CTX_CNT * 4)
#else
  #define SIZEOF_TH_SL     0
#endif

#if (THINKOS_ENABLE_PROFILING)
  #define SIZEOF_CYCCNT    (THINKOS_CTX_CNT * 4)
  #define SIZEOF_CYCREF    4
#else
  #define SIZEOF_CYCCNT    0
  #define SIZEOF_CYCREF    0
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
  #define SIZEOF_XCPT_IPSR 2
  #define SIZEOF_STEP_ID   1
  #define SIZEOF_BREAK_ID  1
  #if THINKOS_ENABLE_DEBUG_STEP
    #define SIZEOF_STEP_REQ  4
    #define SIZEOF_STEP_SVC  4
  #else
    #define SIZEOF_STEP_REQ  0
    #define SIZEOF_STEP_SVC  0
  #endif
#else
  #define SIZEOF_XCPT_IPSR 0
  #define SIZEOF_STEP_ID   0
  #define SIZEOF_BREAK_ID  0
  #define SIZEOF_STEP_REQ  0
  #define SIZEOF_STEP_SVC  0
#endif

#define THINKOS_RT_IDLE_CTX_OFFS   (4 * (THINKOS_THREADS_MAX))
#define THINKOS_RT_VOID_CTX_OFFS   (THINKOS_RT_IDLE_CTX_OFFS + 4)
#define THINKOS_RT_NRT_CTX_OFFS    (THINKOS_RT_VOID_CTX_OFFS + SIZEOF_VOID_CTX)
#define THINKOS_RT_TH_SL_OFFS      (THINKOS_RT_NRT_CTX_OFFS + SIZEOF_NRT_CTX)
#define THINKOS_RT_CYCCNT_OFFS     (THINKOS_RT_TH_SL_OFFS + SIZEOF_TH_SL)
#define THINKOS_RT_CRITCNT_OFFS    (THINKOS_RT_CYCCNT_OFFS + SIZEOF_CYCCNT)
#define THINKOS_RT_XCPT_IPSR_OFFS  (THINKOS_RT_CRITCNT_OFFS + SIZEOF_CRITCNT)
#define THINKOS_RT_STEP_ID_OFFS    (THINKOS_RT_XCPT_IPSR_OFFS + \
									SIZEOF_XCPT_IPSR)
#define THINKOS_RT_BREAK_ID_OFFS   (THINKOS_RT_STEP_ID_OFFS + SIZEOF_STEP_ID)
#define THINKOS_RT_STEP_SVC_OFFS   (THINKOS_RT_BREAK_ID_OFFS + SIZEOF_BREAK_ID)
#define THINKOS_RT_STEP_REQ_OFFS   (THINKOS_RT_STEP_SVC_OFFS + SIZEOF_STEP_SVC)
#define THINKOS_RT_CYCREF_OFFS     (THINKOS_RT_STEP_REQ_OFFS + SIZEOF_STEP_REQ)
#define THINKOS_RT_ACTIVE_OFFS     ((THINKOS_RT_CYCREF_OFFS) + SIZEOF_CYCREF)
#define THINKOS_RT_READY_OFFS      ((THINKOS_RT_ACTIVE_OFFS) + 4)

#if (THINKOS_ENABLE_TIMESHARE)
  #define THINKOS_WQ_TIMESHARE_CNT 1
#else
  #define THINKOS_WQ_TIMESHARE_CNT 0 
#endif

#if (THINKOS_ENABLE_CLOCK)
  #define THINKOS_WQ_CLOCK_CNT 1
#else
  #define THINKOS_WQ_CLOCK_CNT 0 
#endif

#if ((THINKOS_MUTEX_MAX) > 0)
  #define THINKOS_WQ_MUTEX_CNT (THINKOS_MUTEX_MAX)
#else
  #define THINKOS_WQ_MUTEX_CNT 0 
#endif

#if ((THINKOS_COND_MAX) > 0)
  #define THINKOS_WQ_COND_CNT (THINKOS_COND_MAX)
#else
  #define THINKOS_WQ_COND_CNT 0 
#endif

#if ((THINKOS_SEMAPHORE_MAX) > 0)
  #define THINKOS_WQ_SEMAPHORE_CNT (THINKOS_SEMAPHORE_MAX)
#else
  #define THINKOS_WQ_SEMAPHORE_CNT 0 
#endif

#if ((THINKOS_EVENT_MAX) > 0)
  #define THINKOS_WQ_EVENT_CNT (THINKOS_EVENT_MAX)
#else
  #define THINKOS_WQ_EVENT_CNT 0 
#endif

#if ((THINKOS_FLAG_MAX) > 0)
  #define THINKOS_WQ_FLAG_CNT (THINKOS_FLAG_MAX)
#else
  #define THINKOS_WQ_FLAG_CNT 0 
#endif

#if ((THINKOS_GATE_MAX) > 0)
  #define THINKOS_WQ_GATE_CNT (THINKOS_GATE_MAX)
#else
  #define THINKOS_WQ_GATE_CNT 0 
#endif

#if (THINKOS_ENABLE_JOIN)
  #define THINKOS_WQ_JOIN_CNT (THINKOS_THREADS_MAX)
#else
  #define THINKOS_WQ_JOIN_CNT 0 
#endif

#if (THINKOS_ENABLE_CONSOLE)
  #define THINKOS_WQ_CONSOLE_CNT 2
#else
  #define THINKOS_WQ_CONSOLE_CNT 0 
#endif

#if (THINKOS_ENABLE_PAUSE)
  #define THINKOS_WQ_PAUSED_CNT 1
#else
  #define THINKOS_WQ_PAUSED_CNT 0 
#endif

#if (THINKOS_ENABLE_JOIN)
  #define THINKOS_WQ_CANCELED_CNT 1
#else
  #define THINKOS_WQ_CANCELED_CNT 0 
#endif

#if (THINKOS_ENABLE_COMM)
  #define THINKOS_WQ_COMM_CNT 2
#else
  #define THINKOS_WQ_COMM_CNT 0 
#endif

#if (THINKOS_ENABLE_WQ_IRQ)
  #define THINKOS_WQ_IRQ_CNT 1
#else
  #define THINKOS_WQ_IRQ_CNT 0 
#endif

#if (THINKOS_ENABLE_WQ_DMA)
  #define THINKOS_WQ_DMA_CNT 1
#else
  #define THINKOS_WQ_DMA_CNT 0 
#endif

#if (THINKOS_FLASH_MEM_MAX > 0)
  #define THINKOS_WQ_FLASH_MEM_CNT (THINKOS_FLASH_MEM_MAX)
#else
  #define THINKOS_WQ_FLASH_MEM_CNT 0 
#endif


#if (THINKOS_ENABLE_DEBUG_FAULT)
  #define THINKOS_WQ_FAULT_CNT 1
#else
  #define THINKOS_WQ_FAULT_CNT 0 
#endif

#define THINKOS_WQ_CNT (1 + \
  THINKOS_WQ_TIMESHARE_CNT + \
  THINKOS_WQ_CLOCK_CNT + \
  THINKOS_WQ_MUTEX_CNT + \
  THINKOS_WQ_COND_CNT + \
  THINKOS_WQ_SEMAPHORE_CNT + \
  THINKOS_WQ_EVENT_CNT + \
  THINKOS_WQ_FLAG_CNT + \
  THINKOS_WQ_GATE_CNT + \
  THINKOS_WQ_JOIN_CNT + \
  THINKOS_WQ_CONSOLE_CNT + \
  THINKOS_WQ_PAUSED_CNT + \
  THINKOS_WQ_CANCELED_CNT + \
  THINKOS_WQ_COMM_CNT + \
  THINKOS_WQ_IRQ_CNT + \
  THINKOS_WQ_DMA_CNT + \
  THINKOS_WQ_FLASH_MEM_CNT + \
  THINKOS_WQ_FAULT_CNT)

/* Mark for kernel breakpoint numbers. Breakpoints above this
   number are considered errors. */
#define THINKOS_BKPT_EXCEPT_OFF 128

#define THINKOS_ERROR_BKPT(_CODE_) ((THINKOS_BKPT_EXCEPT_OFF) + (_CODE_))

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* -------------------------------------------------------------------------- 
 * Monitor control structure
 * --------------------------------------------------------------------------*/

struct thinkos_monitor { 
	volatile uintptr_t ctl; /* control: semaphore/context pointer [PSP] */
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

#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	uint32_t sp; /* 8 bytes alignment (used for sanity check) */
	uint32_t ret; /* return value <LR> */
#endif

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

#if THINKOS_ENABLE_MPU
struct thinkos_mpu_block {
	uint16_t offs;
	uint16_t size;
};
#endif

/* -------------------------------------------------------------------------- 
 * Run Time RTOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt {
	/* This fields must be at the beginning of this structure 
	   and their order and sizes must not be changed.
	   This is critical for the scheduler operation. */
	/* Thread context pointers */
//	struct thinkos_context * ctx[THINKOS_CTX_CNT]; 
	uintptr_t ctx[THINKOS_CTX_CNT]; 

#if (THINKOS_NRT_THREADS_MAX > 0)
	struct thinkos_context * nrt_ctx[THINKOS_NRT_THREADS_MAX]; 
#endif

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

#if (THINKOS_ENABLE_CRITICAL)
	uint32_t critical_cnt; /* critical section entry counter, if not zero,
							 thread preemption is disabled */
#endif

#if (THINKOS_ENABLE_DEBUG_BKPT)
	uint16_t xcpt_ipsr; /* Exception IPSR */
	int8_t   step_id;   /* current stepping thread id */
	int8_t   res;  /* reserved */
  #if (THINKOS_ENABLE_DEBUG_STEP)
	uint32_t step_svc;  /* step at service call bitmap */
	uint32_t step_req;  /* step request bitmap */
  #endif
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

#if (THINKOS_MUTEX_MAX > 0)
			uint32_t wq_mutex[THINKOS_MUTEX_MAX];
#endif /* THINKOS_MUTEX_MAX > 0 */

#if (THINKOS_COND_MAX > 0)
			uint32_t wq_cond[THINKOS_COND_MAX];
#endif /* THINKOS_COND_MAX > 0 */

#if (THINKOS_SEMAPHORE_MAX > 0)
			uint32_t wq_sem[THINKOS_SEMAPHORE_MAX];
#endif /* THINKOS_SEMAPHORE_MAX > 0 */

#if (THINKOS_EVENT_MAX > 0)
			uint32_t wq_event[THINKOS_EVENT_MAX]; /* event sets wait queues */
#endif /* THINKOS_EVENT_MAX > 0 */

#if (THINKOS_FLAG_MAX > 0)
			uint32_t wq_flag[THINKOS_FLAG_MAX]; /* flags wait queues */
#endif /* THINKOS_FLAG_MAX > 0 */

#if (THINKOS_GATE_MAX > 0)
			uint32_t wq_gate[THINKOS_GATE_MAX]; /* gates wait queues */
#endif /* THINKOS_GATE_MAX > 0 */

#if (THINKOS_ENABLE_JOIN)
			uint32_t wq_join[THINKOS_THREADS_MAX];
#endif /* THINKOS_ENABLE_JOIN */

#if (THINKOS_ENABLE_CONSOLE)
			uint32_t wq_console_wr;
			uint32_t wq_console_rd;
#endif

#if (THINKOS_ENABLE_PAUSE)
			uint32_t wq_paused;
#endif

#if (THINKOS_ENABLE_JOIN)
			uint32_t wq_canceled; /* canceled threads wait queue */
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

#if (THINKOS_FLASH_MEM_MAX > 0) 
			uint32_t wq_flash_mem[THINKOS_FLASH_MEM_MAX];
#endif

#if (THINKOS_ENABLE_DEBUG_FAULT)
			uint32_t wq_fault; /* fault threads wait queue */
#endif
		};
	};

#if (THINKOS_ENABLE_THREAD_STAT)
	uint16_t th_stat[THINKOS_THREADS_MAX]; /* Per thread status */
#endif

#if (THINKOS_ENABLE_CLOCK)
	struct {
		uint32_t ticks;
		/* This fields are used for time wait (e.g. sleep()) */
		uint32_t clock[THINKOS_THREADS_MAX];
  #if (THINKOS_ENABLE_MONITOR_CLOCK)
		/* monitor timer */
		uint32_t monitor_clock;
  #endif
	};
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


#if (THINKOS_SEMAPHORE_MAX > 0)
	uint32_t sem_val[THINKOS_SEMAPHORE_MAX];
#endif /* THINKOS_SEMAPHORE_MAX > 0 */

#if (THINKOS_EVENT_MAX > 0)
	struct {
		uint32_t pend; /* event set pending bitmap */
		uint32_t mask; /* event set mask */
	} ev[THINKOS_EVENT_MAX];
#endif /* THINKOS_EVENT_MAX > 0 */

#if (THINKOS_FLAG_MAX > 0)
	uint32_t flag[(THINKOS_FLAG_MAX + 31) / 32]; /* flag signal */
#endif /* THINKOS_FLAG_MAX > 0 */

#if (THINKOS_GATE_MAX > 0)
	/* gate bitmap, each gate takes two bits: 
	   1 - signal the gate is open or signaled to be open, 
	   2 - the gate is locked and can't be oepn. */
	uint32_t gate[(THINKOS_GATE_MAX + 15) / 16]; /* gates states */
#endif /* THINKOS_GATE_MAX > 0 */

#if (THINKOS_MUTEX_MAX > 0)
	int8_t lock[THINKOS_MUTEX_MAX];
#endif /* THINKOS_MUTEX_MAX > 0 */

#if (THINKOS_IRQ_MAX > 0)
	int8_t irq_th[THINKOS_IRQ_MAX];
#endif /* THINKOS_IRQ_MAX */

#if THINKOS_ENABLE_THREAD_ALLOC
	uint32_t th_alloc[1];
#endif

#if THINKOS_ENABLE_MUTEX_ALLOC
	uint32_t mutex_alloc[(THINKOS_MUTEX_MAX + 31) / 32];
#endif

#if THINKOS_ENABLE_COND_ALLOC
	uint32_t cond_alloc[(THINKOS_COND_MAX + 31) / 32];
#endif

#if THINKOS_ENABLE_SEM_ALLOC
	uint32_t sem_alloc[(THINKOS_SEMAPHORE_MAX + 31) / 32];
#endif

#if THINKOS_ENABLE_EVENT_ALLOC
	uint32_t ev_alloc[(THINKOS_EVENT_MAX + 31) / 32];
#endif

#if THINKOS_ENABLE_FLAG_ALLOC
	uint32_t flag_alloc[(THINKOS_FLAG_MAX + 31) / 32];
#endif

#if THINKOS_ENABLE_GATE_ALLOC
	uint32_t gate_alloc[(THINKOS_GATE_MAX + 31) / 32];
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * th_inf[THINKOS_CTX_CNT];
#endif
#if (THINKOS_FLASH_MEM_MAX > 0)
	struct thinkos_flash_drv flash_drv[THINKOS_FLASH_MEM_MAX];
#endif
#if (THINKOS_ENABLE_IDLE_HOOKS)
	struct thinkos_idle_rt idle_hooks;
#endif
#if (THINKOS_ENABLE_MPU)
	struct { 
		/* Kernel protected memory block descriptor */
		struct thinkos_mpu_block kernel_mem;
	} mpu;
#endif
#if (THINKOS_ENABLE_MONITOR)
	struct thinkos_monitor monitor;
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
	uint8_t id: 7;
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

/* Offset in the error assignment to allow for system exceptions */
#define THINKOS_ERR_OFF    16

/* -------------------------------------------------------------------------- 
 * Idle thread
 * --------------------------------------------------------------------------*/

extern struct thinkos_rt thinkos_rt;

extern uint32_t * const thinkos_obj_alloc_lut[];

extern const uint16_t thinkos_wq_base_lut[];

extern const char thinkos_type_name_lut[][6];

extern const char thinkos_type_prefix_lut[];

extern const char __xcpt_name_lut[16][12];

#if (THINKOS_ENABLE_THREAD_INFO)
extern const struct thinkos_thread_inf thinkos_main_inf;
#endif

extern uint32_t * const thinkos_main_stack;

#ifdef __cplusplus
extern "C" {
#endif

void __attribute__((noreturn)) __thinkos_thread_terminate_stub(int code);

void __attribute__((noreturn)) __thinkos_thread_exit_stub(int code);

void __thinkos_thread_abort(unsigned int thread_id);

/* -------------------------------------------------------------------------- 
 * Support Functions
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_ERROR_TRAP)
  #define __THINKOS_ERROR(__CODE) \
	  asm volatile ("nop\n" \
					"udf %0\n" : : "I" (THINKOS_ERROR_BKPT(__CODE)))
#else
  #define __THINKOS_ERROR(__CODE)
#endif

/* set a bit in a bit map atomically */
static inline void __attribute__((always_inline)) 
thinkos_bit_set(void * bmp, unsigned int bit)
{
	__bit_mem_wr(bmp, bit, 1);  
}

/* clear a bit in a bit map atomically */
static inline void __attribute__((always_inline)) 
thinkos_bit_clr(void * bmp, unsigned int bit)
{
	__bit_mem_wr(bmp, bit, 0);  
}

extern uint32_t thinkos_ffs(uint32_t x);

static inline uint32_t __attribute__((always_inline)) __thinkos_ffs(uint32_t x)
{
#if (__ARM_ARCH == 6)
	return thinkos_ffs(x);
#else
	return __clz(__rbit(x));
#endif
}

/* -------------------------------------------------------------------------- 
 * ThinkOS run time access functions
 * --------------------------------------------------------------------------*/

static inline void __attribute__((always_inline)) 
__thinkos_active_set(unsigned int th) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	thinkos_rt.active = thinkos_rt.th_sl[th];
#else
	thinkos_rt.active = th;
#endif
}

static inline unsigned int __attribute__((always_inline)) 
__thinkos_rt_active_get(struct thinkos_rt * rt) {
	return rt->active & 0x0000003f;
}

static inline unsigned int __attribute__((always_inline)) 
__thinkos_active_get() {
	return thinkos_rt.active & 0x0000003f;
}

static inline unsigned int __attribute__((always_inline)) 
__thinkos_active_sl_get(unsigned th, uint32_t sl) {
	return thinkos_rt.active & 0xffffffc0;
}

/* Set the active thread and stack limit */
static inline void __attribute__((always_inline)) 
__thinkos_active_sl_set(unsigned int th, uint32_t sl) {
	thinkos_rt.active = (sl & 0xffffffc0) + th;
}

/* -------------------------------------------------------------------------- 
 * thread status access methods 
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_THREAD_STAT)

static inline void __attribute__((always_inline)) 
__thinkos_thread_stat_clr(unsigned int th) {
	thinkos_rt.th_stat[th] = 0;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_stat_set(unsigned int th, unsigned int wq, bool tmd) {
	thinkos_rt.th_stat[th] = (wq << 1) + (tmd ? 1 : 0);
}

static inline unsigned int __attribute__((always_inline)) 
__thinkos_thread_stat_wq_get(unsigned int th) {
	return (((uint32_t)thinkos_rt.th_stat[th]) >> 1);
}

static inline bool __attribute__((always_inline)) 
__thinkos_thread_stat_tmd_get(unsigned int th) {
	return (thinkos_rt.th_stat[th] & 1) ? true : false;
}
#else /* THINKOS_ENABLE_THREAD_STAT */
static inline void __attribute__((always_inline)) 
__thinkos_thread_stat_clr(unsigned int th) {
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_stat_set(unsigned int th, unsigned int wq, bool tmd) {
}

static inline unsigned int __attribute__((always_inline)) 
__thinkos_thread_stat_wq_get(unsigned int th) {
	return 0;
}

static inline bool __attribute__((always_inline)) 
__thinkos_thread_stat_tmd_get(unsigned int th) {
	return 0;
}
#endif /* THINKOS_ENABLE_THREAD_STAT */


/* -------------------------------------------------------------------------- 
 * thread stack limit access methods 
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_STACK_LIMIT)
static inline void __attribute__((always_inline)) 
__thinkos_thread_sl_clr(unsigned int th) {
	thinkos_rt.th_sl[th] = th;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_sl_set(unsigned int th, uint32_t addr) {
	thinkos_rt.th_sl[th] = (addr & 0xffffffc0) + th;
}
#endif /* THINKOS_ENABLE_STACK_LIMIT */

/* -------------------------------------------------------------------------- 
 * context structure access methods 
 * --------------------------------------------------------------------------*/

static inline struct thinkos_context * __attribute__((always_inline)) 
__thread_ctx_get(struct thinkos_rt * rt, unsigned int id) {
		uint32_t x = rt->ctx[id] & ~0x07;
		return (struct thinkos_context *)x;
}

static inline int __attribute__((always_inline)) 
__thread_stat_get(struct thinkos_rt * rt, unsigned int id) {
		return rt->th_stat[id] >> 1;
}

/* -------------------------------------------------------------------------- 
 * thread context access methods 
 * --------------------------------------------------------------------------*/

static inline bool __attribute__((always_inline)) 
__thinkos_thread_ctx_is_valid(unsigned int th) {
	return (thinkos_rt.ctx[th] & ~0x7) == 0 ? false : true;
}

static inline struct thinkos_context * __attribute__((always_inline)) 
__thinkos_thread_ctx_get(unsigned int th) {
	return (struct thinkos_context *)(thinkos_rt.ctx[th] & ~0x7);
}

static inline uint32_t __attribute__((always_inline)) 
__thinkos_thread_r0_get(unsigned int th) {
	return __thinkos_thread_ctx_get(th)->r0;
}

static inline uint32_t * __attribute__((always_inline)) 
__thinkos_thread_frame_get(unsigned int th) {
	return (uint32_t *)&__thinkos_thread_ctx_get(th)->r0;
}

static inline uint32_t __attribute__((always_inline)) 
__thinkos_thread_pc_get(unsigned int th) {
	return __thinkos_thread_ctx_get(th)->pc;
}

static inline uint32_t __attribute__((always_inline)) 
__thinkos_thread_lr_get(unsigned int th) {
	return __thinkos_thread_ctx_get(th)->pc;
}

static inline uint32_t __attribute__((always_inline)) 
__thinkos_thread_sp_get(unsigned int th) {
	return thinkos_rt.ctx[th] & ~0x7;
}

static inline uint32_t __attribute__((always_inline)) 
__thinkos_thread_ctrl_get(unsigned int th) {
	return thinkos_rt.ctx[th] & 0x7;
}


#if 0
static inline void  __attribute__((always_inline)) 
__thinkos_thread_ctx_set(unsigned int th, struct thinkos_context * __ctx) {
	thinkos_rt.ctx[th] = (uintptr_t)__ctx;
}

void thinkos_flash_mem_svc(int32_t arg[], int self)
	thinkos_rt.ctx[th] = (uintptr_t)__ctx;
	__thinkos_thread_ctx_set(self, (struct thinkos_context *)&arg[-CTX_R0],

#endif


static inline void  __attribute__((always_inline)) 
__thinkos_thread_ctx_set(unsigned int th, struct thinkos_context * __ctx,
						 unsigned int ctrl) {
	thinkos_rt.ctx[th] = ((uintptr_t)__ctx) | (ctrl & 0x7);
	
}

static inline void  __attribute__((always_inline)) 
__thinkos_thread_ctx_flush(int32_t arg[], unsigned int th) {
	uintptr_t ctx = (uintptr_t)(thinkos_rt.ctx[th]);
	uintptr_t sp = (uintptr_t)&arg[-CTX_R0];
	thinkos_rt.ctx[th] = sp + (ctx & 0x7);
}

static inline void  __attribute__((always_inline)) 
__thinkos_thread_ctx_clr(unsigned int th) {
	thinkos_rt.ctx[th] = (uintptr_t)0;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_r0_set(unsigned int th, uint32_t val) {
	__thinkos_thread_ctx_get(th)->r0 = val;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_r1_set(unsigned int th, uint32_t val) {
	__thinkos_thread_ctx_get(th)->r1 = val;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_r2_set(unsigned int th, uint32_t val) {
	__thinkos_thread_ctx_get(th)->r2 = val;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_r3_set(unsigned int th, uint32_t val) {
	__thinkos_thread_ctx_get(th)->r3 = val;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_pc_set(unsigned int th, uintptr_t val) {
	__thinkos_thread_ctx_get(th)->pc = (uint32_t)val;
}

static inline void __attribute__((always_inline)) 
__thinkos_thread_lr_set(unsigned int th, uintptr_t val) {
	__thinkos_thread_ctx_get(th)->lr = (uint32_t)val;
}

/* -------------------------------------------------------------------------- 
 * thread info access methods 
 * --------------------------------------------------------------------------*/

#if THINKOS_ENABLE_THREAD_INFO
static inline void __attribute__((always_inline)) 
__thinkos_thread_inf_clr(unsigned int th) {
	thinkos_rt.th_inf[th] = NULL;
}

static inline void __attribute__((always_inline))
__thinkos_thread_inf_set(unsigned int th, 
						 const struct thinkos_thread_inf * inf) {
	if (__thinkos_thread_ctx_is_valid(th))
		thinkos_rt.th_inf[th] = inf;
}

static inline const struct thinkos_thread_inf * __attribute__((always_inline))  
__thinkos_thread_inf_get(unsigned int th) {
	return thinkos_rt.th_inf[th];
}
#else /* THINKOS_ENABLE_THREAD_INFO */
static inline void __attribute__((always_inline)) 
__thinkos_thread_inf_clr(unsigned int th) {
}

static inline void __attribute__((always_inline))
__thinkos_thread_inf_set(unsigned int th, 
						 const struct thinkos_thread_inf * inf) {
}

static inline const struct thinkos_thread_inf * __attribute__((always_inline))  
__thinkos_thread_inf_get(unsigned int th) {
	return NULL;
}
#endif /* THINKOS_ENABLE_THREAD_INFO */

static inline void __attribute__((always_inline)) __thinkos_cancel_sched(void) {
	struct cm3_scb * scb = CM3_SCB;
	/* removes the pending status of the PendSV exception */
	scb->icsr = SCB_ICSR_PENDSVCLR;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

/* flags a deferred execution of the scheduler */
static inline void __attribute__((always_inline)) __thinkos_defer_sched(void) {
	struct cm3_scb * scb = CM3_SCB;
	/* rise a pending service interrupt */
	scb->icsr = SCB_ICSR_PENDSVSET;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

/* flags a deferred execution of the scheduler */
static inline void __attribute__((always_inline)) __thinkos_preempt(void) {
#if (THINKOS_ENABLE_PREEMPTION)
	__thinkos_defer_sched();
#endif
}

static inline void __attribute__((always_inline)) __thinkos_ready_clr(void) {
	thinkos_rt.wq_ready = 0;
#if (THINKOS_ENABLE_TIMESHARE)
	thinkos_rt.wq_tmshare = 0;
#endif
}

static inline void __attribute__((always_inline)) 
	__thinkos_suspend(int thread) {
#if !(THINKOS_ENABLE_TIMESHARE)
	/* remove from the ready wait queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread, 0);  
#else
	uint32_t ready;
	uint32_t tmshare;
	do {
		ready = __ldrex(&thinkos_rt.wq_ready);
		/* remove from the ready wait queue */
		ready &= ~(1 << thread);
		/* if the ready queue is empty, collect
		   the threads from the CPU wait queue */
		if (ready == 0) {
			/* no more threads into the ready queue,
			   move the timeshare queue to the ready queue */
			ready |= tmshare;
			tmshare = 0;
		} 
	} while (__strex(&thinkos_rt.wq_ready, ready));
	tmshare = thinkos_rt.wq_tmshare;
	thinkos_rt.wq_tmshare = tmshare;
#endif /* (!THINKOS_ENABLE_TIMESHARE) */
}

static inline int __attribute__((always_inline)) __wq_idx(uint32_t * ptr) {
	return ptr - thinkos_rt.wq_lst;
}

static inline int __attribute__((always_inline)) 
__thinkos_wq_head(unsigned int wq) {
	/* get a thread from the queue bitmap */
	return __thinkos_ffs(thinkos_rt.wq_lst[wq]);
}

static inline void __attribute__((always_inline)) 
__thinkos_wq_insert(unsigned int wq, unsigned int th) {
	/* insert into the event wait queue */
	__bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	thinkos_rt.th_stat[th] = wq << 1;
#endif
}

#if THINKOS_ENABLE_TIMED_CALLS
static inline void __attribute__((always_inline)) 
__thinkos_tmdwq_insert(unsigned int wq, unsigned int th, unsigned int ms) {
	/* set the clock */
	thinkos_rt.clock[th] = thinkos_rt.ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, th, 1);  
	/* insert into the event wait queue */
	__bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 1);
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	thinkos_rt.th_stat[th] = (wq << 1) + 1;
#endif
}
#endif

#if THINKOS_ENABLE_TIMED_CALLS
static inline void __attribute__((always_inline)) 
	__thinkos_wq_clock_insert(unsigned int th, unsigned int ms) {
	/* set the clock */
	thinkos_rt.clock[th] = thinkos_rt.ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, th, 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	thinkos_rt.th_stat[th] = 1;
#endif
	}
#endif

static inline void __attribute__((always_inline)) 
__thinkos_wq_remove(unsigned int wq, unsigned int th) {
	/* remove from the wait queue */
	__bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, th, 0);  
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	thinkos_rt.th_stat[th] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__thinkos_wakeup(unsigned int wq, unsigned int th) {
	/* insert the thread into ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, th, 1);
	/* remove from the wait queue */
	__bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, th, 0);  
	/* set the thread's return value */
	__thinkos_thread_ctx_get(th)->r0 = 0;
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	thinkos_rt.th_stat[th] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__thinkos_wakeup_return(unsigned int wq, unsigned int th, int ret) {
	/* insert the thread into ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, th, 1);
	/* remove from the wait queue */
	__bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, th, 0);  
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	thinkos_rt.th_stat[th] = 0;
#endif
	/* set the thread's return value */
	__thinkos_thread_ctx_get(th)->r0 = ret;
}

#if (THINKOS_ENABLE_CLOCK)
static inline uint32_t __attribute__((always_inline)) __thinkos_ticks(void) {
	return thinkos_rt.ticks;
}
#endif

/* Set the fault flag */
#if (THINKOS_ENABLE_DEBUG_FAULT)
static inline void __thinkos_thread_fault_set(unsigned int th) {
	__bit_mem_wr(&thinkos_rt.wq_fault, th, 1);
#if (THINKOS_ENABLE_THREAD_STAT)
	thinkos_rt.th_stat[th] = THINKOS_WQ_FAULT << 1;
#endif
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, th, 0);  
#endif
}

/* Clear the fault flag */
static inline void __thinkos_thread_fault_clr(unsigned int th) {
	__bit_mem_wr(&thinkos_rt.wq_fault, th, 0);
}
#endif

/* Set the pause flag */
#if (THINKOS_ENABLE_PAUSE)
static inline void __thinkos_thread_pause_set(unsigned int th) {
	__bit_mem_wr(&thinkos_rt.wq_paused, th, 1);
}

/* Clear the pause flag */
static inline void __thinkos_thread_pause_clr(unsigned int th) {
	__bit_mem_wr(&thinkos_rt.wq_paused, th, 0);
}
#endif


void thinkos_trace_rt(struct thinkos_rt * rt);

int thinkos_obj_type_get(unsigned int oid);

void __thinkos_bmp_init(uint32_t bmp[], int bits);

int __thinkos_bmp_alloc(uint32_t bmp[], int bits);

int __thinkos_thread_alloc(int target_id);

struct thinkos_context * __thinkos_thread_ctx_init(unsigned int thread_id, 
                                               uintptr_t sp, 
                                               uintptr_t task,
                                               uintptr_t arg);

bool __thinkos_thread_resume(unsigned int thread_id);

bool __thinkos_thread_pause(unsigned int thread_id);

bool __thinkos_thread_isalive(unsigned int thread_id);

bool __thinkos_thread_ispaused(unsigned int thread_id);

bool __thinkos_thread_isfaulty(unsigned int thread_id);

void __thinkos_thread_inf_set(unsigned int thread_id, 
							  const struct thinkos_thread_inf * inf);

void __thinkos_irq_reset_all(void);

void __thinkos_kill_all(void);

void __thinkos_pause_all(void);

void __thinkos_resume_all(void);

int __thinkos_thread_getnext(int th);

bool __thinkos_active(void);

bool __thinkos_suspended(void);

void __thinkos_memcpy(void * __dst, const void * __src,  
					  unsigned int __len);

void __thinkos_memcpy32(void * __dst, const void * __src,  
						unsigned int __len);

void __thinkos_memset32(void * __dst, uint32_t __val, unsigned int __len);

unsigned int __thinkos_strlen(const char * __s, unsigned int __max);

void __thinkos_core_reset(void);

void __thinkos_system_reset(void);

void __thinkos_sched_stop(void);

/* -------------------------------------------------------------------------- 
 * Scheduler 
 * --------------------------------------------------------------------------*/
void __attribute__((noreturn))
	thinkos_sched_context_restore(struct thinkos_context * __ctx, 
								  uint32_t __new_thread_id);

void __thinkos_sem_post(uint32_t wq);

void __thinkos_ev_raise(uint32_t wq, int ev);

void __thinkos_gate_open(uint32_t wq);

void __thinkos_ev_info(unsigned int wq);


bool thinkos_sched_active(void);
	
bool thinkos_syscall_active(void);

bool thinkos_clock_active(void);

bool thinkos_dbgmon_active(void);

bool thinkos_kernel_active(void);

/* -------------------------------------------------------------------------
 * Kernel Debug
 * ------------------------------------------------------------------------- */

void __context(uintptr_t __sp_ctl, uint32_t __thread_id);
void __trace(uintptr_t __sp_ctl, uint32_t __thread_id);
void __thinkos(struct thinkos_rt * rt);
void __profile(void);

/* -------------------------------------------------------------------------
 * Main thread exec
 * ------------------------------------------------------------------------- */

void __thinkos_exec(int thread_id, void (* func)(void *), 
					void * arg, bool paused);

bool __thinkos_mem_usr_rw_chk(uint32_t addr, uint32_t size);

/* -------------------------------------------------------------------------
 * System timer 
 * ------------------------------------------------------------------------- */
void __krn_systick_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_KERNEL_H__ */

