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
 * @file shell.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#include <sys/delay.h>
#include <sys/dcclog.h>
#include <thinkos.h>
#include <vt100.h>
#include <xmodem.h>
#include <stdio.h>
#include <ctype.h>

#include "board.h"
#include "version.h"
#include "cmd_tab.h"

#include <sys/dcclog.h>

int __flash_ymodem_recv(const char * tag);
int __ymodem_rcv_task(uint32_t addr, unsigned int size);

int boot_cmd_help(int argc, char * argv[])
{
	char s[64];
	int i;

	krn_console_puts("Boot commands:\r\n\r\n");

	if (argc > 2)
		return -1;

	if (argc > 1) {
		if ((i = boot_cmd_lookup(argv[1])) <= 0) {
			krn_console_puts("Not found!\r\n");
			return -1;
		};

		krn_snprintf(s, sizeof(s), "%10s %4s %s\r\n", boot_cmd_sym_tab[i],
					 boot_cmd_alias_tab[i], boot_cmd_brief_tab[i]);
		krn_console_puts(s);

		return 0;
	}

	krn_console_puts("\r\n");
	for (i = BOOT_CMD_FIRST; i <= BOOT_CMD_LAST; ++i) {
		krn_snprintf(s, sizeof(s), "%10s %4s %s\r\n", boot_cmd_sym_tab[i],
					 boot_cmd_alias_tab[i], boot_cmd_brief_tab[i]);
		krn_console_puts(s);
	}

	return 0;
}

void mem_info_print(const struct thinkos_mem_desc * mem)
{
	const char * tag;
	uint32_t base;
	uint32_t size;
	char s[64];
	int align;
	bool ro;
	int i;

	if (mem== NULL)
		return;

	krn_snprintf(s, sizeof(s), "  %s:\r\n", mem->tag);
	for (i = 0; i < mem->cnt; ++i) {
		tag = mem->blk[i].tag;
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->base + mem->blk[i].off;
		ro = (mem->blk[i].opt == M_RO) ? 1 : 0;
		align = ((mem->blk[i].opt & 3) + 1) * 8;
		krn_snprintf(s, sizeof(s), "    %8s %08x-%08x %8d %6s %2d\r\n",
					  tag, base, base + size - 4, size, 
					  ro ? "RO" : "RW", align);
		krn_console_puts(s);
	}

}

extern const struct thinkos_board this_board;

void boot_board_info(const struct thinkos_board * board)
{
	char s[64];
	unsigned int i;

	krn_snprintf(s, sizeof(s), "Board: %s <%s>\r\n"
			   "Hardware: %s rev %d.%d\r\n",
			   board->name, board->desc,
			   board->hw.tag, board->hw.ver.major, board->hw.ver.minor
			  );
	krn_console_puts(s);

	krn_snprintf(s, sizeof(s), "Firmware: %s-%d.%d.%d (%s) " __DATE__ 
				  ", " __TIME__ "\r\n",
				  board->sw.tag, 
				  board->sw.ver.major, 
				  board->sw.ver.minor, 
				  board->sw.ver.build,
#if DEBUG
				  "debug"
#else
				  "release"
#endif
				  ); 
	krn_console_puts(s);

	/* compiler version string */
	krn_console_puts("Compiler: GCC-" __VERSION__ "\r\n");

	/* memory blocks */
	krn_console_puts("\r\nMemory Map:\r\n");
	krn_console_puts("         Tag       Adress span"
				  "     Size  Flags  Align \r\n");

	for (i = 0; i < board->memory->cnt; ++i) {
		mem_info_print(board->memory->desc[i]);
	}
	krn_console_puts("\r\n");
}

int krn_console_puts(const char * s);
int krn_console_putc(int c);
int krn_console_gets(char * s, int size);
int __flash_erase_partition(const char * tag);
int krn_console_ymodem_recv(const char * tag);

int boot_cmd_info(int argc, char ** argv)
{
	boot_board_info(&this_board);

	return 0;
}

static inline int __isspace(int c) {
	return ((c == ' ') || (c == '\t'));
}

