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

#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

#include <thinkos.h>

#include <sys/delay.h>
#include <sys/param.h>
#include <sys/sysclk.h>

#include <thinkos.h>
#include <vt100.h>

#define __PRIORITY(OPT)   (((OPT) >> 16) & 0xff)
#define __ID(OPT)         (((OPT) >> 24) & 0x3f)
#define __PRIVILEGED(OPT) (((OPT) >> 30) & 0x01)
#define __PAUSED(OPT)     (((OPT) >> 31) & 0x01)
#define __STACK_SIZE(OPT) ((OPT) & 0xffff)

#define THINKOS_THREAD_NULL 33

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

struct thread_waitqueue {
	uint32_t bmp;
};

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

static inline bool __attribute__((always_inline)) 
__obj_is_valid(unsigned int oid, unsigned int first, unsigned int cnt) {
	return ((unsigned int)(oid - first)) > cnt ? false : true;
}


/* -------------------------------------------------------------------------- 
 * Scheduler control
 * --------------------------------------------------------------------------*/

/* Set the active thread and stack limit */
static inline unsigned int __attribute__((always_inline)) 
__krn_active_get(struct thinkos_rt * krn) {
	int id = krn->sched.active & 0x7f;
	return (id == 0) ? THINKOS_THREAD_IDLE : id;
}

static inline void __attribute__((always_inline)) 
__krn_active_set(struct thinkos_rt * krn, unsigned int th) {
	krn->sched.active = th;
}

#if 0
static inline uint32_t __attribute__((always_inline)) 
__thread_active_sl_get(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	return krn->active & ~(STACK_ALIGN_MSK);
#else
	return 0;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}
#endif

/* set the break condition */
static inline void __attribute__((always_inline)) 
__krn_sched_brk_set(struct thinkos_rt * krn, int brk, int code) {
	krn->sched.brk = 0x00 + brk;
	krn->sched.mask = 0x00;
	krn->sched.code = code | 0x01;
}

/* clear the break condition */
static inline void __attribute__((always_inline)) 
__krn_sched_brk_clr(struct thinkos_rt * krn) {
	krn->sched.brk = 0x00;
	krn->sched.mask = 0x00;
	krn->sched.code = 0x00;
}

/* get the scheduler break condition */
static inline int __attribute__((always_inline)) 
	__krn_sched_brk_get(struct thinkos_rt * krn) {
	return krn->sched.brk & 0x7f;
}

/* get the scheduler break code */
static inline int __attribute__((always_inline)) 
	__krn_sched_code_get(struct thinkos_rt * krn) {
	return krn->sched.code;
}

/* get the scheduler mask */
static inline int __attribute__((always_inline)) 
	__krn_sched_mask_get(struct thinkos_rt * krn) {
	return krn->sched.mask;
}

/* Force discarding the active thread */
static inline void __krn_sched_discard_active(struct thinkos_rt * krn) {
	krn->sched.brk = 0x00;
	krn->sched.code = 0x00;
	krn->sched.mask = 0x00;
}

/* Set the break thread index */
static inline void __krn_sched_normal(struct thinkos_rt * krn) {
	krn->sched.brk = 0x00;
	krn->sched.code = 0x00;
	krn->sched.mask = 0xff;
}

static inline void __krn_cancel_sched(struct thinkos_rt * krn) {
	struct cm3_scb * scb = CM3_SCB;
	/* removes the pending status of the PendSV exception */
	scb->icsr = SCB_ICSR_PENDSVCLR;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

/* flags a deferred execution of the scheduler */
static inline void __krn_defer_sched(struct thinkos_rt * krn) {
	struct cm3_scb * scb = CM3_SCB;
	/* rise a pending service interrupt */
	scb->icsr = SCB_ICSR_PENDSVSET;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

/* flags a deferred execution of the scheduler */
static inline void __krn_preempt(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_PREEMPTION)
	__krn_defer_sched(krn);
#endif
}

/* -------------------------------------------------------------------------- 
 * Thread Stack Limit
 * --------------------------------------------------------------------------*/

#if 0
static inline void __attribute__((always_inline)) 
__thread_active_sl_set(struct thinkos_rt * krn, uint32_t sl, unsigned int th) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	krn->active = (sl & ~(STACK_ALIGN_MSK)) + (th & STACK_ALIGN_MSK);
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}
#endif

