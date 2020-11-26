/* 
 * thinkos_kr-i.h
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

#ifndef __THINKOS_KRN_I_H__
#define __THINKOS_KRN_I_H__

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#if (THINKOS_ENABLE_MONITOR)
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#endif

#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <thinkos.h>

#include <sys/delay.h>
#include <sys/param.h>
#include <sys/sysclk.h>

#include <thinkos.h>
#include <vt100.h>
#include <sys/dcclog.h>

#define __PRIORITY(OPT)   (((OPT) >> 16) & 0xff)
#define __ID(OPT)         (((OPT) >> 24) & 0x3f)
#define __PRIVILEGED(OPT) (((OPT) >> 30) & 0x01)
#define __PAUSED(OPT)     (((OPT) >> 31) & 0x01)
#define __STACK_SIZE(OPT) ((OPT) & 0xffff)

#if (THINKOS_ENABLE_THREAD_INFO)
extern const struct thinkos_thread_inf thinkos_main_inf;
#endif

extern uintptr_t __vcts_start;
extern uintptr_t __vcts_end;

#ifdef CM3_RAM_VECTORS
extern void * __ram_vectors[];
#endif

extern int32_t udelay_factor;

extern const char thinkos_type_name_lut[][6];

extern const char thinkos_type_prefix_lut[];

extern const char __xcpt_name_lut[16][12];

extern const uint8_t thinkos_obj_kind_lut[];

extern void * __krn_stack_start;
extern void * __krn_stack_end;
extern int __krn_stack_size;


bool __thinkos_obj_alloc_check(unsigned int oid);

void __attribute__((noreturn)) __thinkos_thread_terminate_stub(int code);

void __attribute__((noreturn)) __thinkos_thread_exit_stub(int code);

/* -------------------------------------------------------------------------- 
 * Objects
 * --------------------------------------------------------------------------*/

static inline unsigned int __attribute__((always_inline))
__obj_kind(unsigned int oid) { return thinkos_obj_kind_lut[oid]; }

static inline int __attribute__((always_inline))
__kind_prefix(unsigned int kind) { return thinkos_type_prefix_lut[kind]; }

static inline const char * __attribute__((always_inline))
__kind_name(unsigned int kind) { return thinkos_type_name_lut[kind]; }

/* -------------------------------------------------------------------------- 
 * Bitmaps 
 * --------------------------------------------------------------------------*/


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
 * ThinkOS Kernel Run Time access functions
 * --------------------------------------------------------------------------*/

static inline void __attribute__((always_inline)) 
__thinkos_krn_sched_lock(struct thinkos_rt * krn) {
	/* Single CPU: do nothing */
	return;
}

static inline void __attribute__((always_inline)) 
__thinkos_krn_sched_unlock(struct thinkos_rt * krn) {
	/* Single CPU: do nothing */
	return;
}

/* Set the active thread and stack limit */
static inline unsigned int __attribute__((always_inline)) 
__thread_active_get(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	return krn->active & (STACK_ALIGN_MSK);
#else
	return krn->active;
#endif
}

