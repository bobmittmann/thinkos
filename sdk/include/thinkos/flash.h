/* 
 * File:	 thinkos/flash.h
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

#ifndef __THINKOS_FLASH_H__
#define __THINKOS_FLASH_H__

#ifndef __THINKOS_FLASH__
#error "Never use <thinkos/flash.h> directly; include <thinkos/kernel.h> instead."
#endif 

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>

#ifndef __ASSEMBLER__

#include <sys/memory.h>
#include <sys/flash-dev.h>

struct thinkos_flash_desc {
	const struct thinkos_mem_desc * mem;
	const struct flash_dev * dev;
};

struct flash_op_req {
	union {
		struct {
			uint16_t wq;
			uint8_t arg;
			uint8_t opc;
		};
		int ret;
	};
	union {
		uint32_t offset;
		const char * tag;
	};
	union {
		uint32_t size;
		struct thinkos_mem_part * mp;
	};
	union {
		void * buf;
	};
};

struct thinkos_flash_drv {
	const struct thinkos_mem_desc * mem;
	const struct flash_dev * dev;
	uint8_t ropen;
	uint8_t wopen;
	uint16_t key;
	struct {
		uint32_t offset;
		uint32_t size;
	} partition;
};

struct thinkos_rt;

#ifdef __cplusplus
extern "C" {
#endif

int thinkos_flash_drv_req(struct thinkos_flash_drv * drv, 
						  struct flash_op_req * req);

void thinkos_flash_drv_tasklet(struct thinkos_rt * krn,
                               unsigned int idx, 
                               struct thinkos_flash_drv * drv);

int thinkos_flash_drv_init(unsigned int idx, 
						   const struct thinkos_flash_desc * desc);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_FLASH_H__ */

