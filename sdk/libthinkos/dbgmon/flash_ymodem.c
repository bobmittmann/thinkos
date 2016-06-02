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
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>
#include <sys/param.h>
#include <sys/stm32f.h>
#include <crc.h>

#if (THINKOS_ENABLE_MONITOR)

static unsigned long dec2int(const char * __s)
{
	unsigned long val = 0;
	char * cp = (char *)__s;
	char c;

	while ((c = *cp) != '\0') {
		if ((c < '0') || (c > '9'))
			break;
		val = val * 10;
		val += c - '0';
		cp++;
	}

	return val;
}


extern uint32_t _stack;

/* Receive a file and write it into the flash using the YMODEM preotocol */
int dmon_ymodem_flash(struct dmon_comm * comm,
					  uint32_t addr, unsigned int size)
{
	/* FIXME: generalize the application load by removing the low
	   level flash calls dependency */
#ifdef STM32_FLASH_MEM
	/* The YMODEM state machine is allocated at the top of 
	   the stack, make sure there is no app running before 
	   calling the dmon_ymodem_flash()! */
	struct ymodem_rcv * ry = ((struct ymodem_rcv *)&_stack) - 1;
	uint32_t base = (uint32_t)STM32_FLASH_MEM;
	uint32_t offs = addr - base;
	int ret;

	DCC_LOG2(LOG_INFO, "sp=%p ry=%p", cm3_sp_get(), ry);
	DCC_LOG2(LOG_INFO, "offs=0x%08x size=%d", offs, size);
	dmon_ymodem_rcv_init(ry, true, false);
	ry->fsize = size;

	DCC_LOG(LOG_INFO, "Starting...");
	while ((ret = dmon_ymodem_rcv_pkt(comm, ry)) >= 0) {
		if ((ret == 0) && (ry->xmodem) )
			break;
		int len = ret;
		if (ry->pktno == 1) {
			char * cp;
			int fsize;

			cp = (char *)ry->pkt.data;
			DCC_LOGSTR(LOG_INFO, "fname='%s'", cp);
			while (*cp != '\0')
				cp++;
			/* skip null */
			cp++;
			fsize = dec2int(cp);
			if (fsize == 0) {
				ret = 0;
				break;
			}
			DCC_LOG1(LOG_INFO, "fsize='%d'", fsize);
			ry->fsize = fsize;
			DCC_LOG(LOG_INFO, "YMODEM first packet...");
		} else {
			if (ry->pktno == 2) {
				stm32_flash_erase(offs, ry->fsize);
			}	
			stm32_flash_write(offs, ry->pkt.data, len);
			offs += len;
		}
	}

	return ret;
#else
	return -1;
#endif
}

#endif /* THINKOS_ENABLE_MONITOR */

