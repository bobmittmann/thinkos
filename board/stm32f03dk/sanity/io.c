/* 
 * File:	 io.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2013 Bob Mittmann. All Rights Reserved.
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
#include <thinkos.h>

#include "io.h"
#include "board.h"

#define POLL_PERIOD_MS 16

/* GPIO pin description */ 
struct stm32f_io {
	struct stm32_gpio * gpio;
	uint8_t pin;
};

/* ----------------------------------------------------------------------
 * LEDs 
 * ----------------------------------------------------------------------
 */

const struct stm32f_io led_io[] = {
	{ IO_LED3 },
	{ IO_LED4 },
	{ IO_LED5 },
	{ IO_LED6 }
};

#define UNLOCKED -1

#define LED_CNT (sizeof(led_io) / sizeof(struct stm32f_io))

struct {
	int lock;
	int mutex;
	uint8_t tmr[LED_CNT];

	volatile uint8_t req[LED_CNT];
	volatile uint8_t ack[LED_CNT];
	volatile uint8_t itv[LED_CNT];
	volatile uint8_t on[LED_CNT];
} led_drv;

void led_on(int id)
{
	uint8_t req;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	if ((req = led_drv.req[id]) != led_drv.ack[id]) {
		thinkos_bkpt(1);
		/* pending request */
		return;
	}

	led_drv.itv[id] = 0;
	led_drv.on[id] = 1;
	led_drv.req[id] = req + 1;
}

void led_off(int id)
{
	uint8_t req;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	if ((req = led_drv.req[id]) != led_drv.ack[id]) {
		thinkos_bkpt(1);
		/* pending request */
		return;
	}

	led_drv.itv[id] = 0;
	led_drv.on[id] = 0;

	led_drv.req[id] = req + 1;
}

void led_flash(int id, int ms)
{
	uint8_t on;
	uint8_t req;

	on = (ms * (65536 / POLL_PERIOD_MS) + 32768) / 65536;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	if ((req = led_drv.req[id]) != led_drv.ack[id]) {
		thinkos_bkpt(1);
		/* pending request */
		return;
	}

	led_drv.itv[id] = 0;
	led_drv.on[id] = on;
	led_drv.req[id] = req + 1;
}

void led_blink(int id, int on_ms, int off_ms)
{
	uint8_t off;
	uint8_t on;
	uint8_t req;

	on = (on_ms * (65536 / POLL_PERIOD_MS) + 32768) / 65536;
	off = (off_ms * (65536 / POLL_PERIOD_MS) + 32768) / 65536;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	if ((req = led_drv.req[id]) != led_drv.ack[id]) {
		thinkos_bkpt(1);
		/* pending request */
		return;
	}

	led_drv.itv[id] = on + off;
	led_drv.on[id] = on;
	led_drv.req[id] = req + 1;
}

void leds_all_off(void)
{
	unsigned int id;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	for (id = 0; id < LED_CNT; ++id) {
	}
}

void leds_all_on(void)
{
	unsigned int i;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	for (i = 0; i < LED_CNT; ++i)
		stm32_gpio_set(led_io[i].gpio, led_io[i].pin);
}

void leds_all_flash(int ms)
{
	unsigned int i;
	uint8_t on;

	on = (ms * (65536 / POLL_PERIOD_MS) + 32768) / 65536;

//	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
//		return;

	/* FIXME: atomic request */
	for (i = 0; i < LED_CNT; ++i) {
		led_drv.itv[i] = 0;
		led_drv.on[i] = on;
	}
}

void leds_lock(void)
{
	if (led_drv.lock != UNLOCKED)
		return;

	led_drv.lock = thinkos_thread_self();
}

void leds_unlock(void)
{
	if ((led_drv.lock != UNLOCKED) && (led_drv.lock != thinkos_thread_self()))
		return;

	led_drv.lock = UNLOCKED;
}