static inline void __attribute__((always_inline)) 
__thread_active_set(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	if (idx < THINKOS_THREAD_IDLE)
		krn->active = krn->th_sl[idx];
	else
		krn->active = idx;
#else
	krn->active = idx;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline uint32_t __attribute__((always_inline)) 
__thread_active_sl_get(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	return krn->active & ~(STACK_ALIGN_MSK);
#else
	return 0;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline void __attribute__((always_inline)) 
__thread_active_sl_set(struct thinkos_rt * krn, uint32_t sl, unsigned int idx) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	krn->active = (sl & ~(STACK_ALIGN_MSK)) + (idx & STACK_ALIGN_MSK);
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline void __attribute__((always_inline)) 
__thread_sl_set(struct thinkos_rt * krn, unsigned int idx, uint32_t addr) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	krn->th_sl[idx] = (addr & ~(STACK_ALIGN_MSK)) + (idx & (STACK_ALIGN_MSK));
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline void __attribute__((always_inline)) 
__thread_sl_clr(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	krn->th_sl[idx] = (idx & (STACK_ALIGN_MSK));
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline uint32_t __attribute__((always_inline)) 
__thread_sl_get(struct thinkos_rt * krn, unsigned int idx) {
	uintptr_t sl;
#if (THINKOS_ENABLE_STACK_LIMIT)
	sl = (uintptr_t)krn->th_sl[idx] & ~(STACK_ALIGN_MSK);
#elif (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = krn->th_inf[idx];
	sl = (inf != NULL) ? (uintptr_t)inf->stack_ptr : 0;
#else
	sl = 0;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
	return (uint32_t)sl;
}

/* Set the fault flag */
static inline void __attribute__((always_inline)) 
	__thread_fault_set(struct thinkos_rt * krn, unsigned int idx, int errno) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	__bit_mem_wr(&krn->wq_fault, idx, 1);
#if (THINKOS_ENABLE_THREAD_STAT)
	krn->th_stat[idx] = THINKOS_WQ_FAULT << 1;
#endif
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, idx, 0);  
#endif
	krn->th_errno[idx] = errno;
#endif
}


/* Set the break thread index */
static inline void __attribute__((always_inline)) 
	__krn_brk_set(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_MONITOR)
	krn->brk_idx = idx;
#endif
}

/* Set the break thread index */
static inline int __attribute__((always_inline)) 
	__krn_brk_get(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_MONITOR)
	return krn->brk_idx;
#endif
}



/* Clear the fault flag */
static inline void __attribute__((always_inline))
__thread_fault_clr(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	thinkos_rt.th_errno[idx] = 0;
	__bit_mem_wr(&krn->wq_fault, idx, 0);
#endif
}

/* get the fault flag */
static inline bool __attribute__((always_inline))
__thread_fault_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	return __bit_mem_rd(&krn->wq_fault, idx) ? true : false;
#endif
	return false;
}


/* Set the schedule enable flag */
static inline void  __attribute__((always_inline))
__thread_enable(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_RUNMASK)
	__bit_mem_wr(&krn->runmask, idx, 1);
#endif
}

/* Clear the schedule enable flag */
static inline void  __attribute__((always_inline))
__thread_disable(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_RUNMASK)
	__bit_mem_wr(&krn->runmask, idx, 0);
#endif
}

/* Get the enable flag */
static inline bool  __attribute__((always_inline))
__thread_is_enabled(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_RUNMASK)
	return __bit_mem_rd(&krn->runmask, idx) ? true : false;
#endif
	return true;
}

static inline void  __attribute__((always_inline))
__thread_enable_all(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_RUNMASK)
	krn->runmask = ~0;
#endif
}

static inline void  __attribute__((always_inline))
__thread_disble_all(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_RUNMASK)
	krn->runmask = 0;
#endif
}

/* Set the pause flag */
static inline void  __attribute__((always_inline))
__thread_pause_set(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_PAUSE)
	__bit_mem_wr(&krn->wq_paused, idx, 1);
#endif
}

/* Clear the pause flag */
static inline void  __attribute__((always_inline))
__thread_pause_clr(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_PAUSE)
	__bit_mem_wr(&krn->wq_paused, idx, 0);
#endif
}

/* Get the pause flag */
static inline bool  __attribute__((always_inline))
__thread_pause_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_PAUSE)
	return __bit_mem_rd(&krn->wq_paused, idx) ? true : false;
#endif
	return false;
}

/* -------------------------------------------------------------------------- 
 * context structure access methods 
 * --------------------------------------------------------------------------*/

static inline void  __attribute__((always_inline)) 
__thread_ctx_set(struct thinkos_rt * krn, unsigned int idx, 
				 struct thinkos_context * ctx, unsigned int ctrl) {
	krn->ctx[idx] = ((uintptr_t)ctx) | (ctrl & (CONTROL_MSK));
}

