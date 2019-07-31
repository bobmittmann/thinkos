/* freadline.c - buffered input with limited editing capabilities
 * -----------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file freadline.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <sys/tty.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IN_BS      '\x8'
#define IN_TN_BS     0x7F /* TELNET back space */
#define IN_EOL      '\r'
#define IN_SKIP     '\3'
#define IN_EOF      '\x1A'
#define IN_ESC      '\033'

#define MK_IN_KEY(CODE)   (0x2000 + (CODE))
#define IN_CTRL        0x4000

#define IN_CURSOR_UP    MK_IN_KEY(0)
#define IN_CURSOR_DOWN  MK_IN_KEY(1)
#define IN_CURSOR_RIGHT MK_IN_KEY(2)
#define IN_CURSOR_LEFT  MK_IN_KEY(3)
#define IN_PAGE_UP      MK_IN_KEY(5)
#define IN_PAGE_DOWN    MK_IN_KEY(6)
#define IN_INSERT       MK_IN_KEY(7)
#define IN_DELETE       MK_IN_KEY(8)
#define IN_HOME         MK_IN_KEY(9)
#define IN_END          MK_IN_KEY(10)

#define IN_CTRL_CURSOR_UP    IN_CURSOR_UP + IN_CTRL 
#define IN_CTRL_CURSOR_DOWN  IN_CURSOR_DOWN + IN_CTRL   
#define IN_CTRL_CURSOR_RIGHT IN_CURSOR_RIGHT + IN_CTRL    
#define IN_CTRL_CURSOR_LEFT  IN_CURSOR_LEFT + IN_CTRL   
#define IN_CTRL_PAGE_UP      IN_PAGE_UP + IN_CTRL   
#define IN_CTRL_PAGE_DOWN    IN_PAGE_DOWN + IN_CTRL   

#define OUT_CURSOR_LEFT     "\x8"
#define OUT_BS              "\x8 \x8"
#define OUT_SKIP            "^C"
#define OUT_BEL             "\7"

#define MODE_ESC 1
#define MODE_ESC_VAL1 2
#define MODE_ESC_VAL2 3
#define MODE_ESC_O 4

int freadline(FILE *f, char * buf, unsigned int max,
			  unsigned int tmo)
{
	unsigned int pos;
	unsigned int len;
	unsigned int i;
	int mode;
	int val;
	int ctrl;
	int c;
	char * s;

	if (max < 2)
		return -1;

	if (isfatty(f)) {
		f = ftty_lowlevel(f);
	}

	mode = 0;
	val = 0;
	ctrl = 0;

	/* Reserve space for NULL termination */
	max--;

	s = buf;
	if ((len = strlen(s)) > 0) {
		if (len > max) 
			len = max;

		for (i = 0; i < len; i++)
			fputc(s[i], f);
		s[i] = '\0';
	}
	pos = len;

	for (;;) {
		if ((c = ftmgetc(f, tmo)) == EOF) {
			buf[len] = '\0';
			return 0;
		}

		switch (mode) {
		case MODE_ESC:
			switch (c) {
			case '[':
				mode = MODE_ESC_VAL1;
				val = 0;
				ctrl = 0;
				break;
			case 'O':
				mode = MODE_ESC_O;
				break;
			default:
				mode = 0;
			};
			continue;

		case MODE_ESC_VAL1:
		case MODE_ESC_VAL2:
			switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				val = val * 10 + c - '0';
				continue;
			case 'A':
				/* cursor up */
				c = IN_CURSOR_UP + ctrl;
				break;
			case 'B':
				/* cursor down */
				c = IN_CURSOR_DOWN + ctrl;
				break;
			case 'C':
				/* cursor right */
				c = IN_CURSOR_RIGHT + ctrl;
				break;
			case 'D':
				/* cursor left */
				c = IN_CURSOR_LEFT + ctrl;
				break;
			case '~':
				switch (val) {
				case 1:
					c = IN_HOME + ctrl;
					break;
				case 2:
					c = IN_INSERT + ctrl;
					break;
				case 3:
					/* delete */
					c = IN_DELETE + ctrl;
					break;
				case 4:
					/* end */
					c = IN_END + ctrl;
					break;
				case 5:
					c = IN_PAGE_UP + ctrl;
					break;
				case 6:
					c = IN_PAGE_DOWN + ctrl;
					break;
				default:
					mode = 0;
					continue;
				}
				break;
			case ';':
				mode = MODE_ESC_VAL2;
				ctrl = IN_CTRL;
				val = 0;
				continue;
			default:
				mode = 0;
				continue;
			};
			mode = 0;
			break;

		case MODE_ESC_O:
			switch (c) {
			case 'F':
				/* end */
				c = IN_END;
				break;
			case 'H':
				/* home */
				c = IN_HOME;
				break;
			default:
				mode = 0;
				continue;
			}
			mode = 0;
			break;
		} 

		switch (c) {
		case IN_ESC:
			mode = MODE_ESC;
			continue;

		case IN_CURSOR_RIGHT:
			if (pos < len)
				fputc(buf[pos++], f);
			else
				fputs(OUT_BEL, f);
			continue;

		case IN_CURSOR_LEFT:
			if (pos > 0) {
				fputs(OUT_CURSOR_LEFT, f);
				pos--;
			} else {
				fputs(OUT_BEL, f);
			}
			continue;

		case IN_CTRL_CURSOR_RIGHT:
			while (pos < len) {
				fputc(buf[pos++], f);
				if ((buf[pos - 1] != ' ') && (buf[pos] == ' '))
					break;
			}
			continue;

		case IN_CTRL_CURSOR_LEFT:
			if (pos > 0) {
				do {
					fputs(OUT_CURSOR_LEFT, f);
					pos--;
					if ((buf[pos - 1] == ' ') && (buf[pos] != ' '))
						break;
				} while (pos > 0);
			}
			continue;

		case IN_PAGE_UP:
			continue;

		case IN_PAGE_DOWN:
			continue;

		case IN_INSERT:
			continue;

		case IN_CURSOR_UP:
		case IN_HOME:
			while (pos > 0) {
				fputs(OUT_CURSOR_LEFT, f);
				pos--;
			} 
			continue;

		case IN_CURSOR_DOWN:
		case IN_END:
			while (pos < len) {
				fputc(buf[pos++], f);
			} 
			continue;

		case IN_TN_BS:     
		case IN_BS:
			if (pos == 0) {
				fputs(OUT_BEL, f);
				continue;
			}
			if (len == pos) {
				pos--;
				len--;
				fputs(OUT_BS, f);
				continue;
			}

			fputs(OUT_CURSOR_LEFT, f);
			pos--;

			/* FALLTHROUGH */

		case IN_DELETE:

			if (len == pos) {
				continue;
			}
			len--;

			for (i = pos; i < len; i++) {
				buf[i] = buf[i + 1];
			}
			buf[len] = '\0';
			fputs(&buf[pos], f);
			fputc(' ', f);
			for (i = len + 1; i > pos; i--)
				fputs(OUT_CURSOR_LEFT, f);
			continue;

		case IN_EOL:
			buf[len] = '\0';
			fputs("\r\n", f);
			return len;

		case IN_SKIP:
			fputs(OUT_SKIP, f);
			return -1;
		}

		if (len == max) {
			fputs(OUT_BEL, f);
			continue;
		}

		if (len == pos) {
			fputc(c, f);
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

		fputs(&buf[pos], f);
		pos++;

		for (i = len; i > pos; i--)
			fputs(OUT_CURSOR_LEFT, f);

	}
}

