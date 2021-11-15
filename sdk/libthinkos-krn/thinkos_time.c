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
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#ifndef CLOCK_DRIFT_MAX
/* max drift: 500ppm */
#define CLOCK_DRIFT_MAX FLOAT_Q31(0.000500)
#endif

#define THINKOS_SYSTICK_FREQ (1000) /* T = 1ms */
#define THINKOS_TIME_SYSTICK_INC ((uint64_t)(1LL << 32) / (THINKOS_SYSTICK_FREQ))

#if (THINKOS_ENABLE_DATE_AND_TIME)

uint64_t time_clock_rel_tick(struct krn_clock * clk, uint32_t refcnt)
{
	unsigned int pri;
	int32_t diff;
	uint64_t cyc;

	cyc = clk->hw.cycles + (1LL << 32);
	diff = refcnt - clk->hw.count;
	(void)diff;

	pri = cm3_primask_get();
	cm3_primask_set(1);

	clk->hw.cycles = cyc;
	clk->hw.count = refcnt;
	cm3_primask_set(pri);

#if 0
	{
		uint32_t k = clk->hw.k;
		/* Self calibrate */
		uint32_t k0 = (uint64_t) (1LL << (32 + 8)) / diff;
		/* Exponential Averaging filter */
		k = ((uint64_t) k * 1023 + k0 + 511) / 1024;
		clk->k = k;
		DCC_LOG(LOG_TRACE"k=%u diff=%d", (uint32_t) k, diff);
	}
#endif

	return cyc;
}

uint64_t time_clock_rel_timestamp(struct krn_clock * clk, uint32_t refcnt)
{
	uint32_t hwcnt;
	uint64_t cyc;
	int32_t diff;

	do {
		hwcnt = clk->hw.count;
		cyc = clk->hw.cycles;
	} while (hwcnt != clk->hw.count);

	diff = refcnt - hwcnt;
	return cyc + (((int64_t) diff * clk->hw.k) >> 8);
}

uint64_t time_clock_timestamp(struct krn_clock * clk)
{
	uint32_t refcnt;
	uint32_t hwcnt;
	uint64_t cyc;
	int32_t diff;

	refcnt = thinkos_cyccnt();

	do {
		hwcnt = clk->hw.count;
		cyc = clk->hw.cycles;
	} while (hwcnt != clk->hw.count);

	diff = refcnt - hwcnt;
	return cyc + (((int64_t) diff * clk->hw.k) >> 8);
}

#define THINKOS_TIME_EPOCH ((uint64_t)0 << 32)

#ifndef THINKOS_TIME_EPOCH
#define THINKOS_TIME_EPOCH ((uint64_t)1606149342LL << 32)
#endif

void __thinkos_krn_time_init(struct thinkos_rt * krn)
{
	struct cm3_systick * systick = CM3_SYSTICK;

	krn->clk.resolution = THINKOS_TIME_SYSTICK_INC;
	krn->clk.increment = krn->clk.resolution;
	krn->clk.realtime_offs.sec = THINKOS_TIME_EPOCH >> 32;
	krn->clk.realtime_offs.frac = 0;
	krn->clk.timestamp.sec = 0;
	krn->clk.timestamp.frac = 0;
	/* XXX: assume system tick = 1ms */
	krn->clk.k = (uint64_t) (1LL << 32) / (1000 * systick->rvr);

	DCC_LOG1(LOG_TRACE, "resolution=%d", krn->clk.resolution);
}

uint64_t __krn_clock_timestamp(struct thinkos_rt * krn)
{
	struct cm3_systick * systick = CM3_SYSTICK;
	union krn_time ts;
    register uint32_t dt;
    register uint32_t cvr0;
    register uint32_t cvr1;
    register uint32_t cnt;

	do {
		cvr0 = systick->cvr;
		ts.sec = krn->clk.timestamp.sec;
		ts.frac = krn->clk.timestamp.frac;
		cvr1 = systick->cvr;
	} while (cvr0 > cvr1);

    /* hardware timer ticks */
	cnt = systick->rvr - cvr1;
    dt = (((uint64_t)cnt * krn->clk.k) + (1LL << 30)) >> 31;
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

    return ts.u64 + dt;
}


void thinkos_time_svc(int32_t arg[], unsigned int self, 
					  struct thinkos_rt * krn) 
{
//    struct krn_clock * clk = &krn->time_clk;
	unsigned int oper = arg[0];
	union krn_time tm;

	tm.u64 = __krn_clock_timestamp(krn);

	switch (oper) {
	case THINKOS_TIME_MONOTONIC_GET:
		arg[1] = tm.sec;
		arg[0] = tm.frac;
		DCC_LOG2(LOG_TRACE, "ts=%d.%06d", tm.sec, 
				 (((uint64_t)tm.frac * 1000000) >> 32));
		break;

	case THINKOS_TIME_REALTIME_GET:
//		tm.u64 += clk->realtime_offs;
		arg[1] = tm.sec;
		arg[0] = tm.frac;
		DCC_LOG2(LOG_TRACE, "ts=%d.%06d", tm.sec, 
				 (((uint64_t)tm.frac * 1000000) >> 32));
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
			
			x.u64 = (int64_t)(x.u64 - tm.u64);

			krn->clk.realtime_offs.sec = x.sec;
			krn->clk.realtime_offs.frac = x.frac;
		}
		arg[0] = THINKOS_OK;
		break;

	case THINKOS_TIME_REALTIME_STEP:
	/* Step the clock by the specified interval 'dt'.
	   This will adjust the clock offset only. */
		{
			union krn_time dt;
			union krn_time x;

			dt.sec = arg[1];
			dt.frac = arg[2];
		
			x.sec = krn->clk.realtime_offs.sec;
			x.frac = krn->clk.realtime_offs.frac;

			x.u64 += dt.u64;

			krn->clk.realtime_offs.sec = x.sec;
			krn->clk.realtime_offs.frac = x.frac;
		}
		arg[0] = THINKOS_OK;
		break;

	case THINKOS_TIME_REALTIME_COMP:
		/* Drift compensation */
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
			//	clk->jitter = (clk->jitter + est_err) / 2;


			clk_d = Q31_MUL(drift, krn->clk.resolution);

			/* calculate the drift compenastion per tick */
			//  clk->drift_comp = Q31_MUL(drift, CLK_Q31(clk->resolution));
			//  clk_d = Q31_CLK(clk->drift_comp);

			/* update the drift compensation value */
		//	q31_d = CLK_Q31(clk_d);
		//	clk->drift_comp = q31_d;

			/* Update the increpent per tick */
			krn->clk.increment = krn->clk.resolution + clk_d;

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

int krn_fmt_clk_realtime(struct thinkos_rt * krn, char * s) 
{
	union krn_time tm;
	uint32_t frac;

	tm.u64 = __krn_clock_timestamp(krn);

	frac = ((uint64_t)tm.frac * 1000LL + (1LL << 31)) >> 32;

	return krn_snprintf(s, 16, "%d.%03d", tm.sec, frac);
}

#endif /* (THINKOS_ENABLE_DATE_AND_TIME) */


