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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_APP__
#include <thinkos/app.h>
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>

#include <thinkos.h>
#include <sys/dcclog.h>
#include <vt100.h>
#include <sys/delay.h>

#if (THINKOS_ENABLE_APP)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#ifdef THINKOS_CUSTOM_APP_TAG
  static const uint8_t tag[8] = THINKOS_CUSTOM_APP_TAG;
#else
  static const uint8_t tag[8] = "ThinkApp";
#endif

const struct magic_blk flat_app_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 8
		},
	.rec = {
		{.mask = 0xff0000ff, .comp = 0x08000041 },
		{.mask = 0x0000000f, .comp = 0x00000000 },
		{.mask = 0x00000000, .comp = 0x00000000 },
		{.mask = 0x00000000, .comp = 0x00000000 },
		{.mask = 0xffffffff, .comp = 0x6e696854 },
		{.mask = 0xffffffff, .comp = 0x00534f6b },
		{.mask = 0xffffffff, 
	     .comp = tag[0] + (tag[1]<<8) + (tag[2]<<16) + (tag[3]<<24) },
		{.mask = 0xffffffff, 
         .comp = tag[4] + (tag[5]<<8) + (tag[6]<<16) + (tag[7]<<24) },
	}
};
#pragma GCC diagnostic pop


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

static bool magic_match(const struct magic_blk * magic, uint32_t * mem)
{
	int k;
	int j;

	k = magic->hdr.pos;
	for (j = 0; j < magic->hdr.cnt; ++j) {
		if ((mem[k++] & magic->rec[j].mask) != magic->rec[j].comp)
			return false;
	}	

	return true;
}

int thinkos_flat_check(const struct flat_app * app)
{
	uint32_t * mem = (uint32_t *)app;
	int32_t size;
#if (THINKOS_ENABLE_APP_CRC)
	uint32_t crc;
	uint32_t chk;
#endif
	uintptr_t addr;
	bool ret;

	DCC_LOG2(LOG_TRACE, "pc=0x%08x sp=0x%08x", app->entry, app->stack);

	if (!(ret = magic_match(&flat_app_magic, mem))) {
		DCC_LOG(LOG_ERROR, "invalid application block!");
		return THINKOS_ERR_APP_INVALID;
	}

#if (THINKOS_ENABLE_APP_CRC)
	size = (app->size + 3) & ~3; /* Block alignment */
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
	__bit_mem_wr(&krn->th_alloc, thread_idx, 1);
#endif

	stack_top = app->stack;
	stack_size = app->stack_size;
	stack_base = stack_top - stack_size;
#if (DEBUG)
	if ((stack_top > (uintptr_t)&__krn_stack_start) &&
		(stack_base < (uintptr_t)&__krn_stack_end)) {
		intptr_t diff;
		diff = stack_top - (uintptr_t)&__krn_stack_start;
		
		DCC_LOG1(LOG_WARNING, "stack collision, moving by %d!", diff);
		/* Stack colision */
		stack_top -= diff;
		stack_size -= diff;
	}
#endif
	task_entry = app->entry;
	task_exit = app->entry;

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
		DCC_LOG2(LOG_ERROR, "<%2d> invalid thread %d!", self + 1, 
				 thread_idx + 1);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if ((ret = thinkos_krn_app_start(krn, thread_idx, addr))) {
		DCC_LOG2(LOG_ERROR, "<%2d> thinkos_krn_app_start failed: %d!", 
				 self + 1, ret);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	};

	if (thread_idx == self) {
#if (THINKOS_ENABLE_SANITY_CHECK)
		if (__thread_active_get(krn) != self) {
			DCC_LOG2(LOG_ERROR, "<%2d> thinkos_krn_app_start failed: %d!", 
				 self + 1, ret);
		}
#endif
		__thread_active_set(krn, THINKOS_THREAD_VOID);
		__thinkos_defer_sched();
	}

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	arg[0] = thread_idx + 1;

	return;
}

#endif /* (THINKOS_ENABLE_APP) */


