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

void monitor_print_osinfo(const struct monitor_comm * comm, 
						  struct thinkos_rt * rt,
						  uint32_t cycref[])
{
#if (THINKOS_ENABLE_PROFILING)
	uint32_t cyccnt;
	uint32_t cycsum;
	uint32_t cycbusy;
	uint32_t cycdiv;
	uint32_t cyc[THINKOS_THREAD_LAST + 1];
	uint32_t busy;
	uint32_t idle;
	uint32_t dif;
#endif
	uint32_t active;
	int i;
	int j;


	active = __thinkos_active_get();
	DCC_LOG1(LOG_TRACE, "active=%d", active);

#if (THINKOS_ENABLE_PROFILING)
	cyccnt = rt->cycref;
	cycsum = 0;
	for (i = THINKOS_THREAD_FIRST; i <= THINKOS_THREAD_LAST; ++i) {
		uint32_t cnt = rt->th_cyc[i];
		uint32_t ref = cycref[i];
		cycref[i] = cnt;
		dif = cnt - ref; 
		cycsum += dif;
		cyc[i] = dif;
	}
	cycbusy = cycsum - dif;
	DCC_LOG3(LOG_TRACE, "idle(%u) busy(%u) = sum(%u)", dif, cycbusy, cycsum);
	cycdiv = (cycsum + 500) / 1000;
	busy = cycbusy / cycdiv;

	if (busy > 1000)
		busy  = 1000;
	idle = 1000 - busy;
#endif

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	monitor_printf(comm, " Active: %d", active);

	monitor_printf(comm, ", Clock: %u", rt->ticks);

#if THINKOS_ENABLE_PROFILING
	monitor_printf(comm, ", CycCnt: %u\r\n", cyccnt);
	monitor_printf(comm, " %u cycles, %d.%d%% busy, %d.%d%% idle", 
				   cycsum, busy / 10, busy % 10, idle / 10, idle % 10);

#endif

	monitor_printf(comm, "\r\n");

	monitor_printf(comm, "  #"); 
	monitor_printf(comm, " |     Tag"); 
	monitor_printf(comm, " |    Stack"); 
	monitor_printf(comm, " |       SP"); 
	monitor_printf(comm, " |       PC"); 
	monitor_printf(comm, " | Status"); 
#if THINKOS_ENABLE_TIMESHARE
//	monitor_printf(comm, " |  Val |  Pri"); 
#endif
	monitor_printf(comm, " | Clock (ms)"); 
#if THINKOS_ENABLE_PROFILING
	monitor_printf(comm, " | CPU %%"); 
#endif

#if THINKOS_MUTEX_MAX > 0
	monitor_printf(comm, " | Locks\r\n"); 
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
			bool tmw;

			/* Internal thread ids start form 0 whereas user
			   thread numbers start form one ... */
			monitor_printf(comm, "%3d", i);
			tag = thinkos_dbg_thread_tag_get(i);
			sl = thinkos_dbg_thread_sl_get(i);
			sp = thinkos_dbg_thread_sp_get(i);
			pc = thinkos_dbg_thread_pc_get(i);

			monitor_printf(comm, " | %7s | %08x | %08x | %08x", 
						   tag, sl, sp, pc); 

			oid = thinkos_dbg_thread_wq_get(i);
			tmw = thinkos_dbg_thread_tmw_get(i);
#if (THINKOS_ENABLE_THREAD_FAULT)
			if (oid == THINKOS_WQ_FAULT) {
				int errno = __thinkos_thread_errno_get(i);
				monitor_printf(comm, " | ERR %2d", errno);
			} else 
#endif
#if THINKOS_IRQ_MAX > 0 && THINKOS_ENABLE_WQ_IRQ
			if (oid == THINKOS_WQ_IRQ) {
				if (i != THINKOS_THREAD_IDLE) {
					int irq;
					for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
						if (rt->irq_th[irq] == i) {
							break;
						}
					}
					if (irq < THINKOS_IRQ_MAX) {
						monitor_printf(comm, " | IRQ %2d", irq);
					} else
						monitor_printf(comm, " | IRQ ??");
				}
			} else 
#endif
			if (oid == THINKOS_WQ_READY) {
#if THINKOS_IRQ_MAX > 0 && !THINKOS_ENABLE_WQ_IRQ
				if (i != THINKOS_THREAD_IDLE) {
					int irq;
					for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
						if (rt->irq_th[irq] == i) {
							break;
						}
					}
					if (irq < THINKOS_IRQ_MAX) {
						monitor_printf(comm, " | IRQ %2d", irq);
					} else
						monitor_printf(comm, " | READY ");
				}
#else
				monitor_printf(comm, " | READY ");
#endif
#if THINKOS_ENABLE_TIMESHARE
				/* FIXME: implement some info ...*/
#endif
			} else {
				type = __thinkos_obj_kind(oid);
				monitor_printf(comm, " | %c%c %3d", 
						 tmw ? 'T' : ' ',
						 __thinkos_kind_prefix(type), 
						 oid);
			}
			{
				int32_t dt = thinkos_dbg_thread_clk_itv_get(i);
				int32_t sec;
				int32_t ms;

				sec = dt / 1000;
				ms = ((dt < 0) ? -dt : dt) % 1000;

				monitor_printf(comm, " |%7d.%03d", sec, ms);
			}
#if THINKOS_ENABLE_PROFILING
			busy = (cyc[i] + cycdiv / 2) / cycdiv;
			if (busy > 1000)
				busy  = 1000;
			monitor_printf(comm, " | %3d.%d", busy / 10, busy % 10);
#endif
			monitor_printf(comm, " |");
#if THINKOS_MUTEX_MAX > 0
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
#if THINKOS_MUTEX_MAX > 0
			if (type == THINKOS_OBJ_MUTEX)
				monitor_printf(comm, " [lock=%d]", 
							   thinkos_dbg_mutex_lock_get(j));
#endif 
			monitor_printf(comm, "\r\n");
		}
	}
}

