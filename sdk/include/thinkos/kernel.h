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

#define __THINKOS_COMM__
#include <thinkos/comm.h>

/* -------------------------------------------------------------------------- 
 * Context structure offsets (used in assembler code)
 * --------------------------------------------------------------------------*/

/* Position of register R0 in the context */
#define CTX_R0 8
/* Position of register PC in the context */
#define CTX_PC 14

/* size of struct thinkos_context */
#define SIZEOF_THINKOS_CONTEXT (16 * 4)


/* -------------------------------------------------------------------------- 
 * Monitor structure offsets (used in assembler code)
 * --------------------------------------------------------------------------*/
#define MONITOR_CTX_OFFS    (0) 
#define MONITOR_EVS_OFFS    (MONITOR_CTX_OFFS  + 4) 
#define MONITOR_MSK_OFFS    (MONITOR_EVS_OFFS  + 4) 

/* -------------------------------------------------------------------------- 
 * context register offsets
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_FPU)
  #define THREAD_CTRL_MSK 0x07
#else
  #define THREAD_CTRL_MSK 0x03
#endif

#if (THINKOS_ENABLE_STACK_ALIGN)
  #define STACK_ALIGN_MSK 0x00000007
#else
  #define STACK_ALIGN_MSK 0x00000003
#endif

/* -------------------------------------------------------------------------- 
 * Static trhead references
 * --------------------------------------------------------------------------*/

/* Thread number 0 is reserved. It is used as placeholder for
   thread cancelation, or can indicate "self" in some system
   calls.
*/
#define THINKOS_THREAD_VOID (0)

/* Idle thread number */
#define THINKOS_THREAD_IDLE         (1 + (THINKOS_THREADS_MAX) + \
                                     (THINKOS_NRT_THREADS_MAX))

#define THINKOS_THREAD_FIRST ((THINKOS_THREAD_VOID) + 1)

#define THINKOS_THREAD_LAST  ((THINKOS_THREAD_FIRST) + \
							  (THINKOS_THREADS_MAX) + \
							  (THINKOS_NRT_THREADS_MAX) - 1)

/* Total number of context pointers in the kernel block
   Total number of threads plus one extra for VOID   
 */
#define __KRN_THREAD_LST_SIZ        (1 + (THINKOS_THREADS_MAX) + \
                                     (THINKOS_NRT_THREADS_MAX) + 1)

/* -------------------------------------------------------------------------- 
 * ThinkOS RT structure offsets (used in assembler code)
  * --------------------------------------------------------------------------*/

#define SIZEOF_KRN_VOID_CTX         4
#define SIZEOF_KRN_TH_CTX           ((THINKOS_THREADS_MAX) * 4)
#define SIZEOF_KRN_NRT_CTX          ((THINKOS_NRT_THREADS_MAX) * 4)
#define SIZEOF_KRN_IDLE_CTX         4
#define SIZEOF_KRN_CTX              (SIZEOF_KRN_VOID_CTX + SIZEOF_KRN_TH_CTX + \
                                 SIZEOF_KRN_NRT_CTX + SIZEOF_KRN_IDLE_CTX)

#if (THINKOS_ENABLE_STACK_LIMIT)
  #define SIZEOF_KRN_TH_SL          (__KRN_THREAD_LST_SIZ * 4)
#else
  #define SIZEOF_KRN_TH_SL          0
#endif

#if (THINKOS_ENABLE_DATE_AND_TIME)
  #define SIZEOF_KRN_CLK            (__KRN_THREAD_LST_SIZ * 4) + 32
#else
  #if (THINKOS_ENABLE_FRACTIONAL_CLOCK)
    #define SIZEOF_KRN_CLK            (__KRN_THREAD_LST_SIZ * 4) + 8
  #else
    #define SIZEOF_KRN_CLK            (__KRN_THREAD_LST_SIZ * 4) + 4
  #endif
#endif

#if (THINKOS_ENABLE_PROFILING)
  #define SIZEOF_KRN_TH_CYC    (__KRN_THREAD_LST_SIZ * 4)
  #define SIZEOF_KRN_CYCREF    4
#else
  #define SIZEOF_KRN_TH_CYC    0
  #define SIZEOF_KRN_CYCREF    0
#endif

#if (THINKOS_ENABLE_CRITICAL)
  #define SIZEOF_KRN_CRITCNT   4
#else
  #define SIZEOF_KRN_CRITCNT   0
#endif

#if (THINKOS_ENABLE_TIMESHARE)
  #define SIZEOF_KRN_SCHED_LM  4
#else
  #define SIZEOF_KRN_SCHED_LM  0
