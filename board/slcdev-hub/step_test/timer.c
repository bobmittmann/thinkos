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

#include <sys/stm32f.h>
#include <sys/delay.h>
#include <thinkos.h>
/*--------------------------------------------------------------------------
  Fault generators
  --------------------------------------------------------------------------*/

void read_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		x = *ptr;
		(void)x;
		ptr += 0x10000000 / (2 << 4);
	}
}

void write_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x = 0;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		*ptr = x;
		ptr += 0x10000000 / (2 << 4);
	}
}

#define SLEEP_TMR_RESOLUTION_US 64
#define SLEEP_TMR_FREQENCY_HZ (1000000 / SLEEP_TMR_RESOLUTION_US)

void __attribute__((constructor)) usleep_timer_init(void)
{
	struct stm32f_tim * tim = STM32F_TIM10;
	unsigned int div;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM10);
	/* get the total divisior */
	div = (stm32f_tim2_hz + (SLEEP_TMR_FREQENCY_HZ / 2)) / SLEEP_TMR_FREQENCY_HZ;
	/* Timer configuration */
	tim->psc = div - 1;
	tim->arr = 0;
	tim->cnt = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS;
	tim->sr = 0;
//	cm3_irq_pri_set(STM32F_IRQ_TIM10, IRQ_PRIORITY_HIGH);
	thinkos_irq_priority_set(STM32F_IRQ_TIM10, IRQ_PRIORITY_HIGH);
}

void usleep(unsigned int usec)
{
	struct stm32f_tim * tim = STM32F_TIM10;

	tim->arr = (usec / SLEEP_TMR_RESOLUTION_US) - 1;
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS | TIM_CEN; 
//	while ((tim->sr & TIM_UIF) == 0) {
		thinkos_irq_wait(STM32F_IRQ_TIM10);
//	}
	tim->sr = 0;
}

volatile uint32_t irq_count = 0 ;
volatile uint32_t tmr_count = 0 ;
int tmr_sem;

void stm32f_tim4_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM4;
	/* Clear timer interrupt flags */
	tim->sr = 0;
	tmr_count++;
	thinkos_sem_post_i(tmr_sem);
}

void oneshot_timer(unsigned int usec)
{
	struct stm32f_tim * tim = STM32F_TIM4;

	tim->arr = usec;
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS | TIM_CEN;
}

void oneshot_timer_init(uint32_t freq)
{
	struct stm32f_tim * tim = STM32F_TIM4;
	unsigned int div;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM4);
	/* get the total divisior */
	div = (stm32f_tim2_hz + (freq / 2)) / freq;
	/* Timer configuration */
	tim->psc = div - 1;
	tim->arr = 0;
	tim->cnt = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS;
	cm3_irq_enable(STM32F_IRQ_TIM4);
}

struct {
	unsigned int sec;
	unsigned int min;
} clock;

void stm32f_tim3_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM3;
#if 0
	oneshot_timer(20); /* time to signal the semaphore after the
						  first wakeup, before going to sleep again. */
#endif
//	oneshot_timer(41);
//	oneshot_timer(66); /* rollback 1 */
	oneshot_timer(69); /* rollback 2 */
//	oneshot_timer(80); /* sleep */

	/* Clear timer interrupt flags */
	tim->sr = 0;
	irq_count++;

	if ((irq_count % 2) == 0) {
		clock.sec++;

//		read_fault();

		if (clock.sec == 10) {
			clock.sec = 0;
			clock.min++;
		}
	}

	thinkos_sem_post_i(tmr_sem);
}

void periodic_timer_init(uint32_t freq)
{
	struct stm32f_tim * tim = STM32F_TIM3;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = ((stm32f_tim1_hz) + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM3);

	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = n - 1;
	tim->cr1 = TIM_CEN; /* Enable counter */
	tim->egr = TIM_UG; /* Force an update */
	while (tim->sr == 0);
	tim->sr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */

	thinkos_irq_register(STM32F_IRQ_TIM3, IRQ_PRIORITY_HIGH, stm32f_tim3_isr);
	/* enable interrupts */
//	cm3_irq_enable(STM32F_IRQ_TIM3);
}

void __attribute__((constructor)) irq_timer_init(void)
{
	tmr_sem = thinkos_sem_alloc(0);

//	oneshot_timer_init(168000000 / 2);
//	periodic_timer_init(2);
}
