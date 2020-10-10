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
 * @file sys/types.h
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#define __need_size_t
#include <stddef.h>

typedef long ssize_t;
typedef long clock_t;
typedef long time_t;

typedef int clockid_t;

typedef	unsigned int off_t;

typedef unsigned long useconds_t;
typedef long suseconds_t;

typedef unsigned long dev_t;
typedef unsigned long ino_t;
typedef unsigned short mode_t;
typedef unsigned char nlink_t;
typedef unsigned char uid_t;
typedef unsigned char gid_t;
typedef	int pid_t;


#endif	/* __SYS_TYPES_H__ */