#endif

#if (THINKOS_ENABLE_MONITOR_CLOCK)
  #define SIZEOF_KRN_MON_CLK 4
#else
  #define SIZEOF_KRN_MON_CLK 0
#endif

#define SIZEOF_KRN_TICKS     4
#define SIZEOF_KRN_TH_CLK    (__KRN_THREAD_LST_SIZ * 4)

#if (THINKOS_ENABLE_MONITOR)
  #define SIZEOF_KRN_MONITOR   (4 * 3)
#else
  #define SIZEOF_KRN_MONITOR   0
#endif

#if (THINKOS_ENABLE_DEBUG_BKPT)
  #if (THINKOS_ENABLE_DEBUG_STEP)
    #define SIZEOF_KRN_STEP_REQ  4
    #define SIZEOF_KRN_STEP_SVC  4
  #else
    #define SIZEOF_KRN_STEP_REQ  0
    #define SIZEOF_KRN_STEP_SVC  0
  #endif
  #define SIZEOF_KRN_XCPT_IPSR 2
  #define SIZEOF_KRN_STEP_ID   1
  #define SIZEOF_KRN_BREAK_ID  1
#else
  #define SIZEOF_KRN_STEP_REQ  0
  #define SIZEOF_KRN_STEP_SVC  0
  #define SIZEOF_KRN_XCPT_IPSR 0
  #define SIZEOF_KRN_STEP_ID   0
  #define SIZEOF_KRN_BREAK_ID  0
#endif

#if (THINKOS_ENABLE_ERROR_TRAP)
#define SIZEOF_KRN_DBG_STATUS  4
#else
#define SIZEOF_KRN_DBG_STATUS  0
#endif

#define SIZEOF_KRN_DEBUG (SIZEOF_KRN_STEP_REQ + SIZEOF_KRN_STEP_SVC +\
						  SIZEOF_KRN_XCPT_IPSR + SIZEOF_KRN_STEP_ID +\
						  SIZEOF_KRN_BREAK_ID + SIZEOF_KRN_DBG_STATUS)

#if (THINKOS_ENABLE_READY_MASK)
  #define SIZEOF_KRN_RDY_MSK 4
#else
  #define SIZEOF_KRN_RDY_MSK 0
#endif

#define SIZEOF_KRN_SCHED 4
#define SIZEOF_KRN_ACTIVE 1
#define SIZEOF_KRN_WQ_READY 4

#if (THINKOS_ENABLE_JOIN)
  #define SIZEOF_KRN_WQ_JOIN 4
#else
  #define SIZEOF_KRN_WQ_JOIN 0
#endif

#define SIZEOF_KRN_WQ_CLOCK 4

#if (THINKOS_ENABLE_TIMESHARE)
  #define SIZEOF_KRN_WQ_TMSHARE 4
#else
  #define SIZEOF_KRN_WQ_TMSHARE 0
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
  #define SIZEOF_KRN_TH_INF (__KRN_THREAD_LST_SIZ * 4)
#else
  #define SIZEOF_KRN_TH_INF 0
#endif

#if (THINKOS_ENABLE_THREAD_STAT)
  #define SIZEOF_KRN_TH_STAT (__KRN_THREAD_LST_SIZ * 2)
#else
  #define SIZEOF_KRN_TH_STAT 0
#endif

#if (THINKOS_ENABLE_THREAD_FAULT)
  #define SIZEOF_KRN_TH_ERRNO (__KRN_THREAD_LST_SIZ * 1)
#else
  #define SIZEOF_KRN_TH_ERRNO 0
#endif


#define SIZEOF_KRN_WQ_LST (THINKOS_WQ_CNT * 4) 

#define OFFSETOF_KRN_VOID_CTX   0
#define OFFSETOF_KRN_TH_CTX     (OFFSETOF_KRN_VOID_CTX + SIZEOF_KRN_VOID_CTX)
#define OFFSETOF_KRN_NRT_CTX    (OFFSETOF_KRN_TH_CTX + SIZEOF_KRN_TH_CTX)
#define OFFSETOF_KRN_IDLE_CTX   (OFFSETOF_KRN_NRT_CTX + SIZEOF_KRN_NRT_CTX)
#define OFFSETOF_KRN_SCHED      (OFFSETOF_KRN_IDLE_CTX + SIZEOF_KRN_IDLE_CTX) 
#define OFFSETOF_KRN_WQ_LST     (OFFSETOF_KRN_SCHED + SIZEOF_KRN_SCHED)
#define OFFSETOF_KRN_CLK        (OFFSETOF_KRN_WQ_LST + SIZEOF_KRN_WQ_LST)
#define OFFSETOF_KRN_TH_CYC     (OFFSETOF_KRN_CLK + SIZEOF_KRN_CLK)
#define OFFSETOF_KRN_CYCREF     (OFFSETOF_KRN_TH_CYC + SIZEOF_KRN_TH_CYC)

