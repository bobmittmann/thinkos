/* 
 * memcpy.c
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

/*
 * This memcpy is ment to be used to copy small blocks of data
 * not aligned to 32bits boundaries.
 * It won't be efficient for large amounts of data but it has very litle
 * overhead so is good for small data sets (<64bytes). 
 */

void __thinkos_memcpy(void * __dst, const void * __src,  unsigned int __n)
{
	register uint8_t * cpsrc = (uint8_t *)__src;
	register uint8_t * cpdst = (uint8_t *)__dst;

	while (__n) {
		*cpdst++ = *cpsrc++;
		__n--;
	}
}

void memcpy(void *, const void *, size_t)
	__attribute__ ((weak, alias ("__thinkos_memcpy")));

