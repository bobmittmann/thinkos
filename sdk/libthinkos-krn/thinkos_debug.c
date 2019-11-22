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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#include <thinkos.h>
#include <vt100.h>
 
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
			type = thinkos_obj_type_get(oid);
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
			case THINKOS_OBJ_FAULT:
				DCC_LOG2(LOG_TRACE, "FAULT %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_INVALID:
				DCC_LOG2(LOG_TRACE, "INVALID %d: 0x%08x", oid, *wq);
				break;
			}
		}
	}

#if (THINKOS_ENABLE_CLOCK)
	DCC_LOG1(LOG_TRACE, "Ticks = %d", rt->ticks);
#endif
	DCC_LOG1(LOG_TRACE, "Active = %d", rt->active);

	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		struct thinkos_context * ctx;

		if ((ctx = rt->ctx[i]) != NULL) {
#if (THINKOS_ENABLE_THREAD_STAT)
			DCC_LOG5(LOG_TRACE, "<%2d> %3d sp=%08x lr=%08x pc=%08x", 
					 i + 1, rt->th_stat[i] >> 1, (uint32_t)ctx, ctx->lr, ctx->pc);
#else
			DCC_LOG4(LOG_TRACE, "<%2d> sp=%08x lr=%08x pc=%08x", 
					 i + 1, (uint32_t)ctx, ctx->lr, ctx->pc);
#endif
		}
	}


}

void __context(struct thinkos_context * __ctx, uint32_t __thread_id)
{
	DCC_LOG4(LOG_TRACE, "  r0=%08x  r1=%08x  r2=%08x  r3=%08x", 
			 __ctx->r0, __ctx->r1, __ctx->r2, __ctx->r3);
	DCC_LOG4(LOG_TRACE, "  r4=%08x  r5=%08x  r6=%08x  r7=%08x", 
			 __ctx->r4, __ctx->r7, __ctx->r6, __ctx->r7);
	DCC_LOG4(LOG_TRACE, "  r8=%08x  r9=%08x r10=%08x r11=%08x", 
			 __ctx->r8, __ctx->r9, __ctx->r10, __ctx->r11);
#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	DCC_LOG4(LOG_TRACE, " r12=%08x  sp=%08x  lr=%08x  pc=%08x", 
			 __ctx->r12, __ctx->sp, __ctx->lr, __ctx->pc);
	DCC_LOG4(LOG_TRACE, "xpsr=%08x ret=%08x ctx=%08x th=%d", 
			 __ctx->xpsr, __ctx->ret, __ctx, __thread_id + 1);
#else
	DCC_LOG4(LOG_TRACE, " r12=%08x  sp=%08x  lr=%08x  pc=%08x", 
			 __ctx->r12, __ctx, __ctx->lr, __ctx->pc);
	DCC_LOG2(LOG_TRACE, "xpsr=%08x th=%d", __ctx->xpsr, __thread_id + 1);
#endif
}

/*
   0xFFFFFFE1 | Handler mode | Main         | Extended
   0xFFFFFFE9 | Thread mode  | Main         | Extended
   0xFFFFFFED | Thread mode  | Process      | Extended
   0xFFFFFFF1 | Handler mode | Main         | Basic
   0xFFFFFFF9 | Thread mode  | Main         | Basic
   0xFFFFFFFD | Thread mode  | Process      | Basic
   */

void SCHED(struct thinkos_context * __ctx, 
			 uint32_t __new_thread_id,
			 uint32_t __prev_thread_id, 
			 uint32_t __sp) 
{
	uint32_t ctrl = cm3_control_get();
	uint32_t msp = cm3_msp_get();
	uint32_t psp = cm3_psp_get();
#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	uint32_t ret = __ctx->ret;
#else
	uint32_t ret = 0xfffffffd;
#endif

	if (ctrl & CONTROL_SPSEL)
		psp = __sp;
	else
		msp = __sp;

	(void)ctrl;
	(void)msp;
	(void)psp;
	(void)ret;

	if (__prev_thread_id == THINKOS_THREAD_IDLE) {
		DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_YELLOW_ 
				 "IDLE -> <%2d> CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_,
				 __new_thread_id + 1, __ctx, __ctx->pc, 
				 msp, __retstr(ret));
#if THINKOS_ENABLE_THREAD_VOID
	} else if (__prev_thread_id == THINKOS_THREAD_VOID) {
		DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_YELLOW_ 
				 "VOID -> <%2d> CTX=%08x PC=%08x MSP=%08x %s" _ATTR_POP_, 
				 __new_thread_id + 1, __ctx, __ctx->pc, 
				 msp, __retstr(ret));
