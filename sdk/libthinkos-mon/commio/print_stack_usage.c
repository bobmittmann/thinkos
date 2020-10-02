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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <thinkos.h>
#include <sys/dcclog.h>

int __scan_stack(void * stack, unsigned int size);

extern uint32_t thinkos_monitor_stack[];
extern const uint16_t thinkos_monitor_stack_size;
extern uint32_t thinkos_except_stack[];
extern const uint16_t thinkos_except_stack_size;
extern const uint32_t thinkos_monitor_rt[];

void monitor_print_stack_usage(const struct monitor_comm * comm)
{
	struct thinkos_rt * rt = &thinkos_rt;
	int i;

	monitor_printf(comm, "\r\n Th"); 
	monitor_printf(comm, " |     Tag"); 
	monitor_printf(comm, " |    Stack"); 
	monitor_printf(comm, " |  Context"); 
	monitor_printf(comm, " |   Size"); 
	monitor_printf(comm, " |   Free"); 
	monitor_printf(comm, "\r\n");

	for (i = 0; i <= THINKOS_THREADS_MAX; ++i) {
		if (rt->ctx[i] != NULL) {
			/* Internal thread ids start form 0 whereas user
			   thread numbers start form one ... */
			monitor_printf(comm, "%3d", i + 1);
#if THINKOS_ENABLE_THREAD_INFO
			if (rt->th_inf[i] != NULL) {
				monitor_printf(comm, " | %7s", rt->th_inf[i]->tag); 
				monitor_printf(comm, " | %08x", (uint32_t)rt->th_inf[i]->stack_ptr); 
			} else 
#endif
			{
				monitor_printf(comm, " |     ..."); 
				monitor_printf(comm, " |      ..."); 
			}
			monitor_printf(comm, " | %08x", (uint32_t)rt->ctx[i]); 
#if THINKOS_ENABLE_THREAD_INFO
			if (rt->th_inf[i] != NULL) {
				monitor_printf(comm, " | %6d", rt->th_inf[i]->stack_size); 
				monitor_printf(comm, " | %6d", __scan_stack(rt->th_inf[i]->stack_ptr, 
												rt->th_inf[i]->stack_size));
			} else 
#endif
			{
				monitor_printf(comm, " |    ..."); 
				monitor_printf(comm, " |    ..."); 
			}
			monitor_printf(comm, "\r\n");
		}
	}

	monitor_printf(comm, "%3d", -1);
	monitor_printf(comm, " |   <DBG>"); 
	monitor_printf(comm, " | %08x", (uint32_t)thinkos_monitor_stack); 
	monitor_printf(comm, " | %08x", (uint32_t)cm3_msp_get()); 
	monitor_printf(comm, " | %6d", thinkos_monitor_stack_size); 
	monitor_printf(comm, " | %6d\r\n", __scan_stack(thinkos_monitor_stack, 
										thinkos_monitor_stack_size));
	monitor_printf(comm, "%3d", -2);
	monitor_printf(comm, " |   <IRQ>"); 
	monitor_printf(comm, " | %08x", (uint32_t)thinkos_except_stack); 
	monitor_printf(comm, " | %08x", thinkos_monitor_rt[0]); 
	monitor_printf(comm, " | %6d", thinkos_except_stack_size); 
	monitor_printf(comm, " | %6d\r\n", __scan_stack(thinkos_except_stack, 
										thinkos_except_stack_size));
}

