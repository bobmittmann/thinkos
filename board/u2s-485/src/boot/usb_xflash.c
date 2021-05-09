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
 * @file xflash.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <string.h>
#include <stdint.h>
#include <arch/cortex-m3.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>


/* -------------------------------------------------------------------------::
   Firmware update 
   ------------------------------------------------------------------------- */

extern const uint8_t usb_xflash_pic[];
extern const unsigned int sizeof_usb_xflash_pic;
extern uint32_t __data_start[]; 

struct magic {
	struct {
		uint16_t pos;
		uint16_t cnt;
	} hdr;
	struct {
	    uint32_t mask;
		uint32_t comp;
	} rec[];
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
const struct magic firmware_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 10
	},
	.rec = {
		{  0xffffffff, 0x20002800 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },

		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },

		{  0xffffffff, 0x00000000 },
		{  0xffff0000, 0x08000000 }
	}
};
#pragma GCC diagnostic pop

void __attribute__((noreturn)) usb_xflash(uint32_t offs, uint32_t len)
{
	uint32_t * xflash_code = __data_start;
	uintptr_t thumb;
	int (* xflash_ram)(uint32_t, uint32_t, const struct magic *);

	cm3_cpsid_f();

	__thinkos_memcpy(xflash_code, usb_xflash_pic, sizeof_usb_xflash_pic);

	thumb = (uintptr_t)xflash_code + 1;
	xflash_ram = (int (*)(uint32_t, uint32_t, const struct magic *))thumb;

	xflash_ram(offs, len, &firmware_magic);

	thinkos_krn_sysrst();
}

