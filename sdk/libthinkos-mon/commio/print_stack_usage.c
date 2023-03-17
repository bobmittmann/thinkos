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

#include "thinkos_mon-i.h"

#if (THINKOS_ENABLE_MONITOR)

extern uint32_t thinkos_monitor_stack[];
extern const uint16_t thinkos_monitor_stack_size;

extern uint32_t thinkos_except_stack[];
extern const uint16_t thinkos_except_stack_size;

void monitor_print_stack_usage(const struct monitor_comm * comm)
{
	struct monitor_swap * swp;
	unsigned int size;
	const char * tag;
	uint32_t sl;
	uint32_t sp;
	uint32_t pc;
	int free;
	int i;

	monitor_printf(comm, "\r\n Th"); 
	monitor_printf(comm, " |     Tag"); 
	monitor_printf(comm, " |       PC"); 
	monitor_printf(comm, " |    Stack"); 
	monitor_printf(comm, " |  Context"); 
	monitor_printf(comm, " |   Size"); 
	monitor_printf(comm, " |   Free"); 
	monitor_printf(comm, "\r\n");

	for (i = THINKOS_THREAD_FIRST; i <= THINKOS_THREAD_LAST; ++i) {
		if (thinkos_dbg_thread_ctx_is_valid(i)) {
			uint32_t base;

			tag = thinkos_dbg_thread_tag_get(i);
			sp = thinkos_dbg_thread_sp_get(i);
			pc = thinkos_dbg_thread_pc_get(i);
			sl = thinkos_dbg_thread_sl_get(i);
			base = thinkos_dbg_thread_stack_base_get(i);
			size = thinkos_dbg_thread_stack_size_get(i);
			free = __thinkos_scan_stack((void *)base, size);

			monitor_printf(comm, "%3d | %7s | %08x | %08x | %08x | %6d "
						   "| %6d\r\n", i, tag, pc, sl, sp, size, free);

		}
	}

	tag = "<MON>";
	sp = cm3_sp_get();
	pc = cm3_pc_get();
	sl = (uintptr_t)thinkos_monitor_stack;
	size = thinkos_monitor_stack_size;
	free = __thinkos_scan_stack((void *)sl, size);

	monitor_printf(comm, "%3d | %7s | %08x | %08x | %08x | %6d "
				   "| %6d\r\n", -1, tag, pc, sl, sp, size, free);

	tag = "<IRQ>";
	swp = (struct monitor_swap *)thinkos_rt.monitor.ctx;
	sp = (uintptr_t)swp;
	pc = swp->lr;
	sl = (uintptr_t)thinkos_except_stack;
	size = thinkos_except_stack_size;
	free = __thinkos_scan_stack((void *)sl, size);

	monitor_printf(comm, "%3d | %7s | %08x | %08x | %08x | %6d "
				   "| %6d\r\n", -2, tag, pc, sl, sp, size, free);
}

#endif

