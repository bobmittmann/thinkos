/* 
 * thinkos_time.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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

#include "thinkos_krn-i.h"

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#ifndef CLOCK_DRIFT_MAX
//#define CLOCK_DRIFT_MAX FLOAT_Q31(0.000200)
/* FIXME: change the max drift to 200ppm */
#define CLOCK_DRIFT_MAX FLOAT_Q31(0.000500)
#endif

void clock_init(struct krn_clock * clk, uint32_t tick_itv, uint32_t hw_tmr)
{
	clk->timestamp = 0;
}

#define THINKOS_SYSTICK_FREQ (1000) /* T = 1ms */
#define THINKOS_TIME_SYSTICK_INC ((uint64_t)(1LL << 32) / (THINKOS_SYSTICK_FREQ))


void __thinkos_krn_time_init(struct thinkos_rt * krn)
{
    struct krn_clock * clk = &thinkos_rt.time_clk;


	clk->resolution = THINKOS_TIME_SYSTICK_INC;
	clk->increment = clk->resolution;
	clk->realtime_offs = (uint64_t)1606149342LL << 32;

	DCC_LOG1(LOG_TRACE, "resolution=%d", clk->resolution);
}

static uint64_t clock_timestamp(struct krn_clock * clk)
{
    register uint64_t ts;
    register uint32_t dt;

	ts = clk->timestamp;

    /* hardware timer ticks */
//  dt = ((uint64_t)clk->increment * cnt0 * clk->tmr_k + (1LL << 30)) >> 31;
	dt = 0;

#if 0
    DBG5("cnt=%d k=%.12f dt=%.6f", cnt0, clk->tmr_fk, CLK_FLOAT(dt));

    DBG3("Fclk=%d.%06d cnt=%d inc=%.9f dt=%.9f",
         clk->n_freq, (int)Q31_MUL(clk->q_freq, 1000000),
         cnt1, CLK_FLOAT(clk->increment), CLK_FLOAT(dt));

    /* [sec] * [ticks] */

    if (dt != 0) {
        DBG5("Fclk=%d.%06d cnt=%d inc=%.9f dt=%.9f",
            clk->n_freq, (int)Q31_MUL(clk->q_freq, 1000000),
            cnt1, CLK_FLOAT(clk->increment), CLK_FLOAT(dt));
    }
#endif

    return ts + dt;
}


void thinkos_time_svc(int32_t * arg, unsigned int self)
{
    struct krn_clock * clk = &thinkos_rt.time_clk;
	unsigned int oper = arg[0];
	union krn_time tm;

	tm.u64 = clock_timestamp(clk);

	switch (oper) {
	case THINKOS_TIME_MONOTONIC_GET:
		arg[0] = tm.sec;
		arg[1] = tm.frac;
		break;

	case THINKOS_TIME_REALTIME_GET:
		tm.u64 += clk->realtime_offs;
		arg[0] = tm.sec;
		arg[1] = tm.frac;
		break;

	case THINKOS_TIME_MONOTONIC_SET:
		break;

	case THINKOS_TIME_REALTIME_SET:
	/* Set the clock to the specified timestamp 'ts'.
	   This will adjust the clock offset only. */
		{
			union krn_time x;
			x.sec = arg[1];
			x.frac = arg[2];
			
			clk->realtime_offs = (int64_t)(x.u64 - tm.u64);
		}
		arg[0] = THINKOS_OK;
		break;

	case THINKOS_TIME_REALTIME_STEP:
	/* Step the clock by the specified interval 'dt'.
	   This will adjust the clock offset only. */
		{
			union krn_time dt;

			dt.sec = arg[1];
			dt.frac = arg[2];
			
			clk->realtime_offs += (int64_t)(dt.u64);
		}
		arg[0] = THINKOS_OK;
		break;

	case THINKOS_TIME_REALTIME_COMP:
		{
			int32_t drift = arg[1];
			int32_t est_err = arg[2];
			int32_t clk_d;
		//	int32_t q31_d;

			(void)est_err;

			/* limit the maximum drift correction */
			if (drift > CLOCK_DRIFT_MAX)
				drift = CLOCK_DRIFT_MAX;
			else if (drift < -CLOCK_DRIFT_MAX)
				drift = -CLOCK_DRIFT_MAX;

			/* average the estimated error */
		//:m	clk->jitter = (clk->jitter + est_err) / 2;


			clk_d = Q31_MUL(drift, clk->resolution);

			/* calculate the drift compenastion per tick */
			//  clk->drift_comp = Q31_MUL(drift, CLK_Q31(clk->resolution));
			//  clk_d = Q31_CLK(clk->drift_comp);

			/* update the drift compensation value */
		//	q31_d = CLK_Q31(clk_d);
		//	clk->drift_comp = q31_d;

			/* Update the increpent per tick */
			clk->increment = clk->resolution + clk_d;

    /* return the corrected drift adjustment per second.
       Q31 fixed point format */
//    return q31_d * clk->n_freq + Q31_MUL(q31_d, clk->q_freq);

		}
		arg[0] = THINKOS_OK;
		break;


	default:
		DCC_LOG1(LOG_ERROR, "invalid time operation %d!", oper);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

