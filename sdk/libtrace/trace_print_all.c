/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file trace.c
 * @brief Real-time trace
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "trace-i.h"
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/tty.h>
#include <thinkos.h>
#include <assert.h>
#include <io.h>

void trace_print_all(FILE * f, struct trace_iterator *it,
					 unsigned int lvl_min, unsigned int lvl_ext,
					 bool reltm)
{
	int msg_max = TRACE_PRINT_BUF_LEN - TRACE_PREFIX_MAX;
	char * buf = syssupv_rt.trace_print_buf;
	char * msg = &buf[TRACE_PREFIX_MAX];
	const char suffix[] = "\r\n";
	struct trace_entry * trace;
	int msg_len;

	while ((trace = trace_getnext(it)) != NULL) {
		unsigned int lvl;
		int n;

		msg_len = trace_fmt(trace, msg, msg_max);

		if ((lvl = trace->ref->lvl) > lvl_min)
			continue;

		if (reltm) {
			struct timeval tv;
			trace_ts2timeval(&tv, trace->dt);
			n = sprintf(buf, "%s %2d.%06d: ",
						trace_lvl_nm[lvl], 
						(int)tv.tv_sec, (int)tv.tv_usec);
		} else {
			struct timeval tv;
			int day;
			int hour;
			int min;
			int sec;

			trace_tm2timeval(&tv, trace->tm);

			min = tv.tv_sec / 60;
			sec = tv.tv_sec - (min * 60);
			hour = min / 60;
			min = min - (hour * 60);
			day = hour / 24;
			hour = hour - (day * 24);

			n = sprintf(buf, "%s %2d %02d:%02d:%02d.%03d: ", 
						trace_lvl_nm[lvl], 
						day, hour, min, sec, (int)tv.tv_usec/1000);
		}

		if (lvl <= lvl_ext) {
			/* print extended info */
			n += sprintf(&buf[n], ": %s,%d:",
						 trace->ref->func, trace->ref->line);
		} 

		/* append prefix message and suffix */
		memcpy(&buf[n], msg, msg_len);
		n += msg_len;
		memcpy(&buf[n], suffix, sizeof(suffix));
		n += sizeof(suffix);

		/* write log to local console */
		fwrite(buf, n, 1, f);
	}
}