static inline void __attribute__((always_inline)) 
__thread_sl_set(struct thinkos_rt * krn, unsigned int th, uint32_t addr) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	krn->th_sl[th] = addr;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline void __attribute__((always_inline)) 
__thread_sl_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_STACK_LIMIT)
	krn->th_sl[th] = 0;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
}

static inline uint32_t __attribute__((always_inline)) 
__thread_sl_get(struct thinkos_rt * krn, unsigned int th) {
	uintptr_t sl;
#if (THINKOS_ENABLE_STACK_LIMIT)
	sl = (uintptr_t)krn->th_sl[th];
#elif (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = krn->th_inf[th];
	sl = (inf != NULL) ? (uintptr_t)inf->stack_ptr : 0;
#else
	sl = 0;
#endif /* THINKOS_ENABLE_STACK_LIMIT */
	return (uint32_t)sl;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_stack_size_get(struct thinkos_rt * krn, unsigned int th) {
	uint32_t ss;
#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = krn->th_inf[th];
	ss = (inf != NULL) ? inf->stack_size : 0;
#else
	ss = 0;
#endif /* THINKOS_ENABLE_THREAD_INFO */
	return (uint32_t)ss;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_stack_base_get(struct thinkos_rt * krn, unsigned int th) {
	uint32_t sb;
#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = krn->th_inf[th];
	sb = (inf != NULL) ? (uintptr_t)inf->stack_ptr : 0;
#else
	sb = 0;
#endif /* THINKOS_ENABLE_THREAD_INFO */
	return (uint32_t)sb;
}


/* -------------------------------------------------------------------------- 
 * context structure access methods 
 * --------------------------------------------------------------------------*/

static inline void  __attribute__((always_inline)) 
__thread_ctx_set(struct thinkos_rt * krn, unsigned int th, 
				 struct thinkos_context * ctx, unsigned int ctrl) {
	krn->ctx[th] = ((uintptr_t)ctx) | (ctrl & (CONTROL_MSK));
}

/*
  NEW: 2020-12-02
   Performed by the service entry stub 
static inline void  __attribute__((always_inline)) 
__thread_ctx_flush(struct thinkos_rt * krn, int32_t arg[], unsigned int th) {
	uintptr_t ctx = (uintptr_t)(krn->ctx[th]);
	uintptr_t sp = (uintptr_t)&arg[-CTX_R0];
	krn->ctx[th] = sp + (ctx & (CONTROL_MSK));
}
*/
  
static inline struct thinkos_context * __attribute__((always_inline)) 
__thread_ctx_get(struct thinkos_rt * krn, unsigned int th) {
		uintptr_t ptr = krn->ctx[th] & ~(CONTROL_MSK);
		return (struct thinkos_context *)ptr;
}

static inline bool __attribute__((always_inline)) 
__thread_ctx_is_valid(struct thinkos_rt * krn, unsigned int th) {
	return (krn->ctx[th] & ~(CONTROL_MSK)) == 0 ? false : true;
}

static inline void  __attribute__((always_inline)) 
__thread_ctx_clr(struct thinkos_rt * krn, unsigned int th) {
	krn->ctx[th] = (uintptr_t)0;
}

static inline uint32_t * __attribute__((always_inline)) 
__thread_frame_get(struct thinkos_rt * krn, unsigned int th) {
	return (uint32_t *)&__thread_ctx_get(krn, th)->r0;
}

static inline void  __attribute__((always_inline)) 
__thread_r0_set(struct thinkos_rt * krn, unsigned int th, uint32_t val) {
	__thread_ctx_get(krn, th)->r0 = val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_r0_get(struct thinkos_rt * krn, unsigned int th) {
	return __thread_ctx_get(krn, th)->r0;
}

static inline void  __attribute__((always_inline)) 
__thread_r1_set(struct thinkos_rt * krn, unsigned int th, uint32_t val) {
	__thread_ctx_get(krn, th)->r1 = val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_r1_get(struct thinkos_rt * krn, unsigned int th) {
	return __thread_ctx_get(krn, th)->r1;
}

static inline void  __attribute__((always_inline)) 
__thread_r2_set(struct thinkos_rt * krn, unsigned int th, uint32_t val) {
	__thread_ctx_get(krn, th)->r2 = val;
}

static inline void  __attribute__((always_inline)) 
__thread_r3_set(struct thinkos_rt * krn, unsigned int th, uint32_t val) {
	__thread_ctx_get(krn, th)->r3 = val;
}

static inline void  __attribute__((always_inline)) 
__thread_lr_set(struct thinkos_rt * krn, unsigned int th, uintptr_t val) {
	__thread_ctx_get(krn, th)->lr = (uint32_t)val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_lr_get(struct thinkos_rt * krn, unsigned int th) {
	uintptr_t lr = __thread_ctx_get(krn, th)->lr;
	return (uint32_t)lr;
}

static inline void  __attribute__((always_inline)) 
__thread_pc_set(struct thinkos_rt * krn, unsigned int th, uintptr_t val) {
	__thread_ctx_get(krn, th)->pc = (uint32_t)val;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_pc_get(struct thinkos_rt * krn, unsigned int th) {
	uintptr_t pc = __thread_ctx_get(krn, th)->pc;
	return (uint32_t)pc;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_xpsr_get(struct thinkos_rt * krn, unsigned int th) {
	uint32_t xpsr = __thread_ctx_get(krn, th)->xpsr;
	return xpsr;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_sp_get(struct thinkos_rt * krn, unsigned int th) {
	uint32_t sp_ctrl = krn->ctx[th];
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
__thread_ctrl_get(struct thinkos_rt * krn, unsigned int th) {
	uint32_t sp_ctrl = krn->ctx[th];
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
__thread_exec_ret_get(struct thinkos_rt * krn, unsigned int th) {
	uint32_t sp_ctrl = krn->ctx[th];
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

static inline void  __attribute__((always_inline))
__thread_enable_all(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_READY_MASK)
	krn->rdy_msk = ~0;
#endif
}

static inline void  __attribute__((always_inline))
__thread_disble_all(struct thinkos_rt * krn) {
#if (THINKOS_ENABLE_READY_MASK)
	krn->rdy_msk = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__thread_wq_set(struct thinkos_rt * krn, unsigned int th, unsigned int wq) {
	/* insert into the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 1);  
}

/* -------------------------------------------------------------------------- 
 * wait queue access methods 
 * --------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
	__wq_idx(struct thinkos_rt * krn, uint32_t * ptr) {
	return ptr - krn->wq_lst;
}

static inline int __attribute__((always_inline)) 
__krn_wq_head(struct thinkos_rt * krn, unsigned int wq) {
	/* get a thread from the queue bitmap */
	return __thinkos_ffs(krn->wq_lst[wq]) + 1;
}

static inline void __attribute__((always_inline)) 
__wq_delete(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
}

static inline void __attribute__((always_inline)) 
__krn_wq_insert(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* insert into the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	krn->th_stat[th] = wq << 1;
#endif
}


#if (THINKOS_ENABLE_TIMED_CALLS)
static inline void __attribute__((always_inline)) 
__wq_clock_insert(struct thinkos_rt * krn, unsigned int th, unsigned int ms) {
	/* set the clock */
	krn->th_clk[th] = krn->ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 1);  
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	krn->th_stat[th] = 1;
#endif
	}
#endif

#if (THINKOS_ENABLE_TIMED_CALLS)
static inline void __attribute__((always_inline)) 
__krn_tmdwq_insert(struct thinkos_rt * krn, unsigned int wq, 
			   unsigned int th, unsigned int ms) {
	/* set the th_clk */
	krn->th_clk[th] = krn->ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 1);  
	/* insert into the event wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 1);
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status, mark the thread clock enable bit */
	krn->th_stat[th] = (wq << 1) + 1;
#endif
}
#endif

static inline void __attribute__((always_inline)) 
__wq_remove(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	krn->th_stat[th] = 0;
#endif
}

static inline void __attribute__((always_inline)) 
__krn_wq_wakeup(struct thinkos_rt * krn, unsigned int wq, unsigned int th) {
	/* insert the thread into ready queue */
	__bit_mem_wr(&krn->wq_ready, (th - 1), 1);
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
	/* set the thread's return value */
	__thread_r0_set(krn, th, 0);
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
	__bit_mem_wr(&krn->wq_ready, (th - 1), 1);
	/* remove from the wait queue */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	krn->th_stat[th] = 0;
#endif
	/* set the thread's return value */
	__thread_r0_set(krn, th, ret);
}

static inline int __attribute__((always_inline)) 
__thread_tmw_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_TIMED_CALLS)
	return __bit_mem_rd(&krn->wq_clock, (th - 1));  
#else
	return 0;
#endif
}

static inline bool __attribute__((always_inline)) 
__thread_is_in_wq(struct thinkos_rt * krn, unsigned int th, unsigned int wq) {
	/* is thread in wait queue */
	return __bit_mem_rd(&krn->wq_lst[wq], (th - 1)) ? true : false;  
}


static inline void __attribute__((always_inline)) 
__wq_ready_clr(struct thinkos_rt * krn) {
	/* clear the ready wait queue */
	krn->wq_ready = 0;
#if (THINKOS_ENABLE_TIMESHARE)
	krn->wq_tmshare = 0;
#endif
}

/* -------------------------------------------------------------------------- 
 * Ready Queue
 * --------------------------------------------------------------------------*/

/* Set the ready flag */
static inline void  __attribute__((always_inline))
__thread_ready_set(struct thinkos_rt * krn, unsigned int th) {
	__bit_mem_wr(&krn->wq_ready, (th - 1), 1);
}

/* Clear the ready flag */
static inline void  __attribute__((always_inline))
__thread_ready_clr(struct thinkos_rt * krn, unsigned int th) {
	__bit_mem_wr(&krn->wq_ready, (th - 1), 0);
}

/* Get the ready flag */
static inline bool __attribute__((always_inline)) 
__thread_ready_get(struct thinkos_rt * krn, unsigned int th) {
	return __bit_mem_rd(&krn->wq_ready, (th - 1)) ? true : false;
}

static inline void __attribute__((always_inline)) 
__krn_suspend_all(struct thinkos_rt * krn) {
	/* remove all threads from the ready wait queue */
	krn->wq_ready = 0;  
}

/* -------------------------------------------------------------------------- 
 * Ready mask flag access methods 
 * --------------------------------------------------------------------------*/

/* Set the schedule enable flag */
static inline void  __attribute__((always_inline))
__thread_enable(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_READY_MASK)
	__bit_mem_wr(&krn->rdy_msk, (th - 1), 1);
#endif
}

/* Clear the schedule enable flag */
static inline void  __attribute__((always_inline))
__thread_disable(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_READY_MASK)
	__bit_mem_wr(&krn->rdy_msk, (th - 1), 0);
#endif
}

/* Get the enable flag */
static inline bool  __attribute__((always_inline))
__thread_enable_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_READY_MASK)
	return __bit_mem_rd(&krn->rdy_msk, (th - 1)) ? true : false;
#endif
	return true;
}

/* -------------------------------------------------------------------------- 
 * Paused mask flag access methods 
 * --------------------------------------------------------------------------*/

/* Set the pause flag */
static inline void  __attribute__((always_inline))
__thread_pause_set(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_PAUSE)
	__bit_mem_wr(&krn->wq_paused, (th - 1), 1);
#endif
}

/* Clear the pause flag */
static inline void  __attribute__((always_inline))
__thread_pause_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_PAUSE)
	__bit_mem_wr(&krn->wq_paused, (th - 1), 0);
#endif
}

