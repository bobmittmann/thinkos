/* 
 * thinkos_debug.c
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include "thinkos_krn-i.h"

#if (THINKOS_ENABLE_SCHED_DEBUG)
  #ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_TRACE
  #elif LOG_LEVEL < LOG_TRACE
    #undef LOG_LEVEL 
    #define LOG_LEVEL LOG_TRACE
  #endif
#endif

#include <sys/dcclog.h>

const char * __retstr(uint32_t __ret)
{
	const char * s;

	switch (__ret) {
	case 0xffffffe1:
		s = "HDL MSP EXT";
		break;
	case 0xffffffe9:
		s = "THD MSP EXT";
		break;
	case 0xffffffed:
		s = "THD PSP EXT";
		break;
	case 0xfffffff1:
		s = "HDL MSP BAS";
		break;
	case 0xfffffff9:
		s = "THD MSP BAS";
		break;
	case 0xfffffffd:
		s = "THD PSP BAS";
		break;
	default:
		s = "!INVALID!  ";
	}
	return s;
}

/*
void __objkind(void)
{
	int kind;

	for (kind = 0; kind < THINKOS_OBJ_INVALID; kind++) {
		DCC_LOG2(LOG_TRACE, "%s --> %d", 
				 thinkos_type_name_lut[kind], thinkos_wq_base_lut[kind]);
	}
}
*/

#if (THINKOS_ENABLE_SCHED_DEBUG)

static uint32_t __ret_lut[8] = {
	[0] = CM3_EXC_RET_THREAD_MSP, /* kernel */
	[1] = 0, /* invalid */
	[2] = CM3_EXC_RET_THREAD_PSP, /* user privileged */
	[3] = CM3_EXC_RET_THREAD_PSP, /* user */
	[4] = 0, /* invalid */
	[5] = CM3_EXC_RET_THREAD_PSP_EXT, /* user fp */
	[6] = 0, /* invalid */
	[7] = CM3_EXC_RET_THREAD_PSP_EXT, /* user fp privileged */
};


void __context(uintptr_t __sp_ctl, uint32_t __thread_id)
{
	struct thinkos_context * ctx;
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & ~(CONTROL_MSK));
	ctrl = __sp_ctl & (CONTROL_MSK); 
	ret = __ret_lut[ctrl];
	(void)ret;

	DCC_LOG4(LOG_TRACE, "  r0=%08x  r1=%08x  r2=%08x  r3=%08x", 
			 ctx->r0, ctx->r1, ctx->r2, ctx->r3);
	DCC_LOG4(LOG_TRACE, "  r4=%08x  r5=%08x  r6=%08x  r7=%08x", 
			 ctx->r4, ctx->r7, ctx->r6, ctx->r7);
	DCC_LOG4(LOG_TRACE, "  r8=%08x  r9=%08x r10=%08x r11=%08x", 
			 ctx->r8, ctx->r9, ctx->r10, ctx->r11);
	DCC_LOG4(LOG_TRACE, " r12=%08x  sp=%08x  lr=%08x  pc=%08x", 
			 ctx->r12, ctx, ctx->lr, ctx->pc);
	DCC_LOG2(LOG_TRACE, "xpsr=%08x th=%d", ctx->xpsr, __thread_id);
}


/*
   0xFFFFFFE1 | Handler mode | Main         | Extended
   0xFFFFFFE9 | Thread mode  | Main         | Extended
   0xFFFFFFED | Thread mode  | Process      | Extended
   0xFFFFFFF1 | Handler mode | Main         | Basic
   0xFFFFFFF9 | Thread mode  | Main         | Basic
   0xFFFFFFFD | Thread mode  | Process      | Basic
   */

