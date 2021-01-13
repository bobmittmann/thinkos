#include "vt-i.h"
#include <sys/null.h>

/* -------------------------------------------------------------------------
 * */

void vt_frame(struct vt_win *win, const char * title)
{
	char buf[128];
	char * cp = buf;
	struct vt_size size;
	int w;
	int h;
	int len;
	int n;

	size = vt_win_size(win);
	w = size.w;
	h = size.h;
	vt_cursor_move(win, 0, 0);

	len = strlen(title);
	n = (w - 4) - len;
	if (n >= 0) {
		int m;
		int j;
		int i;

		m = n / 2;
		n = n - m;

		vt_cursor_move(win, 0, 0);
		vt_font_g1(win);
		cp = buf;
		*cp++ = 'l';
		for (j = 0; j < n ; ++j)
			*cp++ = 'q';
		*cp++ = 'u';
		*cp = '\0';
		vt_puts(buf);

		vt_font_g0(win);
		vt_puts(title);

		vt_font_g1(win);
		cp = buf;
		*cp++ = 't';
		for (j = 0; j < m ; ++j)
			*cp++ = 'q';
		*cp++ = 'k';
		*cp = '\0';
		vt_puts(buf);

		for (i = 1; i < (h - 1); ++i) {
			vt_cursor_move(win, 0, i);
			cp = buf;
			*cp++ = 'x';
			for (j = 0; j < (w - 2) ; ++j)
				*cp++ = ' ';
			*cp++ = 'x';
			*cp = '\0';
			vt_puts(buf);
		}

		cp = buf;
		vt_cursor_move(win, 0, i);
		*cp++ = 'm';
		for (j = 0; j < (w - 2) ; ++j)
			*cp++ = 'q';
		*cp++ = 'j';
		*cp++ = '\017';
		*cp = '\0';
		vt_puts(buf);
	}
}