/* Get the pause flag */
static inline bool  __attribute__((always_inline))
__thread_pause_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_PAUSE)
	return __bit_mem_rd(&krn->wq_paused, (th - 1)) ? true : false;
#endif
	return false;
}

/* -------------------------------------------------------------------------- 
 * Time share
 * --------------------------------------------------------------------------*/

/* Set the tmshare flag */
static inline void  __attribute__((always_inline))
__thread_tmshare_set(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_TIMESHARE)
	__bit_mem_wr(&krn->wq_tmshare, (th - 1), 1);
#endif
}

/* Clear the tmshare flag */
static inline void  __attribute__((always_inline))
__thread_tmshare_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_TIMESHARE)
	__bit_mem_wr(&krn->wq_tmshare, (th - 1), 0);
#endif
}

/* Get the tmshare flag */
static inline bool  __attribute__((always_inline))
__thread_tmshare_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_TIMESHARE)
	return __bit_mem_rd(&krn->wq_tmshare, (th - 1)) ? true : false;
#endif
	return false;
}

/* -------------------------------------------------------------------------- 
 * Canceled
 * --------------------------------------------------------------------------*/

/* Set the cancel flag */
static inline void  __attribute__((always_inline))
__thread_cancel_set(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_JOIN)
	__bit_mem_wr(&krn->wq_canceled, (th - 1), 1);
