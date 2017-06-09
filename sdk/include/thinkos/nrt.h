/* 
 * thikos_irq.h
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

#ifndef __THINKOS_NRT_H__
#define __THINKOS_NRT_H__

#ifndef __THINKOS_NRT__
#error "Only use this file on privileged code"
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

/* Non realtime threads scheduler thread */
#define THINKOS_THREAD_NRT_SCHED (THINKOS_THREAD_IDLE - 1)

#ifndef __ASSEMBLER__

enum {
	NRT_THREAD_CREATE = 0x10001
};

#ifdef __cplusplus
extern "C" {
#endif

int __thinkos_nrt_thread_init(unsigned int thread_id);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_NRT_H__ */

