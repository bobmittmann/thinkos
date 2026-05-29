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

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

#include <thinkos.h>
#include <sys/dcclog.h>

const char * __retstr(uint32_t __ret);

void monitor_print_fault(const struct monitor_comm * comm, 
							struct thinkos_fault * fault)
{
#if (THINKOS_ENABLE_MPU)
	uint32_t mmfsr;
#endif
#if (THINKOS_ENABLE_BUSFAULT)
	uint32_t bfsr;
#endif
#if (THINKOS_ENABLE_USAGEFAULT)
	uint32_t ufsr;
#endif
	uint32_t sp;
	uint32_t ret;
	uint32_t ctrl;
	uint32_t shcsr;
	int ipsr;

	monitor_printf(comm, " Error %d at ", fault->errno);

	ipsr = fault->ctx.xpsr & 0x1ff;
	if (ipsr == 0) {
		monitor_printf(comm, "thread %d", fault->thread);
	} else if (ipsr > 15) {
		monitor_printf(comm, "IRQ %d", ipsr - 16);
	} else {
		switch (ipsr) {
		case CM3_EXCEPT_SVC:
			monitor_printf(comm, "SVCall, thread %d", fault->thread);
			break;
		case CM3_EXCEPT_DEBUG_MONITOR:
			monitor_printf(comm, "Monitor");
			break;
		case CM3_EXCEPT_PENDSV:
			monitor_printf(comm, "PendSV");
			break;
		case CM3_EXCEPT_SYSTICK:
			monitor_printf(comm, "sysTick");
			break;
		}
	}

	monitor_printf(comm, ": %s\r\n", thinkos_krn_err_tag(fault->errno));

	ret = 0xffffff00 | fault->ret;
	sp = fault->sp;
	ctrl = fault->control;

	monitor_print_context(comm, &fault->ctx, sp, ctrl);
						
	monitor_printf(comm, " ret=%08x [ %s ] SP=%08x\r\n", 
				  ret, __retstr(ret), fault->sp);

	switch (fault->errno) {
#if THINKOS_ENABLE_MPU 
	case THINKOS_ERR_MEM_MANAGE:
		mmfsr = SCB_CFSR_MMFSR_GET(fault->cfsr);
		monitor_printf(comm, "MMFSR=%02x [", mmfsr);
		if (mmfsr & MMFSR_MMARVALID)
			monitor_printf(comm, " MMARVALID");
		if (mmfsr & MMFSR_MLSPERR)
			monitor_printf(comm, " MLSPERR");
		if (mmfsr & MMFSR_MSTKERR)
			monitor_printf(comm, " MSTKERR");
		if (mmfsr & MMFSR_MUNSTKERR)
			monitor_printf(comm, " MUNSTKERR");
		if (mmfsr & MMFSR_DACCVIOL)  
			monitor_printf(comm, " DACCVIOL");
		if (mmfsr & MMFSR_IACCVIOL)  
			monitor_printf(comm, " IACCVIOL");
		monitor_printf(comm, " ]\r\n");
		if (mmfsr & MMFSR_MMARVALID) 
			monitor_printf(comm, " Fault address --> %08x\r\n", fault->mmfar);
		break;
#endif

#if THINKOS_ENABLE_BUSFAULT
	case THINKOS_ERR_BUS_FAULT:
		bfsr = SCB_CFSR_BFSR_GET(fault->cfsr);
		monitor_printf(comm, " BFSR=%02x [", bfsr);
		if (bfsr & BFSR_BFARVALID)  
			monitor_printf(comm, " BFARVALID");
		if (bfsr & BFSR_LSPERR)
			monitor_printf(comm, " LSPERR");
		if (bfsr & BFSR_STKERR)  
			monitor_printf(comm, " STKERR");
		if (bfsr & BFSR_UNSTKERR)  
			monitor_printf(comm, " UNSTKERR");
		if (bfsr & BFSR_IMPRECISERR)  
			monitor_printf(comm, " IMPRECISERR");
		if (bfsr & BFSR_PRECISERR)
			monitor_printf(comm, " PRECISERR");
		if (bfsr & BFSR_IBUSERR)  
			monitor_printf(comm, " IBUSERR");
		monitor_printf(comm, " ]\r\n");
		if (bfsr & BFSR_BFARVALID) 
			monitor_printf(comm, " Fault address --> %08x\r\n", fault->bfar);
		break;
#endif

#if THINKOS_ENABLE_USAGEFAULT 
	case THINKOS_ERR_USAGE_FAULT: 
		ufsr = SCB_CFSR_UFSR_GET(fault->cfsr);
		monitor_printf(comm, " UFSR=%04x [", ufsr);
		if (ufsr & UFSR_DIVBYZERO)  
			monitor_printf(comm, " DIVBYZERO");
		if (ufsr & UFSR_UNALIGNED)  
			monitor_printf(comm, " UNALIGNED");
		if (ufsr & UFSR_NOCP)  
			monitor_printf(comm, " NOCP");
		if (ufsr & UFSR_INVPC)  
			monitor_printf(comm, " INVPC");
		if (ufsr & UFSR_INVSTATE)  
			monitor_printf(comm, " INVSTATE");
		if (ufsr & UFSR_UNDEFINSTR)  
			monitor_printf(comm, " UNDEFINSTR");
		monitor_printf(comm, " ]\r\n");
		break;
#endif
	}

	shcsr = fault->shcsr;
	monitor_printf(comm, "SHCSR=%08x [%s%s%s%s%s%s%s%s%s%s%s ]", 
/*				   (shcsr & SCB_SHCSR_USGFAULTENA) ? " USGFAULTENA" : "",
				   (shcsr & SCB_SHCSR_BUSFAULTENA) ? " BUSFAULTENA " : "",
				   (shcsr & SCB_SHCSR_MEMFAULTENA) ? " MEMFAULTENA " : "", */
				   shcsr,
				   (shcsr & SCB_SHCSR_SVCALLPENDED) ? " SVCALLPEND" : "",
				   (shcsr & SCB_SHCSR_BUSFAULTPENDED) ?  " BUSFAULTPEND" : "",
				   (shcsr & SCB_SHCSR_MEMFAULTPENDED) ?  " MEMFAULTPEND" : "",
				   (shcsr & SCB_SHCSR_USGFAULTPENDED) ?  " USGFAULTPEND" : "",
				   (shcsr & SCB_SHCSR_SYSTICKACT) ? " SYSTICKACT" : "",
				   (shcsr & SCB_SHCSR_PENDSVACT) ? " PENDSVACT" : "",
				   (shcsr & SCB_SHCSR_MONITORACT) ? " MONITORACT" : "",
				   (shcsr & SCB_SHCSR_SVCALLACT) ? " SVCALLACT" : "",
				   (shcsr & SCB_SHCSR_USGFAULTACT) ?  " USGFAULTACT" : "",
				   (shcsr & SCB_SHCSR_BUSFAULTACT) ?  " BUSFAULTACT" : "",
				   (shcsr & SCB_SHCSR_MEMFAULTACT) ?  " MEMFAULTACT" : "");
}


