/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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
 * @file standby.c
 * @brief ThinkOS monitor example
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/delay.h>

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
/*
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
*/
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>

#include <thinkos.h>
#include <vt100.h>

#include <sys/dcclog.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#ifdef THINKOS_CUSTOM_APP_KEY
#define APP_KEY (uint64_t)(THINKOS_CUSTOM_APP_KEY)
#else
#define THINKOS_CUSTOM_APP_KEY 0x0000000000000000LL
#define APP_KEY (uint64_t)(0x0000000000000000LL)
#endif

const struct magic_blk flat_app_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 8
		},
	.rec = {
		{.mask = 0xff0000ff, .comp = 0x08000041 },
		{.mask = 0x0000000f, .comp = 0x00000000 },
		{.mask = 0x00000000, .comp = 0x00000000 },
		{.mask = 0x00000000, .comp = 0x00000000 },
		{.mask = 0xffffffff, .comp = 0x6e696854 },
		{.mask = 0xffffffff, .comp = 0x00534f6b },
		{.mask = 0xffffffff, .comp = (APP_KEY) },
		{.mask = 0xffffffff, .comp = (APP_KEY) >> 32}
	}
};

static bool magic_match(const struct magic_blk * magic, uint32_t * mem)
{
	int k;
	int j;

	k = magic->hdr.pos;
	for (j = 0; j < magic->hdr.cnt; ++j) {
		if ((mem[k++] & magic->rec[j].mask) != magic->rec[j].comp)
			return false;
	}	

	return true;
}
#pragma GCC diagnostic pop