#endif
}

/* Clear the cancel flag */
static inline void  __attribute__((always_inline))
__thread_cancel_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_JOIN)
	__bit_mem_wr(&krn->wq_canceled, (th - 1), 0);
#endif
}

/* Get the cancel flag */
static inline bool __attribute__((always_inline)) 
__thread_cancel_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_JOIN)
	return __bit_mem_rd(&krn->wq_canceled, (th - 1)) ? true : false;
#else
	return false;
#endif
}



/* -------------------------------------------------------------------------- 
 * Fault flag access methods 
 * --------------------------------------------------------------------------*/

/* Set the fault flag */
static inline void __attribute__((always_inline)) 
	__thread_fault_set(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	__bit_mem_wr(&krn->wq_fault, (th - 1), 1);
#endif
}

/* Clear the fault flag */
static inline void __attribute__((always_inline))
__thread_fault_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	__bit_mem_wr(&krn->wq_fault, (th - 1), 0);
#endif
}

/* get the fault flag */
static inline bool __attribute__((always_inline))
__thread_fault_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	return __bit_mem_rd(&krn->wq_fault, (th - 1)) ? true : false;
#endif
	return false;
}

extern int __thread_wq_lookup(struct thinkos_rt * krn, unsigned int th);

static inline int __attribute__((always_inline)) 
__thread_wq_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_STAT)
	return (krn->th_stat[th] >> 1) & 0x1ff;
