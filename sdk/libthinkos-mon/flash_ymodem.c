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
#include <sys/param.h>
#include <sys/stm32f.h>
#include <crc.h>

/**
 * MONITOR_YMODEM_ENABLE_AUTOERASE - call monitor_flash_erase() to 
 * erase the flash needed for the file. This may cause timeout
 * in the protocol if the block is too large or the memory operation
 * too slow.
 *
 */
#ifndef MONITOR_YMODEM_ENABLE_AUTOERASE
#define MONITOR_YMODEM_ENABLE_AUTOERASE 0
#endif

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

/* Receive a file and write it into the flash using the YMODEM protocol */
int monitor_ymodem_flash(const struct monitor_comm * comm,
						uint32_t addr, unsigned int size)
{
	/* The YMODEM state machine is allocated at the top of 
	   the stack, make sure there is no app running before 
	   calling the monitor_ymodem_flash()! */
	struct ymodem_rcv * ry = ((struct ymodem_rcv *)thinkos_main_stack) - 1;
	int ret;

	DCC_LOG2(LOG_MSG, "sp=%p ry=%p", cm3_sp_get(), ry);
	monitor_ymodem_rcv_init(ry, true, false);
	ry->fsize = size;

	DCC_LOG(LOG_INFO, "Starting...");
	while ((ret = monitor_ymodem_rcv_pkt(comm, ry)) >= 0) {
		int len = ret;
		int i;

		if ((ret == 0) && ((ry->xmodem) || (ry->pktno == 0))) {
			break;
		}

		if (ry->pktno == 1) {
			char * cp;
			int fsize;

			DCC_LOG1(LOG_INFO, "YMODEM pkt=1, len=%d...", len);
			DCC_XXD(LOG_MSG, "YModem pkt", ry->pkt.data, len);

			cp = (char *)ry->pkt.data;
			if (*cp == '\0') {
				DCC_LOG(LOG_WARNING, "file name is empty!");
				ret = 0;
				break;
			}

			DCC_LOGSTR(LOG_INFO, "fname='%s'", cp);
			for (i = 0; (cp[i] != '\0') & (i < len); ++i);
			for (; (cp[i] == '\0') & (i < len); ++i);
			if (i < len) {
				fsize = dec2int(&cp[i]);
				if (fsize == 0) {
					DCC_LOG(LOG_WARNING, "file size is zero!");
					ret = 0;
					break;
				} else {
					DCC_LOG1(LOG_INFO, "fsize='%d'", fsize);
					ry->fsize = fsize;
				}
			} else {
				DCC_LOG(LOG_WARNING, "no file size info!");
			}
			DCC_LOG(LOG_MSG, "YMODEM first packet...");
			/* skip null */
		} else {
			DCC_LOG2(LOG_INFO, "YMODEM pkt=%d, len=%d...", ry->pktno, len);

			if (ry->pktno == 2) {
#if (MONITOR_YMODEM_ENABLE_AUTOERASE)
				DCC_LOG1(LOG_TRACE, "Erasing %d bytes...", ry->fsize);
				monitor_flash_erase(addr, ry->fsize);
#endif

			}	
			DCC_LOG1(LOG_TRACE, "Writing %d bytes...", len);
			monitor_flash_write(addr, ry->pkt.data, len);
			addr += len;
		}
	}

	DCC_LOG(LOG_TRACE, "done.");

	return ret;
}

#endif /* THINKOS_ENABLE_MONITOR */

