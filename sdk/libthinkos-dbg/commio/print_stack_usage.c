/* 
 * File:	 .c
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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>
#include <sys/dcclog.h>

int __scan_stack(void * stack, unsigned int size);

extern uint32_t thinkos_dbgmon_stack[];
extern const uint16_t thinkos_dbgmon_stack_size;
extern uint32_t thinkos_except_stack[];
extern const uint16_t thinkos_except_stack_size;
extern const uint32_t thinkos_dbgmon_rt[];

void dbgmon_print_stack_usage(const struct dbgmon_comm * comm)
{
	struct thinkos_rt * rt = &thinkos_rt;
	int i;

	dbgmon_printf(comm, "\r\n Th"); 
	dbgmon_printf(comm, " |     Tag"); 
	dbgmon_printf(comm, " |    Stack"); 
	dbgmon_printf(comm, " |  Context"); 
	dbgmon_printf(comm, " |   Size"); 
	dbgmon_printf(comm, " |   Free"); 
	dbgmon_printf(comm, "\r\n");

	for (i = 0; i <= THINKOS_THREADS_MAX; ++i) {
		if (rt->ctx[i] != NULL) {
			/* Internal thread ids start form 0 whereas user
			   thread numbers start form one ... */
			dbgmon_printf(comm, "%3d", i + 1);
#if THINKOS_ENABLE_THREAD_INFO
			if (rt->th_inf[i] != NULL) {
				dbgmon_printf(comm, " | %7s", rt->th_inf[i]->tag); 
				dbgmon_printf(comm, " | %08x", (uint32_t)rt->th_inf[i]->stack_ptr); 
			} else 
#endif
			{
				dbgmon_printf(comm, " |     ..."); 
				dbgmon_printf(comm, " |      ..."); 
			}
			dbgmon_printf(comm, " | %08x", (uint32_t)rt->ctx[i]); 
#if THINKOS_ENABLE_THREAD_INFO
			if (rt->th_inf[i] != NULL) {
				dbgmon_printf(comm, " | %6d", rt->th_inf[i]->stack_size); 
				dbgmon_printf(comm, " | %6d", __scan_stack(rt->th_inf[i]->stack_ptr, 
												rt->th_inf[i]->stack_size));
			} else 
#endif
			{
				dbgmon_printf(comm, " |    ..."); 
				dbgmon_printf(comm, " |    ..."); 
			}
			dbgmon_printf(comm, "\r\n");
		}
	}

	dbgmon_printf(comm, "%3d", -1);
	dbgmon_printf(comm, " |   <DBG>"); 
	dbgmon_printf(comm, " | %08x", (uint32_t)thinkos_dbgmon_stack); 
	dbgmon_printf(comm, " | %08x", (uint32_t)cm3_msp_get()); 
	dbgmon_printf(comm, " | %6d", thinkos_dbgmon_stack_size); 
	dbgmon_printf(comm, " | %6d\r\n", __scan_stack(thinkos_dbgmon_stack, 
										thinkos_dbgmon_stack_size));
	dbgmon_printf(comm, "%3d", -2);
	dbgmon_printf(comm, " |   <IRQ>"); 
	dbgmon_printf(comm, " | %08x", (uint32_t)thinkos_except_stack); 
	dbgmon_printf(comm, " | %08x", thinkos_dbgmon_rt[0]); 
	dbgmon_printf(comm, " | %6d", thinkos_except_stack_size); 
	dbgmon_printf(comm, " | %6d\r\n", __scan_stack(thinkos_except_stack, 
										thinkos_except_stack_size));
}

