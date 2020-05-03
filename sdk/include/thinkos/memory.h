/* 
 * File:	 thinkos/memory.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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

#ifndef __THINKOS_MEMORY_H__
#define __THINKOS_MEMORY_H__

#ifndef __THINKOS_MEMORY__
#error "Never use <thinkos/memory.h> directly; include <thinkos/kernel.h> instead."
#endif 

#ifndef THINKOS_MEMORY_STACK_SIZE
#define THINKOS_MEMORY_STACK_SIZE (960 + 16)
#endif

#ifndef THINKOS_MEMORY_ENABLE_RST_VEC
#define THINKOS_MEMORY_ENABLE_RST_VEC CM3_RAM_VECTORS 
#endif

#include <sys/memory.h>

struct thinkos_mem_blk {
	char tag[8];
	uint32_t off; /* offset */
	uint8_t  opt; /* flags */
	uint8_t  siz; /* coded block size */
	uint16_t cnt; /* number of blocks */
};

/* Memory descriptor */
struct thinkos_mem_desc {
	char tag[8]; /* sector tag */
	uint32_t base; /* Base address */
	uint8_t cnt; /* number of blocks in the list */
	struct thinkos_mem_blk blk[]; /* sorted block list */
};

struct thinkos_memory_map {
	const struct thinkos_mem_sct * flash;
	const struct thinkos_mem_sct * ram;
	const struct thinkos_mem_sct * periph;
	const struct thinkos_mem_sct * stack;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_MEMORY_H__ */