static void leds_init(void)
{
	unsigned int i;

	led_drv.lock = UNLOCKED;

	for (i = 0; i < LED_CNT; ++i) {
		stm32_gpio_mode(led_io[i].gpio, led_io[i].pin,
						 OUTPUT, PUSH_PULL | SPEED_LOW);

		stm32_gpio_clr(led_io[i].gpio, led_io[i].pin);
	}

	for (i = 0; i < LED_CNT; ++i) {
		led_drv.tmr[i] = 0;
		led_drv.on[i] = 0;
		led_drv.itv[i] = 0;
		led_drv.req[i] = 0;
		led_drv.ack[i] = 0;
	}

	led_drv.mutex = thinkos_mutex_alloc();
}


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

int btn_event_wait(void)
{
	int event = EVENT_NONE;
	int irq_ev;

//	tracef("%s(): wait...", __func__);
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
	stm32_gpio_mode(IO_PUSH_BTN, INPUT, SPEED_LOW);
	btn_drv.flag = thinkos_flag_alloc();
	btn_drv.st = stm32_gpio_stat(IO_PUSH_BTN) ? 1 : 0;
	btn_drv.fsm = BTN_FSM_IDLE;
}


/* ----------------------------------------------------------------------
 * I/O 
 * ----------------------------------------------------------------------
 */

void stm32_tim2_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	unsigned int i;
	int st;

	/* Clear interrupt flags */
	tim->sr = 0;

	/* process led timers */
	for (i = 0; i < LED_CNT; ++i) {
		uint8_t itv;
		uint8_t on;
		uint8_t ack;
		uint8_t req;

		/* Attention: the reading order is important */
		ack = led_drv.ack[i];
		itv = led_drv.itv[i];
		on = led_drv.on[i];
		req = led_drv.req[i];

		if (req != ack) {
			/* process request */
			if (on)
				stm32_gpio_set(led_io[i].gpio, led_io[i].pin);
			else
				stm32_gpio_clr(led_io[i].gpio, led_io[i].pin);

			led_drv.tmr[i] = on;

			/* acknowledge request */
			led_drv.ack[i] = ack + 1;

			continue;
		}

		if (led_drv.tmr[i] == 0)
			continue;

		if (--led_drv.tmr[i] == 0) {
			stm32_gpio_clr(led_io[i].gpio, led_io[i].pin);
		    led_drv.tmr[i] = itv;
		} else if (led_drv.tmr[i] == on) {
			stm32_gpio_set(led_io[i].gpio, led_io[i].pin);
		}

	}

	if ((btn_drv.tmr) && (--btn_drv.tmr == 0)) {
		/* process button timer */
		btn_drv.event = BTN_TIMEOUT;
		thinkos_flag_give(btn_drv.flag);
	} else {
		/* process push button */
		st = stm32_gpio_stat(IO_PUSH_BTN) ? 1 : 0;
		if (btn_drv.st != st) {
			btn_drv.st = st;
			btn_drv.event = st ? BTN_PRESSED : BTN_RELEASED;
			thinkos_flag_give(btn_drv.flag);
		}
	}
}

void __attribute__((noreturn)) io_task(void * arg)
{
	for (;;) {
		thinkos_irq_wait(STM32F_IRQ_TIM2);
		stm32_tim2_isr();
	}
}

static void io_timer_init(uint32_t freq)
{
	struct stm32_rcc * rcc = STM32_RCC;
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
	rcc->apb1enr |= RCC_TIM2EN;
	
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
	tim->ccmr1 = TIM_OC1M_PWM_MODE1;
	tim->ccr1 = tim->arr / 2;

	tim->cr1 = TIM_URS | TIM_CEN; /* Enable counter */
}

uint32_t io_stack[64] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf io_thread_inf = {
	.stack_ptr = io_stack,
	.stack_size = sizeof(io_stack),
	.priority = 28,
	.thread_id = 28,
	.paused = 0,
	.tag = "RATEGEN"
};

void io_init(void)
{
	/* Enable IO clocks */
	stm32_gpio_mode(IO_PUSH_BTN, INPUT, PULL_UP);

	stm32_gpio_mode(STM32_GPIOB, 10, INPUT, 0);
	stm32_gpio_mode(STM32_GPIOB, 11, INPUT, 0);

	btn_init();
	leds_init();

	io_timer_init(1000 / POLL_PERIOD_MS);

	thinkos_thread_create_inf((void *)io_task, (void *)NULL,
				  &io_thread_inf);
}

