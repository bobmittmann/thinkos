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
 * @file strerr.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/shell.h>

#define __SHELL_I__
#include "shell-i.h"

const char * shell_error_msg[] = {
	[SHELL_OK] = "Ok",
	[-SHELL_ERR_GENERAL] = "General",
	[-SHELL_ERR_CMD_INVALID] = "Command invalid",
	[-SHELL_ERR_ARG_MISSING] = "Argument missing",
	[-SHELL_ERR_ARG_INVALID] = "Argument invalid",
	[-SHELL_ERR_EXTRA_ARGS] = "Extra arguments",
	[-SHELL_ERR_PARSE] = "Parsing Command Line",
	[-SHELL_ERR_LOW_LEVEL] = "Low level",
};

const char * shell_strerror(int errnum)
{
	unsigned int code = -errnum;
	if (code > SHELL_ERR_LOW_LEVEL) {
		return "";
	}

	return shell_error_msg[code]; 
}

