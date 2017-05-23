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

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>

#include <sys/dcclog.h>

#include "board.h"

#if 0
#define VERSION_NUM "0.2"
#define VERSION_DATE "Jul, 2015"

const char * const version_str = "ThinkOS Boot Loader " \
							VERSION_NUM " - " VERSION_DATE;
const char * const copyright_str = "(c) Copyright 2015 - Bob Mittmann";
#endif

void monitor_task(struct dmon_comm * comm);

void monitor_exec_protected(struct dmon_comm * comm)
{
	thinkos_dbgmon_init(comm, monitor_task);
}

void monitor_exec(void)
{
	struct dmon_comm * comm;

	comm = usb_comm_getinstance();

	thinkos_escalate((void *)monitor_exec_protected, comm);
}

int app_exec(void)
{
	return thinkos_escalate((void *)dbgmon_app_exec, 
							(void *)&this_board.application);
}

void comm_init(void)
{
#if STM32_ENABLE_OTG_FS
	usb_comm_init(&stm32f_otg_fs_dev);
#elif STM32_ENABLE_OTG_HS
	usb_comm_init(&stm32f_otg_hs_dev);
#elif STM32_ENABLE_USB_DEV
	usb_comm_init(&stm32f_usb_fs_dev);
#else
#error "Undefined debug monitor comm port!"
#endif
	this_board.on_comm_init();
}

#ifndef BOOT_MEM_RESERVED 
#define BOOT_MEM_RESERVED 0x1000
#endif

int main(int argc, char ** argv)
{
	int opt;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

#ifndef UDELAY_FACTOR 
	DCC_LOG(LOG_TRACE, "1. cm3_udelay_calibrate().");
	cm3_udelay_calibrate();
#endif

	DCC_LOG1(LOG_TRACE, "udelay_factor=%d.", udelay_factor);

	DCC_LOG(LOG_TRACE, "2. thinkos_init().");
	thinkos_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0));

#if THINKOS_ENABLE_CONSOLE
	DCC_LOG(LOG_TRACE, "3. thinkos_console_init()");
	thinkos_console_init();
#endif

	DCC_LOG(LOG_TRACE, "4. board_init().");
	opt = this_board.init();

	if (opt & BOOT_OPT_DBGCOMM) {
		DCC_LOG(LOG_TRACE, "5. usb_comm_init()");
		comm_init();
	}

#if THINKOS_ENABLE_MPU
	DCC_LOG(LOG_TRACE, "6. thinkos_mpu_init()");
	thinkos_mpu_init(BOOT_MEM_RESERVED);

	DCC_LOG(LOG_TRACE, "7. thinkos_userland()");
	thinkos_userland();
#endif

	if (opt & BOOT_OPT_MONITOR) {
		DCC_LOG(LOG_TRACE, "8. monitor_exec()");
		monitor_exec();
	}

	if (opt & BOOT_OPT_APPRUN) {
		DCC_LOG(LOG_TRACE, "9. app_exec()");
		app_exec();
	}

#if 0	
		DCC_LOG(LOG_TRACE, "6. board_softreset().");
		this_board.softreset();
#endif

	/* app exec failed, initializes comm and monitor if not yet done. */
	if ((opt & BOOT_OPT_DBGCOMM) == 0) {
		DCC_LOG(LOG_TRACE, "comm_init()");
		thinkos_escalate((void *)comm_init, (void *)NULL);
	}

	if ((opt & BOOT_OPT_MONITOR) == 0) {
		DCC_LOG(LOG_TRACE, "monitor_exec()");
		monitor_exec();
	}

	DCC_LOG(LOG_TRACE, "board.on_error()");
	this_board.on_error(0);

	DCC_LOG(LOG_TRACE, "10. thinkos_thread_abort()");
	thinkos_thread_abort(0);

	DCC_LOG(LOG_ERROR, "!!!! Unreachable code reached !!!");

	return 0;
}

