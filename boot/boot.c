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
#define __THINKOS_USB__
#include <thinkos/bootldr.h>

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <sys/dcclog.h>

void monitor_task(const struct dbgmon_comm * comm, void * arg);

#ifndef BOOT_MEM_RESERVED 
#define BOOT_MEM_RESERVED 0x1000
#endif

int main(int argc, char ** argv)
{
	const struct dbgmon_comm * comm;

#if DEBUG
	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
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

	DCC_LOG(LOG_TRACE, "5. usb_comm_init()");
#if DEBUG
	udelay(256);
#endif
#if BOOT_COMM_CUSTOM_ENABLE
	comm = custom_comm_init();
#elif STM32_ENABLE_OTG_FS
	comm = usb_comm_init(&stm32f_otg_fs_dev);
#elif STM32_ENABLE_OTG_HS
	comm = usb_comm_init(&stm32f_otg_hs_dev);
#elif STM32_ENABLE_USB_FS
	comm = usb_comm_init(&stm32f_usb_fs_dev);
#else
    /* Undefined debug monitor comm port! */
	comm = NULL;
#endif

#if THINKOS_ENABLE_CONSOLE
	DCC_LOG(LOG_TRACE, "5. thinkos_console_init()");
#if DEBUG
	udelay(256);
#endif
	thinkos_console_init();
#endif

#if THINKOS_ENABLE_MPU
	DCC_LOG(LOG_TRACE, "6. thinkos_mpu_init()");
#if DEBUG
	udelay(256);
#endif
	thinkos_mpu_init(BOOT_MEM_RESERVED);

	DCC_LOG(LOG_TRACE, "7. thinkos_userland()");
#if DEBUG
	udelay(256);
#endif
	thinkos_userland();
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


