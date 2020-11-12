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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <sys/dcclog.h>


#ifndef BOOT_ENABLE_CUSTOM_COMM
#define BOOT_ENABLE_CUSTOM_COMM 0
#endif

#ifndef BOOT_SELFTEST_ENABLE
#define BOOT_SELFTEST_ENABLE    0
#endif

#ifndef BOOT_PREBOOT_ENABLE
#define BOOT_PREBOOT_ENABLE 0
#endif

#ifndef BOOT_MONITOR_ENABLE
#define BOOT_MONITOR_ENABLE 1
#endif


static bool magic_match(const struct magic_blk * magic, uintptr_t addr)
{
	uint32_t * mem = (uint32_t *)addr;
	int k;
	int j;

	k = magic->hdr.pos;
	for (j = 0; j < magic->hdr.cnt; ++j) {
		if ((mem[k++] & magic->rec[j].mask) != magic->rec[j].comp)
			return false;
	}	

	return true;
}

void __attribute__((noreturn)) app_task(const struct thinkos_board * board) 
{
	const struct monitor_app_desc * desc = &board->application;
	uintptr_t app_entry = (uintptr_t)desc->start_addr;

	if (!magic_match(desc->magic, app_entry)) {
		DCC_LOG(LOG_WARNING, "Invalid ThinkOS app.");
	} else {
		int (* app_main)(int argc, const char * argv[]);
		const char * app_argc[1];
		int ret;

		app_entry |= 0x00000001; /* thumb call */
		app_main = (int (*)(int, const char **))app_entry;
		app_argc[0] = board->application.tag;

#if DEBUG
		udelay(32768);
#endif
		DCC_LOG(LOG_TRACE, "Running ThinkOS App...");
		ret = app_main(1, app_argc);
		(void)ret;
		DCC_LOG1(LOG_WARNING, "thinkos_app() returned with code %d.", ret);
	}

	board->default_task((void *)board);
	for(;;);
}

void __attribute__((noreturn)) thinkos_boot(const struct thinkos_board * board,
	void (monitor)(const struct monitor_comm *, void *))
{
#if (BOOT_MONITOR_ENABLE)
	const struct monitor_comm * comm;
#endif
#if (BOOT_SELFTEST_ENABLE) || (BOOT_PREBOOT_ENABLE)
	int ret;
#endif

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	DCC_LOG(LOG_TRACE, "1. thinkos_krn_udelay_calibrate().");
	thinkos_krn_udelay_calibrate();

	DCC_LOG1(LOG_TRACE, "udelay_factor=%d.", udelay_factor);
#if DEBUG
	udelay(0x8000);
#endif

	DCC_LOG(LOG_TRACE, "2. thinkos_krn_init().");
#if DEBUG
	udelay(0x8000);
#endif
	thinkos_krn_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED |
					 THINKOS_OPT_STACK_SIZE(32768), NULL, NULL);

	DCC_LOG(LOG_TRACE, "3. board_init().");
#if DEBUG
	udelay(0x8000);
#endif
	board->init();

	DCC_LOG(LOG_TRACE, "5. board.monitor_comm_init()");
#if DEBUG
	udelay(0x8000);
#endif
#if (BOOT_MONITOR_ENABLE)
	comm = board->monitor_comm_init();
#endif

#if (BOOT_MONITOR_ENABLE)
	DCC_LOG(LOG_TRACE, "8. thinkos_monitor()");
#if DEBUG
	udelay(0x8000);
#endif
	thinkos_krn_monitor_init(comm, monitor, (void *)board);
#endif

#if (BOOT_SELFTEST_ENABLE)
	/* This callback is used as an environment self check. */
	DCC_LOG(LOG_TRACE, "board->selftest_task()");
	if ((ret = board->selftest_task(arg)) < 0) {
		DCC_LOG(LOG_TRACE, "/!\\ self test failed!!!");
		thinkos_thread_abort(ret);
	}
#endif
#if (BOOT_PREBOOT_ENABLE)
	/* This callback is used as a validation.
	   - prevent the application to automatically run. Ex:
	     - using a switch in the board
		 - receiving a break on the serial line */
	if ((ret = board->board_preboot_task(arg)) < 0) {
		DCC_LOG(LOG_TRACE, "board_preboot_task() failed!");
		thinkos_thread_abort(ret);
	}
#endif

#if DEBUG
	udelay(0x8000);
#endif
#if (BOOT_MONITOR_ENABLE)
	DCC_LOG(LOG_TRACE, "9. thinkos_thread_abort()");
	thinkos_thread_abort(0);
//	thinkos_abort();
#else
	DCC_LOG(LOG_TRACE, "10. app_task()");
	app_task(board);
#endif
}


