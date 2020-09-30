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
 * @file yard-ice.c
 * @brief YARD-ICE application main
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "board.h"

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <sys/dcclog.h>

void monitor_task(const struct dbgmon_comm * comm, void * arg);

#ifndef BOOT_MEM_RESERVED 
#define BOOT_MEM_RESERVED 0x1000
#endif

#ifndef BOOT_ENABLE_CUSTOM_COMM
#define BOOT_ENABLE_CUSTOM_COMM 0
#endif

#undef DEBUG

int main(int argc, char ** argv)
{
	const struct dbgmon_comm * comm;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
#if DEBUG
	udelay(8192);
	udelay(8192);
	udelay(8192);
	udelay(8192);
	udelay(8192);
	udelay(8192);
#endif

#ifndef UDELAY_FACTOR 
	DCC_LOG(LOG_TRACE, "1. cm3_udelay_calibrate().");
	cm3_udelay_calibrate();
#endif

	DCC_LOG1(LOG_MSG, "udelay_factor=%d.", udelay_factor);
#if DEBUG
	udelay(256);
#endif

	DCC_LOG(LOG_TRACE, "2. thinkos_krn_init().");
#if DEBUG
	udelay(256);
#endif
	thinkos_krn_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0), NULL, NULL);

	DCC_LOG(LOG_TRACE, "3. board_init().");
#if DEBUG
	udelay(256);
#endif
	this_board.init();

	DCC_LOG(LOG_TRACE, "5. board.dbgmon_comm_init()");
#if DEBUG
	udelay(256);
#endif
	comm = this_board.dbgmon_comm_init();

#if THINKOS_ENABLE_CONSOLE
	DCC_LOG(LOG_TRACE, "5. thinkos_krn_console_init()");
#if DEBUG
	udelay(256);
#endif
	thinkos_krn_console_init();
#endif

#if THINKOS_ENABLE_MPU
	DCC_LOG(LOG_TRACE, "6. thinkos_krn_mpu_init()");
#if DEBUG
	udelay(256);
#endif
	thinkos_krn_mpu_init(0, BOOT_MEM_RESERVED);

	DCC_LOG(LOG_TRACE, "7. thinkos_krn_userland()");
#if DEBUG
	udelay(256);
#endif
	thinkos_krn_userland();
#endif

	DCC_LOG(LOG_TRACE, "8. thinkos_dbgmon()");
#if DEBUG
	udelay(256);
#endif
	thinkos_dbgmon(monitor_task, comm, NULL);

	DCC_LOG(LOG_TRACE, "9. thinkos_thread_abort()");
#if DEBUG
	udelay(256);
#endif
	thinkos_thread_abort(0);

	DCC_LOG(LOG_ERROR, "!!!! Unreachable code reached !!!");

	return 0;
}


