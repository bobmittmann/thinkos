/* 
 * File:	 thinkos_app.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "thinkos_krn-i.h"

#define __THINKOS_APP__
#include <thinkos/app.h>

#define __THINKOS_DEBUG__
#include <thinkos/debug.h>

#include <sys/dcclog.h>

#if (THINKOS_ENABLE_APP)

void thinkos_flat_dump(const struct flat_app * app)
{
	DCC_LOG2(LOG_TRACE, "pc=0x%08x sp=0x%08x", app->entry, app->stack);
	DCC_LOG2(LOG_TRACE, ".text 0x%08x - 0x%08x", 
			 app->text.start, app->text.end);
	DCC_LOG2(LOG_TRACE, ".data 0x%08x - 0x%08x", 
			 app->data.start, app->data.end);
	DCC_LOG2(LOG_TRACE, ".bss  0x%08x - 0x%08x", 
			 app->bss.start, app->bss.end);
	DCC_LOG2(LOG_TRACE, ".ctor 0x%08x - 0x%08x", 
			 app->ctor.start, app->ctor.end);
}

int thinkos_flat_check(const struct flat_app * app)
{
#if (THINKOS_ENABLE_APP_CRC)
	uint32_t * mem = (uint32_t *)app;
	uint32_t crc;
	uint32_t chk;
#endif
	int32_t size;
	uintptr_t addr;

	DCC_LOG3(LOG_TRACE, "app=0x%08x pc=0x%08x sp=0x%08x", app,
			 app->entry, app->stack);

	addr = (uintptr_t)app;
#if (THINKOS_ENABLE_SANITY_CHECK)
	if (!__thinkos_mem_usr_rd_chk(addr, sizeof(struct flat_app))) {
		DCC_LOG1(LOG_ERROR, "invalid pointer: addr=0x%08x", addr);
		return THINKOS_ERR_APP_INVALID;
	}
#endif /* (THINKOS_ENABLE_SANITY_CHECK) */

	app = (const struct flat_app *)addr;
	if (app->entry != addr + 0x41) {
		return THINKOS_ERR_APP_INVALID;
	}

#if (THINKOS_ENABLE_SANITY_CHECK)
	/* can we read and write at the application stack ? */ 
	addr = app->stack;
	size = app->stksz;
	if ((size < 0) || !__thinkos_mem_usr_rw_chk(addr, size)) {
		DCC_LOG2(LOG_ERROR, ".text section invalid addr=0x%08x size=%d", 
				 addr, size);
		return THINKOS_ERR_APP_INVALID;
	}
#endif

	size = (app->size + 3) & ~3; /* Block alignment */

#if (THINKOS_ENABLE_SANITY_CHECK)
	/* can we read and execute from the application address ? */ 
	if (!__thinkos_mem_usr_rx_chk(addr, size)) {
		DCC_LOG1(LOG_ERROR, "invalid pointer: addr=0x%08x", addr);
		return THINKOS_ERR_APP_INVALID;
	}
#endif /* (THINKOS_ENABLE_SANITY_CHECK) */

#if (THINKOS_ENABLE_APP_CRC)
	DCC_LOG1(LOG_TRACE, "app size=%d", size);
	crc = __thinkos_crc32_u32((uint32_t *)mem, size);
	chk = mem[size >> 2];

	if (crc != chk) {
		DCC_LOG2(LOG_ERROR, "crc=0x%08x chk=0x%08x mismatch!", crc, chk);
		return THINKOS_ERR_APP_CRC_ERROR      ;
	}
	DCC_LOG2(LOG_TRACE, "crc=0x%08x chk=0x%08x", crc, chk);
#endif

	(void)addr;
#if (THINKOS_ENABLE_SANITY_CHECK)
	size = (app->text.end - app->text.start);
	addr = app->text.start;

	if ((size < 0) || !__thinkos_mem_usr_rx_chk(addr, size)) {
		DCC_LOG2(LOG_ERROR, ".text section invalid addr=0x%08x size=%d", 
				 addr, size);
		return THINKOS_ERR_APP_CODE_INVALID;
	}
	size = (app->data.end - app->data.start);
	addr = app->data.start;
	if ((size < 0) || !__thinkos_mem_usr_rw_chk(addr, size)) {
		DCC_LOG2(LOG_ERROR, ".data section invalid addr=0x%08x size=%d", 
				 addr, size);
		return THINKOS_ERR_APP_DATA_INVALID;
	}
	size = (app->bss.end - app->bss.start);
	addr = app->bss.start;
	if ((size < 0) || !__thinkos_mem_usr_rw_chk(addr, size)) {
		DCC_LOG2(LOG_ERROR, ".bss section invalid addr=0x%08x size=%d", 
				 addr, size);
		return THINKOS_ERR_APP_BSS_INVALID;
	}
	size = (app->ctor.end - app->ctor.start);
	addr = app->ctor.start;
	if ((size < 0) || !__thinkos_mem_usr_rd_chk(addr, size)) {
		DCC_LOG2(LOG_ERROR, ".ctor table invalid addr=0x%08x size=%d", 
				 addr, size);
		return THINKOS_ERR_APP_BSS_INVALID;
	}
