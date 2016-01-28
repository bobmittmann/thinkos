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

#ifndef THINKOS_ENABLE_RESET_RAM_VECTORS
  #ifdef CM3_RAM_VECTORS
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 1
  #else
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 0
  #endif
#endif

#if (THINKOS_ENABLE_RESET_RAM_VECTORS)

/**
 * __reset_ram_vectors:
 *
 * Copy the default values for the IRQ vectors from the flash into RAM. 
 * 
 * When the a new application replaces the existing one through the GDB
 * or Ymodem some interrupts can be fired due to wrong sequencig of
 * interrupt programming in the application. To avoid potential system
 * crashes the vectors should be initialized to a default value.
 *
 */

void __reset_ram_vectors(void)
{
	/* XXX: 
	   this function assumes the excpetion vectors defaults to be located 
	   just after the .text section! */
	extern unsigned int __text_end;
	extern unsigned int __ram_vectors;
	extern unsigned int __sizeof_ram_vectors;

	unsigned int size = __sizeof_ram_vectors;
	void * src = &__text_end;
	void * dst = &__ram_vectors;

	DCC_LOG3(LOG_TRACE, "dst=%08x src=%08x size=%d", dst, src, size); 
	__thinkos_memcpy32(dst, src, size); 
}
#endif

void __dmon_irq_disable_all(void);

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

#if 0
/*
XXX: I'm not quite sure if this steps are really necessary as 
	__thinkos_reset() will kill all threads. 
	
	TOTO: investigate possible side efects with the the current thread,
		not being flushed before __thinkos_reset(); 
*/
		 
	DCC_LOG(LOG_TRACE, "2. kill all threads...");
	__thinkos_kill_all(); 

	DCC_LOG(LOG_TRACE, "3. wait idle..."); 
	dmon_wait_idle();
#endif

	DCC_LOG(LOG_TRACE, "4. ThinkOS reset...");
	__thinkos_reset();

#if THINKOS_ENABLE_CONSOLE
	DCC_LOG(LOG_TRACE, "5. console reset...");
	__console_reset();
#endif

#if (THINKOS_ENABLE_EXCEPTIONS)
	DCC_LOG(LOG_TRACE, "6. exception reset...");
	__exception_reset();
#endif

#if (THINKOS_ENABLE_RESET_RAM_VECTORS)
	DCC_LOG(LOG_TRACE, "7. clear all breakpoints...");
	dmon_breakpoint_clear_all();
#endif

#ifdef CM3_RAM_VECTORS
	DCC_LOG(LOG_TRACE, "8. reset RAM vectors...");
	__reset_ram_vectors();
#endif

	DCC_LOG(LOG_TRACE, "9. reset this board...");
	this_board.softreset();

	DCC_LOG(LOG_TRACE, "10. done.");
}

