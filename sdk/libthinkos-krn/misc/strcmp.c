/* 
 * thinkos_strcmp.c
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

/** 
 * @file thinkos_strcmp.c
 * @brief ThinkOS kernel
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

/*
 */

int __thinkos_strcmp(const char * __s1, const char * __s2)
{
	int c1;
	int c2;

	do {
		c1 = *__s1++;
		c2 = *__s2++;

		if ((c2 = c1 - c2) != 0)
			return c2;

	} while (c1 != '\0');

	return 0;
}

int strcmp(const char *, const char *)
	__attribute__ ((weak, alias ("__thinkos_strcmp")));

