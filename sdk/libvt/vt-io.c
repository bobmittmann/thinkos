#include "vt-i.h"
#include <sys/null.h>

int __vt_strcpyn(char * dst, const char * src, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; ++i)
		dst[i] = src[i];
	dst[i] = '\0';

	return len;
}

void __vt_console_write(const char * s, unsigned int len)
{
	char * cp = (char *)s;
	char * ep = cp + len;

	while (cp < ep) {
		int n;

		n = thinkos_console_write(cp, ep - cp);
		if (n < 0)
			return;
		cp += n;
	}
}

int uint2dec(char * s, unsigned int val);

int __vt_reset(char * s)
{
	s[0] = '\033';
	s[1] = 'c';
	return 2;
}

/* Save Cursor & Attrs */
int __vt_save(char * s)
{
	s[0] = '\033';
	s[1] = '7';
	return 2;
}

/* Restore Cursor & Attrs */
int __vt_restore(char * s)
{
	s[0] = '\033';
	s[1] = '8';
	return 2;
}


int __vt_clrscr(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '2';
	s[3] = 'J';
	return 4;
}

int __vt_clreol(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = 'E';
	return 3;
}

int __vt_cursor_up(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = 'A';
	return 3;
}

int __vt_cursor_down(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = 'B';
	return 3;
}

int __vt_cursor_right(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = 'C';
	return 3;
}

int __vt_cursor_left(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = 'C';
	return 3;
}

int __vt_cursor_show(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '?';
	s[3] = '2';
	s[4] = '5';
	s[5] = 'h';
	return 6;
}

int __vt_cursor_hide(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '?';
	s[3] = '2';
	s[4] = '5';
	s[5] = 'l';
	return 6;
}

int __vt_insert_on(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '4';
	s[3] = 'h';
	return 4;
}

int __vt_insert_off(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '4';
	s[3] = 'l';
	return 4;
}

int __vt_insert_lines(char * s, int n)
{
	char * cp = s;

	*cp++ = '\033';
	*cp++ = '[';
	*cp++ = 'P';
	cp += uint2dec(cp, n);
	*cp++ = 'M';
	*cp = '\0';

	return cp - s;
}

int __vt_delete_lines(char * s, int n)
{
	char * cp = s;

	*cp++ = '\033';
	*cp++ = '[';
	*cp++ = 'P';
	cp += uint2dec(cp, n);
	*cp++ = 'M';

	return cp - s;
}

int __vt_delete_chars(char * s, int n)
{
	char * cp = s;

	*cp++ = '\033';
	*cp++ = '[';
	*cp++ = 'P';
	cp += uint2dec(cp, n);
	*cp++ = 'M';

	return cp - s;
}

int __vt_set_scroll(char * s, int y0, int y1)
{
	char * cp = s;

	*cp++ = '\033';
	*cp++ = '[';
	cp += uint2dec(cp, y0);
	*cp++ = ';';
	cp += uint2dec(cp, y1);
	*cp++ = 'r';

	return cp - s;
}

int __vt_attr_fg_color(char * s, int color)
{
	s[0] = ';';
	s[1] = '3';
	s[2] = '0' + color;
	return 3;
}

int __vt_attr_bg_color(char * s, int color)
{
	s[0] = ';';
	s[1] = '3';
	s[2] = '0' + color;
	return 3;
}

int __vt_attr_add(char * s, int attr)
{
	s[0] = ';';
	s[1] = '0' + attr;
	return 2;
}

int __vt_attr_end(char * s)
{
	s[0] = 'm';
	return 1;
}

int __vt_set_attr_lst(char * s, uint8_t attr[], int len)
{
	char * cp = s;
	int i;

	*cp++ = '\033';
	*cp++ = '[';

	for (i = 0; i < len; ++i) {
		if (i  > 0)
			*cp++ = ';';
		if (attr[i] > 10)
			*cp++ = '0' + attr[i] / 10;
		*cp++ = '0' + attr[i] % 10;
	}

	*cp++ = 'm';
	return cp - s;
}

int __vt_set_bg_color(char * s, int color)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '4';
	s[3] = '0' + color;
	s[4] = 'm';

	return 5;
}

int __vt_set_fg_color(char * s, int color)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '3';
	s[3] = '0' + color;
	s[4] = 'm';

	return 5;
}

int __vt_set_font_mode(char * s, int mode)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '0' + mode;
	s[3] = 'm';

	return 4;
}

int __vt_clear_attr(char * s)
{
	s[0] = '\033';
	s[1] = '[';
	s[2] = '0';
	return 3;
}

int __vt_set_attr(char * s, int attr)
{
	char * cp = s;

	*cp++ = '\033';
	*cp++ = '[';
	if (attr > 10)
		*cp++ = '0' + attr / 10;
	*cp++ = '0' + attr % 10;
	*cp++ = 'm';

	return cp - s;
}

int __vt_move_to(char * s, int x, int y)
{
	char * cp = s;

	*cp++ = '\033';
	*cp++ = '[';
	cp += uint2dec(cp, y);
	*cp++ = ';';
	cp += uint2dec(cp, x);
	*cp++ = 'f';

	return cp - s;
}

int __vt_font_g0(char * s)
{
	char * cp = s;

	*cp++ = '\017';

	return cp - s;
}

