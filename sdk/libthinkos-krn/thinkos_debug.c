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
void __thinkos(struct thinkos_rt * rt)
{
	unsigned int i;
	unsigned int oid;

	for (oid = 0; oid < THINKOS_WQ_CNT; ++oid) {
		uint32_t * wq;
		int type;

		wq = &rt->wq_lst[oid];
		if (*wq) { 
			type = __thinkos_obj_kind(oid);
			switch (type) {
			case THINKOS_OBJ_READY:
				DCC_LOG2(LOG_TRACE, "READY %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_TMSHARE:
				DCC_LOG2(LOG_TRACE, "TMSHARE %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_CANCELED:
				DCC_LOG2(LOG_TRACE, "CANCELED %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_PAUSED:
				DCC_LOG2(LOG_TRACE, "PAUSED %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_CLOCK:
				DCC_LOG2(LOG_TRACE, "CLOCK %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_MUTEX:
				DCC_LOG2(LOG_TRACE, "MUTEX %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_COND:
				DCC_LOG2(LOG_TRACE, "COND %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_SEMAPHORE:
				DCC_LOG2(LOG_TRACE, "SEMAPHORE %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_EVENT:
				DCC_LOG2(LOG_TRACE, "EVENT %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_FLAG:
				DCC_LOG2(LOG_TRACE, "FLAG %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_JOIN:
				DCC_LOG2(LOG_TRACE, "JOIN %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_CONREAD:
				DCC_LOG2(LOG_TRACE, "CON RD %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_CONWRITE:
				DCC_LOG2(LOG_TRACE, "CON WR %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_COMMSEND:
				DCC_LOG2(LOG_TRACE, "COMM TX %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_COMMRECV:
				DCC_LOG2(LOG_TRACE, "COMM RX %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_IRQ:
				DCC_LOG2(LOG_TRACE, "IRQ %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_DMA:
				DCC_LOG2(LOG_TRACE, "DMA %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_FLASH_MEM:
				DCC_LOG2(LOG_TRACE, "FLASH %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_FAULT:
				DCC_LOG2(LOG_TRACE, "FAULT %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_INVALID:
				DCC_LOG2(LOG_TRACE, "INVALID %d: 0x%08x", oid, *wq);
				break;
			default:
				DCC_LOG2(LOG_WARNING, "ERROR %d: 0x%08x", oid, *wq);
			}
		}
	}

#if (THINKOS_ENABLE_CLOCK)
	DCC_LOG1(LOG_TRACE, "Ticks = %d", rt->ticks);
#endif
	DCC_LOG1(LOG_TRACE, "Active = %d", __thread_active_get(rt));

	DCC_LOG1(LOG_TRACE, "Alloc = %d", thinkos_rt.th_alloc[0]);

	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		if (__thinkos_thread_ctx_is_valid(i)) {
#if (THINKOS_ENABLE_THREAD_STAT)
		DCC_LOG5(LOG_TRACE, "<%2d> %3d sp=%08x lr=%08x pc=%08x", i + 1, 
				 __thread_wq_get(rt, i), __thread_sp_get(rt, i), 
				 __thread_lr_get(rt, i), __thread_pc_get(rt, i));
#else
		DCC_LOG4(LOG_TRACE, "<%2d> sp=%08x lr=%08x pc=%08x", i + 1, 
				 __thread_sp_get(rt, i), __thread_lr_get(rt, i), 
				 __thread_pc_get(rt, i));
#endif
		}
	}

}

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

	ctx = (struct thinkos_context *)(__sp_ctl & 0xfffffff8);
	ctrl = __sp_ctl & 0x000000007; 
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
	DCC_LOG2(LOG_TRACE, "xpsr=%08x th=%d", ctx->xpsr, __thread_id + 1);
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
		   uint32_t __prev_thread_id, 
		   uint32_t __sp) 
{
	struct thinkos_context * ctx;
	uint32_t msp = cm3_msp_get();
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

	if (__prev_thread_id == THINKOS_THREAD_IDLE) {
		if (ctrl & CONTROL_nPRIV) {
			DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_YELLOW_ 
					 "IDLE -> <%2d> CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __new_thread_id + 1, ctx, ctx->pc, 
					 msp, __retstr(ret));
		} else {
			DCC_LOG5(LOG_TRACE,  _ATTR_PUSH_ _FG_MAGENTA_ 
					 "IDLE -> <%2d> CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
					 __new_thread_id + 1, ctx, ctx->pc, 
					 msp, __retstr(ret));
		}
	} else if (__prev_thread_id == THINKOS_THREAD_VOID) {
		DCC_LOG5(LOG_TRACE,  _ATTR_PUSH_ _FG_RED_ 
				 "VOID -> <%2d> " 
				 "CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
				 __new_thread_id + 1, 
				 ctx, ctx->pc, msp, __retstr(ret));
	} else if (__prev_thread_id == THINKOS_THREAD_IDLE) {
		DCC_LOG6(LOG_TRACE, 
				 "<%2d> -> <%2d> " 
				 "CTX=%08x PC=%08x MSP=%08x %s",
				 __prev_thread_id + 1, __new_thread_id + 1, 
				 ctx, ctx->pc, msp, __retstr(ret));
	}
#if 0
	DCC_LOG6(LOG_TRACE, "MSP=%08x PSP=%08x CTRL={%s%s%s } RDY=%08x",
			 msp, psp,
			 ctrl & CONTROL_FPCA? " FPCA" : "",
			 ctrl & CONTROL_SPSEL? " SPSEL" : "",
			 ctrl & CONTROL_nPRIV ? " nPRIV" : "",
			 thinkos_rt.wq_ready
			 );
#endif
}

void _IDLE(uintptr_t __sp_ctl, 
		   uint32_t __new_thread_id,
		   uint32_t __prev_thread_id, 
		   uint32_t __sp) 
{
#if DEBUG
	struct thinkos_context * ctx;
	uint32_t psp = cm3_psp_get();
	uint32_t ctrl = 0;
	uint32_t ret;

	ctx = (struct thinkos_context *)(__sp_ctl & 0xfffffff8);
	ctrl = __sp_ctl & 0x000000007; 
	ret = __ret_lut[ctrl];

	if (__prev_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG4(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_ _DIM_
				 "IDLE -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 ctx, ctx->pc, psp, __retstr(ret));
	else
		DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_
				 "<%2d> -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id + 1, 
				 ctx, ctx->pc, psp, __retstr(ret));
#endif
}

void __tdump(void);

void ERROR(uintptr_t __sp_ctl, 
		   uint32_t __new_thread_id,
		   uint32_t __prev_thread_id, 
		   uint32_t __sp) 
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
				 __new_thread_id + 1, 
				 ctx, ctx->pc, psp, __retstr(ret));
	else if (__new_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_ _DIM_
				 "<%2d> -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id + 1, 
				 ctx, ctx->pc, psp, __retstr(ret));
	else
		DCC_LOG6(LOG_ERROR,  _ATTR_PUSH_ _FG_RED_ 
				 "<%2d> -> <%2d> " 
				 "CTX=%08x PC=%08x MSP=%08x %s"  _ATTR_POP_,
				 __prev_thread_id + 1, __new_thread_id + 1, 
				 ctx, ctx->pc, msp, __retstr(ret));

	__context(__sp_ctl, __new_thread_id); 
	__thinkos(&thinkos_rt);
	__tdump();
#endif
}

void thinkos_sched_dbg(uintptr_t __sp_ctl, 
					   uint32_t __new_thread_id,
					   uint32_t __prev_thread_id, 
					   uint32_t __sp) 
{
	if (__new_thread_id > THINKOS_THREAD_IDLE) {
		ERROR(__sp_ctl, __new_thread_id, __prev_thread_id, __sp);
	} else if (__new_thread_id == THINKOS_THREAD_IDLE) {
		if (__prev_thread_id != THINKOS_THREAD_IDLE) {
			_IDLE(__sp_ctl, __new_thread_id, __prev_thread_id, __sp);
		} else {
			_IDLE(__sp_ctl, __new_thread_id, __prev_thread_id, __sp);
		}
	} else {
		SCHED(__sp_ctl, __new_thread_id, __prev_thread_id, __sp);
	}
}

void thinkos_sched_step_dbg(uintptr_t __sp_ctl, 
							 uint32_t __new_thread_id,
							 uint32_t __prev_thread_id, 
							 uint32_t __sp) 
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
			 __new_thread_id + 1, ctx->pc, (uintptr_t)ctx);
}

void thinkos_stack_limit_dbg(uintptr_t __sp_ctl, 
							 uint32_t __new_thread_id,
							 uint32_t __prev_thread_id, 
							 uint32_t __sp) 
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
			 __new_thread_id + 1, ctx->pc, (uintptr_t)ctx);
}

#endif

bool thinkos_dbg_thread_ctx_is_valid(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;
    int idx;

	if (id > THINKOS_THREADS_MAX) {
		return false;
	}

    idx = (__krn_brk_get(krn) == id) ? THINKOS_THREAD_VOID : id;
    return __thread_ctx_is_valid(krn, idx);
}

int thinkos_dbg_thread_brk_get(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;

    if (!__thread_ctx_is_valid(krn, THINKOS_THREAD_VOID)) {
        return -1;
    }

    return __krn_brk_get(krn);
}

struct thinkos_context * thinkos_dbg_thread_ctx_get(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;
    int idx;

	if (id > THINKOS_THREADS_MAX) {
		return NULL;
	}

    idx = (__krn_brk_get(krn) == id) ? THINKOS_THREAD_VOID : id;
    return __thread_ctx_get(krn, idx);
}

uintptr_t thinkos_dbg_thread_ctrl_get(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;
    int idx = (__krn_brk_get(krn) == id) ? THINKOS_THREAD_VOID : id;

    return __thread_ctrl_get(krn, idx);
}

uintptr_t thinkos_dbg_thread_pc_get(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;
    int idx = (__krn_brk_get(krn) == id) ? THINKOS_THREAD_VOID : id;

    return __thread_pc_get(krn, idx);
}

uint32_t thinkos_dbg_thread_lr_get(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;
    int idx = (__krn_brk_get(krn) == id) ? THINKOS_THREAD_VOID : id;

    return __thread_lr_get(krn, idx);
}

uint32_t thinkos_dbg_thread_sp_get(unsigned int id)
{
    struct thinkos_rt * krn = &thinkos_rt;
    int idx = (__krn_brk_get(krn) == id) ? THINKOS_THREAD_VOID : id;

    return __thread_sp_get(krn, idx);
}

uint32_t thinkos_dbg_thread_sl_get(unsigned int id) 
{
	return __thread_sl_get(&thinkos_rt, id); 
}

const char * thinkos_dbg_thread_tag_get(unsigned int idx) 
{
	return __thread_tag_get(&thinkos_rt, idx);
}

int thinkos_dbg_thread_wq_get(unsigned int thread_idx)
{
	return __thread_wq_get(&thinkos_rt, thread_idx);
}


#if 0
bool thinkos_dbg_thread_rec_get(unsigned int thread,
                           struct thread_rec * rec,
                           struct thread_reg_core * core,
                           struct thread_reg_float * fp)
{
    struct thinkos_rt * krn = &thinkos_rt;
    struct thinkos_context * ctx;
    unsigned int idx;

    idx = (__krn_brk_get(krn) == thread) ? thread : THINKOS_THREAD_VOID;
    ctx = __thread_ctx_get(krn, idx);

    if (ctx != NULL) {
		return false;
    }

	if (rec != NULL) {
        rec->ctrl = __thread_ctrl_get(krn, idx);
	}

	return true;
}
#endif

