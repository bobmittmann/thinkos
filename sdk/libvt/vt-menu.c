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
 * @file vt-menu.c
 * @brief
 * @author Bob Mittmann <bmittmann@mircomgroup.com>
 */

#include <thinkos.h>
#include <vt.h>
#include <vt/vt-menu.h>

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

#include <ctype.h>

#ifndef VT_MENU_LEVEL_MAX 
#define VT_MENU_LEVEL_MAX 4
#endif

struct vt_menu_state {
	struct vt_pos pos;
	uint8_t lvl;
	uint8_t sel[VT_MENU_LEVEL_MAX];
	const struct vt_menu_page * page[VT_MENU_LEVEL_MAX];
};

void vt_win_menu_render(struct vt_ctx * ctx, struct vt_menu_state * menu)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];
	struct vt_size size;
	
	size = vt_ctx_size(ctx);
	vt_frame(ctx, page->title);
	vt_hsplit(ctx, 0, size.h - 3, size.w);
}

void menu_item_render(struct vt_ctx * ctx, int x, int y,
					  const struct vt_menu_item * item)
{
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_attr_bright_set(ctx);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_attr_clear(ctx);
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s  ", item->text);
}

void menu_item_render_sel(struct vt_ctx * ctx, int x, int y,
						  const struct vt_menu_item * item)
{
	vt_attr_reverse_set(ctx);
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s  ", item->text);
}

void menu_item_render_submenu(struct vt_ctx * ctx, int x, int y,
					  const struct vt_menu_item * item)
{
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_attr_bright_set(ctx);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_attr_clear(ctx);
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s ", item->text);
	vt_putc(ctx, '>');
}

void menu_item_render_sel_submenu(struct vt_ctx * ctx, int x, int y,
						  const struct vt_menu_item * item)
{
	vt_attr_reverse_set(ctx);
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s ", item->text);
	vt_putc(ctx, '>');
}

typedef void (* vt_item_render)(struct vt_ctx * ctx, int x, int y,
								const struct vt_menu_item * item);


const vt_item_render render_lut[4] = {
	[VT_MENU_ITEM_RETCODE] = menu_item_render,
	[VT_MENU_ITEM_SUBMENU] = menu_item_render_submenu,
	[VT_MENU_ITEM_TOGGLE] = menu_item_render,
	[VT_MENU_ITEM_BACK] = menu_item_render
};

const vt_item_render render_sel_lut[4] = {
	[VT_MENU_ITEM_RETCODE] = menu_item_render_sel,
	[VT_MENU_ITEM_SUBMENU] = menu_item_render_sel_submenu,
	[VT_MENU_ITEM_TOGGLE] = menu_item_render_sel,
	[VT_MENU_ITEM_BACK] = menu_item_render_sel
};

void menu_desc_render(struct vt_ctx * ctx, int x, int y, const char * text)
{
	int n;

	if ((n = strlen(text)) > 0) {
		vt_mov_puts(ctx, x, y, text);
	} else {
		vt_move(ctx, x, y);
//		n = 0;
	}
	vt_clear_ln(ctx, n);
}

void vt_win_menu_refresh(struct vt_ctx * ctx, struct vt_menu_state * menu)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];
	const struct vt_menu_item * item = page->items;
	int sel = menu->sel[lvl];
	int max = page->nitems - 1;
	int x = menu->pos.x;
	int y = menu->pos.y;
	int i;

	for (i = 0; i <= max; ++i) {
		int kind = item[i].kind;
		
		if (i == sel)
			render_sel_lut[kind](ctx, x, y + i, &item[i]);
		else
			render_lut[kind](ctx, x, y + i, &item[i]);

		vt_attr_clear(ctx);
	}
}

#define VT_MENU_ITEM_TEXT_SZ    13 

typedef bool (* vt_switch_callback_t)(struct vt_win *, bool on, void *);

typedef void (* vt_item_select)(struct vt_win *win, 
								struct vt_menu_state * menu,
								const struct vt_menu_item * item);

void vt_menu_item_retcode(struct vt_win *win, 
						  struct vt_menu_state * menu,
						  const struct vt_menu_item * item)
{
	vt_quit(item->retcode);
}

void vt_menu_item_submenu(struct vt_win *win, 
						  struct vt_menu_state * menu,
						  const struct vt_menu_item * item)
{
	int lvl = menu->lvl;
	
	if (lvl < VT_MENU_LEVEL_MAX) {
		/* go up one level */
		menu->lvl = ++lvl;
		if (menu->page[lvl] != item->submenu) {
			/* update the level item */
			menu->page[lvl] = item->submenu;
			/* set the level selection to 0 (first item) */
			menu->sel[lvl] = 0;
		}
		vt_win_render(win);
		vt_win_refresh(win);
	}
}