static inline void  __attribute__((always_inline)) 
__thread_ctx_flush(struct thinkos_rt * krn, int32_t arg[], unsigned int idx) {
	uintptr_t ctx = (uintptr_t)(thinkos_rt.ctx[idx]);
	uintptr_t sp = (uintptr_t)&arg[-CTX_R0];
	krn->ctx[idx] = sp + (ctx & (CONTROL_MSK));
}
  
static inline struct thinkos_context * __attribute__((always_inline)) 
__thread_ctx_get(struct thinkos_rt * krn, unsigned int idx) {
		uintptr_t ptr = krn->ctx[idx] & ~(CONTROL_MSK);
		return (struct thinkos_context *)ptr;
}

static inline bool __attribute__((always_inline)) 
__thread_ctx_is_valid(struct thinkos_rt * krn, unsigned int idx) {
	return (krn->ctx[idx] & ~(CONTROL_MSK)) == 0 ? false : true;
}

static inline void  __attribute__((always_inline)) 
__thread_ctx_clr(struct thinkos_rt * krn, unsigned int idx) {
	thinkos_rt.ctx[idx] = (uintptr_t)0;
}

static inline uint32_t * __attribute__((always_inline)) 
__thread_frame_get(struct thinkos_rt * krn, unsigned int idx) {
	return (uint32_t *)&__thread_ctx_get(&thinkos_rt, idx)->r0;
}

