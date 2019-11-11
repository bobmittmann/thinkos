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
 * @file stm32f-serial.c
 * @brief STM32F serial driver
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/serial.h>
#include <sys/file.h>
#include <stdio.h>

/* ----------------------------------------------------------------------
 * Serial file operations 
 * ----------------------------------------------------------------------
 */

int serial_send_op(struct serial_dev * dev, const void * buf,
							  size_t len) {
	return dev->op->send(dev->drv, buf, len);
}

int serial_recv_op(struct serial_dev * dev, void * buf,
							  size_t len, unsigned int msec) {
	return dev->op->recv(dev->drv, buf, len, msec);
}

int serial_drain_op(struct serial_dev * dev) {
	return dev->op->drain(dev->drv);
}

int serial_close_op(struct serial_dev * dev){
	return dev->op->close(dev->drv);
}

const struct fileop serial_fileop = {
	.write = (int (*)(void *, const void *, size_t))serial_send_op,
	.read = (int (*)(void *, void *, size_t, unsigned int))serial_recv_op,
	.flush = (int (*)(void *))serial_drain_op,
	.close = (int (*)(void *))serial_close_op
};

struct file * serial_fopen(struct serial_dev * dev)
{
	return file_alloc(dev, &serial_fileop);
}

bool is_serial(struct file * f) 
{
	return (f->op == &serial_fileop) ? true : false;
}
