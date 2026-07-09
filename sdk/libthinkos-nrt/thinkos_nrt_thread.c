/* 
 * thinkos_nonrt_thread.c
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

#define __THINKOS_NRT__
#include <thinkos/nrt.h>
#include <thinkos.h>
#include <stdbool.h>
#include <sys/dcclog.h>

#include "thinkos_nrt-i.h"

bool thinkos_nrt_thread_resume(unsigned int thread_id)
{
	return true;
}

int thinkos_nrt_thread_init(unsigned int thread_id)
{
	return true;
}

void thinkos_nrt_thread_alloc_svc(int32_t arg[], int self, struct thinkos_krn * krn)
{
	int thread_id = 1;

	arg[SVC_RETURN] = 0x8000 + thread_id;
}

void __attribute__((noreturn, noinline)) thread_nrt_at_exit(int code)
{
	thinkos_exit(code);
	for(;;);
}

int __attribute__((noinline)) 
thinkos_nrt_thread_create(thinkos_task_t task_ptr, void * task_arg, 
					  void * stack_ptr, uint32_t opt) 
{
	struct thinkos_thread_initializer init;
	uint32_t stack_size = opt & 0xffff;
	int priority = (opt >> 16) & 0xff;
	int hint = (opt >> 24) & 0x3f;
	int paused = (opt >> 31) & 1;
	uintptr_t stack_base = (uintptr_t)stack_ptr;
	uintptr_t stack_top;
	int thread;
	int ret;

	if ((thread = thinkos_obj_thread_alloc(hint)) < 0) {
		return thread;
	}

	stack_top = stack_base + stack_size;  
	/* ensure page alignment */
	stack_top &= ~0x1f;
	stack_size = stack_top - stack_base;

	init.stack_base = stack_base;
	init.stack_size = stack_size;
	init.task_entry = (uintptr_t)task_ptr;
	init.task_exit = (uintptr_t)thread_nrt_at_exit;
	init.task_arg[0] = (uintptr_t)task_arg;
	init.task_arg[1] = thread;
	init.task_arg[2] = 0;
	init.task_arg[3] = 0;
	init.priority = priority;
	init.paused = paused;
	init.privileged = 0;
	init.inf = NULL;

	if ((ret = thinkos_thread_init(thread, &init)) < 0) {
		thinkos_obj_free(thread);
		return ret;
	}

	return thread;
}

