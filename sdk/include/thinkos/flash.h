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

#define __THINKOS_MEMORY__
#include <thinkos/memory.h>

struct thinkos_flash_memory {
	const struct thinkos_mem_sct * sect;
};

struct thinkos_flash_op {
	const struct thinkos_mem_sct * sect;
	int (* write)(const struct thinkos_flash_memory * mem,
				  const void * buf, size_t count);
	int (* read)(const struct thinkos_flash_memory * mem, 
				 void * buf, size_t count, unsigned int tmo);
	int (* seek)(const struct thinkos_flash_memory * mem,
				 off_t offs);
	int (* erase)(const struct thinkos_flash_memory * mem,
				 off_t offs, size_t count);
	int (* lock)(const struct thinkos_flash_memory * mem,
				 off_t offs, size_t count);
	int (* unlock)(const struct thinkos_flash_memory * mem,
				 off_t offs, size_t count);
	int (* open)(const struct thinkos_flash_memory * mem);
	int (* close)(const struct thinkos_flash_memory * mem);
};

struct thinkos_flash_dev {
	struct thinkos_flash_memory * mem;
	const struct thinkos_flash_op * op;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_FLASH_H__ */

