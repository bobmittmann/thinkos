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
	union {
		char tag[8];
		uint64_t hash;
	};
	uint32_t off; /* offset */
	uint8_t  opt; /* flags */
	uint8_t  siz; /* coded block size */
	uint16_t cnt; /* number of blocks */
};

/* Memory descriptor */
struct thinkos_mem_desc {
	union {
		char tag[8];
		uint64_t hash;
	};
	uint32_t base; /* Base address */
	uint8_t dev;   /* Device Access */
	uint8_t typ;   /* Type */
	uint8_t opt;   /* Flags */
	uint8_t cnt;   /* Number of entries in the list of memory blocks */
	struct thinkos_mem_blk blk[]; /* SORTED list of blocks */
};

struct thinkos_mem_map {
	char tag[7];
	uint8_t cnt; /* Number of entries in the list of memory descriptors */
	const struct thinkos_mem_desc * desc[]; /* Sorted list of descriptors */
};

struct thinkos_rt;

#ifdef __cplusplus
extern "C" {
#endif

void __thinkos_krn_mem_init(struct thinkos_rt * krn, 
                            const struct thinkos_mem_map * map);

/* User read and write memory access check */
bool __thinkos_mem_usr_rw_chk(uintptr_t addr, int32_t size);

/* User read and execute memory access check */
bool __thinkos_mem_usr_rx_chk(uintptr_t addr, int32_t size);

/* User read memory access check */
bool __thinkos_mem_usr_rd_chk(uint32_t addr, int32_t size);


#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_MEMORY_H__ */

