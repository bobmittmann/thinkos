/* 
 * thinkos_mon-i.h
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

#ifndef __THINKOS_BTL_I_H__
#define __THINKOS_BTL_I_H__

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#define __THINKOS_DEBUG__
#include <thinkos/debug.h>

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <arch/cortex-m3.h>
#include <sys/param.h>

#include <sys/dcclog.h>
#include <vt100.h>

struct btl_shell_env {
	const char * motd;
	const char * prompt;
	const btl_cmd_callback_t * cmd_call;
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_BTL_I_H__ */


