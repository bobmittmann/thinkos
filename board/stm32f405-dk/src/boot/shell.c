/* 
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/** 
 * @file shell.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#include <sys/delay.h>
#include <sys/dcclog.h>
#include <thinkos.h>
#include <vt100.h>
#include <xmodem.h>
#include <stdio.h>
#include <ctype.h>

#include "board.h"
#include "version.h"
#include "cmd_tab.h"

#include <sys/dcclog.h>

int btl_cmd_help(struct btl_shell_env * env, int argc, char * argv[])
{
	char s[64];
	int i;

	krn_console_puts("\r\nBoot commands:\r\n");

	if (argc > 2)
		return -1;

	if (argc > 1) {
		if ((i = btl_cmd_lookup(env, argv[1])) <= 0) {
			krn_console_puts("Not found!\r\n");
			return -1;
		};

		krn_snprintf(s, sizeof(s), "%10s %4s %s\r\n", btl_cmd_name(env, i),
					 btl_cmd_alias(env, i), btl_cmd_brief(env, i));
		krn_console_puts(s);

		return 0;
	}

	for (i = btl_cmd_first(env); i <= btl_cmd_last(env); ++i) {
		krn_snprintf(s, sizeof(s), "%10s %4s %s\r\n", btl_cmd_name(env, i),
					 btl_cmd_alias(env, i), btl_cmd_brief(env, i));
		krn_console_puts(s);
	}

	return 0;
}

extern const struct thinkos_board this_board;

int btl_cmd_info(struct btl_shell_env * env, int argc, char * argv[])
{
	btl_board_info(&this_board);

	return 0;
}


int btl_cmd_app(struct btl_shell_env * env, int argc, char * argv[])
{
	return btl_flash_app_exec("app");
}

int btl_cmd_diag(struct btl_shell_env * env, int argc, char * argv[])
{
	return btl_flash_app_exec("diag");
}

int btl_cmd_erase(struct btl_shell_env * env, int argc, char * argv[])
{
	return btl_flash_erase_partition(argv[1]);
}

int btl_cmd_echo(struct btl_shell_env * env, int argc, char * argv[])
{
	int i;

	for (i = 1; i < argc; ++i) {
		if (i != 1)
			krn_console_puts(" ");
		krn_console_puts(argv[i]);
	}

	krn_console_puts("\r\n");

	return 0;
}

int btl_cmd_rcvy(struct btl_shell_env * env, int argc, char * argv[])
{
	return btl_flash_ymodem_recv("app");
}