#else
	return __thread_wq_lookup(krn, th);
#endif
}


static inline void __attribute__((always_inline)) 
	__krn_thread_suspend(struct thinkos_rt * krn, int th) {
#if !(THINKOS_ENABLE_TIMESHARE)
	/* remove from the ready wait queue */
	__bit_mem_wr(&krn->wq_ready, (th - 1), 0);  
#else
	uint32_t ready;
	uint32_t tmshare;
	do {
		ready = __ldrex(&kern->wq_ready);
		/* remove from the ready wait queue */
		ready &= ~(1 << (th - 1));
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
__thread_priority_set(struct thinkos_rt * krn, unsigned int th, int priority) {
#if (THINKOS_ENABLE_TIMESHARE)
	krn->sched_pri[th] = priority;
	if (krn->sched_pri[th] > (THINKOS_SCHED_LIMIT_MAX)) {
		krn->sched_pri[th] = (THINKOS_SCHED_LIMIT_MAX);
	}
	/* update schedule limit */
	if (krn->sched_limit < krn->sched_pri[th]) {
		krn->sched_limit = krn->sched_pri[th];
	}
	krn->sched_val[th] = krn->sched_limit / 2;
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
___thread_stat_get(struct thinkos_rt * krn, unsigned int th) {
		return krn->th_stat[th];
}
#endif


/* -------------------------------------------------------------------------- 
 * thread info access methods 
 * --------------------------------------------------------------------------*/

static inline const struct thinkos_thread_inf * __attribute__((always_inline))  
__thread_inf_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_INFO)
	return krn->th_inf[th];
#else
	return NULL;
#endif
}

static inline void __attribute__((always_inline))
__thread_inf_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_INFO)
	krn->th_inf[th] = NULL;
#endif
}

