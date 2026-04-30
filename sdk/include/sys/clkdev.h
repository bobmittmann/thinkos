
/* 
 * Copyright(c) 2017 Robinson Mittmann. All Rights Reserved.
 *
 * This file is part of the libclkdev
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
 * @file sys/clkdev.h
 * @brief ThinkOS -ICE libclksync
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYS_CLKDEV_H__
#define __SYS_CLKDEV_H__

//#ifndef __SYS_CLKDEV__
//#error "Never use <sys/clkdev.h> directly; include <clock.h> instead."
//#endif 

#include <stdlib.h>
#include <stdint.h>
#include <fixpt.h>

/* 
   = Abstract clock device 

   The clock devices are used to keep track of time or periodic events.

   A clock has a period representing one cycle of a particular unit of 
   time measurement. Ex: seconds. This period can be adjusted to 
   synchronize two different clocks using a particular clock adjustment
   algorithm defined in the clksync ....

   A clock device can use any mechanism to store and update it's internal
   clock. But it has to convert to an appropriate common Public 
   representation.
   
   == Clock public representation (current value):

   Represented as a 64 bits fractional value. The upper 32bits corresponds 
   to an integral count of clock periods. The lower bits are the fraction 
   of the period.

   == Clock tick:

   A clock tick increments the clock value by a fraction of the clock
   period called "tick interval". Each call of to clock_tick() should add 
   one tick interval to the clock value.

   Clock Period:

       +-------------------+
       |
       |
   ----+
*/


/* Convert from float point to clock interval */
#define FLOAT_CLK(X)     ((int64_t)((X) * (float)4294967296.))
/* Convert from clock interval to float point */
#define CLK_FLOAT(X)     ((float)(int64_t)(X) / 4294967296.)
/* Convert from clock interval to double float point */
#define CLK_DOUBLE(X)    ((double)(int64_t)(X) / 4294967296.)
/* convert from clock interval to microsseconds */
#define CLK_US(X)        (((int64_t)(X) * 1000000LL) / 4294967296LL)
/* convert from clock interval to milliseconds */
#define CLK_MS(X)        (((int64_t)(X) * 1000LL) / 4294967296LL)
/* convert from clock interval to seconds */
#define CLK_SEC(X)       ((int32_t)((int64_t)(X) / 4294967296LL))
/* Q31.32 multiplication */
#define CLK_MUL(X1, X2)   (((int64_t)(X1) * (int64_t)(X2)) >> 32)
/* Q31.32 division */
#define CLK_DIV(NUM, DEN) (((int64_t)(NUM) << 32) / (int32_t)(DEN))

/* Return the clock interval from Y to X in fixed point Q1.31 */
#define CLK_ITV(X, Y)   (((int64_t)(X) - (int64_t)(Y) + 1) / 2)
/* Convert from fixed point Q1.31 to clock interval */
#define Q31_CLK(X)       ((int64_t)((X) * 2))
/* Convert from clock interval to fixed point Q1.31 */
#define CLK_Q31(X)       ((int32_t)(((X) + 1) / 2)) 

struct clkdev;

/* Clock device operations */
struct clkdevop {
	/* Increment the clock time/count by one tick interval */
	int (* tick)(void * clk);
	/* Get the present clock value */
	uint64_t (* get)(void * clk);
	/* Set the clock's value */
	void (* set)(void * clk, uint64_t t);
	/* Step the clock by a specified time difference 'dt' */
	void (* step)(void * clk, int64_t dt);
	/* Drift compensation. Apply a drift adjustemnt to the
	   clock resulting in an adjustment to the tick interval. 
	 The drift is a time difference and precision represents
	 the error margin estimation of the drift compensation
	 value. */
	int32_t (* drift_comp)(void * clk, int32_t drift, int32_t precision);
	/* Get the present drift compenation value of the clock */
	int32_t (* drift_get)(void * clk);
};


/* Clock device */
struct clkdev {
	void * clk; /* Implementation specific clock structure */
	const struct clkdevop * op; /* Clock operations 
	*/
};

static inline int clock_tick(struct clkdev * __dev) {
	return __dev->op->tick(__dev->clk);
}

static inline uint64_t clock_get(struct clkdev * __dev) {
	return __dev->op->get(__dev->clk);
}

static inline void clock_set(struct clkdev * __dev, uint64_t __t) {
	__dev->op->set(__dev->clk, __t);
}

static inline void clock_step(struct clkdev * __dev, int64_t __dt) {
	__dev->op->step(__dev->clk, __dt);
}

static inline int32_t clock_drift_comp(struct clkdev * __dev, 
										int32_t __drift, 
										int32_t __precision) {
	return __dev->op->drift_comp(__dev->clk, __drift, __precision);
}

static inline int32_t clock_drift_get(struct clkdev * __dev) {
	return __dev->op->drift_get(__dev->clk);
}

#ifdef __cplusplus
extern "C" {
#endif

int clkdev_free(struct clkdev * __dev);

struct clkdev * clkdev_alloc(void * __clk, const struct clkdevop * __op);

/****************************************************************************
 * Utility functions 
 ****************************************************************************/

/* Format a clock timetamp into a string with upt ot microsseconds resolution.
   The string has to be at least 20 characters long.
   The fields to be printed are adjusted according to their content... */
char * fmt_clk(char * s, int64_t ts);
#define FMT_CLK(TS) __extension__({char __s[20]; \
		fmt_clk(__s, (int64_t)(TS)); })

/* Format a clock timetamp into a string with milliseconds resolution.
   The string has to be at least 18 characters long */
char * fmt_clk_ms(char * s, int64_t ts);
#define FMT_CLK_MS(TS) __extension__({char __s[20]; \
		fmt_clk_ms(__s, (int64_t)(TS)); })

/* Format a clock timetamp into a string with microsseconds resolution.
   The string has to be at least 20 characters long */
char * fmt_clk_us(char * s, int64_t ts);
#define FMT_CLK_US(TS) __extension__({char __s[20]; \
		fmt_clk_us(__s, (int64_t)(TS)); })

/* Format a Q31 number using 3 decimal places */
char * fmt_q31_3(char * s, int64_t x);
#define FMT_Q31_3(X) __extension__({char __s[16]; \
		fmt_q31_3(__s, X); })

/* Format a Q31 number using 6 decimal places */
char * fmt_q31_6(char * s, int64_t x);
#define FMT_Q31_6(X) __extension__({char __s[16]; \
		fmt_q31_6(__s, X); })

/* Format a Q31 number using 9 decimal places */
char * fmt_q31_9(char * s, int64_t x);
#define FMT_Q31_9(X) __extension__({char __s[16]; \
		fmt_q31_9(__s, X); })

/* Format a clock timetamp into a string with 3 fractional decimal digits.
   The string has to be at least 16 characters long */
char * fmt_clk_3(char * s, int64_t x);
#define FMT_CLK_3(X) __extension__({char __s[16]; \
		fmt_clk_3(__s, X); })

/* Format a clock timetamp into a string with 6 fractional decimal digits.
   The string has to be at least 19 characters long */
char * fmt_clk_6(char * s, int64_t x);
#define FMT_CLK_6(X) __extension__({char __s[20]; \
		fmt_clk_6(__s, X); })

/* Format a clock timetamp into a string with 9 fractional decimal digits.
   The string has to be at least 22 characters long */
char * fmt_clk_9(char * s, int64_t x);
#define FMT_CLK_9(X) __extension__({char __s[22]; \
		fmt_clk_9(__s, X); })

#ifdef __cplusplus
}
#endif

#endif /* __SYS_CLKDEV_H__ */

