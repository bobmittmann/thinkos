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
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>

void monitor_print_thread(const struct monitor_comm * comm, 
						 unsigned int thread_id)
{
	struct thinkos_rt * rt = &thinkos_rt;
	struct thinkos_context * ctx;
	int32_t timeout;
	uint32_t cyccnt;
#if (THINKOS_ENABLE_TIMESHARE)
	int sched_val;
	int sched_pri;
#endif
	uint32_t ctrl;
	uint32_t sp;
	int type;
	int tmw;
	int wq;
#if !THINKOS_ENABLE_THREAD_STAT
	int i;
#endif

	if ((thread_id >= THINKOS_THREADS_MAX) || 
		!__thinkos_thread_ctx_is_valid(thread_id)) {
		return;
	}

#if (THINKOS_ENABLE_THREAD_STAT)
	wq = rt->th_stat[thread_id] >> 1;
	tmw = rt->th_stat[thread_id] & 1;
#else
	for (i = 0; i < THINKOS_WQ_CNT; ++i) {
		if (rt->wq_lst[i] & (1 << thread_id))
			break;
	}
	if (i == THINKOS_WQ_CNT)
		return ; /* not found */
	wq = i;
#if (THINKOS_ENABLE_CLOCK)
	tmw = rt->wq_clock & (1 << thread_id) ? 1 : 0;
#else
	tmw = 0;
#endif
#endif /* THINKOS_ENABLE_THREAD_STAT */

#if (THINKOS_ENABLE_TIMESHARE)
	sched_val = rt->sched_val[thread_id];
	sched_pri = rt->sched_pri[thread_id]; 
#endif

#if (THINKOS_ENABLE_CLOCK)
	timeout = (int32_t)(rt->clock[thread_id] - rt->ticks); 
#else
	timeout = -1;
#endif

#if (THINKOS_ENABLE_PROFILING)
	cyccnt = rt->cyccnt[thread_id];
#else
	cyccnt = 0;
#endif

	type = __thinkos_obj_kind(wq);

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	monitor_printf(comm, " - No: %d", thread_id + 1); 
#if (THINKOS_ENABLE_THREAD_INFO)
	if (rt->th_inf[thread_id])
		monitor_printf(comm, ", '%s'", rt->th_inf[thread_id]->tag); 
	else
#endif
		monitor_printf(comm, ", '...'"); 

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
				monitor_printf(comm, " wait on IRQ[%d]\r\n", irq);
			} else
				monitor_printf(comm, " %s.\r\n", thinkos_type_name_lut[type]); 
		} else
#endif
		monitor_printf(comm, " %s.\r\n", thinkos_type_name_lut[type]); 
	} else {
		if (THINKOS_OBJ_FAULT == type) {
			monitor_printf(comm, " FAULT!");
		} else 
			monitor_printf(comm, " %swait on %s(%3d)\r\n", 
						  tmw ? "time" : "", thinkos_type_name_lut[type], wq); 

	}

#if (THINKOS_ENABLE_TIMESHARE)
	monitor_printf(comm, " - sched: val=%3d pri=%3d", 
			 sched_val, sched_pri); 
#endif
	monitor_printf(comm, " - timeout=%8d ms", timeout); 
	monitor_printf(comm, " - cycles=%u\r\n", cyccnt); 

	ctx = __thinkos_thread_ctx_get(thread_id);
	sp = __thinkos_thread_sp_get(thread_id);
	ctrl = __thinkos_thread_ctrl_get(thread_id);
	monitor_print_context(comm, ctx, sp, ctrl);

	monitor_printf(comm, "\r\n");
}