void SCHED(uintptr_t __sp_ctl, 
		   uint32_t __new_thread_id,
		   uint32_t __prev_thread_id)
{
	struct thinkos_context * ctx;
	uint32_t msp = cm3_msp_get();
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & ~(CONTROL_MSK));
	ctrl = __sp_ctl & (CONTROL_MSK); 
	ret = __ret_lut[ctrl];

	if (ret == 0) {
		/* invalid */
		DCC_LOG1(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ 
				" CONTROL=%d invalid!!!" _ATTR_POP_, ctrl);
	}

	if (__prev_thread_id == THINKOS_THREAD_IDLE) {
		if (ctrl & CONTROL_nPRIV) {
			DCC_LOG5(LOG_TRACE,  _ATTR_PUSH_ _FG_MAGENTA_ 
					 "IDLE -> <%2d> CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __new_thread_id, ctx, ctx->pc, 
					 msp, __retstr(ret));
		} else {
			DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_YELLOW_ 
					 "IDLE -> <%2d> CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __new_thread_id, ctx, ctx->pc, 
					 msp, __retstr(ret));
		}
	} else if (__prev_thread_id == THINKOS_THREAD_VOID) {
		if (ctrl & CONTROL_nPRIV) {
			DCC_LOG5(LOG_TRACE,  _ATTR_PUSH_ _FG_BLUE_ 
					 "VOID -> <%2d> " 
					 "CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __new_thread_id, 
					 ctx, ctx->pc, msp, __retstr(ret));
		} else {
			DCC_LOG5(LOG_TRACE,  _ATTR_PUSH_ _FG_RED_ 
					 "VOID -> <%2d> " 
					 "CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __new_thread_id, 
					 ctx, ctx->pc, msp, __retstr(ret));
		}
	} else {
		if (ctrl & CONTROL_nPRIV) {
			DCC_LOG6(LOG_TRACE, 
					 "<%2d> -> <%2d> " 
					 "CTX=%08x PC=%08x MSP=%08x %s",
					 __prev_thread_id, __new_thread_id, 
					 ctx, ctx->pc, msp, __retstr(ret));
		} else {
			DCC_LOG6(LOG_TRACE,  _ATTR_PUSH_ _BRIGHT_ 
					 "<%2d> -> <%2d> " 
					 "CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __prev_thread_id, __new_thread_id, 
					 ctx, ctx->pc, msp, __retstr(ret));
		}
	}
}

void _IDLE(uintptr_t __sp_ctl, 
		   uint32_t __new_thread_id,
		   uint32_t __prev_thread_id)
{
#if DEBUG
	struct thinkos_context * ctx;
	uint32_t psp = cm3_psp_get();
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & 0xfffffff8);
	ctrl = __sp_ctl & 0x000000007; 
	ret = __ret_lut[ctrl];

	if (__prev_thread_id == THINKOS_THREAD_IDLE) {
		DCC_LOG4(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_ _DIM_
				 "IDLE -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 ctx, ctx->pc, psp, __retstr(ret));
	} else if (__prev_thread_id == THINKOS_THREAD_VOID) {
		DCC_LOG4(LOG_TRACE, _ATTR_PUSH_ _FG_BLUE_ _BRIGHT_
				 "VOID -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 ctx, ctx->pc, psp, __retstr(ret));
	} else {
		DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_
				 "<%2d> -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id, 
				 ctx, ctx->pc, psp, __retstr(ret));
	}
#endif
}

void ERROR(uintptr_t __sp_ctl, 
		   uint32_t __new_thread_id,
		   uint32_t __prev_thread_id)
{
#if DEBUG
	struct thinkos_context * ctx;
	uint32_t msp = cm3_msp_get();
	uint32_t psp = cm3_psp_get();
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & 0xfffffff8);
	ctrl = __sp_ctl & 0x000000007; 
	ret = __ret_lut[ctrl];

	if (ret == 0) {
		/* invalid */
		DCC_LOG1(LOG_TRACE, _ATTR_PUSH_ _FG_MAGENTA_ 
				" CONTROL=%d invalid!!!" _ATTR_POP_, ctrl);
	}

	if (__prev_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_ _DIM_
				 "IDLE -> <%2d> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __new_thread_id, 
				 ctx, ctx->pc, psp, __retstr(ret));
	else if (__new_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_ _DIM_
				 "<%2d> -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id, 
				 ctx, ctx->pc, psp, __retstr(ret));
	else
		DCC_LOG6(LOG_ERROR,  _ATTR_PUSH_ _FG_RED_ 
				 "<%2d> -> <%2d> " 
				 "CTX=%08x PC=%08x MSP=%08x %s"  _ATTR_POP_,
				 __prev_thread_id, __new_thread_id, 
				 ctx, ctx->pc, msp, __retstr(ret));

	__context(__sp_ctl, __new_thread_id); 
	__kdump(&thinkos_rt);
	__tdump(&thinkos_rt);
