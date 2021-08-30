#include "vt-i.h"
#include <sys/null.h>

#define TRACE_LEVEL TRACE_LVL_NONE
#include <trace.h>


/* -------------------------------------------------------------------------
 * */
/* get the context of the whole terminal */
struct vt_ctx * vt_root_ctx(void);
/* get the context of a rectangle inside the window */
struct vt_ctx * vt_win_rect_ctx(struct vt_win * win, struct vt_rect * rect);
/* get the context of the window frame */
struct vt_ctx * vt_win_frame_ctx(struct vt_win * win);

static void __vt_ctx_move(struct vt_ctx * ctx, int x, int y)
{
	int x_min;
	int x_max;
	int y_min;
	int y_max;

	x_min = ctx->min.x;
	x_max = ctx->max.x;
	y_min = ctx->min.y;
	y_max = ctx->max.y;

	x += x_min;
	y += y_min;

	/* check x limmits */
	if (x < x_min)
		x = x_min;
	else if (x > x_max)
		x = x_max;

	/* check y limmits */
	if (y < y_min)
		y = y_min;
	else if (y > y_max)
		y = y_max;

	/* update local position */
	ctx->loc.pos_x = x;
	ctx->loc.pos_y = y;
}

/* relative position */
static inline int __vt_ctx_x(struct vt_ctx * ctx) {
	return ctx->loc.pos_x - ctx->min.x;
}
static inline int __vt_ctx_y(struct vt_ctx * ctx) {
	return ctx->loc.pos_y - ctx->min.y;
}

void __vt_ctx_save(struct vt_ctx * ctx, struct vt_win * win)
{
	win->cursor.x = __vt_ctx_x(ctx);
	win->cursor.y = __vt_ctx_y(ctx);
}

void __vt_ctx_prepare(struct vt_ctx * ctx, struct vt_win * win)
{
	int hide;

	ctx->min.x = win->pos.x;
	ctx->max.x = win->pos.x + win->size.w;
	ctx->min.y = win->pos.y;
	ctx->max.y = win->pos.y + win->size.h;

	__vt_ctx_move(ctx, win->cursor.x, win->cursor.y);

	INF("--> min=%d,%d max=%d,%d pos=%d,%d", 
		ctx->min.x, ctx->min.y,
		ctx->max.x, ctx->max.y,
		ctx->loc.pos_x, ctx->loc.pos_y);

	hide = win->has_focus ? win->cursor_hide : 1;
	ctx->loc.cursor_hide = hide;
	ctx->loc.insert_off = 1;
	ctx->loc.font_g1 = 0;
	ctx->loc.res1 = 0;

	ctx->loc.attr_bright = win->attr.bright;
	ctx->loc.attr_dim = win->attr.dim;
	ctx->loc.attr_hidden = win->attr.hidden;
	ctx->loc.attr_underline = win->attr.underline ;
	ctx->loc.attr_blink = win->attr.blink;
	ctx->loc.attr_reverse = win->attr.reverse;
	ctx->loc.color_bg = win->attr.bg_color;
	ctx->loc.color_fg = win->attr.fg_color;
}

const char sync[][16] = {
	"  .   .   .   .",
	"att   .   .   .",
	"  . clr   .   .",
	"att clr   .   .",
	"  .   . fg0   .",
	"att   . fg0   .",
	"  . clr fg0   .",
	"att clr fg0   .",
	"  .   . fg1   .",
	"att   . fg1   .",
	"  . clr fg1   .",
	"att clr fg1   .",
	"  .   . fg1   .",
	"att   . fg1   .",
	"  . clr fg1   .",
	"att clr fg1   .",
};

#define SET_ATTR     1
#define SET_COLOR    2
#define SET_FONT_G0  4
#define SET_FONT_G1  8

