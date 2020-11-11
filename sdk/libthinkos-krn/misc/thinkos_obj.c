/* 
 * thinkos_obj_lut.c
 *
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>


const char thinkos_type_name_lut[][6] = {
	[THINKOS_OBJ_READY]     = "Ready",
	[THINKOS_OBJ_TMSHARE]   = "Sched",
	[THINKOS_OBJ_CLOCK]     = "Clock",
	[THINKOS_OBJ_MUTEX]     = "Mutex",
	[THINKOS_OBJ_COND]      = "Cond",
	[THINKOS_OBJ_SEMAPHORE] = "Sema",
	[THINKOS_OBJ_EVENT]     = "EvSet",
	[THINKOS_OBJ_FLAG]      = "Flag",
	[THINKOS_OBJ_GATE]      = "Gate",
	[THINKOS_OBJ_JOIN]      = "Join",
	[THINKOS_OBJ_CONWRITE]  = "ConWr",
	[THINKOS_OBJ_CONREAD]   = "ConRd",
	[THINKOS_OBJ_PAUSED]    = "Pausd",
	[THINKOS_OBJ_CANCELED]  = "Cancl",
	[THINKOS_OBJ_COMMSEND]  = "ComTx",
	[THINKOS_OBJ_COMMRECV]  = "ComRx",
	[THINKOS_OBJ_IRQ]       = "Irq",
	[THINKOS_OBJ_DMA]       = "Dma",
	[THINKOS_OBJ_FLASH_MEM] = "Flash",
	[THINKOS_OBJ_FAULT]     = "Fault",
	[THINKOS_OBJ_INVALID]   = "Inval"
};

const char thinkos_type_prefix_lut[] = {
	[THINKOS_OBJ_READY]     = '*',
	[THINKOS_OBJ_TMSHARE]   = 'T',
	[THINKOS_OBJ_CLOCK]     = 'C',
	[THINKOS_OBJ_MUTEX]     = 'M',
	[THINKOS_OBJ_COND]      = 'N',
	[THINKOS_OBJ_SEMAPHORE] = 'S',
	[THINKOS_OBJ_EVENT]     = 'E',
	[THINKOS_OBJ_FLAG]      = 'F',
	[THINKOS_OBJ_GATE]      = 'G',
	[THINKOS_OBJ_JOIN]      = 'J',
	[THINKOS_OBJ_CONWRITE]  = 'W',
	[THINKOS_OBJ_CONREAD]   = 'R',
	[THINKOS_OBJ_PAUSED]    = 'P',
	[THINKOS_OBJ_CANCELED]  = '-',
	[THINKOS_OBJ_COMMSEND]  = '>',
	[THINKOS_OBJ_COMMRECV]  = '<',
	[THINKOS_OBJ_IRQ]       = 'I',
	[THINKOS_OBJ_DMA]       = 'D',
	[THINKOS_OBJ_FLASH_MEM] = 'A',
	[THINKOS_OBJ_FAULT]     = '!',
	[THINKOS_OBJ_INVALID]   = '?'
};

