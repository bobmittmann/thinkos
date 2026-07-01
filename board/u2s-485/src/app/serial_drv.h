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
 * @file packet.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SERIAL_DRV_H__ 
#define __SERIAL_DRV_H__ 

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

struct serial_drv;

#ifdef __cplusplus
extern "C" {
#endif

int __serial_read(struct serial_drv * drv, void * buf, 
				unsigned int len, unsigned int tmo);

int __serial_write(struct serial_drv * drv, const void * buf, 
				 unsigned int len);
int __serial_drain(struct serial_drv * drv);

int __serial_ioctl(struct serial_drv * drv, int opt, 
				   uintptr_t arg1, uintptr_t arg2);

int __serial_close(struct serial_drv * drv);

struct serial_drv * serial2_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_DRV_H__ */