#define OFFSETOF_KRN_TH_SL      (OFFSETOF_KRN_CYCREF + SIZEOF_KRN_CYCREF)
#define OFFSETOF_KRN_TH_INF     (OFFSETOF_KRN_TH_SL + SIZEOF_KRN_TH_SL)
#define OFFSETOF_KRN_TH_STAT    (OFFSETOF_KRN_TH_INF + SIZEOF_KRN_TH_INF)
#define OFFSETOF_KRN_TH_ERRNO   (OFFSETOF_KRN_TH_STAT + SIZEOF_KRN_TH_STAT)

#define A4(__X) ((__X + 3) & ~3)
#define OFFSETOF_KRN_DEBUG      A4(OFFSETOF_KRN_TH_ERRNO + SIZEOF_KRN_TH_ERRNO)

#define OFFSETOF_KRN_STEP_REQ   (OFFSETOF_KRN_DEBUG)
#define OFFSETOF_KRN_STEP_SVC   (OFFSETOF_KRN_STEP_REQ + SIZEOF_KRN_STEP_REQ)
#define OFFSETOF_KRN_XCPT_IPSR  (OFFSETOF_KRN_STEP_SVC + SIZEOF_KRN_STEP_SVC)
#define OFFSETOF_KRN_STEP_ID    (OFFSETOF_KRN_XCPT_IPSR + SIZEOF_KRN_XCPT_IPSR)
#define OFFSETOF_KRN_BREAK_ID   (OFFSETOF_KRN_STEP_ID + SIZEOF_KRN_STEP_ID)
#define OFFSETOF_KRN_DBG_STATUS (OFFSETOF_KRN_BREAK_ID + SIZEOF_KRN_BREAK_ID)

#define OFFSETOF_KRN_CRITCNT    (OFFSETOF_KRN_DEBUG + SIZEOF_KRN_DEBUG)

#define OFFSETOF_KRN_RDY_MSK    (OFFSETOF_KRN_CRITCNT + SIZEOF_KRN_CRITCNT)

#define OFFSETOF_KRN_SCHED_THREAD   (OFFSETOF_KRN_SCHED)
#define OFFSETOF_KRN_SCHED_SVC      (OFFSETOF_KRN_SCHED_THREAD + 1)
#define OFFSETOF_KRN_SCHED_ERR      (OFFSETOF_KRN_SCHED_SVC + 1)
#define OFFSETOF_KRN_SCHED_XCPT     (OFFSETOF_KRN_SCHED_ERR + 1)

#define OFFSETOF_KRN_READY      (OFFSETOF_KRN_WQ_LST)
#define OFFSETOF_KRN_WQ_JOIN    (OFFSETOF_KRN_READY + SIZEOF_KRN_WQ_READY)
#define OFFSETOF_KRN_WQ_CLOCK   (OFFSETOF_KRN_WQ_JOIN + SIZEOF_KRN_WQ_JOIN)
#define OFFSETOF_KRN_WQ_TMSAHRE (OFFSETOF_KRN_READY + SIZEOF_KRN_WQ_READY)

//#define OFFSETOF_KRN_MON_CLK    
//(OFFSETOF_KRN_MON_CLK + SIZEOF_KRN_MON_CLK)

#define SCHED_STAT_XCP(__STAT__) (((__STAT__) >> 24) & 0xff)
#define SCHED_STAT_ERR(__STAT__) (((__STAT__) >> 16) & 0xff)
#define SCHED_STAT_SVC(__STAT__) (((__STAT__) >> 8) & 0xff)
#define SCHED_STAT_ACT(__STAT__) ((__STAT__) & 0xff)

#ifndef __ASSEMBLER__

#include <thinkos.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* -------------------------------------------------------------------------- 
 * Opaque declarations
 * --------------------------------------------------------------------------*/

struct thinkos_mem_map;

struct defered_svc_map;

/* -------------------------------------------------------------------------- 
 * Monitor control structure
 * --------------------------------------------------------------------------*/

struct thinkos_monitor { 
	uint32_t events;  /* event set bitmap */
	uint32_t mask;  /* events mask */
	const struct defered_svc_map * svc; /* defered services vectors */
	void * env; /* environment */
	union{
		volatile uintptr_t ctl; /* control: semaphore/context pointer [PSP] */
		uint32_t * ctx;
	};
	const struct thinkos_comm * comm;
	struct comm_tx_req * tx_req;
};

