/* 
 * thinkos_systmr.c
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
#include <sys/delay.h>
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

void __attribute__((noinline)) __thinkos_krn_clk_wakeup(struct thinkos_rt * krn, 
														 unsigned int th) 
{
#if (THINKOS_ENABLE_THREAD_STAT)
	int wq;
	/* update the thread status */
	wq = __thread_stat_wq_get(krn, th);
	__thread_stat_clr(krn, th);
	/* remove from other wait queue, if any */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
#endif
	DCC_LOG1(LOG_TRACE, "<%2d> wakeup!", th);
	/* remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
	/* insert into the ready wait queue */
	__bit_mem_wr(&krn->wq_ready, (th - 1), 1);

	__krn_preempt(krn);
}

#if (THINKOS_ENABLE_TIMESHARE)
static void __thinkos_timeshare(struct thinkos_rt * krn) 
{
	int32_t idx;

	idx = __krn_sched_active_get(krn);

	/*  */
	thinkos_rt.sched_val[idx] -= thinkos_rt.sched_pri[idx];
	if (thinkos_rt.sched_val[idx] < 0) {
		thinkos_rt.sched_val[idx] += thinkos_rt.sched_limit;
		if (__bit_mem_rd(&thinkos_rt.wq_ready, idx) == 0) {
			DCC_LOG1(LOG_TRACE, "thread %d is active but not ready!!!", idx);
		} else {
			/* insert into the CPU wait queue */
			__bit_mem_wr(&thinkos_rt.wq_tmshare, idx, 1);  
			__krn_thread_suspend(krn, idx + 1);
			__krn_preempt(krn);
		}
	}
}
#endif /* THINKOS_ENABLE_TIMESHARE */


/* --------------------------------------------------------------------------
 * ThinkOS - system timer
 * --------------------------------------------------------------------------*/

static inline uint32_t __proc_tmr(uint32_t bmp, uint32_t lst[], uint32_t clk) {
	uint32_t tmp;
	asm volatile (
				  "ldmia\t%1!, {%2}\n" 
				  "subs\t%2, %5, %2\n" 
				  "adcs\t%0, %0, %0\n" 
				  : "=l" (bmp), "=l"(lst), "=l" (tmp) : "0" (bmp), 
				  "1" (lst),  "l" (clk));
	return bmp;
}

#define __PROC_TMR(_BMP, _LST, _CLK, _TMP)  asm volatile ( \
	"ldmia\t%1!, {%2, %3}\n" \
	"subs\t%2, %4, %2\n" \
	"adcs\t%0, %0, %0\n" \
	: "=l" (_BMP), "=l"(_LST), "=l" (_TMP1) : "l" (_CLK), "0" (_BMP), "1" (_LST))


#define __PROC_TMR2(_BMP, _LST, _CLK, _TMP1, _TMP2)  asm volatile ( \
	"ldmia\t%1!, {%2, %3}\n" \
	"subs\t%2, %4, %2\n" \
	"adcs\t%0, %0, %0\n" \
	"subs\t%3, %4, %3\n" \
	"adcs\t%0, %0, %0\n" \
	: "=l" (_BMP), "=l"(_LST), "=l" (_TMP1), "=l" (_TMP2) : \
	"l" (_CLK), "0" (_BMP), "1" (_LST))

#define __PROC_TMR4(_BMP, _LST, _CLK, _TMP1, _TMP2, _TMP3, _TMP4)  asm volatile ( \
	"ldmia\t%1!, {%2, %3, %4, %5}\n" \
	"subs\t%2, %6, %2\n" \
	"adcs\t%0, %0, %0\n" \
	"subs\t%3, %6, %3\n" \
	"adcs\t%0, %0, %0\n" \
	"subs\t%4, %6, %4\n" \
	"adcs\t%0, %0, %0\n" \
	"subs\t%5, %6, %5\n" \
	"adcs\t%0, %0, %0\n" \
	: "=l" (_BMP), "=l"(_LST), "=l" (_TMP1), "=l" (_TMP2), \
	  "=l" (_TMP3), "=l" (_TMP4) : "l" (_CLK), "0" (_BMP), "1" (_LST))


void __thinkos_monitor_on_reset(void);
void __monitor_context_swap(uint32_t ** pctx); 

