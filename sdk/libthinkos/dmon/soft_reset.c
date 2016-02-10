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

#define __THINKOS_DMON__
#include <thinkos_dmon.h>
#include <thinkos.h>
#include <sys/dcclog.h>

void __dmon_irq_disable_all(void);
void __reset_ram_vectors(void);
void __dmon_irq_force_enable(void);

/**
 * dmon_soft_reset:
 *
 * Reinitialize the plataform by reseting all ThinkOS subsystems.
 * 
 */

void dmon_soft_reset(void)
{
	DCC_LOG(LOG_TRACE, "1. disable all interrupts"); 
	__dmon_irq_disable_all();

	DCC_LOG(LOG_TRACE, "2. ThinkOS reset...");
	__thinkos_reset();

#if THINKOS_ENABLE_CONSOLE
	DCC_LOG(LOG_TRACE, "3. console reset...");
	__console_reset();
#endif

#if (THINKOS_ENABLE_EXCEPTIONS)
	DCC_LOG(LOG_TRACE, "4. exception reset...");
	__exception_reset();
#endif

	DCC_LOG(LOG_TRACE, "5. clear all breakpoints...");
	dmon_breakpoint_clear_all();

#if (THINKOS_ENABLE_RESET_RAM_VECTORS)
	DCC_LOG(LOG_TRACE, "6. reset RAM vectors...");
	__reset_ram_vectors();
#endif

	DCC_LOG(LOG_TRACE, "7. reset this board...");
	this_board.softreset();

	DCC_LOG(LOG_TRACE, "8. enablig listed interrupts...");
	__dmon_irq_force_enable();

	DCC_LOG(LOG_TRACE, "9. done.");
}