#endif
}

void thinkos_sched_dbg(uintptr_t __sp_ctl, 
					   uint32_t __new_thread_id,
					   uint32_t __prev_thread_id)
{
	if (__new_thread_id > THINKOS_THREAD_IDLE) {
		ERROR(__sp_ctl, __new_thread_id, __prev_thread_id);
	} else if (__new_thread_id == THINKOS_THREAD_IDLE) {
		if (__prev_thread_id != THINKOS_THREAD_IDLE) {
			_IDLE(__sp_ctl, __new_thread_id, __prev_thread_id);
		} else {
			_IDLE(__sp_ctl, __new_thread_id, __prev_thread_id);
		}
	} else {
		SCHED(__sp_ctl, __new_thread_id, __prev_thread_id);
	}
}

void thinkos_sched_step_dbg(uintptr_t __sp_ctl, 
							 uint32_t __new_thread_id,
							 uint32_t __prev_thread_id) 
{
	struct thinkos_context * ctx;
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & 0xfffffff8);
	ctrl = __sp_ctl & 0x000000007; 
	ret = __ret_lut[ctrl];
	(void)ret;

	DCC_LOG3(LOG_WARNING,  _ATTR_PUSH_ _FG_GREEN_ 
			 "<%2d> STEP " 
			 "PC=%08x SP=%08x"  _ATTR_POP_, 
			 __new_thread_id, ctx->pc, (uintptr_t)ctx);
}

void thinkos_stack_limit_dbg(uintptr_t __sp_ctl, 
							 uint32_t __new_thread_id,
							 uint32_t __prev_thread_id) 
{
	struct thinkos_context * ctx;
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & 0xfffffff8);
	ctrl = __sp_ctl & 0x000000007; 
	ret = __ret_lut[ctrl];
	(void)ret;

	DCC_LOG3(LOG_WARNING,  _ATTR_PUSH_ _FG_GREEN_ 
			 "<%2d> STEP " 
			 "PC=%08x SP=%08x"  _ATTR_POP_, 
			 __new_thread_id, ctx->pc, (uintptr_t)ctx);
}

#endif

bool thinkos_dbg_thread_ctx_is_valid(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;

	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST)) {
		return false;
	}

//    th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;
    return __thread_ctx_is_valid(krn, th);
}

int thinkos_dbg_thread_brk_get(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;

    if (!__thread_ctx_is_valid(krn, THINKOS_THREAD_VOID)) {
        return -1;
    }

    return __krn_sched_brk_get(krn);
}


static struct thinkos_context * __dbg_thread_ctx_get(struct thinkos_rt * krn,
													 unsigned int th)
{
	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST))
		return NULL;

    th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;
    return __thread_ctx_get(krn, th);
}

static unsigned int __dbg_thread_ctrl_get(struct thinkos_rt * krn,
                                          unsigned int th)
{
	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST))
		return 0;

    th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;
    return __thread_ctrl_get(krn, th);
}


struct thinkos_context * thinkos_dbg_thread_ctx_get(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;

	return __dbg_thread_ctx_get(krn, th);
}

unsigned int thinkos_dbg_thread_ctrl_get(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;

    return __dbg_thread_ctrl_get(krn, th);
}

uint32_t thinkos_dbg_thread_pc_get(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;
//    int th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;

    return __thread_pc_get(krn, th);
}

uint32_t thinkos_dbg_thread_lr_get(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;
 //   int th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;

    return __thread_lr_get(krn, th);
}

uint32_t thinkos_dbg_thread_sp_get(unsigned int th)
{
    struct thinkos_rt * krn = &thinkos_rt;
  //  int th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;

    return __thread_sp_get(krn, th);
}

