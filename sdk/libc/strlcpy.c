/* 
 * Copyright(c) 2004-2012 BORESTE (www.boreste.com). All Rights Reserved.
 *
 * This file is part of the YARD-ICE.
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
 * @file strlcpy.c
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <string.h>

size_t strlcpy(char * dst, const char * src, size_t size)
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = size;

    if (n != 0 && --n != 0) {
            do {
                    if ((*d++ = *s++) == 0)
                            break;
            } while (--n != 0);
    }

    if (n == 0) {
            if (size != 0)
                    *d = '\0';
            while (*s++);
    }

    return (s - src - 1);
}
