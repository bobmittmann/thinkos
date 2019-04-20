/* 
 * thikos/version.h
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

#ifndef __THINKOS_VERSION_H__
#define __THINKOS_VERSION_H__

#ifndef __THINKOS_VERSION__
#error "Never use <thinkos/idle.h> directly; include <thinkos.h> instead."
#endif 

/* ----------------------------------------------------------------------------
 * Helpers 
 * ----------------------------------------------------------------------------
 */
#define THINKOS_MKVER(MAJOR, MINOR) #MAJOR "." #MINOR

/* ----------------------------------------------------------------------------
 * Kernel Version
 * ----------------------------------------------------------------------------
 */

#define THINKOS_KERNEL_VERSION_MAJOR 0
#define THINKOS_KERNEL_VERSION_MINOR 1


/* -------------------------------------------------------------------------- 
 * Profile version 
 * --------------------------------------------------------------------------*/

#define THINKOS_PROFILE_VERSION 1

/* ----------------------------------------------------------------------------
 * Version Strings
 * ----------------------------------------------------------------------------
 */

#define THINKOS_KERNEL_VERSION THINKOS_MKVER(THINKOS_KERNEL_VERSION_MAJOR\
											 THINKOS_VERSION_VERSION_MINOR)

#define THINKOS_VERSION_NAME "ThinkOS " THINKOS_KERNEL_VERSION


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_VERSION_H__ */