void __krn_clk_set_wakeup(struct thinkos_rt * krn, uint32_t bmp)
{
	uint32_t msk;

	msk = krn->wq_clock;
	bmp &= msk;

	if (bmp != 0) {
#if (THINKOS_ENABLE_THREAD_STAT)
		int j;

		bmp = __rbit(bmp);
		while ((j = __clz(bmp)) < 32) {
			int32_t th = j + 1;
			bmp &= ~(0x80000000 >> j);  
			int wq;
			/* update the thread status */
			wq = __thread_stat_wq_get(krn, th);
			__thread_stat_clr(krn, th);
			/* remove from other wait queue, if any */
			__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
			DCC_LOG1(LOG_MSG, "<%2d> wakeup!", th);
			/* remove from the time wait queue */
			__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
			/* insert into the ready wait queue */
			__bit_mem_wr(&krn->wq_ready, (th - 1), 1);
		}
#else
		uint32_t ready;

		do {
			ready = __ldrex((uint32_t *)&krn->wq_ready);
			ready |= bmp;
		} while (__strex((uint32_t *)&krn->wq_ready, ready));

		krn->wq_clock = msk & ~bmp;
#endif
		__krn_preempt(krn);
	}
}

void __attribute__((aligned(16))) cm3_systick_isr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
  #if (THINKOS_ENABLE_MONITOR)
	struct cm3_systick * systick = CM3_SYSTICK;
	do {
		uint32_t sigset;
		uint32_t sigmsk;
		uint32_t sigact;
		int ev;

		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
  #endif
		{
			uint32_t clk;
#if (THINKOS_ENABLE_DATE_AND_TIME)
			union krn_time ts;

			ts.sec = krn->clk.timestamp.sec;
			ts.frac = krn->clk.timestamp.frac;

			ts.u64 += krn->clk.increment;
			krn->clk.timestamp.sec = ts.sec;
			krn->clk.timestamp.frac = ts.frac;
			clk = ts.u64 >> 12; 
#else
			clk = krn->clk.time; 
#if (THINKOS_ENABLE_FRACTIONAL_CLOCK)
			clk += krn->clk.increment;
#else
			clk += 1;
#endif

#endif
			krn->clk.time = clk; 

#if 0
			if (krn->wq_clock) 
			{
				uint32_t * lst;
				unsigned int j;
				uint32_t bmp;
				uint32_t tmp;

				lst = krn->clk.th_tmr;
				j = 32 * 4;
				bmp = 0;
				do {
					asm volatile ( 
								  "ldr\t%1, [%3, %4]\n" 
								  "subs\t%1, %2, %1\n" 
								  "adcs\t%0, %0, %0\n" 
								  : "=l" (bmp), "=l" (tmp) : "l" (clk),  
								  "l" (lst),  "l" (j),  "0" (bmp));
					j -= 4;
				} while (j != 0);

				__krn_clk_set_wakeup(krn, bmp);

			}
#endif
#if 0
			if (krn->wq_clock) 
			{
				uint32_t * lst;
				uint32_t bmp;
				register int tmp1 asm("r4");
				register int tmp2 asm("r5");
				register int tmp3 asm("r6");
				register int tmp4 asm("r7");

				bmp = 0;
				lst = &krn->clk.th_tmr[1];
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);
				__PROC_TMR4(bmp, lst, clk, tmp1, tmp2, tmp3, tmp4);

				
				bmp = __rbit(bmp);

				__krn_clk_set_wakeup(krn, bmp);
			}
#endif

#if 1
			uint32_t wq = __rbit(krn->wq_clock);
			int j;

			while ((j = __clz(wq)) < 32) {
				int32_t th = j + 1;
				wq &= ~(0x80000000 >> j);  
				if ((int32_t)(krn->clk.th_tmr[th] - clk) <= 0) {
					__thinkos_krn_clk_wakeup(krn, th); 
				}
			}

#endif


    #if (THINKOS_ENABLE_MONITOR_CLOCK)
			if ((int32_t)(krn->clk.th_tmr[0] - clk) >= 0 ) {
				sigset = krn->monitor.events;
				sigset |= MONITOR_ALARM;
				krn->monitor.events = sigset;
			}
    #endif
		

    #if (THINKOS_ENABLE_TIMESHARE)
			__thinkos_krn_timeshare(krn); 
    #endif /* THINKOS_ENABLE_TIMESHARE */
		}

  #if (THINKOS_ENABLE_MONITOR)
		sigset = krn->monitor.events;
		sigmsk = krn->monitor.mask;
		sigact = sigset & sigmsk;

		/* Process monitor events */
		if (sigact == 0)
			break;

		if ((ev = __clz(sigact)) < 8) {
			/* clear the TASK_INIT event */
			sigset &= ~(1 << (31 - ev));
//			DCC_LOG2(LOG_TRACE, "DSR sigset=%08x, ev=%0d", sigset, ev); 
			krn->monitor.events = sigset;
			krn->monitor.svc->on_event[ev](krn, krn->monitor.env);
		} else {
			DCC_LOG2(LOG_TRACE, "swap sigact=%08x sched=%08x.", sigact,
					 krn->sched.state); 
			__monitor_context_swap(&krn->monitor.ctx); 
		}

	} while (1);

  #endif /* THINKOS_ENABLE_MONITOR */
}