static void __vt_ctx_sync(struct vt_ctx * ctx)
{
	uint8_t lst[8];
	char s[128];
	char * cp;
	int n;
	int up = 0;


	/* Need update ? */
	if (ctx->rem.u32 == ctx->loc.u32) {
		INFS("sync: match");
		return;
	}

	/* Update differences */
	cp = s;
	if (ctx->rem.insert_off != ctx->loc.insert_off) {
		cp += __vt_insert_off(cp);
	}

	if (ctx->rem.cursor_hide != ctx->loc.cursor_hide) {
		if (ctx->loc.cursor_hide)
			cp += __vt_cursor_hide(cp);
		else
			cp += __vt_cursor_show(cp);
	}

	n = 0;
	if ((ctx->rem.u32 & VT_STATE_ATTR_MSK) !=
		(ctx->loc.u32 & VT_STATE_ATTR_MSK)) {

		up += SET_ATTR;

		lst[n++] = VT_ATTR_NORMAL; /* Clear all attribute */

		if (ctx->loc.attr_bright) 
			lst[n++] = VT_ATTR_BRIGHT;
		else if (ctx->loc.attr_dim) 
			lst[n++] = VT_ATTR_DIM;
		else if (ctx->loc.attr_hidden) 
			lst[n++] = VT_ATTR_HIDDEN;

		if (ctx->loc.attr_blink) 
			lst[n++] = VT_ATTR_BLINK;
		if (ctx->loc.attr_underline) 
			lst[n++] = VT_ATTR_UNDERLINE;
		if (ctx->loc.attr_reverse) 
			lst[n++] = VT_ATTR_REVERSE;

		/* Clearing attributes also resets the colors */
		ctx->rem.color_bg = VT_COLOR_BLACK;
		ctx->rem.color_fg = VT_COLOR_WHITE;
	}

	if (ctx->rem.font_g1 != ctx->loc.font_g1) {
		if (ctx->loc.font_g1) {
			cp += __vt_font_g1(cp);
			up |= SET_FONT_G1;
		} else {
			cp += __vt_font_g0(cp);
			up |= SET_FONT_G0;
		}
	}

	if (ctx->rem.color_bg != ctx->loc.color_bg) {
		lst[n++] = VT_ATTR_BG_COLOR(ctx->loc.color_bg);
		up |= SET_COLOR;
	}

	if (ctx->rem.color_fg != ctx->loc.color_fg) {
		lst[n++] = VT_ATTR_FG_COLOR(ctx->loc.color_fg);
		up |= SET_COLOR;
	}

	cp += __vt_set_attr_lst(cp, lst, n);

	if ((ctx->rem.u32 & VT_STATE_POS_MSK) !=
		(ctx->loc.u32 & VT_STATE_POS_MSK)) {
		cp += __vt_move_to(cp, ctx->loc.pos_x, ctx->loc.pos_y);
		INF("sync: %s pos=%d,%d", sync[up], ctx->loc.pos_x, ctx->loc.pos_y);
	} else {
		INF("sync: %s   .", sync[up]);
	}

	if (cp != s) {
		n = cp - s;
		__vt_console_write(s, n);
	}

	ctx->rem = ctx->loc;
}


