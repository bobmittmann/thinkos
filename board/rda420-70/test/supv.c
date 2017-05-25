/* 
 * File:	 usb-test.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <trace.h>
#include <thinkos.h>
#include <assert.h>
#include <io.h>

#include "board.h"
#include "amp.h"

#define IO_POLL_PERIOD_MS 200
#define CLOCK_SYNC_INTERVAL_MS 15000


/* -------------------------------------------------------------------------
 * System Supervision
 * ------------------------------------------------------------------------- */

const char * const trace_lvl_tab[] = {
		" NONE",
		"ERROR",
		" WARN",
		" INFO",
		"  DBG",
		"  YAP"
};

struct {
	FILE * volatile file;
	volatile bool autoflush;
	volatile bool cpureport;
} trace_spv = {
	.file = NULL,
	.autoflush = true,
	.cpureport = false
};

void __attribute__((noreturn)) supervisor_task(void)
{
	struct trace_entry trace;
	uint32_t dump_tmo;
	uint32_t sync_tmo;
	uint32_t count = 0;
	uint32_t clk;
	FILE * f;
	int n;

	INF("<%d> started...", thinkos_thread_self());

#if 1
	/* set the supervisor stream to stdout */
	trace_spv.file = stdout;
	/* enable auto flush */
	trace_spv.autoflush = true;
#endif

	trace_tail(&trace);

	clk = thinkos_clock();
	dump_tmo = clk + 15000;
	sync_tmo = CLOCK_SYNC_INTERVAL_MS;
	for (;;) {
		char msg[80];

		/* 8Hz periodic task */
		clk += IO_POLL_PERIOD_MS / 2;
		thinkos_alarm(clk);
		count++;

		if ((int32_t)(clk - sync_tmo) >= 0) {
			sync_tmo += CLOCK_SYNC_INTERVAL_MS;
//			printf("sync...\n");
			INF("sync...");
		}

		amp_supv();

		if ((f = (FILE *)trace_spv.file) != NULL) {
			while (trace_getnext(&trace, msg, sizeof(msg)) >= 0) {
				struct timeval tv;
				char s[128];

				trace_ts2timeval(&tv, trace.dt);
				if (trace.ref->lvl <= TRACE_LVL_WARN)
					n = sprintf(s, "%s %2d.%06d: %s,%d: %s\n",
							trace_lvl_nm[trace.ref->lvl], (int) tv.tv_sec,
							(int) tv.tv_usec, trace.ref->func, trace.ref->line,
							msg);
				else
					n = sprintf(s, "%s %2d.%06d: %s\n",
							trace_lvl_nm[trace.ref->lvl], (int) tv.tv_sec,
							(int) tv.tv_usec, msg);

				/* write log to local console */
				fwrite(s, n, 1, f);
			}

			if (trace_spv.autoflush)
				trace_flush(&trace);
		}

		if ((int32_t)(clk - dump_tmo) >= 0) {
			if (trace_spv.cpureport) {
				const struct thinkos_thread_inf * infbuf[33];
				uint32_t cycbuf[33];
				uint32_t cycsum = 0;
				uint32_t cycbusy;
				uint32_t cycidle;
				uint32_t cycdiv;
				uint32_t idle;
				uint32_t busy;
				int n;
				int i;

				thinkos_critical_enter();

				thinkos_thread_inf(infbuf);
				n = thinkos_cyccnt(cycbuf);

				thinkos_critical_exit();

				if (f != NULL) {
					cycsum = 0;
					for (i = 0; i < n; ++i)
						cycsum += cycbuf[i];
					cycidle = cycbuf[n - 1]; /* The last item is IDLE */
					cycbusy = cycsum - cycidle;
					cycdiv = (cycsum + 5000) / 10000;
					busy = (cycbusy + (cycdiv / 2)) / cycdiv;
					idle = 1000 - busy;
					fprintf(f, "CPU usage: %d.%02d%% busy, %d.%02d%% idle\n",
							busy / 100, busy % 100, idle / 100, idle % 100);
					for (i = 0; i < n; ++i) {
						const struct thinkos_thread_inf * inf;
						if (((inf = infbuf[i]) != NULL) && (cycbuf[i] != 0)) {
							uint32_t usage;
							usage = (cycbuf[i] + cycdiv / 2) / cycdiv;
							fprintf(f, "%2d %7s %3d.%02d%%\n", i, inf->tag,
									usage / 100, usage % 100);
						}
					}
				}
			}
			dump_tmo += 15000;
		}

	}
}

uint32_t supervisor_stack[256];

const struct thinkos_thread_inf supervisor_inf = {
	.stack_ptr = supervisor_stack,
	.stack_size = sizeof(supervisor_stack),
	.priority = 32,
	.thread_id = 31,
	.paused = false,
	.tag = "SUPV"
};

void supervisor_init(void)
{
	thinkos_thread_create_inf((void *)supervisor_task, (void *)NULL,
							  &supervisor_inf);
}
void spv_trace_file_set(FILE* f)
{
	trace_spv.file = f;
}

void spv_trace_autoflush_set(bool autoflush)
{
	trace_spv.autoflush = autoflush;
}