#define THINKOS_SYSTICK_FREQ (1000) /* T = 1ms */

#define THINKOS_CLK_INCREMENT (((uint64_t)(1LL << 32) / \
								(THINKOS_SYSTICK_FREQ)) >> 12)

#define THINKOS_CLK_RESOLUTION (((uint64_t)(1LL << 32) / \
								(THINKOS_SYSTICK_FREQ)))

extern const uint32_t krn_timer_clk_k;
extern const uint32_t krn_timer_freq;
extern const uint32_t krn_timer_inc_q32;

void thinkos_krn_systick_init(struct thinkos_rt * krn)
{
	struct cm3_systick * systick = CM3_SYSTICK;

	DCC_LOG(LOG_TRACE, "Initializing SysTick..."); 
	/* Initialize the SysTick module */
	systick->rvr = cm3_systick_load_1ms; /* 1ms tick period */
	systick->cvr = 0;

	systick->csr = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT;

	DCC_LOG3(LOG_TRACE, "F=%u Hz, inc=%u, K=%u", krn_timer_freq, 
			krn_timer_inc_q32, krn_timer_clk_k); 


#if (THINKOS_ENABLE_DATE_AND_TIME)
	krn->clk.resolution = THINKOS_CLK_RESOLUTION;
	krn->clk.increment = krn->clk.resolution;
	DCC_LOG1(LOG_TRACE, "clk.increment=%u", krn->clk.increment); 
#elif (THINKOS_ENABLE_FRACTIONAL_CLOCK)
	krn->clk.increment = THINKOS_CLK_INCREMENT;
	DCC_LOG1(LOG_TRACE, "clk.increment=%u", krn->clk.increment); 
#endif
}

#if (THINKOS_ENABLE_UDELAY_CALIBRATE)
static unsigned int __get_ticks(void)
{
	return 0xffffffff - (CM3_SYSTICK->cvr << 8);
}

void thinkos_krn_udelay_calibrate(void)
{
	struct cm3_systick * systick = CM3_SYSTICK;
	uint32_t ticks1ms;
	uint32_t rvr;
	uint32_t csr;

	rvr = systick->rvr;
	csr = systick->csr;

	systick->rvr = 0x00ffffff;
	systick->csr = SYSTICK_CSR_ENABLE;
	ticks1ms = cm3_systick_load_1ms << 8;

	udelay_calibrate(ticks1ms, __get_ticks);

	systick->rvr = rvr;
	systick->csr = csr;
}
#endif

#if (THINKOS_ENABLE_PAUSE)
bool clock_resume(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int wq, bool tmw) 
{
//	if ((int32_t)(krn->clock[th] - krn->ticks) <= 0) {
		/* thread's clock is in the past, wakeup now. */
//		DCC_LOG1(LOG_INFO, "timeout PC=%08x .......", __thinkos_thread_pc_get(th)); 
		/* update the thread status */
//		__thinkos_thread_stat_clr(th);
		/* insert into the ready wait queue */
//		__bit_mem_wr(&krn->wq_ready, th, 1);  
//	} else {
	DCC_LOG2(LOG_INFO, "th=%d PC=%08x +++++", th, __thread_pc_get(krn, th)); 
	if (tmw)
		__thread_clk_enable(krn, th);
//	}
	return true;
}
#endif

