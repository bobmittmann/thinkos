/* 
 * /thinkos/debug.h
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


#ifndef __THINKOS_DEBUG_H__
#define __THINKOS_DEBUG_H__

#ifndef __THINKOS_DEBUG__
#error "Never use <thinkos/debug.h> on user code, this is for kenel debug."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Exception API 
 * ------------------------------------------------------------------------- */

static inline void __attribute__((always_inline)) __thinkos_dbg_halt(void) 
{
	register uint32_t tst= 1;

	asm volatile ("1:\n" 
				  "cmp %0, #0\n" 
				  "bne 1b\n" : "=r"(tst) : "r"(tst));
}

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_DEBUG_H__ */