static inline void  __attribute__((always_inline)) 
__thread_r0_set(struct thinkos_rt * krn, unsigned int idx, uint32_t val) {
	__thread_ctx_get(krn, idx)->r0 = val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_r0_get(struct thinkos_rt * krn, unsigned int idx) {
	return __thread_ctx_get(krn, idx)->r0;
}

static inline void  __attribute__((always_inline)) 
__thread_r1_set(struct thinkos_rt * krn, unsigned int idx, uint32_t val) {
	__thread_ctx_get(krn, idx)->r1 = val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_r1_get(struct thinkos_rt * krn, unsigned int idx) {
	return __thread_ctx_get(krn, idx)->r1;
}

static inline void  __attribute__((always_inline)) 
__thread_r2_set(struct thinkos_rt * krn, unsigned int idx, uint32_t val) {
	__thread_ctx_get(krn, idx)->r2 = val;
}

static inline void  __attribute__((always_inline)) 
__thread_r3_set(struct thinkos_rt * krn, unsigned int idx, uint32_t val) {
	__thread_ctx_get(krn, idx)->r3 = val;
}

static inline void  __attribute__((always_inline)) 
__thread_lr_set(struct thinkos_rt * krn, unsigned int idx, uintptr_t val) {
	__thread_ctx_get(krn, idx)->lr = (uint32_t)val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_lr_get(struct thinkos_rt * krn, unsigned int idx) {
	uintptr_t lr = __thread_ctx_get(krn, idx)->lr;
	return (uint32_t)lr;
}

static inline void  __attribute__((always_inline)) 
__thread_pc_set(struct thinkos_rt * krn, unsigned int idx, uintptr_t val) {
	__thread_ctx_get(krn, idx)->pc = (uint32_t)val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_pc_get(struct thinkos_rt * krn, unsigned int idx) {
	uintptr_t pc = __thread_ctx_get(krn, idx)->pc;
	return (uint32_t)pc;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_xpsr_get(struct thinkos_rt * krn, unsigned int idx) {
	uint32_t xpsr = __thread_ctx_get(krn, idx)->xpsr;
	return xpsr;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_sp_get(struct thinkos_rt * krn, unsigned int idx) {
	uint32_t sp_ctrl = krn->ctx[idx];
	uintptr_t sp = sp_ctrl & ~(CONTROL_MSK);
#if (THINKOS_ENABLE_FPU)
	uint32_t ctrl = sp_ctrl & (CONTROL_MSK);
	sp += (ctrl & CONTROL_FPCA) ? (26*4) : (8*4);
#else
	sp += (8*4);
#endif 
	return (uint32_t)sp;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_ctrl_get(struct thinkos_rt * krn, unsigned int idx) {
	uint32_t sp_ctrl = krn->ctx[idx];
	uint32_t ctrl = sp_ctrl & (CONTROL_MSK);
	return ctrl;
}

/*
=========== ============== ============= ============ ===== =====
 EXC_RETURN    Return to   Return stack   Frame type  Priv   Ctrl
=========== ============== ============= ============ ===== =====
 0xFFFFFFE1  Handler mode          Main     Extended  Err    xxx 
 0xFFFFFFE9   Thread mode          Main     Extended  Err    xxx 
 0xFFFFFFED   Thread mode       Process     Extended  Ufp    101 
 0xFFFFFFF1  Handler mode          Main        Basic  Err    xxx 
 0xFFFFFFF9   Thread mode          Main        Basic  Krn    000 
 0xFFFFFFFD   Thread mode       Process        Basic  Usr    001 
=========== ============== ============= ============ ===== =====
*/

#define CONTROL_nPRIV       (1 << 0)
#define CONTROL_SPSEL       (1 << 1)
#define CONTROL_FPCA        (1 << 2)

#define EXEC_RET_THREAD      (1 << 1)
#define EXEC_RET_SPSEL       (1 << 2)
#define EXEC_RET_nFPCA       (1 << 4)

static inline uint32_t __attribute__((always_inline)) 
__thread_exec_ret_get(struct thinkos_rt * krn, unsigned int idx) {
	uint32_t sp_ctrl = krn->ctx[idx];
	uint32_t ctrl = sp_ctrl & (CONTROL_MSK);
#if (THINKOS_ENABLE_FPU)
	return 0xffffffe1 | EXEC_RET_THREAD | 
		((ctrl & CONTROL_FPCA) ? 0 : EXEC_RET_nFPCA) | 
		((ctrl & CONTROL_SPSEL) ? EXEC_RET_SPSEL : 0);
#else
	return 0xfffffff1 | EXEC_RET_THREAD | 
		((ctrl & CONTROL_SPSEL) ? EXEC_RET_SPSEL : 0);
#endif
}

/* -------------------------------------------------------------------------- 
 * thread status access methods 
 * --------------------------------------------------------------------------*/

static inline unsigned int __attribute__((always_inline)) 
__thread_stat_wq_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_STAT)
	return (((uint32_t)krn->th_stat[th]) >> 1);
#else
	return 0;
#endif
}

static inline bool __attribute__((always_inline)) 
__thread_stat_tmw_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_STAT)
	return (thinkos_rt.th_stat[th] & 1) ? true : false;
#else
	return __bit_mem_rd(&krn->wq_clock, th) ? true : false;
#endif
}

static inline void __attribute__((always_inline)) 
__thread_stat_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_STAT)
	krn->th_stat[th] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__thread_stat_set(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int wq, bool tmd) {
#if (THINKOS_ENABLE_THREAD_STAT)
	krn->th_stat[th] = (wq << 1) + (tmd ? 1 : 0);
#endif
}

#if (THINKOS_ENABLE_THREAD_STAT)
static inline int __attribute__((always_inline)) 
___thread_stat_get(struct thinkos_rt * krn, unsigned int idx) {
		return krn->th_stat[idx];
}
#endif

int __thread_wq_lookup(struct thinkos_rt * krn, unsigned int thread_idx);

static inline int __attribute__((always_inline)) 
__thread_wq_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_STAT)
	return (krn->th_stat[idx] >> 1) & 0x1ff;
#else
	return __thread_wq_lookup(&thinkos_rt, idx);
#endif
}

static inline int __attribute__((always_inline)) 
__thread_tmw_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_TIMED_CALLS)
  #if (THINKOS_ENABLE_THREAD_STAT)
	return krn->th_stat[idx] & 1;
  #else
	return __bit_mem_rd(&krn->wq_clock, th);  
  #endif
#else
	return false;
#endif
}

