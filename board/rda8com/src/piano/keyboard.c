/* 
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the usb-serial converter.
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
 * @file usbamp.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "board.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <thinkos.h>
#include <midi.h>

#include "spidrv.h"
#include "encoder.h"
#include "keyboard.h"

#define IO_PUSH_BTN STM32_GPIOB, 11

#define KBD_KEY_CNT  17
#define KBD_FIFO_LEN 32

#define KBD_POLL_ITV_MS 4

#define KBD_TMR_CNT 1

struct keyboard_drv {
	uint8_t flag; 
	uint8_t mutex;
	uint8_t keycode[KBD_KEY_CNT];
	struct {
		volatile uint32_t en;
		uint16_t itv[KBD_TMR_CNT];
	} tmr;
	struct {
		volatile uint8_t head;
		volatile uint8_t tail;
		uint16_t buf[KBD_FIFO_LEN];
	} fifo;
};

static inline void __kbd_ev_put(struct keyboard_drv * drv,
								uint8_t opc, uint8_t arg)
{
	uint8_t head = drv->fifo.head;
	uint8_t tail = drv->fifo.tail;
	
	if ((int8_t)(head - tail) < KBD_FIFO_LEN) {
		drv->fifo.buf[head % KBD_FIFO_LEN] = KBD_EVENT_ENCODE(opc, arg);
		drv->fifo.head = head + 1;
	}
	thinkos_flag_give(drv->flag);
}

static inline uint32_t __kbd_io_poll(uint32_t seq)
{
	uint32_t spi_in;
	uint32_t gpio_in;

	/* process SPI inputs */
	spi_in = spidrv_xfer(seq) & 0x0000ffff;
	/* process GPIO inputs */
	gpio_in = stm32_gpio_stat(IO_PUSH_BTN) ? 0 : (1 << 16);

	return spi_in | gpio_in;
}

int keyboard_task(struct keyboard_drv * drv)
{
	uint32_t tmr_clk[KBD_TMR_CNT];
	uint32_t key_filt;
	uint32_t key_stat;
	uint32_t seq = 0;
	uint32_t clk;
	int j;

	key_filt = __kbd_io_poll(seq++);
	key_stat = key_filt;

	/* Initialize timers */
	clk = thinkos_clock();
	for (j = 0; j < KBD_TMR_CNT; j++) {
		tmr_clk[j] = clk;
	}

	thinkos_mutex_lock(drv->mutex);

	for (;;) {
		uint32_t key_down = 0;
		uint32_t key_up = 0;
		uint32_t prev;
		uint32_t bmp;

		thinkos_mutex_unlock(drv->mutex);

		/* periodic task, wait for alarm clock */
		clk += KBD_POLL_ITV_MS;
		/* update next alarm */
		thinkos_alarm(clk);

		bmp = __rbit(drv->tmr.en);
		while ((j = __clz(bmp)) < 32) {
			bmp &= ~(0x80000000 >> j);  

			if ((int32_t)(tmr_clk[j] - clk) <= 0) {
				__kbd_ev_put(drv, KBD_EV_TIMEOUT, j);
				tmr_clk[j] += drv->tmr.itv[j];
			}
		}

		/* IO polling */
		prev = key_filt;
		key_filt = __kbd_io_poll(seq++);
		/* Debouncing */
		if (prev == key_filt) {
			int32_t diff;
			if ((diff = (key_stat ^ key_filt)) != 0) {
				key_down = diff & ~key_filt;
				key_up |= diff & key_filt;
			}
			key_stat = key_filt;
		} else {
		}

		thinkos_mutex_lock(drv->mutex);

		if (key_down) {
			bmp = __rbit(key_down);
			while ((j = __clz(bmp)) < 32) {
				int code = drv->keycode[j];

				if (code < 128) {
					__kbd_ev_put(drv, KBD_EV_KEY_ON, code);
				} else {
					__kbd_ev_put(drv, KBD_EV_SWITCH_ON, code & 0x7f);
				}
				bmp &= ~(0x80000000 >> j);  
			}
		}

		if (key_up) {
			bmp = __rbit(key_up);
			while ((j = __clz(bmp)) < 32) {
				int code = drv->keycode[j];

				if (code < 128) {
					__kbd_ev_put(drv, KBD_EV_KEY_OFF, code);
				} else {
					__kbd_ev_put(drv, KBD_EV_SWITCH_OFF, code & 0x7f);
				}

				bmp &= ~(0x80000000 >> j);  
			}
		}

	}

	return 0;
}

