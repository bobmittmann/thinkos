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
#include <vt100.h>

#include "board.h"
#include "amp.h"
#include "audio.h"
#include "booster.h"
#include "rcp/amp4.h"
#include "ampctrl.h"
#include "supv.h"
#include "fixpt.h"

#ifndef ENABLE_TERMINAL_RESET_ON_FAULT
#define ENABLE_TERMINAL_RESET_ON_FAULT  0
#endif

#ifndef SYSSUPV_REPORT_ENABLE
#define SYSSUPV_REPORT_ENABLE 0
#endif

#ifndef SYSSUPV_MOD_SUPV_ENABLE
#define SYSSUPV_MOD_SUPV_ENABLE 0
#endif

#define WATCHDOG_KICK_INTERVAL_MS       100
#define SUPV_HIGH_WAIT_TIME_MS          1000
#define SUPV_CPU_USAGE_COLLECT_ITV_MS   30000
#define SUPVTMR_INITIAL_DELAY_MS        5000 
#define SUPV_NET_REPORT_ITV_MS          15000 

#ifndef TRACE_COLOR_ENABLE
#define TRACE_COLOR_ENABLE 1
#endif

#ifndef TRACE_COMPACT_ENABLE
#define TRACE_COMPACT_ENABLE 1
#endif

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

/* -------------------------------------------------------------------------
 * Private control structures  
 * ------------------------------------------------------------------------- */

#define VT_ATTR_PUSH	"\0337"
#define VT_ATTR_POP		"\0338"

#if TRACE_COLOR_ENABLE

const char *const lvl_attr_nm[] = {
	_FG_RED_ _BRIGHT_ _BLINK_ "PANIC" _FG_WHITE_,
	_FG_RED_ _BRIGHT_ " CRIT" _FG_WHITE_ _NORMAL_,
	_FG_RED_ _BRIGHT_ "ALERT" _FG_WHITE_ _NORMAL_,
	_FG_RED_ "ERROR" _FG_WHITE_ _NORMAL_,

	_FG_YELLOW_ " WARN" _FG_WHITE_ _NORMAL_,
	_FG_GREEN_ " NOTE" _FG_WHITE_ _NORMAL_,
	_FG_BLUE_ " INFO" _FG_WHITE_ _NORMAL_,
	_FG_CYAN_ "DEBUG" _FG_WHITE_ _NORMAL_,

	_FG_CYAN_ _DIM_ "  YAP" _FG_WHITE_ _NORMAL_,
	" !!!!",
	" ????"
};

const char *const txt_attr[] = {
	_FG_WHITE_,
	_FG_WHITE_,
	_FG_WHITE_,
	_FG_WHITE_,

	_FG_WHITE_,
	_FG_WHITE_,
	_FG_WHITE_,
	_FG_WHITE_,

	_FG_WHITE_,
	_FG_WHITE_,
	_FG_WHITE_
};

#endif

#define TRACE_PREFIX_MAX        128
#define TRACE_PRINT_BUF_LEN     512
#define MAX_THREADS 34

struct {
	FILE * volatile outfile;
	volatile int8_t thread_low;
	uint8_t mutex;
	struct {
		uint8_t enabled;
		uint8_t flag;
	} watchdog;

	volatile struct syssupv_trace_opt trace_opt;
	volatile struct syssupv_report_opt report_opt;

	char trace_print_buf[TRACE_PRINT_BUF_LEN];

	uint32_t cycref[MAX_THREADS];
} syssupv_rt;

#if (SYSSUPV_REPORT_ENABLE)
int syssupv_report_dcsupv_print(FILE * f)
{
	struct dcsupv_eol eol[NAMP_CKTS];
	enum dcsupv_status st[NAMP_CKTS];
	int i;

	for (i = 0; i < NAMP_CKTS; ++i) {
		int ckt = i + 1;
		st[i] = dcsupv_status_get(ckt);
		eol[i] = dcsupv_eol_get(ckt);
	}
	dcsupv_eol_report(f, st, eol);

	return 0;
}

int syssupv_report_pwrsupv_print(FILE * f)
{
	struct pwrsupv_ckt_info pwr[NAMP_CKTS];
	int i;

	for (i = 0; i < NAMP_CKTS; ++i) {
		int ckt = i + 1;
		pwrsupv_ckt_info_get(ckt, &pwr[i]);
	}

	pwrsupv_info_report(f, pwr);

	return 0;
}

int syssupv_report_audsupv_print(FILE * f)
{
	struct ac_readings ac[NAMP_CKTS];
	enum audsupv_status st[NAMP_CKTS];
	int i;

	for (i = 0; i < NAMP_CKTS; ++i) {
		int ckt = i + 1;
		st[i] = audsupv_status_get(ckt);
		ac[i] = audsupv_ac_get(ckt);
	}
	audsupv_ac_report(f, st, ac);

	return 0;
}
#endif

/* -------------------------------------------------------------------------
 * Private functions
 * ------------------------------------------------------------------------- */

