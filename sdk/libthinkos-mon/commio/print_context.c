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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <thinkos.h>
#include <sys/dcclog.h>

extern const char __xcpt_name_lut[16][12];


void monitor_print_context(const struct monitor_comm * comm, 
						  const struct thinkos_context * ctx, 
						  uint32_t sp)
{
	uint32_t xpsr = ctx->xpsr;
	int ipsr;

	monitor_printf(comm, "   r0=%08x   r4=%08x   r8=%08x  r12=%08x\r\n",
				ctx->r0, ctx->r4, ctx->r8, ctx->r12);
	monitor_printf(comm, "   r1=%08x   r5=%08x   r9=%08x   sp=%08x\r\n", 
				ctx->r1, ctx->r5, ctx->r9, sp);
	monitor_printf(comm, "   r2=%08x   r6=%08x  r10=%08x   lr=%08x\r\n", 
				ctx->r2, ctx->r6, ctx->r10, ctx->lr);
	monitor_printf(comm, "   r3=%08x   r7=%08x  r11=%08x   pc=%08x\r\n",  
				ctx->r3, ctx->r7, ctx->r11, ctx->pc);

	ipsr = xpsr & 0x1ff;
	if (ipsr < 16) { 
		monitor_printf(comm, " xpsr=%08x [N=%c Z=%c C=%c V=%c Q=%c "
				 "ICI/IT=%02x GE=%d IPSR=%d (%s)]\r\n", 
				 xpsr,
				 ((xpsr >> 31) & 0x01) + '0',
				 ((xpsr >> 30) & 0x01) + '0',
				 ((xpsr >> 29) & 0x01) + '0',
				 ((xpsr >> 28) & 0x01) + '0',
				 ((xpsr >> 27) & 0x01) + '0',
				 ((xpsr >> 19) & 0xc0) | ((xpsr >> 10) & 0x3f),
				 ((xpsr >> 16) & 0x0f),
				 ipsr, __xcpt_name_lut[ipsr]);
	} else {
		monitor_printf(comm, " xpsr=%08x [N=%c Z=%c C=%c V=%c Q=%c "
				 "ICI/IT=%02x GE=%d IPSR=%d (IRQ %d) ]\r\n", 
				 xpsr,
				 ((xpsr >> 31) & 0x01) + '0',
				 ((xpsr >> 30) & 0x01) + '0',
				 ((xpsr >> 29) & 0x01) + '0',
				 ((xpsr >> 28) & 0x01) + '0',
				 ((xpsr >> 27) & 0x01) + '0',
				 ((xpsr >> 19) & 0xc0) | ((xpsr >> 10) & 0x3f),
				 ((xpsr >> 16) & 0x0f),
				 ipsr, ipsr - 16);
	}
}


