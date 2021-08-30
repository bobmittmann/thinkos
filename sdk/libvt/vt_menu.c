/* vt_menu.c 
 * ---------
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
 * @file vtmon.c
 * @brief
 * @author Bob Mittmann <bmittmann@mircomgroup.com>
 */

#include <thinkos.h>
#include <sys/vt.h>

#include "vt_menu.h"

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

struct vt_menu {
	uint8_t selected;
	uint8_t count;
	uint8_t help_y;
	uint8_t help_w;
	const char * title;
	const struct vt_menu_entry * item;
};

void vt_menu_render(struct vt_menu * menu, struct vt_ctx * ctx)
{
	struct vt_size size;

	size = vt_ctx_size(ctx);
	vt_frame(ctx, menu->title);
	vt_hsplit(ctx, 0, size.h - 3, size.w);
	menu->help_y = size.h - 2;
	menu->help_w = size.w - 4;
}

void vt_menu_refresh(struct vt_menu * menu, struct vt_ctx * ctx)
{
	const struct vt_menu_entry * item = menu->item;
	int sel = menu->selected;
	int i;
	int n;

	for (i = 0; i < menu->count; ++i) {
		int y = i * 1 + 1;
		
		if (i == sel) {
			vt_attr_reverse_set(ctx);
			vt_fg_color_set(ctx, VT_COLOR_YELLOW);
//			vt_attr_bright_set(ctx);
			vt_mov_putc(ctx, 3, y, item[i].key);
//			vt_attr_clear(ctx);
			vt_fg_color_set(ctx, VT_COLOR_WHITE);
			vt_mov_printf(ctx, 5, y, " %-15s ", item[i].tag);
//			vt_attr_dim_set(ctx);
//			vt_mov_puts(ctx, 20, y, item->desc);
		} else {
			vt_fg_color_set(ctx, VT_COLOR_YELLOW);
			vt_attr_bright_set(ctx);
			vt_mov_putc(ctx, 3, y, item[i].key);
			vt_attr_clear(ctx);
			vt_fg_color_set(ctx, VT_COLOR_WHITE);
			vt_mov_printf(ctx, 5, y, " %-15s ", item[i].tag);
//			vt_attr_dim_set(ctx);
	//		vt_mov_puts(ctx, 20, y, item->desc);
		}
		vt_attr_clear(ctx);
	}

	n = strlen(item[sel].desc);
	n = menu->help_y - n;

	vt_mov_puts(ctx, 2, menu->help_y, item[sel].desc);
	vt_clear_ln(ctx, n);
	
}

bool __vt_menu_shortcut_sel(struct vt_win *win, 
						   struct vt_menu * menu, int c)
{
	const struct vt_menu_entry * item = menu->item;
	int i;

	for (i = 0; i < menu->count; ++i) {
		if (c == item->key) {
			menu->selected = i;
			return true;
		}
		item++;
	}	

	return false;
}

void vt_menu_msg_handler(struct vt_win *win, enum vt_msg msg,
					   uintptr_t arg, void * dat)
{
	struct vt_menu * menu = (struct vt_menu *)dat;
	struct vt_ctx * ctx;

	/* get screen size */
	switch (msg) {
	case VT_CHAR_RECV: {
		int c = arg;

		if ((c & 0x2000) == 0) {
			if (__vt_menu_shortcut_sel(win, menu, c)) {
				vt_win_refresh(win);
				break;
			}
		}

		switch (c) {

		case '\r':
			break;

		case VT_CURSOR_UP:
			if (menu->selected > 0) {
				menu->selected -= 1;
				vt_win_refresh(win);
			}
			break;

		case VT_CURSOR_DOWN:
			if (menu->selected < menu->count) {
				menu->selected += 1;
				vt_win_refresh(win);
			}
			break;

		case VT_PAGE_UP:
			if (menu->selected != 0) {
				menu->selected = 0;
				vt_win_refresh(win);
			}
			break;

		case VT_PAGE_DOWN:
			if (menu->selected == menu->count - 1) {
				menu->selected = menu->count - 1;
				vt_win_refresh(win);
			}
			break;
		}
		break;
	}

	case VT_WIN_DRAW:
		ctx = vt_win_ctx_open(win);
		vt_menu_render(menu, ctx);
		vt_ctx_close(ctx);
		break;

	case VT_WIN_REFRESH:
		ctx = vt_win_ctx_open(win);
		vt_menu_refresh(menu, ctx);
		vt_ctx_close(ctx);
		break;

	default:
		vt_default_msg_handler(win, msg, arg, dat);
	}
}

int vt_menu_init(struct vt_win_menu * win, const struct vt_menu_def * def)
{
	struct vt_menu menu;
	//	vt_win_set_data(win, def);

	menu.selected = 0;
	menu.count = def->len;
	menu.item = def->entry;
	menu.title = def->title;

	vt_win_store(VT_WIN(win), &menu, sizeof(menu));

	return 0;
}

int vt_menu_select(struct vt_win_menu * win)
{
//	struct vt_win_def def */
	int ret = 0;

	return ret;
}

struct vt_win_menu * vt_menu_create(struct vt_pos pos, 
								 struct vt_size size,
								 struct vt_attrs attr)
{
	struct vt_win_def def;
	struct vt_win *win;

	def.pos = pos;
	def.size = size;
	def.attr = attr;
	def.parent = NULL;

	win = vt_win_create(&def, vt_menu_msg_handler, NULL);
	return (struct vt_win_menu *)win;
}


