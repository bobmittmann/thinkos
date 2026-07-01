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
 * @file cmd_debug.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shell.h>
#include <arch/cortex-m3.h>
#include <thinkos.h>

/*--------------------------------------------------------------------------
  Fault generators
  --------------------------------------------------------------------------*/

void read_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		x = *ptr;
		(void)x;
		ptr += 0x10000000 / (2 << 4);
	}
}

void write_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x = 0;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		*ptr = x;
		ptr += 0x10000000 / (2 << 4);
	}
}

void breakpoint_test(void)
{
	__bkpt(22);
}

/*****************************************************************************
 * 
 * Debug helper 
 *****************************************************************************/

int cmd_debug(FILE *f, int argc, char ** argv)
{
	if (argc > 2) {
		return SHELL_ERR_EXTRA_ARGS;
	}
	
	if (argc == 2) {
		argv++;
		argc--;

		if (strcmp(*argv, "err") == 0) {
			thinkos_error(1);
		} else if (strcmp(*argv, "bkp") == 0) {
			breakpoint_test();
		} else if (strcmp(*argv, "rd") == 0) {
			read_fault();
		} else if (strcmp(*argv, "wr") == 0) {
			write_fault();
		} else {
			return SHELL_ERR_ARG_INVALID;
		}
	}

	return 0;
}

