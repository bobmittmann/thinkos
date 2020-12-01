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

static int __app_exec_task(const struct monitor_app_desc * desc)
{
	uintptr_t addr = (uintptr_t)desc->start_addr;
	int ret;

	if ((ret = thinkos_app_exec(addr))) {
		DCC_LOG1(LOG_ERROR, "Can't start app: err=%d!", ret);
	}

	return ret;
}

bool monitor_app_exec(const struct monitor_comm * comm, 
					 const struct monitor_app_desc * desc)
{
	int thread_id;
	int ret;

	DCC_LOG(LOG_TRACE, "creating a thread to call app_exec()!");

	ret = thinkos_dbg_thread_create(C_TASK(__app_exec_task), C_ARG(desc), true);
	thread_id = ret;
	(void)thread_id;

	DCC_LOG1(LOG_TRACE, "monitor_thread_exec() = %d!", ret);

	return (ret < 0) ? false : true;
}

#endif