uint32_t keyboard_stack[128] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf keyboard_thread_inf = {
	.stack_ptr = keyboard_stack,
	.stack_size = sizeof(keyboard_stack),
	.priority = 4,
	.thread_id = 4,
	.paused = false,
	.tag = "KBOARD"
};

struct keyboard_drv keyboard_drv_rt;

static void __kbd_io_init(void)
{
	/* Enable IO clocks */
	stm32_gpio_mode(IO_PUSH_BTN, INPUT, PULL_UP);
	stm32_gpio_mode(STM32_GPIOB, 10, OUTPUT, PUSH_PULL);
	stm32_gpio_clr(STM32_GPIOB, 10);
}

static void __kbd_keymap(struct keyboard_drv * drv, 
				   unsigned int idx, unsigned int code)
{
	//printf("key[%2d] = %d\n", idx + 1, code);
	drv->keycode[idx] = code;
}

int keyboard_init(void)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	int i;

	if (drv->mutex != 0)
		return -1;

	__kbd_io_init();

	drv->mutex = thinkos_mutex_alloc();
	drv->flag = thinkos_flag_alloc();

	for (i = 0; i < KBD_KEY_CNT; ++i) {
		__kbd_keymap(drv, i, 0x80 + i);
	}

	for (i = 0; i < KBD_TMR_CNT; ++i) {
		drv->tmr.itv[i] = 0;
	}
	drv->tmr.en = 0;
	
	drv->fifo.head = 0;
	drv->fifo.tail = 0;

	thinkos_thread_create_inf(C_TASK(keyboard_task), (void *)drv,
							  &keyboard_thread_inf);
	return 0;
}

int keyboard_event_wait(void)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	uint8_t tail;
	int ev;

	//thinkos_mutex_lock(drv->mutex);

	tail = drv->fifo.tail;
	while (drv->fifo.head == tail) {
		thinkos_flag_take(drv->flag);
	}
	ev = drv->fifo.buf[tail % KBD_FIFO_LEN];
	drv->fifo.tail = tail + 1;

	//thinkos_mutex_unlock(drv->mutex);

	return ev;
}

int keyboard_keymap(unsigned int key, unsigned int code)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	unsigned int idx = key < 1;

	if (idx > KBD_KEY_CNT)
		return -1;

	thinkos_mutex_lock(drv->mutex);
	__kbd_keymap(drv, idx, code);
	thinkos_mutex_unlock(drv->mutex);

	return 0;
}

int keyboard_config(const struct keyboard_cfg * cfg)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	int ret = 0;
	int i;

	thinkos_mutex_lock(drv->mutex);

	for (i = 0; i < cfg->keymap_cnt; ++i) {
		unsigned int idx;

		if ((idx = (cfg->keymap[i].key - 1)) >= KBD_KEY_CNT) {
			ret = -1;
			break;
		}

		__kbd_keymap(drv, idx, cfg->keymap[i].code);
	}
	
	thinkos_mutex_unlock(drv->mutex);

	return ret;
}

int keyboard_timer_set(unsigned int timer_id, uint32_t itv_ms)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	unsigned int idx;
	int ret = -1;

	thinkos_mutex_lock(drv->mutex);

	if ((idx = timer_id - 1) < KBD_TMR_CNT) {
		drv->tmr.itv[idx] = itv_ms;
//		drv->tmr.en |= (1 << idx);
		ret = 0;
	}

	thinkos_mutex_unlock(drv->mutex);

	return ret;
}

int keyboard_timer_enable(unsigned int timer_id)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	unsigned int idx;
	int ret = -1;

	thinkos_mutex_lock(drv->mutex);

	if ((idx = timer_id - 1) < KBD_TMR_CNT) {
		drv->tmr.en |= (1 << idx);
		ret = 0;
	}

	thinkos_mutex_unlock(drv->mutex);

	return ret;
}

int keyboard_timer_disable(unsigned int timer_id)
{
	struct keyboard_drv * drv = &keyboard_drv_rt;
	unsigned int idx;
	int ret = -1;

	thinkos_mutex_lock(drv->mutex);

	if ((idx = timer_id - 1) < KBD_TMR_CNT) {
		drv->tmr.en &= ~(1 << idx);
		ret = 0;
	}

	thinkos_mutex_unlock(drv->mutex);

	return ret;
}

