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
 * @file monitor_flash.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include "thinkos_mon-i.h"

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <sys/delay.h>
#include <vt100.h>
#include <xmodem.h>

#include "board.h"
#include "version.h"

#include <sys/dcclog.h>

#if !(THINKOS_ENABLE_CONSOLE_READ)
#error "need THINKOS_ENABLE_CONSOLE_READ"
#endif

#if !(THINKOS_ENABLE_TIMED_CALLS)
#error "need THINKOS_ENABLE_TIMED_CALLS"
#endif

static const struct comm_dev console_comm_dev = {
	.arg = NULL,
	.op = {
		.send = krn_console_dev_send,
		.recv = krn_console_dev_recv
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


/* Erase a flash partition */
static int __flash_erase(int key)
{
	uint32_t offs = 0;
	int ret;

	krn_console_puts("\r\nErasing... ");
	if ((ret = thinkos_flash_mem_erase(key, offs, 256*1024)) < 0) {
		krn_console_puts("failed!\r\n");
	} else {
		krn_console_puts("Ok.\r\n");
	}

	return ret;
}

/* Erase a flash partition */
int __flash_erase_partition(const char * tag)
{
	int key;
	int ret;

	krn_console_puts("\r\nErase partition \"");
	krn_console_puts(tag);
	krn_console_puts("\" [y]? ");
	if (krn_console_getc(10000) != 'y') {
		krn_console_puts("\r\n");
		return -1;
	}

	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		krn_console_puts("\r\nError: can't open partition!\r\n");
		return key;
	}

	ret = __flash_erase(key);

	thinkos_flash_mem_close(key);

	return ret;
}

/* Receive a file and write it into the flash using the YMODEM protocol */
int __flash_ymodem_recv(const char * tag)
{
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t offs = 0;
	uint8_t buf[1024];
	char * fname;
	int ret;
	int key;

	krn_console_puts("\r\nUpload \"");
	krn_console_puts(tag);
	krn_console_puts("\" [y]? ");
	if (krn_console_getc(10000) != 'y') {
		krn_console_puts("\r\n");
		return -1;
	}

	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		krn_console_puts("\r\nError: can't open partition!\r\n");
		return key;
	}

	krn_console_puts("\r\nYMODEM receive ");
	krn_console_puts("(Ctrl+X to cancel)... ");

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);

	thinkos_console_raw_mode(true);

	fname = (char *)buf;
	if ((ret = ymodem_rcv_start(&ry, fname, &fsize)) >= 0) {

		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			int cnt = ret;
			DCC_LOG1(LOG_TRACE, "cnt=%d", cnt);
			if ((ret = thinkos_flash_mem_write(key, offs, buf, cnt)) < 0) {
				break;
			}
			offs += cnt;
		}
	}

	thinkos_console_raw_mode(false);

	thinkos_flash_mem_close(key);


	thinkos_sleep(500);

	krn_console_puts("\r\nYMODEM receive ");
	if (ret < 0)
		krn_console_puts("failed!\r\n");
	else
		krn_console_puts("Ok.\r\n");

	return ret;
}

int __flash_ymodem_rcv_task(const char * tag)
{
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t offs = 0;
	uint8_t buf[1024];
	char * fname;
	int ret;
	int key;

	krn_console_puts("\r\nUpload \"");
	krn_console_puts(tag);
	krn_console_puts("\" [y]? ");
	if (krn_console_getc(10000) != 'y') {
		krn_console_puts("\r\n");
		return -1;
	}

	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		DCC_LOGSTR(LOG_ERROR, "thinkos_flash_mem_open('%s') fail.", tag);
		return key;
	}

	if ((ret = __flash_erase(key)) < 0) {
		return ret;
	}

	krn_console_puts("\r\nYMODEM receive (Ctrl+X to cancel)... ");

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

	thinkos_sleep(500);

	if (ret < 0)
		krn_console_puts("\r\nFailed!\r\n");
	else
		krn_console_puts("\r\nOk.\r\n");

	return ret;
}


int console_flash_ry_task(const char * tag);

int monitor_flash_ymodem_recv(const struct monitor_comm * comm, 
							 const char * tag)
{
	return monitor_thread_exec(comm, C_TASK(__flash_ymodem_rcv_task), 
							   C_ARG(tag));

}

#if 0
int console_flash_erase_all_task(const char * tag);

int monitor_flash_erase_all(const struct monitor_comm * comm, 
						const char * tag)
{
	return monitor_thread_exec(comm, C_TASK(__flash_erase_partition),
							   C_ARG(tag));
}

int __shell_cmd_rcv(char * arg) 
{
	char buf[64];
	char * cp;
	int cmd;

	do {
		int i;
		buf[0] = '\0';
		if (krn_console_gets(buf, sizeof(buf)) <= 0) {
			return -1;
		}

		krn_console_puts(buf);
		for (cp = buf; (*cp != '\0') && ((*cp == ' ') || (*cp == '\t')); cp++);
		if ((cmd = *cp) == 0)
			continue;

		for (++cp; (*cp != '\0') && ((*cp == ' ') || (*cp == '\t')); cp++);

		i = 0;
		for (; (*cp != '\0') && !((*cp == ' ') || (*cp == '\t')); cp++) {
			arg[i++] = *cp;
		}
		arg[i] = '\0';
		break;
	} while (1);

	return cmd;
}

int krn_console_shell(const char * msg, const char * prompt)
{
	char arg[64];
	int cmd;

	while (thinkos_console_is_connected() <= 0) {
		thinkos_sleep(100);
	}

	krn_console_puts("\r\n");
	if (msg != NULL) {
		krn_console_puts(prompt);
	}

	do {
		krn_console_puts("\r\n");
		krn_console_puts(prompt);
		cmd = __shell_cmd_rcv(arg);
		if (thinkos_console_is_connected() <= 0)
			return -1;

		switch (cmd) {
		case 'e':
			__flash_erase_partition(arg);
			break;

		case 'y':
			__flash_ymodem_recv(arg);
			break;

		case 'q':
			break;

		default:
			krn_console_putc(cmd);
		}
	} while (cmd != 'q');

	krn_console_puts("\r\nBye\r\n");
	return 0;
}

int __console_shell_task(void * parm)
{
	return __console_shell(NULL, "monitor> ");
}

int monitor_console_shell(const struct monitor_comm * comm)
{
	return monitor_thread_exec(comm, C_TASK(__console_shell_task), 
							   C_ARG(NULL));
}

#endif