static inline void __attribute__((always_inline))
__thread_inf_set(struct thinkos_rt * krn, unsigned int th, 
				 const struct thinkos_thread_inf * inf) {
#if (THINKOS_ENABLE_THREAD_INFO)
	krn->th_inf[th] = inf;
#endif
}

static inline const char *  __attribute__((always_inline))  
__thread_tag_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = krn->th_inf[th];
	return (inf == NULL) ? "..." : inf->tag;
#else
	return "...";
#endif
}

static inline void __attribute__((always_inline))
__thread_errno_set(struct thinkos_rt * krn, unsigned int th, int errno) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	krn->th_errno[th] = errno;
#endif
}

static inline int __attribute__((always_inline))
__thread_errno_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	return krn->th_errno[th];
#else
	return 0;
#endif
}

static inline void __attribute__((always_inline))
__thread_errno_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	krn->th_errno[th] = 0;
#endif
}

static inline void __attribute__((always_inline))
__thread_cyccnt_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_PROFILE)
	krn->cyccnt[th] = 0;
#endif
}

static inline uint32_t __attribute__((always_inline))
__thread_cyccnt_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_PROFILE)
	return krn->cyccnt[th];
#else
	return 0;
#endif
}

static inline uint32_t __attribute__((always_inline))
__thread_sched_val_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_TIMESHARE)
	return krn->sched_val[th];
#else
	return 0;
#endif
}

static inline uint32_t __attribute__((always_inline))
__thread_sched_pri_get(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_TIMESHARE)
	return krn->sched_pri[th];
#else
	return 0;
#endif
}

static inline void __attribute__((always_inline)) 
__thread_alloc_set(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_ALLOC)
	__bit_mem_wr(krn->th_alloc, (th - 1), 1);
#endif
}

static inline void __attribute__((always_inline)) 
__thread_alloc_clr(struct thinkos_rt * krn, unsigned int th) {
#if (THINKOS_ENABLE_THREAD_ALLOC)
	__bit_mem_wr(krn->th_alloc, (th - 1), 0);
#endif
}

/* -------------------------------------------------------------------------- 
 * Interval clock access methods 
 * --------------------------------------------------------------------------*/

static inline uint32_t __attribute__((always_inline)) 
__krn_ticks_get(struct thinkos_rt * krn) {
	return krn->ticks;
}

static inline void __attribute__((always_inline)) 
__thread_clk_set(struct thinkos_rt * krn, unsigned int th, uint32_t clk) {
	krn->th_clk[th] = clk;
}

static inline uint32_t __attribute__((always_inline)) 
__thread_clk_get(struct thinkos_rt * krn, unsigned int th) {
	return krn->th_clk[th];
}

static inline void __attribute__((always_inline)) 
__thread_clk_itv_set(struct thinkos_rt * krn, unsigned int th, int32_t itv) {
	krn->th_clk[th] = krn->ticks + itv;
}

static inline int32_t __attribute__((always_inline)) 
__thread_clk_itv_get(struct thinkos_rt * krn, unsigned int th) {
	return krn->th_clk[th] - krn->ticks;
}

/* Set the schedule enable flag */
static inline void  __attribute__((always_inline))
__thread_clk_enable(struct thinkos_rt * krn, unsigned int th) {
	__bit_mem_wr(&krn->wq_clock, (th - 1), 1);
}

/* Clear the schedule enable flag */
static inline void  __attribute__((always_inline))
__thread_clk_disable(struct thinkos_rt * krn, unsigned int th) {
	__bit_mem_wr(&krn->wq_clock, (th - 1), 0);
}

/* Get the enable flag */
static inline bool  __attribute__((always_inline))
__thread_clk_is_enabled(struct thinkos_rt * krn, unsigned int th) {
	return __bit_mem_rd(&krn->wq_clock, (th - 1)) ? true : false;
}


/* -------------------------------------------------------------------------- 
 * Composite methods 
 * --------------------------------------------------------------------------*/

static inline bool __krn_obj_is_thread(struct thinkos_rt * krn, 
										unsigned int th) {
	return __obj_is_valid(th, THINKOS_THREAD_BASE, THINKOS_THREADS_MAX);
}