static uint32_t cpu_usage_collect(const struct thinkos_thread_inf *infbuf[],
									  uint32_t cyc[], uint32_t cycref[], 
									  size_t len)
{
	unsigned int i;
	uint32_t sum;
	uint32_t max;
	int n;

	/* The cycle counter and thread info must be collected with no 
	   interruptions when threads are created/destroyed at runtime. */
	thinkos_critical_enter();

	thinkos_thread_inf(infbuf, 0, len);

	n = thinkos_thread_cyccnt(cyc, 0, len);

	thinkos_critical_exit();

	sum = 0;
	max = 0;
	for (i = 0; i < n; ++i) {
		uint32_t dif;
		uint32_t cnt = cyc[i];
		uint32_t ref = cycref[i];

		cycref[i] = cnt;
		dif = cnt - ref; 
		cyc[i] = dif;
		sum += dif;
		if (dif > max) {
			max = dif;
		}
	}

	return n;
}


static void cpu_usage_report(FILE * f,
			     const struct thinkos_thread_inf *infbuf[],
			     uint32_t cyc[], unsigned int cnt)
{
	uint32_t cycsum = 0;
	uint32_t cycbusy;
	uint32_t cycidle;
	uint32_t cycdiv;
	uint32_t idle;
	uint32_t busy;
	unsigned int i;

	if (f != NULL) {
		fprintf(f, "\n");

		cycsum = 0;
		for (i = 0; i < cnt; ++i)
			cycsum += cyc[i];
		cycidle = cyc[cnt - 1];	/* The last item is IDLE */
		cycbusy = cycsum - cycidle;
		cycdiv = (cycsum + 5000) / 10000;

		busy = (cycdiv == 0) ? 10000 : (cycbusy / cycdiv);
	    if (busy > 10000)
			busy  = 10000;

    	idle = 10000 - busy;
	    (void) idle;

		fprintf(f, "CPU usage: %d.%02d%% busy, %d.%02d%% idle\r\n",
			busy / 100, busy % 100, idle / 100, idle % 100);

		for (i = 0; i < cnt; ++i) {
			const struct thinkos_thread_inf *inf;
			if (((inf = infbuf[i]) != NULL) && (cyc[i] != 0)) {
				uint32_t usage;
				usage = (cycdiv == 0) ? 10000 : (cyc[i] / cycdiv);
				if (usage > 10000)
					usage = 10000;

				fprintf(f, "%2d %7s %3d.%02d%%\r\n", i, inf->tag,
					usage / 100, usage % 100);
			}
		}
	}
}

void syssupv_report_uptime_print(FILE * f, uint32_t clk)
{
	int day;
	int hour;
	int min;
	int sec;
	int ms;

	sec = clk / 1000;
	ms = clk - (sec * 1000);

	min = sec / 60;
	sec = sec - (min * 60);

	hour = min / 60;
	min = min - (hour * 60);

	day = hour / 24;
	hour = hour - (day * 24);

	fprintf(f, "| Uptime: %2d %02d:%02d:%02d.%03d ",
			day, hour, min, sec, ms);
}

void syssupv_report_temperature_print(FILE * f, int32_t temp)
{
	fprintf(f, "| T: %5.2f ", (double)Q15F(temp));
}

void syssupv_report_vcc_print(FILE * f, int32_t vcc)
{
	fprintf(f, "| VCC: %5.3f ", (double)Q15F(vcc));
}

void syssupv_report_vboost_print(FILE * f, int32_t v)
{
	fprintf(f, "| VBS: %5.1f ", (double)Q15F(v));
}

static void trace_print_all(FILE * f, struct trace_iterator *it,
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

static void trace_color_print_all(FILE * f, struct trace_iterator *it,
			    unsigned int lvl_min, unsigned int lvl_ext,
				bool reltm)
{
	const char suffix[] = _ATTR_POP_ "\r\n";
	char * buf = syssupv_rt.trace_print_buf;
	char * msg = &buf[TRACE_PREFIX_MAX];
	struct trace_entry *trace;

	while ((trace = trace_getnext(it)) != NULL) {
		int msg_max = TRACE_PRINT_BUF_LEN - TRACE_PREFIX_MAX;
		unsigned int lvl;
		int msg_len;
		int n;

		msg_len = trace_fmt(trace, msg, msg_max);

		if ((lvl = trace->ref->lvl) > lvl_min)
			continue;

		if (reltm) {
			struct timeval tv;

			trace_ts2timeval(&tv, trace->dt);
			n = sprintf(buf, _ATTR_PUSH_ "%s %2d.%06d" _NORMAL_ ":  ",
						lvl_attr_nm[lvl],
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

			n = sprintf(buf, _ATTR_PUSH_ "%s %2d %02d:%02d:%02d.%03d" 
						_NORMAL_ ": ", lvl_attr_nm[lvl],
						day, hour, min, sec, (int)tv.tv_usec/1000);
		}

		if ((lvl = trace->ref->lvl) <= lvl_ext) {
			/* extra info */
			n += sprintf(&buf[n], "%s,%d: %s ",
				    trace->ref->func, trace->ref->line,
				    txt_attr[lvl]);

		} else  {
			n += sprintf(&buf[n], "%s ", txt_attr[lvl]);
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

