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

#include <sys/delay.h>

#include <sys/dcclog.h>


#ifndef BOOT_ENABLE_CUSTOM_COMM
#define BOOT_ENABLE_CUSTOM_COMM 0
#endif

#ifndef BOOT_SELFTEST_ENABLE
#define BOOT_SELFTEST_ENABLE    0
#endif

#ifndef BOOT_PREBOOT_ENABLE
#define BOOT_PREBOOT_ENABLE 1
#endif

#ifndef BOOT_MONITOR_ENABLE
#define BOOT_MONITOR_ENABLE 1
#endif


void boot_monitor_task(const struct monitor_comm * comm, void * arg);

void __attribute__((noreturn)) thinkos_boot(const struct thinkos_board * board,
	void (monitor)(const struct monitor_comm *, void *))
{
	struct thinkos_rt * krn = &thinkos_rt;
#if (BOOT_MONITOR_ENABLE)
	const struct monitor_comm * comm;
#endif
#if (BOOT_SELFTEST_ENABLE) || (BOOT_PREBOOT_ENABLE)
//	int ret;
#endif

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	DCC_LOG(LOG_TRACE, "1. thinkos_krn_udelay_calibrate().");
	thinkos_krn_udelay_calibrate();

	DCC_LOG1(LOG_TRACE, "udelay_factor=%d.", udelay_factor);
#if DEBUG
//	udelay(0x8000);
#endif

	DCC_LOG(LOG_TRACE, "2. thinkos_krn_init().");
#if DEBUG
//	udelay(0x8000);
#endif
	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED |
					 THINKOS_OPT_STACK_SIZE(32768), NULL);

//	DCC_LOG(LOG_TRACE, "3. board_init().");
#if DEBUG
//	udelay(0x8000);
#endif
//	board_init();

//	DCC_LOG(LOG_TRACE, "5. board.monitor_comm_init()");
#if DEBUG
//	udelay(0x8000);
#endif
#if (BOOT_MONITOR_ENABLE)
//	comm = board->monitor_comm_init();
	comm = usb_comm_init(&stm32f_otg_fs_dev);
#endif

#if (BOOT_MONITOR_ENABLE)
//	DCC_LOG(LOG_TRACE, "8. thinkos_monitor()");
#if DEBUG
//	udelay(0x8000);
#endif

	thinkos_krn_monitor_init(krn, comm, boot_monitor_task, (void *)&board);
#endif

#if (BOOT_PREBOOT_ENABLE)
	/* This callback is used as a validation.
	   - prevent the application to automatically run. Ex:
	     - using a switch in the board
		 - receiving a break on the serial line */
//	if ((ret = board->preboot_task((void *)board)) < 0) {
//		DCC_LOG(LOG_TRACE, "board_preboot_task() failed!");
//		thinkos_abort();
//	}
#endif

#if DEBUG
//	udelay(0x8000);
#endif

//	uintptr_t addr = board->application.start_addr;
//	thinkos_app_exec(addr);
	
//	board->default_task((void*)board);

	for(;;);
}