/* -------------------------------------------------------------------------- 
 * Debug status structure
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_DEBUG)
#endif /* THINKOS_ENABLE_DEBUG */

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

#define FP_CTX(CTX) (struct thinkos_fp_context *)((uintptr_t)(CTX) - (16 * 4))

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
		uintptr_t ctx[__KRN_THREAD_LST_SIZ]; 
		struct {
			uintptr_t void_ctx;
			uintptr_t th_ctx[THINKOS_THREADS_MAX]; 
#if (THINKOS_NRT_THREADS_MAX) > 0
			uintptr_t nrt_ctx[THINKOS_NRT_THREADS_MAX]; 
#endif
			uintptr_t idle_ctx;
		};
	};

	union {
		volatile uint32_t state; /* scheduler state */
		struct {
			volatile uint8_t act;    /* current active thread */
			volatile uint8_t svc;    /* deferred service request */
			volatile uint8_t err;    /* thread error number - 
										errors from syscalls */
			volatile uint8_t xcp;    /* exception error number */
		};
	} sched;

	union {
		uint32_t wq_lst[THINKOS_WQ_CNT]; /* queue list */
		struct {
			uint32_t wq_ready; /* ready threads queue */

			uint32_t wq_thread[THINKOS_THREADS_MAX];

			uint32_t wq_clock;

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

#if (THINKOS_ENABLE_TIMESHARE)
			uint32_t wq_tmshare; /* Threads waiting for time share cycle */
#endif

#if ((THINKOS_COMM_MAX) > 0)
			uint32_t wq_comm_tx[THINKOS_WQ_COMM_TX_CNT];
			uint32_t wq_comm_rx[THINKOS_WQ_COMM_RX_CNT];
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

	struct {
		/* Per thread timer. Used for time wait (e.g. sleep()) */
		uint32_t th_tmr[__KRN_THREAD_LST_SIZ];
		uint32_t time;      /* clock present value */
#if (THINKOS_ENABLE_FRACTIONAL_CLOCK)
		uint32_t increment; /* fractional per tick increment */
#endif
#if (THINKOS_ENABLE_DATE_AND_TIME)
		/* date and time fractional value */
		struct {
			uint32_t frac;
			uint32_t sec;
		} timestamp;
		struct {
			uint32_t frac;
			uint32_t sec;
		} realtime_offs;
		uint32_t resolution; /* fractional clock resolution */
		uint32_t k; /* residual clock multiplication factor */
#endif
	} clk;

#if (THINKOS_ENABLE_PROFILING)
	/* Per thread cycle count */
	uint32_t th_cyc[__KRN_THREAD_LST_SIZ]; 
	/* Reference cycle ... */
	uint32_t cycref;
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	/* Per thread stack limit */
	uint32_t th_sl[__KRN_THREAD_LST_SIZ]; 
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
	/* Per thread info block */
	const struct thinkos_thread_inf * th_inf[__KRN_THREAD_LST_SIZ];
#endif

#if (THINKOS_ENABLE_THREAD_STAT)
	/* Per thread status */
	uint16_t th_stat[__KRN_THREAD_LST_SIZ];
#endif

#if (THINKOS_ENABLE_THREAD_FAULT)
	/* Per thread error code */
	int8_t th_errno[__KRN_THREAD_LST_SIZ]; 
#endif

#if (THINKOS_ENABLE_DEBUG_BASE)
	struct {
#if (THINKOS_ENABLE_DEBUG_BKPT)
#if (THINKOS_ENABLE_DEBUG_STEP)
		uint32_t step_req;  /* step request bitmap */
		uint32_t step_svc;  /* step at service call bitmap */
#endif /* THINKOS_ENABLE_DEBUG_STEP */
		uint16_t xcpt_ipsr; /* Exception IPSR */
		int8_t   step_id;   /* current stepping thread id */
		int8_t   brk_idx;   /* break thread index */
#endif /* THINKOS_ENABLE_DEBUG_BKPT */
#if (THINKOS_ENABLE_ERROR_TRAP)
		union {
			uint32_t status;
			struct {
				uint8_t thread;   /* active thread */
				uint8_t errno;    /* error number */
				uint8_t xcptno;   /* exception number */
				uint8_t kfault;   /* kernel fault ??  */
			};
		};
#endif
	} debug;
#endif

#if (THINKOS_ENABLE_CRITICAL)
	uint32_t critical_cnt; /* critical section entry counter, if not zero,
							 thread preemption is disabled */
#endif

#if (THINKOS_ENABLE_READY_MASK)
	uint32_t rdy_msk;
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
	uint8_t mtx_lock[THINKOS_MUTEX_MAX];
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

#if ((THINKOS_FLASH_MEM_MAX) > 0)
	struct thinkos_flash_drv flash_drv[THINKOS_FLASH_MEM_MAX];
#endif
#if (THINKOS_ENABLE_MEMORY_MAP)
	const struct thinkos_mem_map * mem_map;
#endif
#if ((THINKOS_COMM_MAX) > 0)
	const struct thinkos_comm * comm[THINKOS_COMM_MAX];
#endif

#if (THINKOS_ENABLE_DATE_AND_TIME)
//	struct krn_clock time_clk;
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

#define THINKOS_THREAD_BASE ((offsetof(struct thinkos_rt, wq_thread) \
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

#define THINKOS_COMM_TX_BASE ((offsetof(struct thinkos_rt, wq_comm_tx) \
								- offsetof(struct thinkos_rt, wq_lst)) \
							   / sizeof(uint32_t))

