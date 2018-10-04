/* 
 * File:	 usb-cdc.c
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
#include <thinkos.h>

/* -------------------------------------------------------------------------
 * Fast thread execution
 * ------------------------------------------------------------------------- */

int dbgmon_thread_create(void (* func)(void *), void * arg, 
						 const struct thinkos_thread_inf * inf)
{
	int thread_id = (inf->thread_id > 0) ? inf->thread_id - 1 : 0;
	uint32_t sp = (uint32_t)inf->stack_ptr + inf->stack_size;

	DCC_LOG(LOG_MSG, "__thinkos_thread_abort()");
	__thinkos_thread_abort(thread_id);

#if THINKOS_ENABLE_THREAD_ALLOC
	/* allocate the thread block */
	__bit_mem_wr(&thinkos_rt.th_alloc, thread_id, 1);
#endif

	DCC_LOG2(LOG_MSG, "__thinkos_thread_init(func=%p arg=%p)", func, arg);
	__thinkos_thread_init(thread_id, sp, func, arg);

#if THINKOS_ENABLE_THREAD_INFO
	DCC_LOG(LOG_MSG, "__thinkos_thread_inf_set()");
	__thinkos_thread_inf_set(thread_id, inf);
#endif

	return thread_id;
}

void dbgmon_thread_resume(int thread_id)
{
	if (__thinkos_thread_resume(thread_id))
		__thinkos_defer_sched();
} 