static inline bool __attribute__((always_inline)) 
__thread_is_in_wq(struct thinkos_rt * krn, unsigned int idx, unsigned int wq) {
	/* is thread in wait queue */
	return __bit_mem_rd(&krn->wq_lst[wq], idx) ? true : false;  
}

static inline void __attribute__((always_inline)) 
__thread_wq_ready_insert(struct thinkos_rt * krn, unsigned int idx) {
	/* insert into the rady wait queue */
	__bit_mem_wr(&krn->wq_ready, idx, 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	krn->th_stat[idx] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__wq_ready_clr(struct thinkos_rt * krn) {
	/* clear the ready wait queue */
	krn->wq_ready = 0;
#if (THINKOS_ENABLE_TIMESHARE)
	krn->wq_tmshare = 0;
#endif
}

static inline void __attribute__((always_inline)) 
	__thread_suspend(struct thinkos_rt * krn, int idx) {
#if !(THINKOS_ENABLE_TIMESHARE)
	/* remove from the ready wait queue */
	__bit_mem_wr(&krn->wq_ready, idx, 0);  
#else
	uint32_t ready;
	uint32_t tmshare;
	do {
		ready = __ldrex(&kern->wq_ready);
		/* remove from the ready wait queue */
		ready &= ~(1 << idx);
		/* if the ready queue is empty, collect
		   the threads from the CPU wait queue */
		if (ready == 0) {
			/* no more threads into the ready queue,
			   move the timeshare queue to the ready queue */
			ready |= tmshare;
			tmshare = 0;
		} 
	} while (__strex(&kern->wq_ready, ready));
	tmshare = kern->wq_tmshare;
	kern->wq_tmshare = tmshare;
#endif /* (!THINKOS_ENABLE_TIMESHARE) */
}

static inline void __attribute__((always_inline))
__thread_priority_set(struct thinkos_rt * krn, unsigned int idx, int priority) {
#if (THINKOS_ENABLE_TIMESHARE)
	krn->sched_pri[idx] = priority;
	if (krn->sched_pri[idx] > (THINKOS_SCHED_LIMIT_MAX)) {
		krn->sched_pri[idx] = (THINKOS_SCHED_LIMIT_MAX);
	}
	/* update schedule limit */
	if (krn->sched_limit < krn->sched_pri[idx]) {
		krn->sched_limit = krn->sched_pri[idx];
	}
	krn->sched_val[idx] = krn->sched_limit / 2;
#endif
}

/* -------------------------------------------------------------------------- 
 * thread info access methods 
 * --------------------------------------------------------------------------*/

static inline const struct thinkos_thread_inf * __attribute__((always_inline))  
__thread_inf_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_INFO)
	return krn->th_inf[idx];
#else
	return NULL;
#endif
}

static inline void __attribute__((always_inline))
__thread_inf_clr(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_INFO)
	krn->th_inf[idx] = NULL;
#endif
}

static inline void __attribute__((always_inline))
__thread_inf_set(struct thinkos_rt * krn, unsigned int idx, 
				 const struct thinkos_thread_inf * inf) {
#if (THINKOS_ENABLE_THREAD_INFO)
	krn->th_inf[idx] = inf;
#endif
}

static inline const char *  __attribute__((always_inline))  
__thread_tag_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = krn->th_inf[idx];
	return (inf == NULL) ? "..." : inf->tag;
#else
	return "...";
#endif
}

static inline void __attribute__((always_inline))
__thread_errno_set(struct thinkos_rt * krn, unsigned int idx, int errno) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	krn->th_errno[idx] = errno;
#endif
}

static inline int __attribute__((always_inline))
__thread_errno_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	return krn->th_errno[idx];
#else
	return 0;
#endif
}

static inline void __attribute__((always_inline))
__thread_errno_clr(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	krn->th_errno[idx] = 0;
#endif
}

static inline void __attribute__((always_inline))
__thread_cyccnt_clr(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_PROFILE)
	krn->cyccnt[idx] = 0;
#endif
}

