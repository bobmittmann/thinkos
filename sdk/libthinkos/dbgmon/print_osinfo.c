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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>
#include <sys/dcclog.h>

#if THINKOS_ENABLE_THREAD_VOID
#define CYCCNT_MAX ((THINKOS_THREADS_MAX) + 2) /* extra slot for void thread */
#else
#define CYCCNT_MAX ((THINKOS_THREADS_MAX) + 1)
#endif

int dmon_print_osinfo(struct dbgmon_comm * comm)
{
	struct thinkos_rt * rt = &thinkos_rt;
#if THINKOS_ENABLE_PROFILING
	uint32_t cycbuf[CYCCNT_MAX];
	uint32_t cyccnt;
	int32_t delta;
	uint32_t cycdiv;
	uint32_t busy;
#endif
	int i;
	int j;

#if THINKOS_ENABLE_PROFILING
	cyccnt = CM3_DWT->cyccnt;
	delta = cyccnt - thinkos_rt.cycref;
	/* update the reference */
	thinkos_rt.cycref = cyccnt;
	/* update active thread's cycle counter */
	thinkos_rt.cyccnt[thinkos_rt.active] += delta; 
	/* copy the thread counters to a buffer */
	__thinkos_memcpy32(cycbuf, rt->cyccnt, sizeof(cycbuf));
	/* reset cycle counters */
	__thinkos_memset32(rt->cyccnt, 0, sizeof(cycbuf));
#endif

//	__thinkos_memcpy32(rt, &thinkos_rt, sizeof(struct thinkos_rt));

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	dmprintf(comm, " Current: %d", rt->active + 1);

#if THINKOS_ENABLE_CLOCK
	dmprintf(comm, ", Clock: %u", rt->ticks);
#endif

#if THINKOS_ENABLE_PROFILING
	dmprintf(comm, ", CycCnt: %u\r\n", cyccnt);
	{
		uint32_t cycsum = 0;
		uint32_t cycbusy;
		uint32_t idle;

		cycsum = 0;
		for (i = 0; i < THINKOS_THREADS_MAX; ++i)
			cycsum += cycbuf[i];
		cycbusy = cycsum;
		cycsum += cycbuf[THINKOS_CYCCNT_IDLE];

		cycdiv = (cycsum + 500) / 1000;
		busy = (cycbusy + cycdiv / 2) / cycdiv;
		idle = 1000 - busy;
		dmprintf(comm, " %u cycles, %d.%d%% busy, %d.%d%% idle", 
				cycsum, busy / 10, busy % 10, idle / 10, idle % 10);
	}

#endif

	dmprintf(comm, "\r\n");

	dmprintf(comm, "  #"); 
#if THINKOS_ENABLE_THREAD_INFO
	dmprintf(comm, " |     Tag"); 
	dmprintf(comm, " |    Stack"); 
#endif
	dmprintf(comm, " |  Context"); 
#if THINKOS_ENABLE_THREAD_STAT
	dmprintf(comm, " | Status"); 
#endif
#if THINKOS_ENABLE_TIMESHARE
//	dmprintf(comm, " |  Val |  Pri"); 
#endif
#if THINKOS_ENABLE_CLOCK
	dmprintf(comm, " | Clock (ms)"); 
#endif
#if THINKOS_ENABLE_PROFILING
	dmprintf(comm, " | CPU %%"); 
#endif

#if THINKOS_MUTEX_MAX > 0
	dmprintf(comm, " | Locks\r\n"); 
#else
	dmprintf(comm, " |\r\n");
#endif

	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		if (rt->ctx[i] != NULL) {
			int oid;
			int type;
			bool tmw;

			/* Internal thread ids start form 0 whereas user
			   thread numbers start form one ... */
			dmprintf(comm, "%3d", i + 1);
#if THINKOS_ENABLE_THREAD_INFO
			if (rt->th_inf[i] != NULL) {
				dmprintf(comm, " | %7s", rt->th_inf[i]->tag); 
				dmprintf(comm, " | %08x", (uint32_t)rt->th_inf[i]->stack_ptr); 
			} else {
				dmprintf(comm, " |     ..."); 
				dmprintf(comm, " |      ..."); 
			}
#endif
			dmprintf(comm, " | %08x", (uint32_t)rt->ctx[i]); 

#if THINKOS_ENABLE_THREAD_STAT
			oid = thinkos_rt.th_stat[i] >> 1;
			tmw = thinkos_rt.th_stat[i] & 1;
#else
			oid = THINKOS_WQ_READY; /* FIXME */
			tmw = (thinkos_rt.wq_clock & (1 << i)) ? true : false;
#endif
#if THINKOS_ENABLE_DEBUG_FAULT
			if (oid == THINKOS_WQ_FAULT) {
				struct thinkos_except * xcpt = &thinkos_except_buf;
				switch (xcpt->type) {
				case CM3_EXCEPT_HARD_FAULT:
					dmprintf(comm, " | !HARD ");
					break;
				case CM3_EXCEPT_MEM_MANAGE:
					dmprintf(comm, " | !MEM  ");
					break;
				case CM3_EXCEPT_BUS_FAULT:
					dmprintf(comm, " | !BUS  ");
					break;
				case CM3_EXCEPT_USAGE_FAULT: 
					dmprintf(comm, " | !USAGE");
					break;
				default:
					dmprintf(comm, " | ERR %2d", xcpt->type - THINKOS_ERR_OFF);
				}
			} else 
#endif
#if THINKOS_IRQ_MAX > 0 && THINKOS_ENABLE_WQ_IRQ
			if (oid == THINKOS_WQ_IRQ) {
				if (i != THINKOS_THREAD_IDLE) {
					int irq;
					for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
						if (thinkos_rt.irq_th[irq] == i) {
							break;
						}
					}
					if (irq < THINKOS_IRQ_MAX) {
						dmprintf(comm, " | IRQ %2d", irq);
					} else
						dmprintf(comm, " | IRQ ??");
				}
			} else 
#endif
			if (oid == THINKOS_WQ_READY) {
#if THINKOS_IRQ_MAX > 0 && !THINKOS_ENABLE_WQ_IRQ
				if (i != THINKOS_THREAD_IDLE) {
					int irq;
					for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
						if (thinkos_rt.irq_th[irq] == i) {
							break;
						}
					}
					if (irq < THINKOS_IRQ_MAX) {
						dmprintf(comm, " | IRQ %2d", irq);
					} else
						dmprintf(comm, " | READY ");
				}
#else
				dmprintf(comm, " | READY ");
#endif
#if THINKOS_ENABLE_TIMESHARE
				/* FIXME: implement some info ...*/
#endif
			} else {
				type = thinkos_obj_type_get(oid);
				dmprintf(comm, " | %c%c %3d", 
						 tmw ? 'T' : ' ',
						 thinkos_type_prefix_lut[type], 
						 oid);
			}
