#include "vt-i.h"
#include <sys/tty.h>
#include <stdlib.h>

#include "vt-i.h"
#include <sys/tty.h>
#include <stdlib.h>

#define VT_BS      '\x8'
#define VT_TN_BS     0x7F /* TELNET back space */
#define VT_EOL      '\r'
#define VT_SKIP     '\3'
#define VT_EOF      '\x1A'
#define VT_ESC      '\033'

char * vt_readline(char * buf, unsigned int max, unsigned int tmo)
{
	unsigned int pos;
	unsigned int len;
	unsigned int i;
	int c;
	char * s;

	if (max < 2)
		return NULL;

	/* Reserve space for NULL termination */
	max--;

	s = buf;
	if ((len = strlen(s)) > 0) {
		if (len > max) 
			len = max;

		for (i = 0; i < len; i++)
			vt_putc(s[i]);
		s[i] = '\0';
	}
	pos = len;

	for (;;) {
		if ((c = vt_getkey(tmo)) < 0) {
//			return NULL;
			return buf;
		}

		switch (c) {
		case VT_CURSOR_RIGHT:
			if (pos < len)
				vt_putc(buf[pos++]);
			else
				vt_puts(OUT_BEL);
			continue;

		case VT_CURSOR_LEFT:
			if (pos > 0) {
				vt_puts(OUT_CURSOR_LEFT);
				pos--;
			} else {
				vt_puts(OUT_BEL);
			}
			continue;

		case VT_CTRL_CURSOR_RIGHT:
			while (pos < len) {
				vt_putc(buf[pos++]);
				if ((buf[pos - 1] != ' ') && (buf[pos] == ' '))
					break;
			}
			continue;

		case VT_CTRL_CURSOR_LEFT:
			if (pos > 0) {
				do {
					vt_puts(OUT_CURSOR_LEFT);
					pos--;
					if ((buf[pos - 1] == ' ') && (buf[pos] != ' '))
						break;
				} while (pos > 0);
			}
			continue;

		case VT_PAGE_UP:
			continue;

		case VT_PAGE_DOWN:
			continue;

		case VT_INSERT:
			continue;

		case VT_CURSOR_UP:
		case VT_HOME:
			while (pos > 0) {
				vt_puts(OUT_CURSOR_LEFT);
				pos--;
			} 
			continue;

		case VT_CURSOR_DOWN:
		case VT_END:
			while (pos < len) {
				vt_putc(buf[pos++]);
			} 
			continue;

		case VT_BS:
			if (pos == 0) {
				vt_puts(OUT_BEL);
				continue;
			}
			if (len == pos) {
				pos--;
				len--;
				vt_puts(OUT_BS);
				continue;
			}

			vt_puts(OUT_CURSOR_LEFT);
			pos--;

			/* FALLTHROUGH */

		case VT_DELETE:

			if (len == pos) {
				continue;
			}
			len--;

			for (i = pos; i < len; i++) {
				buf[i] = buf[i + 1];
			}
			buf[len] = '\0';
			vt_puts(&buf[pos]);
			vt_putc(' ');
			for (i = len + 1; i > pos; i--)
				vt_puts(OUT_CURSOR_LEFT);
			continue;

		case VT_EOL:
			buf[len] = '\0';
			vt_puts("\r\n");
			return buf;

		case VT_SKIP:
			vt_puts(OUT_SKIP);
			return NULL;
		}

		if (len == max) {
			vt_puts(OUT_BEL);
			continue;
		}

		if (len == pos) {
			vt_putc(c);
			buf[pos] = c;
			pos++;
			len++;
			continue;
		}

		for (i = len; i > pos; i--) {
			buf[i] = buf[i - 1];
		}

		len++;
		buf[pos] = c;
		buf[len] = '\0';

		vt_puts(&buf[pos]);
		pos++;

		for (i = len; i > pos; i--)
			vt_puts(OUT_CURSOR_LEFT);

	}
}
