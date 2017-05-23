/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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
 * @file sys/stm32f.h
 * @brief YARD-ICE libstm32f
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifndef __SYS_USB_AUDIO_H__
#define __SYS_USB_AUDIO_H__

#include <stdint.h>
#include <stdio.h>
#include <sys/cdc.h>
#include <sys/usb-dev.h>
#include <sys/serial.h>
#include <sys/usb-cdc.h>

#define USB_AUDIO_EINVAL 1
#define USB_AUDIO_EINTR 2
#define USB_AUDIO_ETIMEDOUT 3

struct usbaudio_state {
	union {
		struct {
			uint32_t reserved: 1;
			uint32_t suspended: 1;
		};
		uint32_t flags;
	};
	uint32_t rx_chars;
	uint32_t tx_chars;
	struct serial_config cfg;
    struct serial_control ctrl;
    struct serial_status stat;
    struct serial_error err;
};

typedef struct usbaudio_state usbaudio_state_t;

struct usbaudio_class;

typedef struct usbaudio_class usbaudio_class_t;

extern const uint8_t * const usbaudio_def_str[];
extern const uint8_t usbaudio_def_strcnt;

extern const struct serial_dev usbaudio_serial_dev;

#ifdef __cplusplus
extern "C" {
#endif

usbaudio_class_t * usbaudio_init(const usb_dev_t * usb, 
							   const uint8_t * const str[], 
							   unsigned int strcnt);

int usbaudio_write(usbaudio_class_t * cdc,
				  const void * buf, unsigned int len);

int usbaudio_read(usbaudio_class_t * cdc, void * buf,
				 unsigned int len, unsigned int msec);

void usbaudio_connect(usbaudio_class_t * cdc);

void usbaudio_disconnect(usbaudio_class_t * cdc);

void usbaudio_enumaration_wait(usbaudio_class_t * cdc);

void usbaudio_reset_wait(usbaudio_class_t * cdc);

void usbaudio_ctrl_event_wait(usbaudio_class_t * cdc);

void usbaudio_device_init(usbaudio_class_t * cdc);

int usbaudio_flush(usbaudio_class_t * cdc);

int usbaudio_release(usbaudio_class_t * cdc);

int usbaudio_state_get(usbaudio_class_t * cdc, usbaudio_state_t * state);

int usbaudio_status_set(usbaudio_class_t * cl, struct serial_status * stat);

int usbaudio_ctl_wait(usbaudio_class_t * cdc, unsigned int msec);

void usbaudio_sn_set(uint64_t sn);

void usbaudio_product_set(const char * s);

struct file * usbaudio_fopen(usbaudio_class_t * cdc);

bool usbaudio_is_usb_file(FILE * f);

int usbaudio_lc_wait(usbaudio_class_t * cl);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_USB_AUDIO_H__ */