#if THINKOS_ENABLE_CLOCK
			{
				int32_t dt = (int32_t)(rt->clock[i] - rt->ticks);
				if (dt < 0)
					dmprintf(comm, " | <timedout>"); 
				else
					dmprintf(comm, " | %10d", dt); 
			}
#endif
#if THINKOS_ENABLE_PROFILING
			busy = (cycbuf[i] + cycdiv / 2) / cycdiv;
			dmprintf(comm, " | %3d.%d", busy / 10, busy % 10);
#endif
			dmprintf(comm, " |");
#if THINKOS_MUTEX_MAX > 0
			for (j = 0; j < THINKOS_MUTEX_MAX ; ++j) {
				if (rt->lock[j] == i)
					dmprintf(comm, " %d", j + THINKOS_MUTEX_BASE);
			}
#endif 
			dmprintf(comm, "\r\n");
		}
	}

	for (j = 0; j < (rt->wq_end - rt->wq_lst); ++j) {
		uint32_t wq;
		int type;
		wq = rt->wq_lst[j];
#if (THINKOS_THREADS_MAX < 32) 
		if (wq & ~(1 << THINKOS_THREADS_MAX)) { 
#else
		if (wq) { 
#endif
			type = thinkos_obj_type_get(j);
			dmprintf(comm, "%3d %5s: {", j, thinkos_type_name_lut[type]);
			for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
				if (wq & (1 << i)) 
					dmprintf(comm, " %d", i + 1);
			}
			dmprintf(comm, " }");
#if THINKOS_MUTEX_MAX > 0
			if (type == THINKOS_OBJ_MUTEX)
				dmprintf(comm, " [lock=%d]", 
						 rt->lock[j - THINKOS_MUTEX_BASE]);
#endif 
			dmprintf(comm, "\r\n");
		}
	}

	return 0;
}

