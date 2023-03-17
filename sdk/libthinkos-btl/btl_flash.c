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
 * @file btl_flash.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include "thinkos_btl-i.h"

#include <xmodem.h>
#include <sys/delay.h>

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

/* Erase a flash partition */
static int __flash_erase(int key, size_t size)
{
	uint32_t offs = 0;
	char s[64];
	int ret;

	krn_snprintf(s, sizeof(s), "Erasing %d bytes... ", size);
	krn_console_puts(s);
	if ((ret = thinkos_flash_mem_erase(key, offs, size)) != size) {
		krn_console_wrln("failed!");
	} else {
		krn_console_wrln("ok.");
	}

	return ret;
}

/* Erase a flash partition */
int btl_flash_erase_partition(const char * tag)
{
	struct thinkos_mem_stat stat;
	size_t size;
	int key;
	int ret;
	int c;

	krn_console_puts("\r\nErase partition \"");
	krn_console_puts(tag);
	krn_console_puts("\" [y]? ");
	if ((c = krn_console_getc(10000)) != 'y') {
		krn_console_putc(c);
		DCC_LOG1(LOG_WARNING, "c=%d", c);
		krn_console_crlf();
		return BTL_SHELL_OK;
	}
	krn_console_crlf();

	if (thinkos_flash_mem_stat(tag, &stat) < 0) {
		return BTL_SHELL_ERR_ARG_INVALID;
	}

	size = stat.end - stat.begin;

	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		return BTL_SHELL_ERR_ARG_INVALID;
	}

	ret = __flash_erase(key, size);

	thinkos_flash_mem_close(key);

	return ret;
}

/* Receive a file and write it into the flash using the YMODEM protocol */
int btl_flash_ymodem_recv(const char * tag)
{
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t offs = 0;
	uint8_t buf[1024];
	char * fname;
	int ret;
	int key;

	krn_console_puts("Upload \"");
	krn_console_puts(tag);
	krn_console_puts("\" [y]? ");
	if (krn_console_getc(10000) != 'y') {
		krn_console_crlf();
		return BTL_SHELL_OK;
	}

	krn_console_crlf();
	if ((key = thinkos_flash_mem_open(tag)) < 0) {
		return BTL_SHELL_ERR_ARG_INVALID;
	}

	krn_console_puts("YMODEM ");
	krn_console_wrln("receive (Ctrl+X to cancel)... ");

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);

	thinkos_console_raw_mode(true);

	fname = (char *)buf;
	if ((ret = ymodem_rcv_start(&ry, fname, &fsize)) >= 0) {
		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			int cnt = ret;
			if ((ret = thinkos_flash_mem_write(key, offs, buf, cnt)) < 0) {
				DCC_LOG1(LOG_ERROR, "thinkos_flash_mem_write()=>%d", ret);
				break;
			}
			offs += cnt;
		}
		DCC_LOG1(LOG_WARNING, "ret=%d", ret);
	} else {
		DCC_LOG1(LOG_WARNING, "err=%d", ret);
	}

	thinkos_console_raw_mode(false);

	thinkos_flash_mem_close(key);

	thinkos_sleep(500);

	krn_console_crlf();
	krn_console_puts("YMODEM ");
	if (ret < 0)
		krn_console_wrln("failed!");
	else
		krn_console_wrln("ok.");

	return ret;
}

int monitor_flash_ymodem_recv(const struct monitor_comm * comm, 
							 const char * tag)
{
	return monitor_thread_exec(comm, C_TASK(btl_flash_ymodem_recv), 
							   C_ARG(tag));

}

int btl_flash_app_exec(const char * tag, uintptr_t arg0, uintptr_t arg1)
{
	struct thinkos_mem_stat stat;
	int ret;

	if ((ret = thinkos_flash_mem_stat(tag, &stat)) < 0) {
		DCC_LOGSTR(LOG_ERROR, "thinkos_flash_mem_stat('%s') fail.", tag);
		return ret;
	}

	return thinkos_app_exec(stat.begin, arg0, arg1, 0, 0);
}

int btl_cmd_exec(struct btl_shell_env * env, int argc, char * argv[])
{
	return btl_flash_app_exec(argv[0], 0, 0);
}


/* Receive a file and write it into the flash using the YMODEM protocol */
int btl_flash_xxd(const char * __tag, uint32_t __offs, uint32_t __max)
{
	int32_t rem = __max;
	uint32_t offs = __offs;
	uint32_t buf[32];
	char ln[128];
	int ret = 0;
	int key;

	if ((key = thinkos_flash_mem_open(__tag)) < 0) {
		return BTL_SHELL_ERR_ARG_INVALID;
	}

	while (rem > 0) {
		int cnt;

		cnt = 16;
		if ((ret = thinkos_flash_mem_read(key, offs, buf, cnt)) <= 0) {
			break;
		}

        cnt = ret;

		krn_fmt_line_hex32(ln, offs, buf, cnt);
		krn_console_wrln(ln);

		offs += cnt;
		rem -= cnt;
	}

	thinkos_flash_mem_close(key);

	return ret;
}

