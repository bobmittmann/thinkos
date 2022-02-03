/* 
 * Copyright(C) 2021 Robinson Mittmann. All Rights Reserved.
 *
 * This file is part of the libstm32.
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
 * @file stm32f-krn.c
 * @brief STM32 Kernel 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

void thinkos_arch_version_get(struct thinkos_version * ver)
{
	ver->major = THINKOS_KRN_VERSION_MAJOR;
	ver->minor = THINKOS_KRN_VERSION_MINOR;
	__thinkos_memcpy(ver->mach, THINKOS_KRN_MACH, sizeof(THINKOS_KRN_MACH)); 
}