static inline bool __attribute__((always_inline)) 
__thread_is_alloc(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_THREAD_ALLOC)
	return __bit_mem_rd(krn->th_alloc, idx) ? true : false;
#else
	return true;
#endif
}

/* Initilize a context at stack_top Return a context structure */
static inline struct thinkos_context * __thinkos_thread_ctx_init(
	uint32_t thread_idx, uint32_t stack_top, uint32_t stack_size, 
	uint32_t task_entry, uint32_t task_exit, uint32_t task_arg[]) {
	struct thinkos_context * ctx;
	uintptr_t sp;

	sp = stack_top - sizeof(struct thinkos_context);
	ctx = (struct thinkos_context *)sp;

	ctx->r0 = task_arg[0];
	ctx->r1 = task_arg[1];
	ctx->r2 = task_arg[2];
	ctx->r3 = task_arg[3];
	ctx->r12 = stack_top - stack_size;
	ctx->lr = task_exit; /* Thumb function pointer */
	ctx->pc = task_entry; /* Thumb function pointer */
	ctx->xpsr = CM_EPSR_T; /* SET the thumb bit */

	return ctx;
}

/* -------------------------------------------------------------------------- 
 * wait queue access methods 
 * --------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
	__wq_idx(struct thinkos_rt * krn, uint32_t * ptr) {
	return ptr - krn->wq_lst;
}

static inline int __attribute__((always_inline)) 
__wq_head(struct thinkos_rt * krn, unsigned int wq) {
	/* get a thread from the queue bitmap */
	return __thinkos_ffs(krn->wq_lst[wq]);
}

static inline void __attribute__((always_inline)) 
__wq_delete(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], th, 0);  
}

static inline void __attribute__((always_inline)) 
__wq_insert(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* insert into the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], th, 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	krn->th_stat[th] = wq << 1;
#endif
}

#if (THINKOS_ENABLE_TIMED_CALLS)
static inline void __attribute__((always_inline)) 
__tmdwq_insert(struct thinkos_rt * krn, unsigned int wq, 
			   unsigned int th, unsigned int ms) {
	/* set the th_clk */
	krn->th_clk[th] = krn->ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, th, 1);  
	/* insert into the event wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], th, 1);
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	krn->th_stat[th] = (wq << 1) + 1;
#endif
}
#endif

#if (THINKOS_ENABLE_TIMED_CALLS)
static inline void __attribute__((always_inline)) 
__wq_clock_insert(struct thinkos_rt * krn, unsigned int th, unsigned int ms) {
	/* set the clock */
	krn->th_clk[th] = krn->ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, th, 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	krn->th_stat[th] = 1;
#endif
	}
#endif

static inline void __attribute__((always_inline)) 
__wq_remove(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], th, 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, th, 0);  
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	krn->th_stat[th] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__wq_wakeup(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* insert the thread into ready queue */
	__bit_mem_wr(&krn->wq_ready, th, 1);
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], th, 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, th, 0);  
	/* set the thread's return value */
	__thread_ctx_get(&thinkos_rt, th)->r0 = 0;
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	krn->th_stat[th] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__wq_wakeup_return(struct thinkos_rt * krn, unsigned int wq, 
					   unsigned int th, int ret) {
	/* insert the thread into ready queue */
	__bit_mem_wr(&krn->wq_ready, th, 1);
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], th, 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, th, 0);  
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	krn->th_stat[th] = 0;
#endif
	/* set the thread's return value */
	__thread_ctx_get(&thinkos_rt, th)->r0 = ret;
}

