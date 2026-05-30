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
 * @file thinkos-sysrst.c
 * @brief ThinkOS
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#include "thinkos_krn-i.h"

void __attribute__((noreturn)) thinkos_krn_sysrst(void)
{
	/* request system reset */
	CM3_SCB->aircr =  SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
//	CM3_SCB->aircr =  SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ |
//		SCB_AIRCR_VECTRESET;

	thinkos_krn_halt();
}

void _exit(void) 
{
	thinkos_abort();
}