#endif /* (THINKOS_ENABLE_SANITY_CHECK) */

	return 0;
}

/* -------------------------------------------------------------------------
 * Application execution
 * ------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_THREAD_INFO)
const struct thinkos_thread_inf thinkos_app_inf = {
	.tag = "APP",
	.stack_ptr = 0,
	.stack_size = 1024,
	.priority = 0,
	.thread_id = 1,
	.paused = 0
};
#endif

extern void * __krn_stack_start;
extern void * __krn_stack_end;
extern int __krn_stack_size;

void __attribute__((noreturn, noinline)) krn_app_at_exit(int code)
{
	DCC_LOG1(LOG_WARNING, VT_PSH VT_REV VT_FYW "app exit, code=%d ! " VT_POP, 
			 code);
	thinkos_abort();
}

int thinkos_krn_app_start(struct thinkos_rt * krn, unsigned int thread_idx,
						 uintptr_t addr)
{
	struct thinkos_thread_initializer init;
	const struct flat_app * app = (struct flat_app *)addr;
	const struct thinkos_thread_inf * inf = &thinkos_app_inf;
	uintptr_t stack_top;
	uintptr_t stack_size;
	uintptr_t stack_base;
	uintptr_t task_entry;
	uintptr_t task_exit;
	int ret;

	if ((ret = thinkos_flat_check(app))) {
		DCC_LOG(LOG_ERROR, "thinkos_flat_check failed!");
		return ret;
	}

	thinkos_flat_dump(app);

#if THINKOS_ENABLE_THREAD_ALLOC
	/* force allocate the thread block */
	__thread_alloc_set(krn, thread_idx);
#endif

	stack_top = app->stack;
	stack_size = app->stksz;

#if (DEBUG)
	/* Prevent the application stack to overlap with the DCC buffer
	   at the stack top */
	DCC_LOG2(LOG_TRACE, "krn stack: 0x%08x - 0x%08x", 
	 (uintptr_t)&__krn_stack_start,
	 (uintptr_t)&__krn_stack_end);

	if (stack_top > (uintptr_t)&__krn_stack_end) {
		intptr_t diff;

		diff = stack_top - (uintptr_t)&__krn_stack_end;
		
		DCC_LOG1(LOG_WARNING, "stack collision, moving by %d!", diff);
		/* Stack colision */
		stack_top -= diff;
		stack_size -= diff;
	}
#endif

	/* ensure page alignment */
	stack_size &= ~(STACK_ALIGN_MSK);
	stack_top &= ~(STACK_ALIGN_MSK);
	stack_base = stack_top - stack_size;  
	
	DCC_LOG2(LOG_TRACE, "app stack: 0x%08x - 0x%08x", 
	 (uintptr_t)stack_base, (uintptr_t)stack_top);

	task_entry = app->entry;
	task_exit = (uintptr_t)krn_app_at_exit;

	init.stack_base = stack_base;
	init.stack_size = stack_size;
	init.task_entry = task_entry;
	init.task_exit = task_exit;
	init.task_arg[0] = 0;
	init.task_arg[1] = 0;
	init.task_arg[2] = 0;
	init.task_arg[3] = 0;
	init.priority = 0;
	init.paused = false;
	init.privileged = true;
	init.inf = inf;

	return thinkos_krn_thread_init(krn, thread_idx, &init);
}

void thinkos_app_exec_svc(int32_t * arg, unsigned int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int thread_idx;
	uintptr_t addr = arg[0];
	int ret;

	/* collect call arguments */
	addr = arg[0];
	
	DCC_LOG1(LOG_TRACE, "<%2d>.........................", self);

	thread_idx = self;

	if (thread_idx >= (THINKOS_THREADS_MAX) + (THINKOS_NRT_THREADS_MAX)) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid thread %d!", self, 
				 thread_idx);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if ((ret = thinkos_krn_app_start(krn, thread_idx, addr))) {
		DCC_LOG2(LOG_ERROR, "<%2d> thinkos_krn_app_start failed: %d!", 
				 self, ret);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	};

	if (thread_idx == self) {
#if (THINKOS_ENABLE_SANITY_CHECK)
		if (__krn_sched_active_get(krn) != self) {
			DCC_LOG2(LOG_ERROR, "<%2d> sched.act=%d!", self, 
					 __krn_sched_active_get(krn));
		}
#endif
		DCC_LOG1(LOG_WARNING, "<%2d> self == thread_idx, discarding...", self);
		__krn_sched_active_clr(krn);
		__krn_defer_sched(krn);
	}

	arg[0] = thread_idx;

	return;
}

#endif /* (THINKOS_ENABLE_APP) */

