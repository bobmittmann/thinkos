#include "vt-i.h"
#include <sys/null.h>

#define TRACE_LEVEL TRACE_LVL_NONE
#include <trace.h>

int __vt_strcpyn(char * dst, const char * src, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; ++i)
		dst[i] = src[i];
	dst[i] = '\0';

	return len;
}

int __vt_console_write(const char * s, unsigned int len)
{
	char * cp = (char *)s;
	char * ep = cp + len;

	while (cp < ep) {
		int n;

		n = thinkos_console_write(cp, ep - cp);
		if (n < 0)
			return n;
		cp += n;
	}

	return len;
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

	if (len == 0)
		return 0;

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

int __vt_con_raw_decode(struct vt_console * con, int c)
{
	if (c != IN_ESC)
		return c;

	con->idx = 0;
	con->val[0] = 0;
	con->val[1] = 0;
	con->mode = VT_CON_MODE_ESC;

	return -1;
}

int __vt_con_esc_decode(struct vt_console * con, int c)
{
	if (c == '[') {
		con->mode = VT_CON_MODE_ESC_VAL;
	} else  if (c == 'O') {
		con->mode = VT_CON_MODE_ESC_O;
	} else { 
		INF("key ESC+0x%02x", c);
		con->mode = VT_CON_MODE_RAW;
	}
	return -1;
}

int __vt_con_esc_o_decode(struct vt_console * con, int c)
{
	con->mode = VT_CON_MODE_RAW;

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
		INF("key ESC+O+0x%02x", c);
		c = -1;
	}

	return c;
} 

int __vt_con_esc_val_decode(struct vt_console * con, int c)
{
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
	case '9': {
		int val = con->val[con->idx];
		val = val * 10 + c - '0';
		con->val[con->idx] = val;
		c = -1;
	}
		break;
	case ';':
		c = -1;
		if (con->idx < VT_CON_VAL_LST_SZ)
			con->idx++;
		break;
	case 'A':
		/* cursor up */
		c = VT_CURSOR_UP;
		con->mode = VT_CON_MODE_RAW;
		INF("key ESC+[+A = %04x", c);
		break;
	case 'B':
		/* cursor down */
		c = VT_CURSOR_DOWN;
		con->mode = VT_CON_MODE_RAW;
		INF("key ESC+[+B = %04x", c);
		break;
	case 'C':
		/* cursor right */
		c = VT_CURSOR_RIGHT;
		con->mode = VT_CON_MODE_RAW;
		INF("key ESC+[+C = %04x", c);
		break;
	case 'D':
		/* cursor left */
		c = VT_CURSOR_LEFT;
		con->mode = VT_CON_MODE_RAW;
		INF("key ESC+[+D = %04x", c);
		break;
	case '~': {
		int ctrl = con->idx ? VT_CTRL : 0;
		con->mode = VT_CON_MODE_RAW;
		switch (con->val[0]) {
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
			INF("key ESC+[+%d~", con->val[0]);
			c = -1;
			break;
		}
		break;
	}
	default:
		INF("key ESC+[+%d+%08x", con->val[0], c);
		c = -1;
		con->mode = VT_CON_MODE_RAW;
		break;
	};

	return c;
}

const vt_key_decode_t __vt_con_decode_by_mode[]  = {
	[VT_CON_MODE_RAW] = __vt_con_raw_decode,
	[VT_CON_MODE_ESC] = __vt_con_esc_decode,
	[VT_CON_MODE_ESC_O] = __vt_con_esc_o_decode,
	[VT_CON_MODE_ESC_VAL] = __vt_con_esc_val_decode
};

int __vt_putc(int c)
{
	char buf[1];

	buf[0] = c;
	return __vt_console_write(buf, 1);
}

int __vt_puts(const char * s)
{
	return __vt_console_write(s, strlen(s));
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

