#include "vt-i.h"

int vt_screen_init(const struct vt_screen_def * def)
{
	struct vt_win * win;
	struct vt_pos pos;
	char s[16];
	int n;
	int c;

	win = __vt_win_root();
	win->pos.x = 1;
	win->pos.y = 1;

	if ((c = __vt_get_cursor_pos(&pos)) != 0) {
	} else {
		win->cursor.x = pos.x;
		win->cursor.y = pos.y;
	}
	win->cursor_hide = 0;

	n = __vt_move_to(s, 255, 255);
	__vt_console_write(s, n);
	if ((c = __vt_get_cursor_pos(&pos)) != 0) {
		win->size.w = 80;
		win->size.h = 25;
	} else {
		win->size.w = pos.x;
		win->size.h = pos.y;
	}
	__vt_move_to(s, win->cursor.x, win->cursor.y);

	/* Update tree */
	win->parent = 0;
	win->child = 0;
	win->sibiling = 0;
	win->fg_color = def->fg_color;
	win->bg_color = def->bg_color;
	win->attr = def->attr;
	win->msg_handler = (def->msg_handler == NULL) ? vt_default_msg_handler :
		def->msg_handler;
	win->data = def->data;

	vt_msg_post(win, VT_WIN_CREATE, 0);

	return 0;
}

struct vt_size vt_screen_size(void)
{
	struct vt_win * win = __vt_win_root();

	return win->size;
}

int vt_screen_open(void)
{
	struct vt_win * win;
	char s[64];
	int n;

	win = __vt_win_root();
	n = __vt_reset(s);
	__vt_console_write(s, n);
	thinkos_sleep(200);


	vt_msg_post(win, VT_WIN_REFRESH, 0);

	return 0;
}

int vt_screen_close(void)
{
	char s[64];
	int n;

	n = __vt_reset(s);
	__vt_console_write(s, n);

	thinkos_sleep(100);

	return 0;
}

void vt_reset(void)
{
	char s[64];
	int n;

	n = __vt_reset(s);
	__vt_console_write(s, n);
}

void vt_refresh(void)
{
	struct vt_win * win;

	win = __vt_win_root();
	vt_msg_post(win, VT_WIN_REFRESH, 0);
}

