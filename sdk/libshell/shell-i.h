/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
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
 * @file shell-i.h
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


/*****************************************************************************
 * libshell internal (private) header file
 *****************************************************************************/

#ifndef __SHELL_I_H__
#define __SHELL_I_H__

#ifndef __SHELL_I__
#error "Never use <shell-i.h> directly; include <shell.h> instead."
#endif

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/shell.h>
#include <sys/tty.h>
#include <errno.h>

#ifndef SHELL_ARG_MAX 
#define SHELL_ARG_MAX 16
#endif

#ifndef SHELL_LINE_MAX
#define SHELL_LINE_MAX 72
#endif

#ifndef SHELL_HISTORY_MAX
#define SHELL_HISTORY_MAX 8
#endif

#ifndef SHELL_ENV_MAX
#define SHELL_ENV_MAX 32
#endif

#define CMD_SHELL_BUF (SIZEOF_CMD_HISTORY) + (SHELL_HISTORY_MAX) * (SHELL_LINE_MAX)

#define SIZEOF_SHELL_BUF (SIZEOF_CMD_HISTORY) + (SHELL_HISTORY_MAX) * (SHELL_LINE_MAX)


struct cmd_history {
	uint8_t pos;
	uint8_t tail;
	uint8_t head;
	uint8_t max;
	uint8_t len;
	char buf[];
};

struct shell_env {
	FILE * fout;
	FILE * fin;
	char buf[];
};


#ifdef __cplusplus
extern "C" {
#endif

char * history_prev(struct cmd_history * ht);

char * history_next(struct cmd_history * ht);

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_I_H__ */

