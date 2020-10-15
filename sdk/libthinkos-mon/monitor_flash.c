/* 
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

/** 
 * @file console_ymodem_recv.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <sys/delay.h>
#include <sys/dcclog.h>
#include <thinkos.h>
#include <trace.h>
#include <vt100.h>
#include <xmodem.h>
#include <alloca.h>

#include "board.h"
#include "version.h"

#include <thinkos.h>
#include <vt100.h>

#include <sys/dcclog.h>


static int __console_comm_send(void * dev, const void * buf, unsigned int len) 
{
	uint8_t * cp = (uint8_t *)buf;
	unsigned int rem = len;
	int n;

	while (rem) {
		n = thinkos_console_write(cp, rem);
		cp += n;
		rem -= n;
	}

	return len;
}

static int __console_comm_recv(void * dev, void * buf, 
					  unsigned int len, unsigned int msec) 
{
	int ret = 0;

	do {
		ret = thinkos_console_timedread(buf, len, msec);
	} while (ret == 0);

	return ret;
}

static const struct comm_dev console_comm_dev = {
	.arg = NULL,
	.op = {
		.send = __console_comm_send,
		.recv = __console_comm_recv
	}
};

/* Receive a file and discad using the YMODEM protocol */
int __ymodem_rcv_task(uint32_t addr, unsigned int size)
{
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint8_t buf[1024];
	char * fname;
	int ret;

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);

	thinkos_console_raw_mode(true);
	
	fname = (char *)buf;
	if ((ret = ymodem_rcv_start(&ry, fname, &fsize)) >= 0) {

		DCC_LOG(LOG_TRACE, "ymodem_rcv_loop()");

		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			//		int len = ret;
			DCC_LOG1(LOG_TRACE, "len=%d", ret);
		}
	} else {
		DCC_LOG1(LOG_TRACE, "ret=%d", ret);
	}

	thinkos_console_raw_mode(false);

	return ret;
}

/* Receive a file and write it into the flash using the YMODEM protocol */
int __flash_ymodem_rcv_task(const char * tag)
{
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t offs = 0;
	uint8_t buf[1024];
	char * fname;
	int ret;
	int key;

	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		DCC_LOG(LOG_ERROR, "thinkos_flash_mem_open() fail.");
		return key;
	}

	if ((ret = thinkos_flash_mem_erase(key, offs, 256*1024)) < 0) {
		DCC_LOG(LOG_ERROR, "thinkos_flash_mem_erase() fail.");
		thinkos_flash_mem_close(key);
		return ret;
	}

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);
	//thinkos_console_raw_mode(true);

	fname = (char *)buf;
	if ((ret = ymodem_rcv_start(&ry, fname, &fsize)) >= 0) {

		DCC_LOGSTR(LOG_TRACE, "fname='%s'", fname);
		DCC_LOG1(LOG_TRACE, "fsize=%d'", fsize);

		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			int cnt = ret;
			DCC_LOG1(LOG_TRACE, "cnt=%d", cnt);
			if ((ret = thinkos_flash_mem_write(key, offs, buf, cnt)) < 0) {
				break;
			}
			offs += cnt;
		}
	}

	DCC_LOG1(LOG_TRACE, "ret=%d", ret);

	//thinkos_console_raw_mode(false);

	thinkos_flash_mem_close(key);

	return ret;
}

/* Erase a flash partition */
static int __flash_erase_all_task(const char * tag)
{
	uint32_t offs = 0;
	int ret;
	int key;

	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		DCC_LOG(LOG_ERROR, "thinkos_flash_mem_open() fail.");
		return key;
	}

	if ((ret = thinkos_flash_mem_erase(key, offs, 256*1024)) < 0) {
		DCC_LOG(LOG_ERROR, "thinkos_flash_mem_erase() fail.");
	}

	thinkos_flash_mem_close(key);

	return ret;
}


int console_flash_ry_task(const char * tag);

int monitor_flash_ymodem_recv(const struct monitor_comm * comm, 
							 const char * tag)
{
	return monitor_thread_exec(comm, C_TASK(__flash_ymodem_rcv_task), 
							   C_ARG(tag));

}

int console_flash_erase_all_task(const char * tag);

int monitor_flash_erase_all(const struct monitor_comm * comm, 
						const char * tag)
{
	return monitor_thread_exec(comm, C_TASK(__flash_erase_all_task),
							   C_ARG(tag));
}