#endif
	} else {
		DCC_LOG6(LOG_TRACE, 
				 "<%2d> -> <%2d> " 
				 "CTX=%08x PC=%08x MSP=%08x %s",
				 __prev_thread_id + 1, __new_thread_id + 1, 
				 __ctx, __ctx->pc, msp, __retstr(ret));
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

void _IDLE(struct thinkos_context * __ctx, 
			 uint32_t __new_thread_id,
			 uint32_t __prev_thread_id, 
			 uint32_t __sp) 
{
#if DEBUG
	uint32_t psp = cm3_psp_get();
#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	uint32_t ret = __ctx->ret;
#else
	uint32_t ret = 0xfffffffd;
#endif

	if (__prev_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG4(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_ _DIM_
				 "IDLE -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
#if THINKOS_ENABLE_THREAD_VOID
	else if (__prev_thread_id == THINKOS_THREAD_VOID)
		DCC_LOG4(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_
				 "VOID -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
#endif
	else
		DCC_LOG5(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_
				 "<%2d> -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id + 1, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
#endif
}

void __tdump(void);

void ERROR(struct thinkos_context * __ctx, 
			 uint32_t __new_thread_id,
			 uint32_t __prev_thread_id, 
			 uint32_t __sp) 
{
#if DEBUG
	uint32_t msp = cm3_msp_get();
	uint32_t psp = cm3_psp_get();
#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	uint32_t ret = __ctx->ret;
#else
	uint32_t ret = 0xfffffffd;
#endif

	if (__prev_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_ _DIM_
				 "IDLE -> <%2d> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __new_thread_id + 1, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
#if THINKOS_ENABLE_THREAD_VOID
	else if (__prev_thread_id == THINKOS_THREAD_VOID)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_
				 "VOID -> <%2d> CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __new_thread_id + 1, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
	else if (__new_thread_id == THINKOS_THREAD_VOID)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_
				 "<%2d> VOID -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id + 1, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
#endif
	else if (__new_thread_id == THINKOS_THREAD_IDLE)
		DCC_LOG5(LOG_ERROR, _ATTR_PUSH_ _FG_RED_ _DIM_
				 "<%2d> -> IDLE CTX=%08x PC=%08x PSP=%08x %s" _ATTR_POP_, 
				 __prev_thread_id + 1, 
				 __ctx, __ctx->pc, psp, __retstr(ret));
	else
		DCC_LOG6(LOG_ERROR,  _ATTR_PUSH_ _FG_RED_ 
				 "<%2d> -> <%2d> " 
				 "CTX=%08x PC=%08x MSP=%08x %s"  _ATTR_POP_,
				 __prev_thread_id + 1, __new_thread_id + 1, 
				 __ctx, __ctx->pc, msp, __retstr(ret));

	__context(__ctx, __new_thread_id); 
	__thinkos(&thinkos_rt);
	__tdump();
#endif
}

void thinkos_sched_dbg(struct thinkos_context * __ctx, 
					   uint32_t __new_thread_id,
					   uint32_t __prev_thread_id, 
					   uint32_t __sp) 
{
#if THINKOS_ENABLE_THREAD_VOID
	if ((__prev_thread_id > THINKOS_THREAD_VOID) || 
		(__new_thread_id > THINKOS_THREAD_IDLE)) {
#else
	if (__new_thread_id > THINKOS_THREAD_IDLE) {
#endif
		ERROR(__ctx, __new_thread_id, __prev_thread_id, __sp);
	} else if (__new_thread_id == THINKOS_THREAD_IDLE) {
		if (__prev_thread_id != THINKOS_THREAD_IDLE) {
			_IDLE(__ctx, __new_thread_id, __prev_thread_id, __sp);
		} else {
			_IDLE(__ctx, __new_thread_id, __prev_thread_id, __sp);
		}
	} else {
		SCHED(__ctx, __new_thread_id, __prev_thread_id, __sp);
	}
}

void thinkos_sched_step_dbg(struct thinkos_context * __ctx, 
							 uint32_t __new_thread_id,
							 uint32_t __prev_thread_id, 
							 uint32_t __sp) 
{
#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	DCC_LOG3(LOG_WARNING,  _ATTR_PUSH_ _FG_GREEN_ 
			 "<%2d> STEP " 
			 "PC=%08x SP=%08x"  _ATTR_POP_, 
			 __new_thread_id + 1, __ctx->pc, __ctx->sp);
#else
	DCC_LOG3(LOG_WARNING,  _ATTR_PUSH_ _FG_GREEN_ 
			 "<%2d> STEP " 
			 "PC=%08x SP=%08x"  _ATTR_POP_, 
			 __new_thread_id + 1, __ctx->pc, (uintptr_t)__ctx);
#endif
}

#endif

