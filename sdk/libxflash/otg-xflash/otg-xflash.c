/* 
 * File:	 xflash.c
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

#include <stdint.h>
#include <stdbool.h>
#include <arch/cortex-m3.h>
#include "xflash.h"
#if 0
int __attribute__((section (".init"), naked, noreturn)) 
	usb_xflash(uint32_t blk_offs, 
			   unsigned int blk_size, 
			   const struct magic * magic,
			   unsigned int opt)
{
	asm volatile ("push {lr}\n" 
				  "mov lr, sp\n"
				  "add r12, pc, %0\n" 
				  "mov sp, r12\n" 
				  "push {lr}\n" 
				  : : "i" (3072));

	yflash(blk_offs, blk_size, magic, opt);

	asm volatile ("ldr sp, [sp]\n"
				  "pop {pc}\n" : : );
}
#endif

int __attribute__((section (".init"), naked, noreturn)) 
	usb_xflash(uint32_t blk_offs, 
			   unsigned int blk_size, 
			   const struct magic * magic)
{
	/* This applet should run in RAM. 
	   Initialize the stack just above the program code. */

	asm volatile ("add r12, pc, %0\n" 
				  "bic r12, r12, #0x03\n" 
				  "mov sp, r12\n" 
				  : : "i" (4092));

	yflash(blk_offs, blk_size, magic);
}

