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

/* ----------------------------------------------------------------------------
 *  ThinkOS Memory Block sizes 
 * ----------------------------------------------------------------------------
 */
#define SZ_1     0
#define SZ_2     1
#define SZ_4     2
#define SZ_8     3
#define SZ_16    4
#define SZ_32    5
#define SZ_64    6
#define SZ_128   7
#define SZ_256   8
#define SZ_1K   10
#define SZ_2K   11
#define SZ_4K   12
#define SZ_8K   13
#define SZ_16K  14
#define SZ_32K  15
#define SZ_64K  16
#define SZ_128K 17
#define SZ_256K 18
#define SZ_512K 19
#define SZ_1M   20
#define SZ_2M   21
#define SZ_4M   22
#define SZ_8M   23
#define SZ_16M  24
#define SZ_32M  25
#define SZ_64M  26
#define SZ_128M 27
#define SZ_256M 28
#define SZ_512M 29
#define SZ_1G   30
#define SZ_2G   31

/* Memory flags */
#define M_XC  (1 << 7) /* Execution */
#define M_DM  (1 << 6) /* DMA  */
#define M_PE  (1 << 5) /* Peripheral  */
#define M_NV  (1 << 4) /* Non volatile  */
#define M_EX  (1 << 3) /* External  */
#define M_RW  (0 << 2) /*   */
#define M_RO  (1 << 2) /* Read Only  */
#define M_64  (3 << 0) /* 64 bit alignement  */
#define M_32  (2 << 0) /* 32 bits alignement  */
#define M_16  (1 << 0) /* 16 bits alignment */
#define M_8   (0 << 0) /* 8 bits alignment/access */

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

