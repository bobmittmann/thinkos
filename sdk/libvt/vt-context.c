#include "vt-i.h"
#include <sys/null.h>

/* -------------------------------------------------------------------------
 * */
/* get the context of the whole terminal */
struct vt_ctx * vt_root_ctx(void);
/* get the context of a rectangle inside the window */
struct vt_ctx * vt_win_rect_ctx(struct vt_win * win, struct vt_rect * rect);
/* get the context of the window frame */
struct vt_ctx * vt_win_frame_ctx(struct vt_win * win);

struct vt_ctx * __vt_ctx_prepare(struct vt_ctx * ctx, struct vt_win * win, 
								 int x, int y)
{
	char s[128];
	char * cp;
	int n;

	cp = s;

	if (!ctx->insert_off) {
		ctx->insert_off = 1;
		cp += __vt_insert_off(cp);
	}

	if (ctx->cursor_hide != win->cursor_hide) {
		if (win->cursor_hide)
			cp += __vt_cursor_hide(cp);
		else
			cp += __vt_cursor_show(cp);
		ctx->cursor_hide = win->cursor_hide;
	}

	if (ctx->font_g1 != win->font_g1) {
		if (ctx->font_g1)
			cp += __vt_font_g1(cp);
		else
			cp += __vt_font_g0(cp);
	}

	if (ctx->__attr != win->__attr) {
		uint8_t lst[8];

		ctx->__attr = win->__attr;

		lst[0] = VT_ATTR_NORMAL; /* Clear all attribute */
		n = 1;

		if (ctx->attr.bright) 
			lst[n++] = VT_ATTR_BRIGHT;
		else if (ctx->attr.dim) 
			lst[n++] = VT_ATTR_DIM;
		else if (ctx->attr.hidden) 
			lst[n++] = VT_ATTR_HIDDEN;
		if (ctx->attr.hidden) 
			lst[n++] = VT_ATTR_HIDDEN;
		if (ctx->attr.underline) 
			lst[n++] = VT_ATTR_UNDERLINE;
		if (ctx->attr.reverse) 
			lst[n++] = VT_ATTR_REVERSE;
		lst[n++] = VT_ATTR_BG_COLOR(ctx->attr.bg_color);
		lst[n++] = VT_ATTR_FG_COLOR(ctx->attr.fg_color);

		cp += __vt_set_attr_lst(cp, lst, n);
	}

	if ((ctx->x != x) || (ctx->y != y)) {
		ctx->x = x;
		ctx->y = y;
		cp += __vt_move_to(cp, x, y);
	}

	if (cp != s) {
		n = cp - s;
		__vt_console_write(s, n);
	}

	return ctx;
}

void __vt_win_close(struct vt_win * win)
{
	char s[64];
	char * cp;
	int n;

	cp = s;
	cp += __vt_restore(cp);
	n = cp - s;
	__vt_console_write(s, n);

	win->open = 0;
}

/* get the context of the whole window */
struct vt_ctx * vt_win_ctx(struct vt_win * win)
{
	struct vt_ctx * ctx = __vt_ctx();
	int x;
	int y;

	if (__vt_lock() < 0) {
		return NULL;
	}

	ctx->min.x = win->pos.x;
	ctx->max.x = win->pos.x + win->size.w;
	ctx->min.y = win->pos.y;
	ctx->max.y = win->pos.y + win->size.h;

	x = win->pos.x + win->cursor.x;
	y = win->pos.y + win->cursor.y;


	win->open = 1;

	__vt_ctx_prepare(ctx, win, x, y);

	return ctx;
}

int vt_ctx_close(struct vt_ctx * ctx)
{
//	if (win->open)
//		__vt_win_close(win);

	__vt_unlock();
	return 0;
}

# if 0
struct vt_ctx * __vt_win_open(struct vt_win * win)
{
	struct vt_ctx * ctx = __vt_ctx();
	char s[128];
	uint8_t lst[8];
	char * cp;
	int n;

	win->open = 1;

	ctx->attr = win->attr;

	cp = s;
	cp += __vt_save(cp);
	cp += __vt_insert_off(cp);
	if (win->font_g1)
		cp += __vt_font_g1(cp);
	else
		cp += __vt_font_g0(cp);

	lst[0] = VT_ATTR_NORMAL; /* Clear all attribute */
	n = 1;

	if (ctx->attr.bright) 
		lst[n++] = VT_ATTR_BRIGHT;
	else if (ctx->attr.dim) 
		lst[n++] = VT_ATTR_DIM;
	else if (ctx->attr.hidden) 
		lst[n++] = VT_ATTR_HIDDEN;
	if (ctx->attr.hidden) 
		lst[n++] = VT_ATTR_HIDDEN;
	if (ctx->attr.underline) 
		lst[n++] = VT_ATTR_UNDERLINE;
	if (ctx->attr.reverse) 
		lst[n++] = VT_ATTR_REVERSE;
	lst[n++] = VT_ATTR_BG_COLOR(ctx->attr.bg_color);
	lst[n++] = VT_ATTR_FG_COLOR(ctx->attr.fg_color);

	cp += __vt_set_attr_lst(cp, lst, n);

	if (win->cursor_hide)
		cp += __vt_cursor_hide(cp);
	else
		cp += __vt_cursor_show(cp);
	cp += __vt_set_scroll(cp, win->pos.y, win->pos.y + win->size.h);
	cp += __vt_move_to(cp, win->pos.x + win->cursor.x, 
					   win->pos.y + win->cursor.y);
	n = cp - s;
	__vt_console_write(s, n);
}

#endif



