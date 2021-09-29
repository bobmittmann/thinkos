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

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#if 0
#if !(THINKOS_ENABLE_CONSOLE_READ)
#error "need THINKOS_ENABLE_CONSOLE_READ"
#endif

#if !(THINKOS_ENABLE_TIMED_CALLS)
#error "need THINKOS_ENABLE_TIMED_CALLS"
#endif
#endif

#include <sys/dcclog.h>

void tp12_on(void);
void tp12_off(void);
void tp13_on(void);
void tp13_off(void);

int krn_console_dev_send(void * dev, const void * buf, unsigned int len) 
{
	uint8_t * cp = (uint8_t *)buf;
	unsigned int rem = len;
	int n;

//	tp12_on();

	while (rem) {
		n = thinkos_console_write(cp, rem);
		cp += n;
		rem -= n;
	}

//	tp12_off();

	return len;
}

int krn_console_write(const void * buf, unsigned int len) 
{
	return krn_console_dev_send(NULL, buf, len);
}

int krn_console_wrln(const char * ln)
{
	int n = 0;

	if (ln) {
		while (ln[n] != '\0')
			n++;
		krn_console_dev_send(NULL, ln, n);
	}
	return krn_console_dev_send(NULL, "\r\n", 2);
}

int krn_console_puts(const char * s)
{
	int n = 0;

	while (s[n] != '\0')
		n++;
	return krn_console_dev_send(NULL, s, n);
}

int krn_console_putc(int c)
{
	uint8_t buf[4];

	buf[0] = c;
	return krn_console_dev_send(NULL, buf, 1);
}


int krn_console_dev_recv(void * dev, void * buf, 
					  unsigned int len, unsigned int msec) 
{
	int ret = 0;

//	tp13_on();
	do {
		ret = thinkos_console_timedread(buf, len, msec);
	} while (ret == 0);
//	tp13_off();

	return ret;
}

int krn_console_getc(unsigned int tmo)
{
	uint8_t buf[4];

	if (krn_console_dev_recv(NULL, buf, 1, tmo)  < 0)
		return -1;

	return buf[0];
}

#define IN_BS      '\x8'
#define IN_DEL      0x7F
#define IN_EOL      '\r'
#define IN_SKIP     '\3'
#define IN_EOF      '\x1A'
#define IN_ESC      '\033'

#define OUT_DEL     "\x8 \x8"
#define OUT_EOL     "\r\n"
#define OUT_SKIP    "^C\r\n"
#define OUT_EOF     "^Z"
#define OUT_BEL     "\7"

int krn_console_gets(char * s, int size)
{
	char buf[1];
	int ret;
	int pos;
	int c;

	/* left room to '\0' */
	size--;
	pos = 0;

	for (;;) {
		if ((ret = krn_console_dev_recv(NULL, buf, sizeof(char), 1000)) <= 0) {
			if (ret >= THINKOS_ETIMEDOUT)
				continue;	
			return ret;
		}

		c = buf[0];

		if (c == IN_EOL) {
			krn_console_puts(OUT_EOL);
			break;
		} else if (c == IN_SKIP) {
			krn_console_puts(OUT_SKIP);
			return -1;
		} else if (c == IN_BS || c == IN_DEL) {
			if (pos == 0) {
				krn_console_puts(OUT_BEL);
			} else {
				pos--;
				krn_console_puts(OUT_DEL);
			}
			continue;
		} else if (c == IN_ESC) {
		} else if (pos == size) {
			krn_console_puts(OUT_BEL);
			continue;
		}

		s[pos++] = c;
		krn_console_dev_send(NULL, buf, sizeof(char));
	}

	s[pos] = '\0';

	return pos;
}