int __vt_ctx_write(struct vt_ctx * ctx, 
				   const void * buf, unsigned int len) 
{
//	const char scroll[] = "\n";
	char s[12];
	char * cp;
	int rem;
	int cnt;
	int x;
	int y;
	int x_min;
	int x_max;
	int y_min;
	int y_max;
	int w;
	int h;


	cp = (char *)buf;
	rem = len;
	cnt = 0;

	__vt_ctx_sync(ctx);

	x = ctx->rem.pos_x;
	y = ctx->rem.pos_y;
	x_min = ctx->min.x;
	x_max = ctx->max.x;
	y_min = ctx->min.y;
	y_max = ctx->max.y;
	
	w = x_max - x_min;
	h = y_max - y_min;

	if (w == 0 || h == 0) {
		/* rectangle is closed (0) in at least one dimension */
		return cnt;
	}


	while (rem > 0) {
		int k;
		int n;
		int i;

		if (x >= x_max) {
			if (y < y_max) {
				y++;
			} else {
				INFS("ctx write: y >= y_max ?");
//				__vt_console_write(scroll, sizeof(scroll) - 1);
				y = y_min;
			}
			x = x_min;
			INF("write LIM: move:%d,%d", x, y);
			k = __vt_move_to(s, x, y);
			__vt_console_write(s, k);
			ctx->rem.pos_x = x;
			ctx->rem.pos_y = y;
		}

		/* search for next '\n' or '\r' */
		n = MIN(rem, w);
		for (i = 0; (i < n) && (cp[i] != '\n') && 
			 (cp[i] != '\r'); ++i);

		__vt_console_write(cp, i);

		if (cp[i] == '\n') {
			if (y < y_max) {
				y++;
			} else {
				INFS("write NL: y >= y_max ?");
				y = y_min;
			}
			x = x_min;
			INF("write NL: move:%d,%d", x, y);
			k = __vt_move_to(s, x, y);
			__vt_console_write(s, k);
			ctx->rem.pos_x = x;
			ctx->rem.pos_y = y;
			/* skip '\n' */
			i++;
		} else if (cp[i] == '\r') {
			x = x_min;
			INF("write CR: move:%d,%d", x, y);
			k = __vt_move_to(s, x, y);
			__vt_console_write(s, k);
			ctx->rem.pos_x = x;
			/* skip '\r' */
			i++;
		} else {
			x += i;
			ctx->rem.pos_x = x;
		}
		rem -= i;
		cnt += i;
		cp += i;
	};

	ctx->loc.pos_x = x;
	ctx->loc.pos_y = y;


	INF("ctx write: pos=%d,%d", x, y);

	return cnt;
}

static int __vt_ctx_drain(struct vt_ctx * ctx)
{
	while (thinkos_console_drain() != 0);
	return 0;
}

static const struct fileop __ctx_fops = {
	.write = (int (*)(void *, const void *, size_t))__vt_ctx_write,
	.read = (int (*)(void *, void *, size_t, unsigned int))null_read,
	.flush = (int (*)(void *))__vt_ctx_drain,
	.close = (int (*)(void *))null_close
};

int vt_printf(struct vt_ctx * ctx, const char * fmt, ...)
{
	struct file f = {
		.data = (void *)ctx,
		.op = &__ctx_fops 
	};
	va_list ap;
	int cnt;

	assert(ctx != NULL);
	assert(fmt != NULL);

	va_start(ap, fmt);
	cnt = vfprintf(&f, fmt, ap);
	va_end(ap);

	return cnt;
}

int vt_write(struct vt_ctx * ctx, const void * buf, unsigned int len) 
{
	int cnt;

	assert(ctx != NULL);
	assert(buf != NULL);

	cnt = __vt_ctx_write(ctx, buf, len); 

	return cnt;
}

int vt_puts(struct vt_ctx * ctx, const char * buf) 
{
	unsigned int len;

	assert(ctx != NULL);
	assert(buf != NULL);

	len = strlen(buf);
	return __vt_ctx_write(ctx, buf, len); 
}

int vt_putc(struct vt_ctx * ctx, int c)
{
	char buf[1];

	assert(ctx != NULL);

	buf[0] = c;
	return __vt_ctx_write(ctx, buf, 1); 
}


int vt_mov_printf(struct vt_ctx * ctx, int x, int y, const char * fmt, ...)
{
	struct file f = {
		.data = (void *)ctx,
		.op = &__ctx_fops 
	};
	va_list ap;
	int cnt;

	assert(ctx != NULL);
	assert(fmt != NULL);

	__vt_ctx_move(ctx, x, y);

	va_start(ap, fmt);
	cnt = vfprintf(&f, fmt, ap);
	va_end(ap);

	return cnt;
}

int vt_mov_puts(struct vt_ctx * ctx, int x, int y, const char * buf) 
{
	unsigned int len;

	assert(ctx != NULL);
	assert(buf != NULL);

	__vt_ctx_move(ctx, x, y);
	len = strlen(buf);
	return __vt_ctx_write(ctx, buf, len); 
}