int __parseline(char * line, char ** argv, int argmax)
{
	char * cp = line;
	char * tok;
	int n;
	int c;

	for (n = 0; (c = *cp) && (n < argmax); ) {

		/* Remove lead blanks */
		while (__isspace(c)) {
			c = *(++cp);
		}

		/* Quotes: copy verbatim */
		if ((c == '\'') || (c == '\"')) {
			int qt = c;
			tok = ++cp;
			for (; ((c = *cp) != qt); cp++) {
				if (c == '\0') {
					/* parse error, unclosed quotes */
					return -1;
				}
			}
			*cp++ = '\0';
			argv[n++] = tok;
			continue;
		}
	
		tok = cp;

		for (;;) {

			if (c == '\0') {
				if (tok != cp)
					argv[n++] = tok;
				return n;
			}

			if (__isspace(c)) {
				*cp++ = '\0';
				argv[n++] = tok;
				break;
			}

			cp++;
			c = *cp;
		}
	}

	return n;
}


int flash_app_exec(const char * tag)
{
	struct thinkos_mem_stat stat;
	int ret;

	if ((ret = thinkos_flash_mem_stat(tag, &stat)) < 0) {
		DCC_LOGSTR(LOG_ERROR, "thinkos_flash_mem_stat('%s') fail.", tag);
		return ret;
	}

	return thinkos_app_exec(stat.begin);
}

int boot_cmd_exec(int argc, char * argv[])
{
	krn_console_puts("exec\r\n");
	return flash_app_exec(argv[1]);
}

int boot_cmd_app(int argc, char * argv[])
{
	return flash_app_exec("app");
}

int boot_cmd_diag(int argc, char * argv[])
{
	return flash_app_exec("diag");
}

int boot_cmd_version(int argc, char * argv[])
{

	return 0;
}

int boot_cmd_erase(int argc, char * argv[])
{
	krn_console_puts("erase\r\n");
	return __flash_erase_partition(argv[1]);
}

int boot_cmd_rcvy(int argc, char * argv[])
{
	krn_console_puts("ry\r\n");
	return __flash_ymodem_recv(argv[1]);
}

int boot_cmd_xxd(int argc, char * argv[])
{
	krn_console_puts("yxd\r\n");
	__ymodem_rcv_task(0x20000000, 256 * 1024);
	return 0;
}

int boot_cmd_reboot(int argc, char * argv[])
{
	krn_console_puts("\r\nRestarting...\r\n");
	thinkos_sleep(1000);
	thinkos_reboot(THINKOS_CTL_REBOOT_KEY);
	return 0;
}

int boot_cmd_quit(int argc, char * argv[])
{
	krn_console_puts("\r\nBye...\r\n");
	return 0;
}

int boot_cmd_delay(int argc, char * argv[])
{
	krn_console_puts("\r\nSleeping...\r\n");
	return 0;
}

int boot_cmd_echo(int argc, char ** argv)
{
	int i;

	for (i = 1; i < argc; ++i) {
		if (i != 1)
			krn_console_puts(" ");
		krn_console_puts(argv[i]);
	}

	krn_console_puts("\r\n");

	return 0;
}

int boot_console_shell(const char * msg, const char * prompt)
{
	char line[80];
	char * argv[8];
	int argc;
	int cmd;

	while (thinkos_console_is_connected() <= 0) {
		thinkos_sleep(100);
	}

	krn_console_puts("\r\n");
	if (msg != NULL) {
		krn_console_puts(msg);
	}

	for (;;) {
		int ret;

		krn_console_puts("\r\n");
		krn_console_puts(prompt);
		if ((ret = krn_console_gets(line, sizeof(line))) < 0) {
			return ret;
		}

		if (ret == 0) {
			continue;
		}

//		if (thinkos_console_is_connected() <= 0)
//			return -1;

		if ((argc = __parseline(line, argv, 8)) <= 0) {
			continue;
		};

		if ((cmd = boot_cmd_lookup(argv[0])) <= 0) {
			continue;
		};

		boot_cmd_call_tab[cmd](argc, argv); 
	} 

	return 0;
}