uint32_t thinkos_dbg_thread_sl_get(unsigned int th) 
{
	return __thread_sl_get(&thinkos_rt, th); 
}

uint32_t thinkos_dbg_krn_ticks_get(unsigned int th) 
{
	return thinkos_rt.ticks;
}

const char * thinkos_dbg_thread_tag_get(unsigned int th) 
{
	return __thread_tag_get(&thinkos_rt, th);
}

int thinkos_dbg_thread_wq_get(unsigned int th)
{
	return __thread_wq_get(&thinkos_rt, th);
}

int thinkos_dbg_thread_tmw_get(unsigned int th)
{
	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST)) {
		return 0;
	}
	return __thread_tmw_get(&thinkos_rt, th);
}

int thinkos_dbg_thread_errno_get(unsigned int th)
{
	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST)) {
		return 0;
	}
	return __thread_errno_get(&thinkos_rt, th);
}

uint32_t thinkos_dbg_thread_cyccnt_get(unsigned int th)
{
#if (THINKOS_ENABLE_PROFILING)
	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST)) {
		return 0;
	}
	return __thread_cyccnt_get(&thinkos_rt, th);
#else
	return 0;
#endif
}

int32_t thinkos_dbg_thread_clk_itv_get(unsigned int th)
{
	if ((th < THINKOS_THREAD_FIRST) || (th > THINKOS_THREAD_LAST)) {
		return 0;
	}
	return __thread_clk_itv_get(&thinkos_rt, th);
}

bool thinkos_dbg_thread_get(unsigned int th, struct thinkos_thread * st, 
							struct cortex_m_core * core, 
							struct cortex_m_fp * fp)
{
    struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_context * ctx;
	unsigned int ctrl;

	if ((ctx = __dbg_thread_ctx_get(krn, th)) == NULL) {
		return false;
	}

    ctrl = __dbg_thread_ctrl_get(krn, th);

	if (st != NULL) {
		st->thread_no = th;
		st->wq = __thread_wq_get(krn, th);
		st->tmw = __thread_tmw_get(krn, th);
		st->sched_val = __thread_sched_val_get(krn, th);
		st->sched_pri = __thread_sched_pri_get(krn, th);
		st->clk = __thread_clk_itv_get(krn, th);
		st->cyc = __thread_cyccnt_get(krn, th);
		__thinkos_memcpy(st->tag, __thread_tag_get(krn, th), 10);
		st->privileged = (ctrl & CONTROL_nPRIV) ? 0 : 1;
#if (THINKOS_ENABLE_FPU)
		st->fpca = (ctrl & CONTROL_FPCA) ? 1 : 0;
#else
		st->fpca = 0;
#endif
	}

    th = (__krn_sched_brk_get(krn) == th) ? THINKOS_THREAD_VOID : th;
    ctx = __thread_ctx_get(krn, th);

	if (core != NULL) {
		core->r0 = ctx->r0;
		core->r1 = ctx->r1;
		core->r2 = ctx->r2;
		core->r3 = ctx->r3;
		core->r4 = ctx->r4;
		core->r5 = ctx->r5;
		core->r6 = ctx->r6;
		core->r7 = ctx->r7;
		core->r8 = ctx->r8;
		core->r9 = ctx->r9;
		core->r10 = ctx->r10;
		core->r11 = ctx->r11;
		core->r12 = ctx->r12;
		core->sp = __thread_sp_get(krn, th);
		core->lr = __thread_lr_get(krn, th);
		core->pc = __thread_pc_get(krn, th);
		core->xpsr = __thread_xpsr_get(krn, th);
	}

#if (THINKOS_ENABLE_FPU) 
	if ((fp != NULL) && (ctrl & CONTROL_FPCA)) {
		struct thinkos_fp_context * fp_ctx;
		unsigned int i;

		fp_ctx = FP_CTX(ctx);

		fp->fpscr = fp_ctx->fpscr;

		for (i = 0; i < 16; ++i)
			fp->s[i] = fp_ctx->s0[i];

		for (i = 0; i < 16; ++i)
			fp->s[i + 16] = fp_ctx->s1[i];
	}
#endif

