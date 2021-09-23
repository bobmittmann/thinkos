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

void monitor_print_thread(const struct monitor_comm * comm, 
						 unsigned int thread_id)
{
	struct thinkos_context * ctx;
	int32_t timeout;
	uint32_t cyccnt;
#if (THINKOS_ENABLE_TIMESHARE)
	struct thinkos_rt * rt = &thinkos_rt;
	int sched_val;
	int sched_pri;
#endif
	uint32_t ctrl;
	uint32_t sp;
	const char * tag;
	int type;
	int tmw;
	int wq;

	if (!thinkos_dbg_thread_ctx_is_valid(thread_id)) {
		return;
	}

	wq = thinkos_dbg_thread_wq_get(thread_id);
	tmw = thinkos_dbg_thread_tmw_get(thread_id);
	timeout = thinkos_dbg_thread_clk_itv_get(thread_id);
	cyccnt = thinkos_dbg_thread_cyccnt_get(thread_id);
	tag = thinkos_dbg_thread_tag_get(thread_id);

#if (THINKOS_ENABLE_TIMESHARE)
	sched_val = rt->sched_val[thread_id];
	sched_pri = rt->sched_pri[thread_id]; 
#endif

	type = __thinkos_obj_kind(wq);

	monitor_printf(comm, " - No: %d '%s'", thread_id, tag); 

	if (THINKOS_OBJ_READY == type) {
#if (THINKOS_IRQ_MAX) > 0
		if (thread_id != THINKOS_THREAD_IDLE) {
			int irq;
			for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
				if (thinkos_rt.irq_th[irq] == (int)thread_id) {
					break;
				}
			}
			if (irq < THINKOS_IRQ_MAX) {
				monitor_printf(comm, " wait on IRQ[%d]", irq);
			} else
				monitor_printf(comm, " %s.", __thinkos_kind_name(type)); 
		} else
#endif
		monitor_printf(comm, " %s.", __thinkos_kind_name(type)); 
	} else {
		if (THINKOS_OBJ_FAULT == type) {
			monitor_printf(comm, " FAULT!");
		} else 
			monitor_printf(comm, " %swait on %s(%3d)", 
						  tmw ? "time" : "", __thinkos_kind_name(type), wq); 

	}

	monitor_newln(comm);

#if (THINKOS_ENABLE_TIMESHARE)
	monitor_printf(comm, " - sched: val=%3d pri=%3d", 
			 sched_val, sched_pri); 
#endif
	monitor_printf(comm, " - timeout=%8d ms", timeout); 
	monitor_printf(comm, " - cycles=%u", cyccnt); 

	ctx = thinkos_dbg_thread_ctx_get(thread_id);
	sp = thinkos_dbg_thread_sp_get(thread_id);
	ctrl = thinkos_dbg_thread_ctrl_get(thread_id);
	monitor_print_context(comm, ctx, sp, ctrl);

	monitor_newln(comm);
}


