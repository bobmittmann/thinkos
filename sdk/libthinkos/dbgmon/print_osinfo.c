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
	delta = cyccnt - rt->cycref;
	/* update the reference */
	rt->cycref = cyccnt;
	/* update active thread's cycle counter */
	rt->cyccnt[rt->active] += delta; 
	/* copy the thread counters to a buffer */
	__thinkos_memcpy32(cycbuf, rt->cyccnt, sizeof(cycbuf));
	/* reset cycle counters */
	__thinkos_memset32(rt->cyccnt, 0, sizeof(cycbuf));
#endif

//	__thinkos_memcpy32(rt, &thinkos_rt, sizeof(struct thinkos_rt));

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	dbgmon_printf(comm, " Current: %d", rt->active + 1);

#if THINKOS_ENABLE_CLOCK
	dbgmon_printf(comm, ", Clock: %u", rt->ticks);
#endif

#if THINKOS_ENABLE_PROFILING
	dbgmon_printf(comm, ", CycCnt: %u\r\n", cyccnt);
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
		dbgmon_printf(comm, " %u cycles, %d.%d%% busy, %d.%d%% idle", 
				cycsum, busy / 10, busy % 10, idle / 10, idle % 10);
	}

#endif

	dbgmon_printf(comm, "\r\n");

	dbgmon_printf(comm, "  #"); 
#if THINKOS_ENABLE_THREAD_INFO
	dbgmon_printf(comm, " |     Tag"); 
	dbgmon_printf(comm, " |    Stack"); 
#endif
	dbgmon_printf(comm, " |  Context"); 
#if THINKOS_ENABLE_THREAD_STAT
	dbgmon_printf(comm, " | Status"); 
#endif
#if THINKOS_ENABLE_TIMESHARE
//	dbgmon_printf(comm, " |  Val |  Pri"); 
#endif
#if THINKOS_ENABLE_CLOCK
	dbgmon_printf(comm, " | Clock (ms)"); 
#endif
#if THINKOS_ENABLE_PROFILING
	dbgmon_printf(comm, " | CPU %%"); 
#endif

#if THINKOS_MUTEX_MAX > 0
	dbgmon_printf(comm, " | Locks\r\n"); 
#else
	dbgmon_printf(comm, " |\r\n");
#endif

	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		if (rt->ctx[i] != NULL) {
			int oid;
			int type;
			bool tmw;

			/* Internal thread ids start form 0 whereas user
			   thread numbers start form one ... */
			dbgmon_printf(comm, "%3d", i + 1);
#if THINKOS_ENABLE_THREAD_INFO
			if (rt->th_inf[i] != NULL) {
				dbgmon_printf(comm, " | %7s", rt->th_inf[i]->tag); 
				dbgmon_printf(comm, " | %08x", (uint32_t)rt->th_inf[i]->stack_ptr); 
			} else {
				dbgmon_printf(comm, " |     ..."); 
				dbgmon_printf(comm, " |      ..."); 
			}
#endif
			dbgmon_printf(comm, " | %08x", (uint32_t)rt->ctx[i]); 

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
					dbgmon_printf(comm, " | !HARD ");
					break;
				case CM3_EXCEPT_MEM_MANAGE:
					dbgmon_printf(comm, " | !MEM  ");
					break;
				case CM3_EXCEPT_BUS_FAULT:
					dbgmon_printf(comm, " | !BUS  ");
					break;
				case CM3_EXCEPT_USAGE_FAULT: 
					dbgmon_printf(comm, " | !USAGE");
					break;
				default:
					dbgmon_printf(comm, " | ERR %2d", xcpt->type - THINKOS_ERR_OFF);
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
						dbgmon_printf(comm, " | IRQ %2d", irq);
					} else
						dbgmon_printf(comm, " | IRQ ??");
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
						dbgmon_printf(comm, " | IRQ %2d", irq);
					} else
						dbgmon_printf(comm, " | READY ");
				}
#else
				dbgmon_printf(comm, " | READY ");
#endif
#if THINKOS_ENABLE_TIMESHARE
				/* FIXME: implement some info ...*/
#endif
			} else {
				type = thinkos_obj_type_get(oid);
				dbgmon_printf(comm, " | %c%c %3d", 
						 tmw ? 'T' : ' ',
						 thinkos_type_prefix_lut[type], 
						 oid);
			}
#if THINKOS_ENABLE_CLOCK
			{
				int32_t dt = (int32_t)(rt->clock[i] - rt->ticks);
				if (dt < 0)
					dbgmon_printf(comm, " | <timedout>"); 
				else
					dbgmon_printf(comm, " | %10d", dt); 
			}
#endif
#if THINKOS_ENABLE_PROFILING
			busy = (cycbuf[i] + cycdiv / 2) / cycdiv;
			dbgmon_printf(comm, " | %3d.%d", busy / 10, busy % 10);
#endif
			dbgmon_printf(comm, " |");
#if THINKOS_MUTEX_MAX > 0
			for (j = 0; j < THINKOS_MUTEX_MAX ; ++j) {
				if (rt->lock[j] == i)
					dbgmon_printf(comm, " %d", j + THINKOS_MUTEX_BASE);
			}
#endif 
			dbgmon_printf(comm, "\r\n");
		}
	}

	for (j = 0; j < THINKOS_WQ_CNT; ++j) {
		uint32_t wq;
		int type;
		wq = rt->wq_lst[j];
		if (wq) { 
			type = thinkos_obj_type_get(j);
			dbgmon_printf(comm, "%3d %5s: {", j, thinkos_type_name_lut[type]);
			for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
				if (wq & (1 << i)) 
					dbgmon_printf(comm, " %d", i + 1);
			}
			dbgmon_printf(comm, " }");
#if THINKOS_MUTEX_MAX > 0
			if (type == THINKOS_OBJ_MUTEX)
				dbgmon_printf(comm, " [lock=%d]", 
						 rt->lock[j - THINKOS_MUTEX_BASE]);
#endif 
			dbgmon_printf(comm, "\r\n");
		}
	}

	return 0;
}