int vt_mov_putc(struct vt_ctx * ctx, int x, int y, int c)
{
	char buf[1];

	assert(ctx != NULL);

	__vt_ctx_move(ctx, x, y);
	buf[0] = c;
	return __vt_ctx_write(ctx, buf, 1); 
}


/* get the context of the whole window */
struct vt_ctx * vt_win_ctx_open(struct vt_win * win)
{
	struct vt_ctx * ctx = __vt_ctx();

	if (__vt_lock() < 0) {
		return NULL;
	}

	__vt_ctx_prepare(ctx, win);

	return ctx;
}

int __vt_ctx_close(struct vt_ctx * ctx)
{


	return 0;
}

int vt_ctx_close(struct vt_ctx * ctx)
{
	__vt_ctx_close(ctx);
	__vt_unlock();
	return 0;
}

const char __vt_clrln[] = "                            "
	"                                                  "
	"                                                  "; 

void vt_clear_ln(struct vt_ctx * ctx, unsigned int n)
{
	int w;
	int x;

	__vt_ctx_sync(ctx);

	x = ctx->loc.pos_x;
	w = ctx->max.x - ctx->min.x - x;

	if (n > w)
		n = w;

	__vt_console_write(__vt_clrln, n);
	x += n;

	ctx->loc.pos_x = x;
	ctx->rem.pos_x = x;
}

void vt_clear(struct vt_ctx * ctx)
{
	char s[32];
	char * cp;
	int w;
	int y;
	int x;

	__vt_ctx_sync(ctx);

	w = ctx->max.x - ctx->min.x;

	x = ctx->min.x;
	for (y = ctx->min.y; y < ctx->max.y; ++y) {
		int n;

		n = __vt_move_to(s, x, y);
		__vt_console_write(s, n);

		cp = (char *)__vt_clrln;
		__vt_console_write(cp, w);
	}
	x += w;

	ctx->loc.pos_x = x;
	ctx->loc.pos_y = y;
	ctx->rem.pos_x = x;
	ctx->rem.pos_y = y;
	INF("clear: pos=%d,%d", ctx->loc.pos_x, ctx->loc.pos_y);
}

/* Save Cursor & Attrs */
void vt_save(struct vt_ctx * ctx)
{
	char s[2];

	__vt_ctx_sync(ctx);

	s[0] = '\033';
	s[1] = '7';

	__vt_console_write(s, 2);
}

/* Restore Cursor & Attrs */
void vt_restore(struct vt_ctx * ctx)
{
	char s[2];

	s[0] = '\033';
	s[1] = '8';

	__vt_console_write(s, 2);
}

#if 0
const char __hbar[] = "----------------------------------------"; 

int vt_hbar(struct vt_ctx * ctx, unsigned int y) 
{
	char s[16];
	char * cp;
	unsigned int rem;
	int rect_w;

	assert(ctx != NULL);

	cp = s;
	cp += __vt_move_to(cp, ctx->min.x, ctx->min.y + y);
	__vt_console_write(s, cp - s);

	rect_w = ctx->max.x - ctx->min.x;
	rem = rect_w;
	while (rem) {
		int n;
		n = MIN(rem, sizeof(__hbar) - 1);
		__vt_console_write(__hbar, n);
		rem -= n;
	}

	return 0;
}
#endif

void vt_move(struct vt_ctx * ctx, int x, int y)
{
	int x_min;
	int x_max;
	int y_min;
	int y_max;

	assert(ctx != NULL);

	x_min = ctx->min.x;
	x_max = ctx->max.x;
	y_min = ctx->min.y;
	y_max = ctx->max.y;

	x += x_min;
	y += y_min;

	/* check x limmits */
	if (x < x_min)
		x = x_min;
	else if (x > x_max)
		x = x_max;

	/* check y limmits */
	if (y < y_min)
		y = y_min;
	else if (y > y_max)
		y = y_max;

	/* update local position */
	ctx->loc.pos_x = x;
	ctx->loc.pos_y = y;
	INF("move: pos=%d,%d", ctx->loc.pos_x, ctx->loc.pos_y);
}

