/* 
 * board.h
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

/** 
 * @file board.h
 * @brief Board description 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifndef __THINKOS_BOARD_H__
#define __THINKOS_BOARD_H__

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#define __THINKOS_MEMORY__
#include <thinkos/memory.h>

/* Board description */
struct thinkos_board_desc {
	char name[16];
	char desc[38];
	struct {
		char tag[8];
		struct {
			uint8_t minor;
			uint8_t major;
		} ver;
	} hw;

	struct {
		char tag[8];
		struct {
			uint8_t minor;
			uint8_t major;
			uint16_t build;
		} ver;
	} sw;

	struct {
		char tag[8];
		uint32_t id;
	} cpu;

	struct {
		uint8_t cnt;
		union {
			struct {
				const struct mem_desc * flash;
				const struct mem_desc * ram;
				const struct mem_desc * periph;
			};
			const struct mem_desc * lst[3];
		};
	} memory;

	int (* init)(void);
	int (* reset)(void);
	int (* sleep)(void);
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_BOARD_H__ */

