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
 * @file btl_cmd_xxd.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#include <thinkos.h>

int btl_cmd_xxd(struct btl_shell_env * env, int argc, char * argv[])
{
	if (argc < 2)
		return BTL_SHELL_ERR_ARG_MISSING;

	if (argc > 2)
		return BTL_SHELL_ERR_EXTRA_ARGS;

	return btl_flash_xxd(argv[1], 0, 256 * 1024);
}

