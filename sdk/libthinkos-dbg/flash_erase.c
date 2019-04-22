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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>
#include <sys/dcclog.h>
#include <sys/param.h>
#include <sys/stm32f.h>

#if (THINKOS_ENABLE_MONITOR)

bool dmon_app_erase(const struct dbgmon_comm * comm, 
					uint32_t addr, unsigned int size)
{
	/* FIXME: generalize the application load by removing the low
	   level flash calls dependency */
#ifdef STM32_FLASH_MEM
	uint32_t base = (uint32_t)STM32_FLASH_MEM;
	uint32_t offs = addr - base;

	return (stm32_flash_erase(offs, size) < 0) ? false : true;
#else
	return false;
#endif
}

#endif /* THINKOS_ENABLE_MONITOR */

