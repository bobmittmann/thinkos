/* 
 * mon_shced_dbg.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_MONITOR__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#include <thinkos.h>
#include <vt100.h>
 
#if (THINKOS_ENABLE_SCHED_DEBUG)
  #ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_TRACE
  #elif LOG_LEVEL < LOG_TRACE
    #undef LOG_LEVEL 
    #define LOG_LEVEL LOG_TRACE
  #endif
#endif

#include <sys/dcclog.h>

#if (THINKOS_ENABLE_SCHED_DEBUG)
void MON(struct thinkos_monitor * mon, uintptr_t ctl, 
		 uint32_t msp, uint32_t psp)
{

	if (ctl == 0) {
		DCC_LOG2(LOG_TRACE, VT_PSH VT_REV VT_FRD  
				 "BUSY MSP=%08x PSP=%08x" VT_POP, msp, psp);
	} else if (ctl == 1) {
		DCC_LOG3(LOG_TRACE, VT_PSH VT_REV VT_FYW
				 "SLEEP CTX=%08x MSP=%08x PSP=%08x" VT_POP, 
				 mon, msp, psp);
	} else {
		DCC_LOG3(LOG_TRACE, VT_PSH VT_FCY VT_REV 
				 "WKEUP CTX=%08x MSP=%08x PSP=%08x" VT_POP, 
				 mon, msp, psp);
	}
}

struct thinkos_monitor * monitor_sched_dbg(struct thinkos_monitor * mon, 
										   uintptr_t ctl, uint32_t msp,
										   uint32_t psp)
{

	MON(mon, ctl, msp, psp);

	return mon;
}
#endif
