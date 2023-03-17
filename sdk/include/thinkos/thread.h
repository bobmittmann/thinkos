/* 
 * File:	 thinkos/thread.h
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

#ifndef __THINKOS_THREAD_H__
#define __THINKOS_THREAD_H__

#ifndef __THINKOS_THREAD__
#error "Never use <thinkos/thread.h> directly; include <thinkos/kernel.h> instead."
#endif 

struct thinkos_thread_initializer {
	uintptr_t stack_base;
	uint32_t stack_size;
	union {
		uint32_t opt;
		struct {
			uint8_t priority;
			uint8_t privileged: 1;
			uint8_t paused: 1;
			uint8_t res: 6;
		};
	};
	uintptr_t task_entry;
	uintptr_t task_exit;
	uint32_t task_arg[4];
	const struct thinkos_thread_inf * inf;
};


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_THREAD_H__ */

