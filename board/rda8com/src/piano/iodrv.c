/* 
 * File:	 iodrv.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2020 Bob Mittmann. All Rights Reserved.
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
#include <stdint.h>
#include <stdio.h>
#include <thinkos.h>

#include "board.h"
#include "iodrv.h"

#define POLL_PERIOD_MS 10

#define IO_PUSH_BTN STM32_GPIOB, 11

/* GPIO pin description */ 
struct stm32f_io {
	struct stm32_gpio * gpio;
	uint8_t pin;
};

/* ----------------------------------------------------------------------
 * I/O 
 * ----------------------------------------------------------------------
 */

/* Button internal events */
enum {
	BTN_PRESSED = 1,
	BTN_RELEASED,
	BTN_TIMEOUT
};

/* Button FSM states */
enum {
	BTN_FSM_IDLE = 0,
	BTN_FSM_CLICK1_WAIT,
	BTN_FSM_PRE_WAIT1,
	BTN_FSM_CLICK2_WAIT,
	BTN_FSM_HOLD_TIME_WAIT,
	BTN_FSM_CLICK_N_HOLD_TIME_WAIT,
	BTN_FSM_HOLD_RELEASE_WAIT,
};

struct btn_drv {
	uint8_t st;
	volatile uint16_t tmr;
	uint8_t fsm;
	int8_t event;
	int flag;
} btn_drv;

bool btn_push_wait(void)
{
	do {
		thinkos_flag_take(btn_drv.flag);

	} while (btn_drv.event != BTN_PRESSED);

	return true;
}

int btn_event_wait(void)
{
	int event = EVENT_NONE;
	int irq_ev;

	thinkos_flag_take(btn_drv.flag);

	/* FIXME: possible race condition on this variable */
	irq_ev = btn_drv.event;
	btn_drv.event = 0;

	switch (btn_drv.fsm) {
	case BTN_FSM_IDLE:
		if (irq_ev == BTN_PRESSED) {
			/* double click timer */
			btn_drv.tmr = 500 / POLL_PERIOD_MS;
			btn_drv.fsm = BTN_FSM_CLICK1_WAIT;
		}
		break;

	case BTN_FSM_CLICK1_WAIT:
		/* wait for button release after first press */
		if (irq_ev == BTN_RELEASED) {
			event = EVENT_CLICK;
			btn_drv.fsm = BTN_FSM_PRE_WAIT1;
		} else if (irq_ev == BTN_TIMEOUT) {
			/* hold timer */
			btn_drv.tmr = 500 / POLL_PERIOD_MS;;
			btn_drv.fsm = BTN_FSM_HOLD_TIME_WAIT;
		}
		break;

	case BTN_FSM_PRE_WAIT1:
		/* wait for button press after first press and release
		 inside the double click timer */
		if (irq_ev == BTN_PRESSED) {
			btn_drv.fsm = BTN_FSM_CLICK2_WAIT;
		} else if (irq_ev == BTN_TIMEOUT) {
			btn_drv.fsm = BTN_FSM_IDLE;
		}
		break;

	case BTN_FSM_CLICK2_WAIT:
		/* wait for button release after press/release/press
		 inside the double click timer */
		if (irq_ev == BTN_RELEASED) {
			event = EVENT_DBL_CLICK;
			btn_drv.tmr = 0;
			btn_drv.fsm = BTN_FSM_IDLE;
		} else if (irq_ev == BTN_TIMEOUT) {
			btn_drv.fsm = BTN_FSM_IDLE;
			/* hold and click timer */
			btn_drv.tmr = 500 / POLL_PERIOD_MS;
			btn_drv.fsm = BTN_FSM_CLICK_N_HOLD_TIME_WAIT;
		}
		break;


	case BTN_FSM_HOLD_TIME_WAIT:
		/* wait for button release after press
		 inside the hold timer */
		if (irq_ev == BTN_RELEASED) {
			btn_drv.tmr = 0;
			btn_drv.fsm = BTN_FSM_IDLE;
		} else if (irq_ev == BTN_TIMEOUT) {
			event = EVENT_HOLD1;
			/* long hold timer */
			btn_drv.tmr = 3500 / POLL_PERIOD_MS;
			btn_drv.fsm = BTN_FSM_HOLD_RELEASE_WAIT;
		}
		break;

	case BTN_FSM_CLICK_N_HOLD_TIME_WAIT:
		/* wait for button release after press/release/press
		 inside the hold timer */
		if (irq_ev == BTN_RELEASED) {
			btn_drv.tmr = 0;
			btn_drv.fsm = BTN_FSM_IDLE;
		} else if (irq_ev == BTN_TIMEOUT) {
			event = EVENT_CLICK_N_HOLD;
			/* long hold timer */
			btn_drv.tmr = 3500 / POLL_PERIOD_MS;
			btn_drv.fsm = BTN_FSM_HOLD_RELEASE_WAIT;
		}
		break;

	case BTN_FSM_HOLD_RELEASE_WAIT:
		/* wait for button release after the hold timer expired */
		if (irq_ev == BTN_RELEASED) {
			btn_drv.tmr = 0;
			btn_drv.fsm = BTN_FSM_IDLE;
		} else if (irq_ev == BTN_TIMEOUT) {
			event = EVENT_HOLD2;
			btn_drv.fsm = BTN_FSM_IDLE;
		}
		break;
	};

	return event;
}

