/* 
 * File:	 sys/flash_dev.h
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

#ifndef __SYS_MEMORY_H__
#define __SYS_MEMORY_H__

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* ----------------------------------------------------------------------------
 *  Memory Block sizes 
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
#define SZ_4G   32
#define SZ_8G   33
#define SZ_16G  34

/* Memory flags */
#define M_XC  (1 << 7) /* Execution */
#define M_DM  (1 << 6) /* DMA  */
#define M_RW  (0 << 4) /*   */
#define M_RO  (1 << 4) /* Read Only  */


/* Memory block descriptor */
struct blk_desc {
	union {
		char tag[8];
		uint64_t hash;
	};
	uint32_t off;
	uint8_t  opt;
	uint8_t  siz;
	uint16_t cnt;
};

/* Memory block type */
#define T_RAM           0 /* Generic RAM */
#define T_ROM           1 /* ROM */
#define T_OTP           2 /* OTP */
#define T_FLASH         3 /* Flash */
#define T_EEPROM        4 /* EEPROM */
#define T_PERIPHERAL 	5 /* Peripheral */
#define T_CCM           6 /* Core Coupled Memory */

#define M_NV  (1 << 4) /* Non volatile  */
#define M_PE  (1 << 5) /* Peripheral  */
#define M_EX  (1 << 3) /* External  */

#define M_AM  (3 << 0) /* alignment/access mask */
#define M_64  (3 << 0) /* 64 bit alignement  */
#define M_32  (2 << 0) /* 32 bits alignment  */
#define M_16  (1 << 0) /* 16 bits alignment */
#define M_8   (0 << 0) /* 8 bits alignment/access */

/* Memory region/type descriptor */
struct mem_desc {
	union {
		char tag[8];
		uint64_t hash;
	};
	uint32_t base;         /* Base address */
	uint8_t dev;           /* Device Access */
	uint8_t typ;           /* Type */
	uint8_t opt;           /* Flags */
	uint8_t cnt;           /* Number of entries in the block list */
	struct blk_desc blk[]; /* sorted block list */
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SYS_MEMORY_H__ */

