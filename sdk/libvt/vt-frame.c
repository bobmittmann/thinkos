#include "vt-i.h"
#include <sys/null.h>

/* -------------------------------------------------------------------------
 * */
void vt_rect_frame(struct vt_win *win, const char * title, 
				   struct vt_rect rect)
{
	char buf[128];
	char * cp;
	int w;
	int h;
	int x;
	int y;
	int len;
	int n;

	x = rect.pos.x;
	y = rect.pos.y;
	w = rect.size.w;
	h = rect.size.h;


	len = strlen(title);
	n = (w - 4) - len;
	if (n >= 0) {
		int m;
		int j;
		int i;

		vt_cursor_move(win, x, y);
		vt_font_g1(win);
		cp = buf;
		*cp++ = 'l';
		if (len > 0) {
			m = n / 2;
			n = n - m;

			for (j = 0; j < m ; ++j)
				*cp++ = 'q';
			*cp++ = 'u';
			*cp = '\0';
			vt_puts(buf);

			vt_font_g0(win);
			vt_puts(title);

			vt_font_g1(win);
			cp = buf;
			*cp++ = 't';
		} else {
			n += 2;
		}
		for (j = 0; j < n ; ++j)
			*cp++ = 'q';
		*cp++ = 'k';
		*cp = '\0';
		vt_puts(buf);

		for (i = 1; i < (h - 1); ++i) {
			vt_cursor_move(win, x, y + i);
			cp = buf;
			*cp++ = 'x';
			for (j = 0; j < (w - 2) ; ++j)
				*cp++ = ' ';
			*cp++ = 'x';
			*cp = '\0';
			vt_puts(buf);
		}

		cp = buf;
		vt_cursor_move(win, x, y + i);
		*cp++ = 'm';
		for (j = 0; j < (w - 2) ; ++j)
			*cp++ = 'q';
		*cp++ = 'j';
		*cp++ = '\017';
		*cp = '\0';
		vt_puts(buf);
	}
}

void vt_nc_frame(struct vt_win *win, const char * title)
{
	struct vt_win * parent = vt_win_parent(win);
	struct vt_rect rect;

	rect.pos = vt_win_pos(win);
	rect.size = vt_win_size(win);
	rect.pos.x -= 1;
	rect.pos.y -= 1;
	rect.size.w += 2;
	rect.size.h += 2;

	vt_win_open(parent);
	vt_rect_frame(parent, title, rect);
	vt_win_close(parent);
}

void vt_frame(struct vt_win *win, const char * title)
{
	struct vt_rect rect;

	rect.pos.x = 0;
	rect.pos.y = 0;
	rect.size = vt_win_size(win);

	vt_rect_frame(win, title, rect);
}