static inline bool __krn_thread_is_alloc(struct thinkos_rt * krn, 
										 unsigned int th) {
	return __bit_mem_rd(krn->th_alloc, 
						th - THINKOS_THREAD_BASE) ? true : false;
}

static inline bool __krn_thread_is_valid(struct thinkos_rt * krn, int th) {
	return ((th >= (THINKOS_THREAD_FIRST)) && (th <= (THINKOS_THREAD_LAST))) ?
			 __thread_ctx_is_valid(krn, th) : false;
}

/* Initilize a context at stack_top Return a context structure */
static inline struct thinkos_context * __thinkos_thread_ctx_init(
	uint32_t stack_top, uint32_t stack_size, 
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
 * mutex access methods 
 * --------------------------------------------------------------------------*/

#if ((THINKOS_MUTEX_MAX) > 0)

static inline bool __attribute__((always_inline)) 
__krn_obj_is_mutex(struct thinkos_rt * krn, unsigned int mtx) {
	return __obj_is_valid(mtx, THINKOS_MUTEX_BASE, THINKOS_MUTEX_MAX);
}

static inline bool __attribute__((always_inline)) 
__krn_mutex_is_alloc(struct thinkos_rt * krn, unsigned int mtx) {
	return __bit_mem_rd(krn->mutex_alloc, mtx - THINKOS_MUTEX_BASE) ? 
		true : false;
}

static inline void __attribute__((always_inline)) 
__krn_mutex_lock(struct thinkos_rt * krn, unsigned int mtx, unsigned int th) {
	krn->mtx_lock[mtx - THINKOS_MUTEX_BASE] = th;
}

static inline void __attribute__((always_inline)) 
__krn_mutex_unlock(struct thinkos_rt * krn, unsigned int mtx) {
	krn->mtx_lock[mtx - THINKOS_MUTEX_BASE] = 0;
}

static inline unsigned int __attribute__((always_inline)) 
__krn_mutex_lock_get(struct thinkos_rt * krn, unsigned int mtx) {
	return krn->mtx_lock[mtx - THINKOS_MUTEX_BASE];
}

static inline bool __attribute__((always_inline)) 
__krn_mutex_is_locked(struct thinkos_rt * krn, unsigned int mtx) {
	return (krn->mtx_lock[mtx - THINKOS_MUTEX_BASE]) ? true : false;
}

static inline unsigned int __attribute__((always_inline)) 
__krn_mutex_idx(struct thinkos_rt * krn, unsigned int mtx) {
	return (mtx - THINKOS_MUTEX_BASE);
}

#endif



#ifdef __cplusplus
extern "C" {
#endif

#if ((THINKOS_MUTEX_MAX) > 0)
int krn_mutex_check(struct thinkos_rt * krn, int mtx);

int krn_mutex_unlock_wakeup(struct thinkos_rt * krn, int mtx);
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

bool __krn_thread_pause(struct thinkos_rt *, unsigned int th);

bool __krn_thread_resume(struct thinkos_rt *, unsigned int th);


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

static inline uint32_t __attribute__((always_inline)) thinkos_clock_i(void)  {
	return (volatile uint32_t)thinkos_rt.ticks;
}

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
								unsigned int thread);

void __thread_fault_raise(struct thinkos_rt * krn, 
						  unsigned int th, int errno);

void __thinkos_krn_core_reset(struct thinkos_rt * krn);

/* wait for event and signal the scheduler ... */
void __krn_thread_wait(struct thinkos_rt * krn, unsigned int th, 
					   unsigned int wq);

void __krn_thread_timedwait(struct thinkos_rt * krn, unsigned int th, 
										  unsigned int wq, unsigned int ms);

void __krn_thread_clk_itv_wait(struct thinkos_rt * krn, unsigned int th, 
							  unsigned int ms);

void __krn_wq_wakeup_all(struct thinkos_rt * krn, unsigned int wq);

int __krn_thread_check(struct thinkos_rt * krn, unsigned int th);

int __krn_threads_cyc_get(struct thinkos_rt * krn, uint32_t cyc[], 
						  unsigned int from, unsigned int cnt);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_KRN_I_H__ */