/* -------------------------------------------------------------------------- 
 * clock access methods 
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_CLOCK)
static inline uint32_t __attribute__((always_inline)) 
__krn_ticks(struct thinkos_rt * krn) {
	return krn->ticks;
}
#endif

static inline void __attribute__((always_inline)) 
__thread_clk_set(struct thinkos_rt * krn, unsigned int idx, uint32_t clk) {
#if (THINKOS_ENABLE_CLOCK)
	krn->th_clk[idx] = clk;
#endif
}

static inline uint32_t __attribute__((always_inline)) 
__thread_clk_get(struct thinkos_rt * krn, unsigned int idx) {
#if (THINKOS_ENABLE_CLOCK)
	return krn->th_clk[idx];
#else
	return 0;
#endif
}

static inline void __attribute__((always_inline)) 
__thread_clk_itv_set(struct thinkos_rt * krn, unsigned int idx, 
					 int32_t itv) {
#if (THINKOS_ENABLE_CLOCK)
	krn->th_clk[idx] = krn->ticks + itv;
#endif
}

/* -------------------------------------------------------------------------- 
 * kernel rt scheduler access methods 
 * --------------------------------------------------------------------------*/

static inline void __attribute__((always_inline)) 
__krn_cancel_sched(struct thinkos_rt * krn) {
	struct cm3_scb * scb = CM3_SCB;
	/* removes the pending status of the PendSV exception */
	scb->icsr = SCB_ICSR_PENDSVCLR;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

/* flags a deferred execution of the scheduler */
static inline void __attribute__((always_inline)) 
__krn_defer_sched(struct thinkos_rt * krn) {
	struct cm3_scb * scb = CM3_SCB;
	/* rise a pending service interrupt */
	scb->icsr = SCB_ICSR_PENDSVSET;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

/* flags a deferred execution of the scheduler */
static inline void __attribute__((always_inline)) 
__krn_preempt(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_PREEMPTION)
	__thinkos_defer_sched();
#endif
}

static inline void __attribute__((always_inline)) 
__krn_suspend_all(struct thinkos_rt * krn) {
	/* remove from all from the ready wait queue */
	krn->wq_ready = 0;  
}


#ifdef __cplusplus
extern "C" {
#endif


void thinkos_trace_rt(struct thinkos_rt * krn);

int thinkos_obj_type_get(unsigned int oid);

int __thinkos_thread_alloc(int target_id);

void __krn_alloc_init(struct thinkos_rt * krn);

void __thinkos_krn_kill_all(struct thinkos_rt * krn);

void __thinkos_krn_abort_all(struct thinkos_rt * krn);

void __thinkos_krn_core_init(struct thinkos_rt * krn);

void __krn_pause_all(struct thinkos_rt * krn);

void __krn_suspend_all(struct thinkos_rt * krn);

void __krn_resume_all(struct thinkos_rt * krn);

bool __krn_thread_pause(struct thinkos_rt *, unsigned int idx);

bool __krn_thread_resume(struct thinkos_rt *, unsigned int idx);


//void __thinkos_irq_reset_all(void);

void __krn_irq_reset_all(struct thinkos_rt * krn);

void __thinkos_krn_irq_init(struct thinkos_rt * krn);

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
void __thinkos(struct thinkos_rt * krn);
void __profile(void);

/* -------------------------------------------------------------------------
 * Main thread exec
 * ------------------------------------------------------------------------- */

void __thinkos_exec(int thread_id, void (* func)(void *), 
					void * arg, bool paused);

/* -------------------------------------------------------------------------
 * System timer (Cortex-M SysTick)
 * ------------------------------------------------------------------------- */
void thinkos_krn_systick_init(void);

#if (THINKOS_ENABLE_CLOCK)
static inline uint32_t __attribute__((always_inline)) thinkos_clock_i(void)  {
	return (volatile uint32_t)thinkos_rt.ticks;
}
#endif

/* -------------------------------------------------------------------------
 * Misc 
 * ------------------------------------------------------------------------- */

/**
 * thinkos_krn_mpu_init() - Initializes Memory Protection Unit (MPU).
 * @krn_offs: Kernel RAM memory reserved space address
 * @krn_size:  Kernel RAM memory reserved space size
 * 
 */
void thinkos_krn_mpu_init(uint32_t code_start, uint32_t code_end, 
						  uint32_t data_start,  uint32_t data_end);

void __thinkos_krn_thread_abort(struct thinkos_rt * krn, 
								unsigned int thread_idx);



#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_KRN_I_H__ */

