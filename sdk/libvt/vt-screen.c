#include "vt-i.h"

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

const struct vt_screen_def vt_default_screen_def = {
	.attr = {
		.fg_color = VT_COLOR_GREEN,
		.bg_color = VT_COLOR_BLUE,
	},
	.size = {.h = 0, .w = 0}
};

#define VT_SCREEN_DEFAULT_SIZE VT_SIZE(80, 25)

int vt_screen_init(const struct vt_screen_def * def,
				   vt_msg_handler_t msg_handler,
				   void * data)
{
	struct vt_win * win;
	struct vt_pos pos;
	char s[16];
	int ret;
	int n;
	int c;

	if ((ret = __vt_lock()) < 0)
		return ret;

	if (def == NULL)
		def = &vt_default_screen_def;

	win = __vt_win_root();
	win->pos = VT_POS(1, 1);
	win->size = VT_SCREEN_DEFAULT_SIZE;

	__vt_get_term_type(); 

	if ((c = __vt_get_cursor_pos(&pos)) != 0) {
	} else {
		win->cursor.x = pos.x;
		win->cursor.y = pos.y;
	}
	win->cursor_hide = 0;

	if (def->size.h == 0 || def->size.w == 0) {
		n = __vt_move_to(s, 255, 255);
		__vt_console_write(s, n);
		if ((c = __vt_get_cursor_pos(&pos)) == 0) {
			win->size.w = pos.x;
			win->size.h = pos.y;
		}
		__vt_move_to(s, win->cursor.x, win->cursor.y);
	} else {
		win->size = def->size;
	}

	/* Update tree */
	win->parent = 0;
	win->child = 0;
	win->sibiling = 0;
	win->attr = def->attr;
	win->msg_handler = (msg_handler == NULL) ? 
		vt_default_msg_handler : msg_handler;
	win->data = data;
	win->visible = true;

	__vt_msg_post(win, VT_WIN_CREATE, 0);

	__vt_unlock();

	return 0;
}

struct vt_size vt_screen_size(void)
{
	struct vt_win * win = __vt_win_root();

	return win->size;
}

void __vt_screen_reset(struct vt_ctx * ctx)
{
	char s[64];
	int n;

	/* Reset screen */ 
	n = __vt_reset(s);
	__vt_console_write(s, n);

	/* Set remote state to defaults */ 
	ctx->rem.attr_bright = 0;
	ctx->rem.attr_dim = 0;
	ctx->rem.attr_underline = 0;
	ctx->rem.attr_blink = 0;
	ctx->rem.attr_reverse = 0;
	ctx->rem.attr_hidden = 0;

	ctx->rem.insert_off = 0;
	ctx->rem.cursor_hide = 0;
	ctx->rem.font_g1 = 0;
	ctx->rem.color_fg = VT_COLOR_WHITE;
	ctx->rem.color_bg = VT_COLOR_BLACK;
	ctx->rem.pos_x = 0;
	ctx->rem.pos_y = 0;

	thinkos_sleep(200);
}

int vt_screen_open(void)
{
	struct vt_win * win;
	struct vt_ctx * ctx;
	int ret;

	if ((ret = __vt_lock()) < 0)
		return ret;

	ctx = __vt_ctx();
	win = __vt_win_root();

	__vt_ctx_prepare(ctx, win);

	__vt_screen_reset(ctx);

	__vt_msg_post(win, VT_WIN_DRAW, 0);
	__vt_msg_post(win, VT_WIN_REFRESH, 0);

	__vt_unlock();

	return 0;
}

int vt_screen_close(void)
{
	char s[64];
	int ret;
	int n;

	if ((ret = __vt_lock()) < 0)
		return ret;

	n = __vt_reset(s);
	__vt_console_write(s, n);
	thinkos_sleep(128);

	__vt_unlock();

	return 0;
}

void vt_screen_reset(void)
{
	char s[64];
	int n;

	if ((__vt_lock()) >= 0) {
		n = __vt_reset(s);
		__vt_console_write(s, n);
		thinkos_sleep(128);

		__vt_unlock();
	}
}

void vt_screen_refresh(void)
{
	if ((__vt_lock()) >= 0) {
		struct vt_win * win = __vt_win_root();
		__vt_msg_post(win, VT_WIN_REFRESH, 0); 
		__vt_unlock();
	}
}

void vt_screen_render(void)
{
	if ((__vt_lock()) >= 0) {
		struct vt_win * win = __vt_win_root();
		__vt_msg_post(win, VT_WIN_DRAW, 0); 
		__vt_unlock();
	}
}