	return 0;
}


#if 0
bool thinkos_dbg_thread_rec_get(unsigned int thread,
                           struct thread_rec * rec,
{
    struct thinkos_rt * krn = &thinkos_rt;
    struct thinkos_context * ctx;
    unsigned int th;

    th = (__krn_sched_brk_get(krn) == thread) ? thread : THINKOS_THREAD_VOID;
    ctx = __thread_ctx_get(krn, th);

    if (ctx != NULL) {
		return false;
    }

	if (rec != NULL) {
        rec->ctrl = __thread_ctrl_get(krn, th);
	}

	return true;
}


#endif

int __thread_break_get(struct thinkos_rt * krn, int32_t * pcode)
{
	int th;

	if ((th = __krn_sched_brk_get(krn)) == 0)
		return -1;

	if (__dbg_thread_ctx_get(krn, th) == NULL)
		return -1;

	if (pcode)
		*pcode = __thread_errno_get(krn, th);

	return th;
}

int thinkos_dbg_thread_break_get(int32_t * pcode)
{
	struct thinkos_rt * krn = &thinkos_rt;
	int th;

	DCC_LOG(LOG_TRACE, "...");

	if ((th = __krn_sched_brk_get(krn)) == 0)
		return -1;

	DCC_LOG(LOG_TRACE, "...");

	if (pcode)
		*pcode = __thread_errno_get(krn, th);

	return th;
}

int thinkos_dbg_thread_break_clr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
	int th;

	DCC_LOG(LOG_TRACE, "clearing break condition...");

	if ((th = __krn_sched_brk_get(krn)) == 0)
		return -1;

	__krn_sched_brk_clr(krn);

	__thread_errno_clr(krn, th);

	return 0;
}


int thinkos_dbg_mutex_lock_get(unsigned int mtx)
{
	struct thinkos_rt * krn = &thinkos_rt;

	if (__krn_obj_is_mutex(krn, mtx)) {
		return __krn_mutex_lock_get(krn, mtx);
	}

	return 0;
}

void __krn_cyccnt_flush(struct thinkos_rt * krn, unsigned int th)
{
	uint32_t ref;
	uint32_t cnt;

	cnt = CM3_DWT->cyccnt;
	ref = krn->cycref;
	krn->cycref = cnt;

	krn->th_cyc[th] += cnt - ref;
}

int thinkos_dbg_threads_cyc_get(uint32_t cyc[], unsigned int from, 
								unsigned int cnt)
{
#if (THINKOS_ENABLE_PROFILING)
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int to = from + cnt;

	if (to > __KRN_THREAD_LST_SIZ)
		return -THINKOS_EINVAL;

	__krn_cyccnt_flush(krn, __krn_active_get(krn));
	__thinkos_memcpy32(cyc, &krn->th_cyc[from], cnt * sizeof(uint32_t)); 

	return to;
#else
	return -THINKOS_ENOSYS;
#endif
}

struct thread_waitqueue * thinkos_dbg_wq_from_oid(unsigned int oid)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct thread_waitqueue * wq;
	unsigned int idx = oid - THINKOS_OBJECT_FIRST;

	if (idx > THINKOS_OBJECT_LAST) {
		return NULL;
	}

	wq = (struct thread_waitqueue *)&krn->wq_lst[idx];

	return wq;
}

static inline bool __attribute__((always_inline)) 
__krn_wq_contain(struct thinkos_rt * krn, struct thread_waitqueue * wq, 
				 unsigned int th) {
	uint32_t * ptr = (uint32_t *)wq;

	/* is thread in wait queue */
	return __bit_mem_rd(ptr, (th - 1)) ? true : false;  
}

static inline bool __attribute__((always_inline)) 
__krn_wq_is_empty(struct thinkos_rt * krn, struct thread_waitqueue * wq) {
	return (wq->bmp == 0) ? true : false;  
}