void vt_cursor_home(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.pos_x = ctx->min.x;
	ctx->loc.pos_y = ctx->min.y;
	INF("home: pos=%d,%d", ctx->loc.pos_x, ctx->loc.pos_y);
}

void vt_bg_color_set(struct vt_ctx * ctx, enum vt_color_code color)
{
	assert(ctx != NULL);

	ctx->loc.color_bg = color;
	INF("color: bg=%d", ctx->loc.color_fg);
}

void vt_fg_color_set(struct vt_ctx * ctx, enum vt_color_code color)
{
	assert(ctx != NULL);

	ctx->loc.color_fg = color;
	INF("color: fg=%d", ctx->loc.color_fg);
}

void vt_attr_clear(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.attr_bright = 0;
	ctx->loc.attr_dim = 0;
	ctx->loc.attr_hidden = 0;
	ctx->loc.attr_underline = 0;
	ctx->loc.attr_blink = 0;
	ctx->loc.attr_reverse= 0;
	INF("attr: clr");
}

void vt_attr_bright_set(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.attr_bright = 1;
	ctx->loc.attr_dim = 0;
	ctx->loc.attr_hidden = 0;
	ctx->loc.attr_underline = 0;
	ctx->loc.attr_blink = 0;
	ctx->loc.attr_reverse= 0;
	INF("attr: bright");
}

void vt_attr_dim_set(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.attr_bright = 0;
	ctx->loc.attr_dim = 1;
	ctx->loc.attr_hidden = 0;
	ctx->loc.attr_underline = 0;
	ctx->loc.attr_blink = 0;
	ctx->loc.attr_reverse= 0;
	INF("attr: dim");
}

void vt_attr_underline_set(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.attr_bright = 0;
	ctx->loc.attr_dim = 0;
	ctx->loc.attr_hidden = 0;
	ctx->loc.attr_underline = 1;
	ctx->loc.attr_blink = 0;
	ctx->loc.attr_reverse= 0;
}

void vt_attr_blink_set(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.attr_bright = 0;
	ctx->loc.attr_dim = 0;
	ctx->loc.attr_hidden = 0;
	ctx->loc.attr_underline = 0;
	ctx->loc.attr_blink = 1;
	ctx->loc.attr_reverse= 0;
}

void vt_attr_reverse_set(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.attr_bright = 0;
	ctx->loc.attr_dim = 0;
	ctx->loc.attr_hidden = 0;
	ctx->loc.attr_underline = 0;
	ctx->loc.attr_blink = 0;
	ctx->loc.attr_reverse = 1;
}

void vt_font_g0(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.font_g1 = 0;
}

void vt_font_g1(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.font_g1 = 1;
}

void vt_cursor_hide(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.cursor_hide = 1;
}

void vt_cursor_show(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	ctx->loc.cursor_hide = 0;
}

struct vt_size vt_ctx_size(struct vt_ctx * ctx)
{
	struct vt_size size;

	size.w = ctx->max.x - ctx->min.x;
	size.h = ctx->max.y - ctx->min.y;

	return size;
}

#if 0
void vt_attr_get(struct vt_ctx * ctx)
{

}

void vt_attr_set(struct vt_ctx * ctx)
{

}
int vt_push(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	if (ctx->open) {
		char s[8];
		int n = __vt_save(s);
		__vt_console_write(s, n);
	}
	return 0;
}

int vt_pop(struct vt_ctx * ctx)
{
	assert(ctx != NULL);

	if (ctx->open) {
		char s[8];
		int n = __vt_restore(s);
		__vt_console_write(s, n);
	}
	return 0;
}

#endif


# if 0
struct vt_ctx * __vt_win_open(struct vt_ctx * ctx)
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



