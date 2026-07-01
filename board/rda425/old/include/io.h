/* 
 * File:	 board.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(c) 2003-2006 BORESTE (www.boreste.com). All Rights Reserved.
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

#ifndef __IO_H__
#define __IO_H__

#include <stdbool.h>
#include <stdint.h>

struct io_obj_op {
	void (* set)(unsigned int, bool);
	bool (* get)(unsigned int);
};

struct io_obj_def {
	unsigned int id;
	const struct io_obj_op * op;
};

extern const struct io_obj_op led_obj_op;

extern const struct io_obj_def io_obj_tab[];

#define RATE_NAME_MAX 14

struct rate_def {
	char name[RATE_NAME_MAX + 1];
	uint8_t length;
	uint16_t pattern[];
};

struct gpio_pin {
	struct stm32_gpio * gpio;
	int pin;
};

/* Standard rate ids */
#define RATE_OFF        0
#define RATE_ON         1
#define RATE_20BPM      2
#define RATE_120BPM     3
#define RATE_TEMPORAL3  4
#define RATE_PULSE1     5
#define RATE_STD_MAX    6

#define LED1A           0
#define LED1B           1
#define LED1C           2
#define LED1D           3

#define LED2A           4
#define LED2B           5
#define LED2C           6
#define LED2D           7

#define LED3A           8
#define LED3B           9
#define LED3C          10
#define LED3D          11

#define LED4A          12
#define LED4B          13
#define LED4C          14
#define LED4D          15


#ifdef __cplusplus
extern "C" {
#endif

void iodrv_init(void);

void io_set_rate(unsigned int id, unsigned int rate);

#ifdef __cplusplus
}
#endif

#endif /* __IO_H__ */

