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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
#include <thinkos.h>
#include <sys/dcclog.h>

void monitor_print_osinfo(const struct monitor_comm * comm, uint32_t cycref[])
{
#if (THINKOS_ENABLE_PROFILING)
    int max = thinkos_krn_threads_max();
    uint32_t cyc[max];
    uint32_t cycdiv;
    uint32_t busy;
    uint32_t cycsum = 0;
    uint32_t cycbusy;
    uint32_t cycidle;
    uint32_t idle;
#endif
	uint32_t active;
	int i;
	int j;

	active = thinkos_dbg_active_get();
	monitor_printf(comm, " Active: %d", active);

#if (THINKOS_ENABLE_PROFILING)
    cycsum = 0;

    cycsum = monitor_threads_cyc_sum(cyc, cycref, 0, max);
    cycidle = cyc[THINKOS_THREAD_IDLE];
    cycbusy = cycsum - cycidle;
    cycdiv = (cycsum + 500) / 1000;

	busy = (cycdiv == 0) ? 1000 : (cycbusy / cycdiv);
    if (busy > 1000)
        busy  = 1000;

    idle = 1000 - busy;
    (void) idle;

	monitor_printf(comm, " %u cycles, %d.%d%% busy, %d.%d%% idle", 
				   cycsum, busy / 10, busy % 10, idle / 10, idle % 10);

#endif

	monitor_printf(comm, "\r\n");

	monitor_printf(comm, "  #"); 
	monitor_printf(comm, "     Tag"); 
	monitor_printf(comm, " |    Stack"); 
	monitor_printf(comm, " |       SP"); 
	monitor_printf(comm, " |       PC"); 
	monitor_printf(comm, " | Status"); 
#if (THINKOS_ENABLE_TIMESHARE)
	/* TODO: add timeshare info */
#endif
	monitor_printf(comm, " |  Clk (ms)"); 
#if (THINKOS_ENABLE_PROFILING)
	monitor_printf(comm, " | CPU %%"); 
#endif

#if (THINKOS_MUTEX_MAX) > 0
	monitor_printf(comm, " | Lock\r\n"); 
#else
	monitor_printf(comm, " |\r\n");
#endif

	for (i = THINKOS_THREAD_FIRST; i <= THINKOS_THREAD_LAST; ++i) {
		if (thinkos_dbg_thread_ctx_is_valid(i)) {
			const char * tag;
			uint32_t sl;
			uint32_t sp;
			uint32_t pc;
			int oid;
			int type;
			int irq;
			int errno; 
			bool tmw;

			monitor_printf(comm, "%3d", i);
			tag = thinkos_dbg_thread_tag_get(i);
			sl = thinkos_dbg_thread_sl_get(i);
			sp = thinkos_dbg_thread_sp_get(i);
			pc = thinkos_dbg_thread_pc_get(i);

			monitor_printf(comm, " %7s | %08x | %08x | %08x", 
						   tag, sl, sp, pc); 

			oid = thinkos_dbg_thread_wq_get(i);
			tmw = thinkos_dbg_thread_tmw_get(i);
			if ((errno = thinkos_dbg_thread_errno_get(i)) > 0) {
				monitor_printf(comm, " | ERR %2d", errno);
			} else if ((irq = thinkos_dbg_thread_irq_get(i)) >= 0) {
				monitor_printf(comm, " | IRQ %2d", irq);
			} else if (thinkos_dbg_thread_is_ready(i)) {
				monitor_printf(comm, " | READY ");
			} else {
				type = __thinkos_obj_kind(oid);
				monitor_printf(comm, " | %c%c %3d", 
						 tmw ? 'T' : ' ',
						 __thinkos_kind_prefix(type), 
						 oid);
			}
#if (THINKOS_ENABLE_TIMESHARE)
			/* TODO: add timeshare info */
#endif
			{
				int32_t dt = thinkos_dbg_thread_clk_itv_get(i);
#if 0
				int32_t sec;
				int32_t ms;

				sec = dt / 1000;
				ms = ((dt < 0) ? -dt : dt) % 1000;

				monitor_printf(comm, " |%7d.%03d", sec, ms);
#else
				monitor_printf(comm, " |%10d", dt);
#endif

			}
#if (THINKOS_ENABLE_PROFILING)
			busy = (cycdiv == 0) ? 1000 : (cyc[i] / cycdiv);
			if (busy > 1000)
				busy  = 1000;
			monitor_printf(comm, " | %3d.%d", busy / 10, busy % 10);
#endif
			monitor_printf(comm, " |");
#if (THINKOS_MUTEX_MAX) > 0
			for (j = THINKOS_MUTEX_FIRST; j <= THINKOS_MUTEX_LAST; ++j) {
				if (thinkos_dbg_mutex_lock_get(j) == i)
					monitor_printf(comm, " %d", j + THINKOS_MUTEX_BASE);
			}
#endif 
			monitor_printf(comm, "\r\n");
		}
	}


	for (j = THINKOS_OBJECT_FIRST; j <= THINKOS_OBJECT_LAST; ++j) {
		struct thread_waitqueue * wq;
		int type;

		wq = thinkos_dbg_wq_from_oid(j);
		if (!thinkos_dbg_wq_is_empty(wq)) { 
			type = __thinkos_obj_kind(j);
			monitor_printf(comm, "%3d %5s: {", j, __thinkos_kind_name(type));
			for (i = THINKOS_THREAD_FIRST; i <= THINKOS_THREAD_LAST; ++i) {
				if (thinkos_dbg_wq_contains(wq, i))
					monitor_printf(comm, " %d", i);
			}
			monitor_printf(comm, " }");
#if (THINKOS_MUTEX_MAX) > 0
			if (type == THINKOS_OBJ_MUTEX)
				monitor_printf(comm, " [lock=%d]", 
							   thinkos_dbg_mutex_lock_get(j));
#endif 
			monitor_printf(comm, "\r\n");
		}
	}
}

