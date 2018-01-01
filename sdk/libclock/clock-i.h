/*
 * Copyright(c) 2014-2017 Bob Mittmann. All Rights Reserved.
 *
 * This file is part of the libclock.
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
 * @file clock-i.h
 * @brief Clock library private header
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __CLOCK_I_H__
#define __CLOCK_I_H__

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/clkdev.h>

#ifdef CONFIG_H
#include "config.h"
#endif

#ifndef CLOCK_POOL_SIZE
#define CLOCK_POOL_SIZE 1
#endif

#ifndef CLOCK_SINGLETON_ENABLE
#define CLOCK_SINGLETON_ENABLE 1
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#ifndef CLOCK_DRIFT_MAX
//#define CLOCK_DRIFT_MAX FLOAT_Q31(0.000200)
/* FIXME: change the max drift to 200ppm */
#define CLOCK_DRIFT_MAX FLOAT_Q31(0.000500)
#endif

/****************************************************************************
 * Clock
 ****************************************************************************/

struct clock {
	volatile uint64_t timestamp; /* clock timestamp */
	uint64_t offset; /* clock offset */
	uint32_t resolution; /* fractional clock resolution */
	uint32_t increment; /* fractional per tick increment */
	int32_t n_freq; /* frequency itegral part */
	int32_t q_freq; /* frequency decimal part */
	int32_t drift_comp; /* drift compensation seconds per ticks */
	int32_t jitter;
	int64_t tmr_k; /* multiplication factor for hw timer ticks conversion */
	float tmr_fk; /* multiplication factor for hw timer ticks conversion */
	bool pps_flag;
	int hw_tmr;
};

struct clock sys_clk;

/* Hardware timer frequency */
#define HW_TMR_FREQ_HZ 1000000


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_I_H__ */