int __vt_font_g1(char * s)
{
	char * cp = s;

	*cp++ = '\016';

	return cp - s;
}

void __vt_flush(void)
{
	unsigned int tmo = 100;
	char buf[8];

	while (thinkos_console_timedread(buf, 8, tmo) >= 0);
}

int __vt_getc(void)
{
	unsigned int tmo = 100;
	char buf[1];
	int ret = 0;

	while ((ret = thinkos_console_timedread(buf, 1, tmo)) == 0);
	if (ret < 0)
		return ret;

	return buf[0];
}

int __vt_get_cursor_pos(struct vt_pos * pos) 
{
	int c;
	int x;
	int y;

	__vt_flush();

	__vt_console_write(VT100_QUERY_CURSOR_POS, 
					   sizeof(VT100_QUERY_CURSOR_POS));

	if ((c = __vt_getc()) != '\033') { /* ESC */
		return c;
	}
	if ((c = __vt_getc()) != '[') {
		return c;
	}
	y = 0;
	for (;;) {
		if ((c = __vt_getc()) < 0)
			return c;
		if (c >= '0' && c <= '9') {
			y *= 10;
			y += c - '0';
		} else {
			if (c != ';')
				return c;
			break;
		}
	}
	x = 0;
	for (;;) {
		if ((c = __vt_getc()) < 0)
			return c;
		if (c >= '0' && c <= '9') {
			x *= 10;
			x += c - '0';
		} else {
			if (c != 'R') {
				return c;
			}
			break;
		}
	}

	pos->x = x;
	pos->y = y;

	return 0;
}


int __vt_console_decode(struct vt_console * con, int c)
{
	int ret = -1;
	
	unsigned int ctrl = con->ctrl;

	do {
		switch (con->mode) {
		case MODE_RAW:
			switch (c) {
			case IN_ESC:
				con->mode = MODE_ESC;
				break;
			default:
				ret = c;
			}
			continue;

		case MODE_ESC:
			switch (c) {
			case '[':
				con->mode = MODE_ESC_VAL1;
				con->val = 0;
				con->ctrl = 0;
				break;
			case 'O':
				con->mode = MODE_ESC_O;
				break;
			default:
				con->mode = MODE_RAW;
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
				con->val = con->val * 10 + c - '0';
				continue;
			case 'A':
				/* cursor up */
				c = VT_CURSOR_UP + ctrl;
				break;
			case 'B':
				/* cursor down */
				c = VT_CURSOR_DOWN + ctrl;
				break;
			case 'C':
				/* cursor right */
				c = VT_CURSOR_RIGHT + ctrl;
				break;
			case 'D':
				/* cursor left */
				c = VT_CURSOR_LEFT + ctrl;
				break;
			case '~':
				switch (con->val) {
				case 1:
					c = VT_HOME + ctrl;
					break;
				case 2:
					c = VT_INSERT + ctrl;
					break;
				case 3:
					/* delete */
					c = VT_DELETE + ctrl;
					break;
				case 4:
					/* end */
					c = VT_END + ctrl;
					break;
				case 5:
					c = VT_PAGE_UP + ctrl;
					break;
				case 6:
					c = VT_PAGE_DOWN + ctrl;
					break;
				default:
					con->mode = 0;
					continue;
				}
				break;
			case ';':
				con->mode = MODE_ESC_VAL2;
				con->ctrl = VT_CTRL;
				con->val = 0;
				continue;
			default:
				con->mode = 0;
				continue;
			};
			con->mode = 0;
			break;

		case MODE_ESC_O:
			switch (c) {
			case 'F':
				/* end */
				c = VT_END;
				break;
			case 'H':
				/* home */
				c = VT_HOME;
				break;
			default:
				con->mode = 0;
				continue;
			}
			con->mode = 0;
			break;
		} 
	} while (0);

	return ret;
}

int vt_getc(unsigned int tmo)
{
	char buf[1];
	int ret = 0;

	ret = thinkos_console_timedread(buf, 1, tmo);
	if (ret < 0)
		return ret;

	return buf[0];
}

int vt_getkey(unsigned int tmo_ms)
{
	char buf[1];
	int ret = 0;

	while ((ret = thinkos_console_timedread(buf, 1, tmo_ms)) > 0) {
		int c;
		if ((c = __vt_console_decode(&__sys_vt.con, buf[0])) > 0)
			return c;
	}

	return ret;
}

int __vt_win_fwrite(struct vt_win * win, const void * buf, unsigned int len) 
{
	int cnt;

	__vt_lock();
	__vt_win_open(win);

	cnt = __vt_win_write(win, buf, len); 

	__vt_win_close(win);
	__vt_unlock();

	return cnt;
}

const struct fileop vt_win_fops = {
	.write = (int (*)(void *, const void *, size_t))__vt_win_fwrite,
	.read = (int (*)(void *, void *, size_t, unsigned int))null_read,
	.flush = (int (*)(void *))__vt_win_drain,
	.close = (int (*)(void *))null_close
};

FILE * vt_console_fopen(struct vt_win * win)
{
	static struct file f;

	f.data = (void *)win;
	f.op = &vt_win_fops;

	return &f;
}

