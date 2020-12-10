/* 
 * File:	 usb-cdc.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
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

#include "thinkos_mon-i.h"

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

#if THINKOS_ENABLE_APP

/* -------------------------------------------------------------------------
 * Application execution
 * ------------------------------------------------------------------------- */

void __app_exec_on_exit(unsigned int code)
{
	DCC_LOG1(LOG_WARNING, "code=%d", code);
#if 1
	thinkos_thread_abort(code);
#else
	thinkos_abort();
#endif
}


static int __app_exec_task(uintptr_t addr, unsigned int thread)
{
	int ret;

	__console_puts("\r\nStarting application... ");

	DCC_LOG(LOG_TRACE, "thinkos_app_exec()!");

	if ((ret = thinkos_app_exec(addr))) {
		DCC_LOG1(LOG_ERROR, "Can't start app: err=%d!", ret);
	}

	return ret;
}

bool monitor_app_exec(const struct monitor_comm * comm)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_mem_part part;
	uintptr_t addr;
	int thread_id;
	int ret;

	DCC_LOG(LOG_TRACE, "creating a thread to call app_exec()!");

	if (!__krn_mem_part_lookup(krn, "FLASH", "APP", &part)) {
		DCC_LOG(LOG_ERROR, "Can't locate application...!");
		return false;
	}

	DCC_LOG2(LOG_TRACE, "patition: %08x ~ %08x", part.begin, part.end);
	addr = part.begin;

	ret = thinkos_dbg_thread_create(C_TASK(__app_exec_task), C_ARG(addr), 
									__app_exec_on_exit, true);
	thread_id = ret;
	(void)thread_id;

	DCC_LOG1(LOG_TRACE, "monitor_thread_exec() = %d!", ret);

	return (ret < 0) ? false : true;
}

#endif

