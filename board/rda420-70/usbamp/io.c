/* 
 * File:	 io.c
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

#include <sys/dcclog.h>
#include <assert.h>
#include <thinkos.h>

#include "io.h"

const struct io_obj_def io_obj_tab[] = {
	{ .id = LED1A, .op = &led_obj_op},
	{ .id = LED1B, .op = &led_obj_op},
	{ .id = LED1C, .op = &led_obj_op},
	{ .id = LED1D, .op = &led_obj_op},

	{ .id = LED2A, .op = &led_obj_op},
	{ .id = LED2B, .op = &led_obj_op},
	{ .id = LED2C, .op = &led_obj_op},
	{ .id = LED2D, .op = &led_obj_op},

	{ .id = LED3A, .op = &led_obj_op},
	{ .id = LED3B, .op = &led_obj_op},
	{ .id = LED3C, .op = &led_obj_op},
	{ .id = LED3D, .op = &led_obj_op},

	{ .id = LED4A, .op = &led_obj_op},
	{ .id = LED4B, .op = &led_obj_op},
	{ .id = LED4C, .op = &led_obj_op},
	{ .id = LED4D, .op = &led_obj_op}
};

const struct rate_def rate_def_off = {
	.name = "OFF",
	.length = 0
};

const struct rate_def rate_def_20bpm = {
	.name = "20BPM",
	.length = 2,
	.pattern = { 1500, 1500 }
};

const struct rate_def rate_def_120bpm = {
	.name = "120BPM",
	.length = 2,
	.pattern = { 250, 250 }
};

const struct rate_def rate_def_temporal3 = {
	.name = "TMP3",
	.length = 6,
	.pattern = { 500, 500, 500, 500, 500, 1500 }
};

const struct rate_def rate_def_pulse1 = {
	.name = "PULSE1",
	.length = 32,
	.pattern = { 100, 1000, 100, 900, 100, 800, 100, 700, 100, 600, 
		100, 500, 100, 400, 100, 300, 100, 200, 100, 100, 100, 100, 
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 2000 }
};

/* Standard rates */
const struct rate_def * rate_def_std[RATE_STD_MAX] = {
	[RATE_OFF] = &rate_def_off,
	[RATE_20BPM] = &rate_def_20bpm,
	[RATE_120BPM] = &rate_def_120bpm,
	[RATE_TEMPORAL3] = &rate_def_temporal3,
	[RATE_PULSE1] = &rate_def_pulse1
};

#define RATE_MAX RATE_STD_MAX

struct rate_ctl {
	volatile uint32_t io_bmp;
	const struct rate_def * def;
};

void __attribute__((noreturn)) rates_task(struct rate_ctl ctl[])
{
	unsigned int cnt[RATE_MAX];
	uint32_t tmr[RATE_MAX];
	uint32_t clk;
	int j;

	clk = thinkos_clock();
	for (j = 1; j < RATE_MAX; j++) {
		tmr[j] = clk;
		cnt[j] = 0;
	}

	DCC_LOG(LOG_TRACE, "...");

	for (;;) {
		int32_t dt_min = INT32_MAX;
		int j;

		DCC_LOG(LOG_MSG, "tick");

		for (j = 1; j < RATE_MAX; ++j) {
			int32_t dt;

			if ((dt = (int32_t)(tmr[j] - clk)) <= 0) {
				struct io_obj_def * io;
				unsigned int itvl;
				uint32_t bmp;
				int i;

				itvl = ctl[j].def->pattern[cnt[j]];
				tmr[j] += itvl; 
				if (itvl < dt_min)
					dt_min = itvl;

				bmp = ctl[j].io_bmp;
				DCC_LOG2(LOG_MSG, "rate: %s 0x%08x", 
						 ctl[j].def->name, ctl[j].io_bmp);

				if ((cnt[j] & 1) == 0) {
					while ((i = __clz(__rbit(bmp))) < 32) {
						DCC_LOG1(LOG_YAP, "i=%d", i);
						io = (struct io_obj_def *)&io_obj_tab[i];
						io->op->set(io->id, true);
						bmp &= ~(1 << i);
					}
				} else {
					while ((i = __clz(__rbit(bmp))) < 32) {
						io = (struct io_obj_def *)&io_obj_tab[i];
						io->op->set(io->id, false);
						bmp &= ~(1 << i);
					}
				}

				if ((++cnt[j] == ctl[j].def->length))
					cnt[j] = 0;
			} else if (dt < dt_min)
					dt_min = dt;
		}

		DCC_LOG1(LOG_MSG, "dt_min=%d", dt_min);
		clk += dt_min;
		thinkos_alarm(clk);
	}
}

#define TIMER_PWM_FREQ 8000

uint32_t rates_stack[1024];

const struct thinkos_thread_inf rates_thread_inf = {
	.stack_ptr = rates_stack, 
	.stack_size = sizeof(rates_stack), 
	.priority = 32,
	.thread_id = 8, 
	.paused = 0,
	.tag = "RATEGEN"
};

struct {
	struct rate_ctl rategen[RATE_MAX];
} iodrv;

void io_set_rate(unsigned int io, unsigned int rate)
{
	unsigned int old = 0;

	iodrv.rategen[old].io_bmp &= ~(1 << io);
	iodrv.rategen[rate].io_bmp |= (1 << io);
}

void iodrv_init(void)
{
	int i;

	for (i = 0; i < RATE_STD_MAX; ++i) 
		iodrv.rategen[i].def = rate_def_std[i];

	for (i = 0; i < RATE_MAX; ++i) 
		iodrv.rategen[i].io_bmp = 0;

	thinkos_thread_create_inf((void *)rates_task, (void *)iodrv.rategen,
							  &rates_thread_inf);
}

