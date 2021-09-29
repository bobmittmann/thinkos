/* 
 * File:	 keyboard.h
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

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdint.h>

#define KBD_EV_KEY_ON      0x01
#define KBD_EV_KEY_OFF     0x02
#define KBD_EV_SWITCH_ON   0x03
#define KBD_EV_SWITCH_OFF  0x04
#define KBD_EV_ENCODER_INC 0x05
#define KBD_EV_ENCODER_DEC 0x06
#define KBD_EV_TIMEOUT     0x80

#define KBD_EVENT_OPC(__EV) (((__EV) >> 8) & 0xff)
#define KBD_EVENT_ARG(__EV) ((__EV) & 0xff)

#define KBD_EVENT_ENCODE(__OPC, __ARG) ((((__OPC) & 0xff) << 8) | \
                                        ((__ARG) & 0xff))

#define KBD_SWITCH(__ID) (128 + (__ID))

struct keyboard;

/*
	DAHDSR (delay, attack, hold, decay, sustain, release)
*/

struct keyboard_cfg {
	uint8_t keymap_cnt;
	struct {
		uint8_t key;
		uint8_t code;
	} keymap[];
};

const struct keyboard_cfg keyboard_xilophone_cfg;
const struct keyboard_cfg keyboard_piano_cfg;

#ifdef __cplusplus
extern "C" {
#endif

int keyboard_init(void);

int keyboard_event_wait(void);

int keyboard_keymap(unsigned int key, unsigned int code);

int keyboard_config(const struct keyboard_cfg * cfg);

int keyboard_timer_set(unsigned int timer_id, uint32_t itv_ms);
int keyboard_timer_enable(unsigned int timer_id);
int keyboard_timer_disable(unsigned int timer_id);

#ifdef __cplusplus
}
#endif
#endif /* __KEYBOARD_H__ */