void vt_menu_item_toggle(struct vt_win *win, 
						  struct vt_menu_state * menu,
						  const struct vt_menu_item * item)
{
		item->f_switch(win, true, NULL);
}

void vt_menu_item_back(struct vt_win *win, 
					   struct vt_menu_state * menu,
					   const struct vt_menu_item * item)
{
	if (menu->lvl > 0) {
		menu->lvl -= 1;
		vt_win_render(win);
		vt_win_refresh(win);
	}
}

const vt_item_select select_lut[4] = {
	[VT_MENU_ITEM_RETCODE] = vt_menu_item_retcode,
	[VT_MENU_ITEM_SUBMENU] = vt_menu_item_submenu,
	[VT_MENU_ITEM_TOGGLE] = vt_menu_item_toggle,
	[VT_MENU_ITEM_BACK] = vt_menu_item_back
};

void vt_win_menu_on_input(struct vt_win *win, 
						  struct vt_menu_state * menu, 
						  int c)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];
	int sel = menu->sel[lvl];
	int max = page->nitems - 1;
	const struct vt_menu_item * item = page->items;

	if ((c >= '0') && (c <= 'z')) {

		DBG("VtMenu: c=%c", c);

		int i;
		c = tolower(c);
		for (i = 0; i <= max; ++i) {
			if ((c == item[i].shortcut) && (sel != i)) { 
				/* New selection */
				menu->sel[lvl] = i;
				vt_win_refresh(win);
				break;
			}
		}	
		return;
	}

	switch (c) {
	case '\r': 
		select_lut[item[sel].kind](win, menu, &item[sel]);
		break;
	
	case VT_CURSOR_LEFT:
		vt_menu_item_back(win, menu, &item[sel]);
		break;

	case ' ': 
	case VT_CURSOR_RIGHT:
		if (item[sel].kind == VT_MENU_ITEM_SUBMENU)
			select_lut[item[sel].kind](win, menu, &item[sel]);
		break;

	case VT_CURSOR_UP:
		if (sel > 0) {
			menu->sel[lvl] = sel - 1;
			vt_win_refresh(win);
		}
		break;

	case VT_CURSOR_DOWN:
		if (sel < max) {
			menu->sel[lvl] = sel + 1;
			vt_win_refresh(win);
		}
		break;

	case VT_PAGE_UP:
		if (sel != 0) {
			menu->sel[lvl] = 0;
			vt_win_refresh(win);
		}
		break;

	case VT_PAGE_DOWN:
		if (sel != max) {
			menu->sel[lvl] = max;
			vt_win_refresh(win);
		}
		break;
	}
}

void vt_menu_msg_handler(struct vt_win *win, enum vt_msg msg,
					   uintptr_t arg, void * dat)
{
	struct vt_menu_state * menu = (struct vt_menu_state *)dat;
	struct vt_ctx * ctx;

	if (menu == NULL) {
		vt_default_msg_handler(win, msg, arg, dat);
		return;
	}

	/* get screen size */
	switch (msg) {
	case VT_CHAR_RECV:
		vt_win_menu_on_input(win, menu, arg);
		break;

	case VT_WIN_DRAW:
		DBG("VtMenu: VT_WIN_DRAW");

		ctx = vt_win_ctx_open(win);
		vt_win_menu_render(ctx, menu);
		vt_ctx_close(ctx);
		break;

	case VT_WIN_REFRESH:
		YAP("VtMenu: VT_WIN_REFRESH");
		ctx = vt_win_ctx_open(win);
		vt_win_menu_refresh(ctx, menu);
		vt_ctx_close(ctx);
		break;

	default:
		vt_default_msg_handler(win, msg, arg, dat);
	}
}

int vt_menu_select(struct vt_win_menu * win)
{
//	struct vt_win_def def */
	int ret = 0;

	return ret;
}

struct vt_win_menu * vt_win_menu_create(struct vt_pos pos, 
										struct vt_size size,
										struct vt_attrs attr, 
										const struct vt_menu_page * page)
{
	struct vt_menu_state menu;
	struct vt_win_def def;
	struct vt_win *win;

	def.pos = pos;
	def.size = size;
	def.attr = attr;
	def.parent = NULL;

	memset(&menu, 0, sizeof(menu));

	/* first page */
	menu.page[0] = page;
	menu.pos.x = 2;
	menu.pos.y = 2;

	win = vt_win_create(&def, vt_menu_msg_handler, NULL);

	vt_win_store(VT_WIN(win), &menu, sizeof(menu));


	return (struct vt_win_menu *)win;
}


