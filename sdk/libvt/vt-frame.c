#include "vt-i.h"
#include <sys/null.h>

#define TRACE_LEVEL TRACE_LVL_NONE
#include <trace.h>

/* -------------------------------------------------------------------------
 * */
void vt_rect_frame(struct vt_ctx *ctx, 
				   struct vt_rect rect, 
				   const char * title, bool hlight) 
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

	INF("frame initial: pos=%d,%d font_g1=%d", ctx->loc.pos_x, 
		ctx->loc.pos_y, ctx->rem.font_g1);

	len = strlen(title);
	n = (w - 4) - len;
	if (n >= 0) {
		int m;
		int j;
		int i;

		vt_move(ctx, x, y);
		vt_font_g1(ctx);
		cp = buf;
		*cp++ = 'l';
		if (len > 0) {
			m = n / 2;
			n = n - m;

			for (j = 0; j < m ; ++j)
				*cp++ = 'q';
			*cp++ = 'u';
			*cp = '\0';
			vt_puts(ctx, buf);

			vt_font_g0(ctx);
			if (hlight)
				vt_attr_reverse_set(ctx);
			vt_puts(ctx, title);
			if (hlight)
				vt_attr_dim_set(ctx);
//				vt_attr_clear(ctx);

			vt_font_g1(ctx);
			cp = buf;
			*cp++ = 't';
		} else {
			n += 2;
		}
		for (j = 0; j < n ; ++j)
			*cp++ = 'q';
		*cp++ = 'k';
		*cp = '\0';
		vt_puts(ctx, buf);

		for (i = 1; i < (h - 1); ++i) {
			vt_move(ctx, x, y + i);
			cp = buf;
			*cp++ = 'x';
			for (j = 0; j < (w - 2) ; ++j)
				*cp++ = ' ';
			*cp++ = 'x';
			*cp = '\0';
			vt_puts(ctx, buf);
		}

		cp = buf;
		vt_move(ctx, x, y + i);
		*cp++ = 'm';
		for (j = 0; j < (w - 2) ; ++j)
			*cp++ = 'q';
		*cp++ = 'j';
		*cp = '\0';
		vt_puts(ctx, buf);
		vt_font_g0(ctx);
	}

	INF("frame final: pos=%d,%d font_g1=%d", ctx->loc.pos_x, 
		ctx->loc.pos_y, ctx->rem.font_g1);
}

void vt_nc_frame(struct vt_ctx *ctx, const char * title)
{
	struct vt_rect rect;

	rect.pos.x = -1;
	rect.pos.y = -1;
	rect.size.w = ctx->max.x - ctx->min.x + 2;
	rect.size.h = ctx->max.y - ctx->min.y + 2;

	vt_rect_frame(ctx, rect, title, false);
}


void vt_frame(struct vt_ctx *ctx, const char * title)
{
	struct vt_rect rect;

	rect.pos.x = 0;
	rect.pos.y = 0;
	rect.size.w = ctx->max.x - ctx->min.x;
	rect.size.h = ctx->max.y - ctx->min.y;

	vt_rect_frame(ctx, rect, title, false);
}

void vt_hsplit(struct vt_ctx *ctx, int x, int y, int w)
{
	char buf[128];
	char * cp;
	int n;
	
	n = ctx->max.x - ctx->min.x - x;
	if (n > w)
		n = w;

	if (n > 0) {
		int j;

		vt_move(ctx, x, y);
		vt_font_g1(ctx);
		cp = buf;
		*cp++ = 't';
		if (n > 2) {
			for (j = 0; j < n - 2; ++j)
				*cp++ = 'q';
		}

		if (n > 1)
			*cp++ = 'u';

		*cp = '\0';
		vt_puts(ctx, buf);
		vt_font_g0(ctx);
	}
}