#define THINKOS_COMM_RX_BASE ((offsetof(struct thinkos_rt, wq_comm_rx) \
								- offsetof(struct thinkos_rt, wq_lst)) \
							   / sizeof(uint32_t))

#if (THINKOS_ENABLE_WQ_IRQ)
#define THINKOS_WQ_IRQ ((offsetof(struct thinkos_rt, wq_irq) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))
#endif

#define THINKOS_WQ_DMA ((offsetof(struct thinkos_rt, wq_dma) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_FLASH_MEM_BASE ((offsetof(struct thinkos_rt, wq_flash_mem) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))

#define THINKOS_WQ_FAULT ((offsetof(struct thinkos_rt, wq_fault) \
						   - offsetof(struct thinkos_rt, wq_lst)) \
						  / sizeof(uint32_t))


#define THINKOS_MUTEX_FIRST  (THINKOS_MUTEX_BASE)
#define THINKOS_MUTEX_LAST   (THINKOS_MUTEX_FIRST + (THINKOS_MUTEX_MAX) - 1)

#define THINKOS_OBJECT_FIRST (0)
#define THINKOS_OBJECT_LAST  (THINKOS_OBJECT_FIRST + (THINKOS_WQ_CNT) - 1)

#define THINKOS_COMM_RX_FIRST  (THINKOS_COMM_RX_BASE)
#define THINKOS_COMM_RX_LAST   (THINKOS_COMM_RX_FIRST + (THINKOS_COMM_MAX) - 1)

#define THINKOS_COMM_TX_FIRST  (THINKOS_COMM_TX_BASE)
#define THINKOS_COMM_TX_LAST   (THINKOS_COMM_TX_FIRST + (THINKOS_COMM_MAX) - 1)

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

#include <stdarg.h>

extern struct thinkos_rt thinkos_rt;

#if (THINKOS_ENABLE_THREAD_INFO)
extern const struct thinkos_thread_inf thinkos_main_inf;
#endif

static inline unsigned int thinkos_krn_threads_max(void) {
	return __KRN_THREAD_LST_SIZ;
}

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

int __thinkos_strcmp(const char * __s1, const char * __s2);

void __thinkos_system_reset(void);

uint32_t __thinkos_crc32_u32(uint32_t __buf[], unsigned int __len);

uint32_t __thinkos_crc32_u8(const void * __buf, unsigned int __len); 

int krn_snprintf(char * str, size_t size, const char *fmt, ...);
int krn_vsnprintf(char * str, size_t size, const char *fmt, va_list ap);


int krn_console_dev_send(void * dev, const void * buf, unsigned int len);

int krn_console_dev_recv(void * dev, void * buf, 
					  unsigned int len, unsigned int msec);

int krn_console_write(const void * buf, unsigned int len);

int krn_console_puts(const char * s);

int krn_console_putc(int c);

int krn_console_getc(unsigned int tmo);

int krn_console_gets(char * s, int size);

int krn_console_wrln(const char * ln);

int krn_fmt_line_hex32(char * __ln, uint32_t __addr, 
					   const void * __buf, int __cnt);

/* -------------------------------------------------------------------------- 
 * kernel core functions 
 * --------------------------------------------------------------------------*/

int thinkos_krn_thread_init(struct thinkos_rt * krn,
	unsigned int thread_idx,
	const struct thinkos_thread_initializer * init);

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

void thinkos_krn_sched_off(struct thinkos_rt * krn);

void thinkos_krn_sched_on(struct thinkos_rt * krn);

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