bool thinkos_dbg_wq_contains(struct thread_waitqueue * wq, unsigned int thread)
{
	struct thinkos_rt * krn = &thinkos_rt;

	if ((thread < THINKOS_THREAD_FIRST) || (thread > THINKOS_THREAD_LAST)) {
		return false;
	}

	return __krn_wq_contain(krn, wq, thread);
}

bool thinkos_dbg_wq_is_empty(struct thread_waitqueue * wq)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __krn_wq_is_empty(krn, wq);
}

void thinkos_dbg_reset(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__thinkos_krn_core_reset(krn);
}

/* -------------------------------------------------------------------------
 * Fast thread execution
 * ------------------------------------------------------------------------- */

void __attribute__((noreturn)) __dbg_thread_exit_stub(int code)
{
	DCC_LOG1(LOG_WARNING, "code=%d", code);
#if 0
	thinkos_thread_abort(code);
#else
	thinkos_abort();
#endif
}

extern void * __krn_stack_start;
extern void * __krn_stack_end;
extern int __krn_stack_size;

int thinkos_dbg_thread_create(int (* func)(void *, unsigned int), void * arg,
						  bool privileged)
{
#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = &thinkos_main_inf;
#endif
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_thread_initializer init;
	unsigned int thread_idx;
	uintptr_t stack_base;
	uint32_t stack_size;
	int ret;

#if (THINKOS_ENABLE_THREAD_INFO)
	thread_idx = (inf->thread_id > 0) ? inf->thread_id : 1;
	stack_base = (uintptr_t)inf->stack_ptr;
	stack_size = inf->stack_size;
#else
	thread_idx = 1;
	stack_base = (uintptr_t)&__krn_stack_start;
	stack_size = (uint32_t)&__krn_stack_size;
#endif

	/* force allocate the thread block */
	__thread_alloc_set(krn, thread_idx);

	if (stack_base & (STACK_ALIGN_MSK)) {
		DCC_LOG1(LOG_PANIC, "stack_top=%08x unaligned", stack_base); 
		return -1;
	}

	if (stack_size & (STACK_ALIGN_MSK)) {
		DCC_LOG1(LOG_TRACE, "stack_Size=%08x unaligned", stack_size); 
		stack_size &= ~(STACK_ALIGN_MSK);
	}

	init.stack_base = stack_base;
	init.stack_size = stack_size;
	init.task_entry = (uintptr_t)func;
	init.task_exit = (uintptr_t)__dbg_thread_exit_stub;
	init.task_arg[0] = (uintptr_t)arg;
	init.task_arg[1] = 0;
	init.task_arg[2] = 0;
	init.task_arg[3] = 0;
	init.priority = 0;
	init.paused = false;
	init.privileged = privileged;
#if (THINKOS_ENABLE_THREAD_INFO)
	init.inf = inf;
#endif

	/* Make sure the scheduler is in normal state */
	__krn_sched_normal(krn);

	if ((ret = thinkos_krn_thread_init(krn, thread_idx, &init))) {
		return -ret;
	};

	DCC_LOG1(LOG_TRACE, "thread=%d", thread_idx + 1);

	return thread_idx;
}

void thinkos_dbg_resume_all(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_resume_all(krn);
}

void thinkos_dbg_pause_all(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_pause_all(krn);
}

int thinkos_dbg_thread_irq_get(unsigned int th)
{
	struct thinkos_rt * krn = &thinkos_rt;

#if (THINKOS_IRQ_MAX > 0)
	if ((th >= THINKOS_THREAD_FIRST) && (th <= THINKOS_THREAD_LAST)) {
		int irq;

		for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
			if (krn->irq_th[irq] == th)
				return irq;
		}
	}
#endif
	return -1;
}

bool thinkos_dbg_thread_is_ready(unsigned int th)
{
	struct thinkos_rt * krn = &thinkos_rt;

	if ((th >= THINKOS_THREAD_FIRST) && (th <= THINKOS_THREAD_LAST)) {
		return __thread_ready_get(krn, th);
	}
	return false;
}

int thinkos_dbg_active_get(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __krn_active_get(krn);
}
