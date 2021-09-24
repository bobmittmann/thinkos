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
 * @file btl_cmd_reboot.c
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

int btl_cmd_reboot(int argc, char * argv[])
{
	krn_console_puts("\r\nRestarting...\r\n");
	thinkos_sleep(1000);
	thinkos_reboot(THINKOS_CTL_REBOOT_KEY);
	return 0;
}