static void btn_init(void)
{
	btn_drv.flag = thinkos_flag_alloc();
	btn_drv.st = stm32_gpio_stat(IO_PUSH_BTN) ? 0 : 1;
	btn_drv.fsm = BTN_FSM_IDLE;
}


/* ----------------------------------------------------------------------
 * I/O 
 * ----------------------------------------------------------------------
 */

void __attribute__((noreturn)) io_task(void * arg)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	uint32_t seq = 0;

	for (;;) {
		int st;

		while ((tim->sr & TIM_UIF) == 0) {
			thinkos_irq_wait(STM32_IRQ_TIM2);
		}
		/* Clear interrupt flags */
		tim->sr = 0;

		if ((btn_drv.tmr) && (--btn_drv.tmr == 0)) {
			/* process button timer */
			btn_drv.event = BTN_TIMEOUT;
			thinkos_flag_give(btn_drv.flag);
		} else {
			/* process push button */
			st = stm32_gpio_stat(IO_PUSH_BTN) ? 0 : 1;
			if (btn_drv.st != st) {
				btn_drv.st = st;
				btn_drv.event = st ? BTN_PRESSED : BTN_RELEASED;
//				printf("BTN: %d", st ? 1: 0);
				thinkos_flag_give(btn_drv.flag);
			}
		}

		if (seq & 64) {
			stm32_gpio_clr(STM32_GPIOB, 10);
		} else {
			stm32_gpio_set(STM32_GPIOB, 10);
		}
		seq++;
	}
}

static void io_timer_init(uint32_t freq)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = ((2 * stm32f_apb1_hz) + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div * 2 + pre) / (2 * pre);

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	
	tim->cr1 = 0;		/* Disable the counter */
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n / 2;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
	tim->ccmr1 = 0;
	tim->ccr1 = 0;
	tim->cr2 = 0;
	tim->sr = 0;
	tim->cr1 = TIM_URS | TIM_CEN; /* Enable counter */

}

uint32_t io_stack[64] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf io_thread_inf = {
	.stack_ptr = io_stack,
	.stack_size = sizeof(io_stack),
	.priority = 28,
	.thread_id = 28,
	.paused = 0,
	.tag = "IODRV"
};

void iodrv_init(void)
{
	/* Enable IO clocks */
	stm32_gpio_mode(IO_PUSH_BTN, INPUT, PULL_UP);
	stm32_gpio_mode(STM32_GPIOB, 10, OUTPUT, PUSH_PULL);
	stm32_gpio_clr(STM32_GPIOB, 10);

	btn_init();

	io_timer_init(1000 / POLL_PERIOD_MS);

	thinkos_thread_create_inf((void *)io_task, (void *)NULL,
				  &io_thread_inf);
}

