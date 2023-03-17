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
#define VT_MENU_LEVEL_MAX 5
#endif

const struct vt_item_flags_bmp vt_default_flags_bmp = {
	.inactive = 0, /* Do not display, invalid */
	.disabled = 0, /* Out-of-Service, gray out */
	.status = 0,   /* Present value */
	.error = 0     /* Error, fault or invalid condition */
};


struct vt_menu_state {
	struct vt_pos pos;
	uint8_t lvl;
	uint8_t sel[VT_MENU_LEVEL_MAX];
	const struct vt_menu_page * page[VT_MENU_LEVEL_MAX];
	void * data; 
};

static struct vt_item_flags_bmp * __page_flags(const struct vt_menu_page * page)
{
	return (page->flags != NULL) ? page->flags : 
		(struct vt_item_flags_bmp *)&vt_default_flags_bmp;
}

int __menu_select_up(const struct vt_menu_page * page, int sel)
{
	struct vt_item_flags_bmp * flags = __page_flags(page);
	int i;

	for (i = sel; i >= 0; --i) {
		if (!(flags->disabled & (1 << i))) {
			break;
		}
	}
	return i;
}

int __menu_select_down(const struct vt_menu_page * page, int sel)
{
	struct vt_item_flags_bmp * flags = __page_flags(page);
	int max = page->nitems - 1;
	int i;

	for (i = sel; i <= max; ++i) {
		if (!(flags->disabled & (1 << i))) {
			return i;
		}
	}
	return -1;
}

typedef uint32_t (* vt_item_get_value)(struct vt_win *win, 
								   struct vt_menu_state * menu,
								   const struct vt_menu_item * item);

uint32_t menu_item_get_value(struct vt_win *win, 
							 struct vt_menu_state * menu,
							 const struct vt_menu_item * item)
{
	return item->retcode;
};

uint32_t menu_item_submenu_get_value(struct vt_win *win, 
									 struct vt_menu_state * menu,
									 const struct vt_menu_item * item)
{
	const struct vt_menu_page * page = item->submenu;
	if (page->action == NULL)
		return 0;
	/* get selected item */
	return page->action(win, VT_MENU_GET_SELECT, 0, menu->data);
};

uint32_t menu_item_switch_get_value(struct vt_win *win, 
							   struct vt_menu_state * menu,
							   const struct vt_menu_item * item)
{
	return item->f_switch(win, VT_SWITCH_GET, menu->data);
};

const vt_item_get_value get_value_lut[5] = {
	[VT_MENU_ITEM_RETCODE] = menu_item_get_value,
	[VT_MENU_ITEM_SUBMENU] = menu_item_submenu_get_value,
	[VT_MENU_ITEM_LIST] = menu_item_submenu_get_value,
	[VT_MENU_ITEM_SWITCH] = menu_item_switch_get_value,
	[VT_MENU_ITEM_BACK] = menu_item_get_value
};

void vt_win_menu_render(struct vt_ctx * ctx, struct vt_menu_state * menu)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];
	struct vt_size size;
	
	size = vt_ctx_size(ctx);
	(void)size;
	vt_frame(ctx, page->title);
//	vt_hsplit(ctx, 0, size.h - 3, size.w);
}

void menu_item_render(struct vt_ctx * ctx, int x, int y,
					  const struct vt_menu_item * item, uint32_t value)
{
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_attr_bright_set(ctx);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_attr_clear(ctx);
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s  ", item->text);
}

void menu_item_render_sel(struct vt_ctx * ctx, int x, int y,
						  const struct vt_menu_item * item, uint32_t value)
{
	vt_attr_reverse_set(ctx);
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s  ", item->text);
}

void menu_item_render_submenu(struct vt_ctx * ctx, int x, int y,
					  const struct vt_menu_item * item, uint32_t value)
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
						  const struct vt_menu_item * item, uint32_t value)
{
	vt_attr_reverse_set(ctx);
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s ", item->text);
	vt_putc(ctx, '>');
}


void menu_item_list_render(struct vt_ctx * ctx, int x, int y,
						   const struct vt_menu_item * item, uint32_t value)
{
	const struct vt_menu_page * page = item->submenu;

	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_attr_bright_set(ctx);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_attr_clear(ctx);
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s ", item->text);
	vt_printf(ctx, "= %-15s ", page->items[value].text);
}

void menu_item_list_sel_render(struct vt_ctx * ctx, int x, int y,
								 const struct vt_menu_item * item, 
								 uint32_t value)
{
	const struct vt_menu_page * page = item->submenu;

	vt_attr_reverse_set(ctx);
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s ", item->text);
	vt_printf(ctx, "= %-15s ", page->items[value].text);
	vt_attr_clear(ctx);
}

void menu_item_switch_render(struct vt_ctx * ctx, int x, int y,
							 const struct vt_menu_item * item, 
							 uint32_t value)
{
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_attr_bright_set(ctx);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_attr_clear(ctx);
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s  ", item->text);

	if (value) {
		vt_printf(ctx, " [");
		vt_fg_color_set(ctx, VT_COLOR_BLACK);
		vt_printf(ctx, "--");
	//	vt_fg_color_set(ctx, VT_COLOR_GREEN);
		vt_fg_color_set(ctx, VT_COLOR_WHITE);
		vt_attr_bright_set(ctx);
		vt_printf(ctx, "<>");
		vt_attr_clear(ctx);
		vt_printf(ctx, "]");
	} else {					  
		vt_printf(ctx, " [");
		vt_attr_dim_set(ctx);
		vt_printf(ctx, "<>");
		vt_attr_clear(ctx);
		vt_fg_color_set(ctx, VT_COLOR_BLACK);
		vt_printf(ctx, "--");
		vt_fg_color_set(ctx, VT_COLOR_WHITE);
		vt_printf(ctx, "]");
	}

}

void menu_item_switch_sel_render(struct vt_ctx * ctx, int x, int y,
								 const struct vt_menu_item * item, 
								 uint32_t value)
{
	vt_attr_reverse_set(ctx);
	vt_fg_color_set(ctx, VT_COLOR_YELLOW);
	vt_mov_putc(ctx, x, y, toupper(item->shortcut));
	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	vt_mov_printf(ctx, x + 2, y, " %-15s  ", item->text);
	vt_attr_clear(ctx);

	vt_fg_color_set(ctx, VT_COLOR_WHITE);
	if (value) {
		vt_printf(ctx, " [");
		vt_fg_color_set(ctx, VT_COLOR_BLACK);
		vt_printf(ctx, "--");
		vt_fg_color_set(ctx, VT_COLOR_WHITE);
		vt_attr_reverse_set(ctx);
		vt_printf(ctx, "<>");
		vt_attr_clear(ctx);
		vt_fg_color_set(ctx, VT_COLOR_WHITE);
		vt_printf(ctx, "]");
	} else {					  
		vt_printf(ctx, " [");
		vt_fg_color_set(ctx, VT_COLOR_WHITE);
		vt_attr_reverse_set(ctx);
		vt_printf(ctx, "<>");
		vt_attr_clear(ctx);
		vt_fg_color_set(ctx, VT_COLOR_BLACK);
		vt_printf(ctx, "--");
		vt_fg_color_set(ctx, VT_COLOR_WHITE);
		vt_printf(ctx, "]");
	}
}


typedef void (* vt_item_render)(struct vt_ctx * ctx, int x, int y,
								const struct vt_menu_item * item,
								uint32_t value);


const vt_item_render render_lut[5] = {
	[VT_MENU_ITEM_RETCODE] = menu_item_render,
	[VT_MENU_ITEM_SUBMENU] = menu_item_render_submenu,
	[VT_MENU_ITEM_LIST] = menu_item_list_render,
	[VT_MENU_ITEM_SWITCH] = menu_item_switch_render,
	[VT_MENU_ITEM_BACK] = menu_item_render
};

const vt_item_render render_sel_lut[5] = {
	[VT_MENU_ITEM_RETCODE] = menu_item_render_sel,
	[VT_MENU_ITEM_SUBMENU] = menu_item_render_sel_submenu,
	[VT_MENU_ITEM_LIST] = menu_item_list_sel_render,
	[VT_MENU_ITEM_SWITCH] = menu_item_switch_sel_render,
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

void vt_win_menu_refresh(struct vt_win *win,
						 struct vt_menu_state * menu)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];
	const struct vt_menu_item * item = page->items;
	struct vt_item_flags_bmp * flags = __page_flags(page);
	int sel = menu->sel[lvl];
	int max = page->nitems - 1;
	int x = menu->pos.x;
	int y = menu->pos.y;
	struct vt_ctx * ctx;
	uint32_t value[max + 1];
	int i;

	(void)flags;

	for (i = 0; i <= max; ++i) {
		int kind = item[i].kind;
		value[i] = get_value_lut[kind](win, menu, &item[i]);
	}

	ctx = vt_win_ctx_open(win);
	for (i = 0; i <= max; ++i) {
		int kind = item[i].kind;
		if (!((1 << i) & flags->inactive)) {
			if (!((1 << i) & flags->disabled)) {
				if (i == sel)
					render_sel_lut[kind](ctx, x, y, &item[i], value[i]);
				else
					render_lut[kind](ctx, x, y, &item[i], value[i]);
			}
			y++;
			vt_attr_clear(ctx);
		}
	}
	vt_ctx_close(ctx);
}

#define VT_MENU_ITEM_TEXT_SZ    13 

typedef void (* vt_item_select)(struct vt_win *win, 
								struct vt_menu_state * menu,
								const struct vt_menu_item * item);

void vt_menu_item_retcode(struct vt_win *win, 
						  struct vt_menu_state * menu,
						  const struct vt_menu_item * item)
{
	vt_quit(item->retcode);
}

void vt_menu_item_submenu_select(struct vt_win *win, 
						  struct vt_menu_state * menu,
						  const struct vt_menu_item * item)
{
	int lvl = menu->lvl;
	
	DBG("VtSub: data=%08x", (uintptr_t)menu->data);

	if (lvl < VT_MENU_LEVEL_MAX) {
		const struct vt_menu_page * page;
		/* go up one level */
		menu->lvl = ++lvl;
		page = item->submenu;
		if (menu->page[lvl] != page) {
			/* set the selection to 0 (first item) */
			int sel = 0;
			/* update the level item */
			menu->page[lvl] = page;
			if (page->action != NULL)
				sel = page->action(win, VT_MENU_GET_SELECT, 0, menu->data);
			/* set the level selection */
			menu->sel[lvl] = sel;
		}
		vt_win_render(win);
		vt_win_refresh(win);
	}
}

void vt_menu_item_switch_select(struct vt_win *win, 
								struct vt_menu_state * menu,
								const struct vt_menu_item * item)
{
	item->f_switch(win, VT_SWITCH_TOGGLE, menu->data);
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

const vt_item_select select_lut[5] = {
	[VT_MENU_ITEM_RETCODE] = vt_menu_item_retcode,
	[VT_MENU_ITEM_SUBMENU] = vt_menu_item_submenu_select,
	[VT_MENU_ITEM_LIST] = vt_menu_item_submenu_select,
	[VT_MENU_ITEM_SWITCH] = vt_menu_item_switch_select,
	[VT_MENU_ITEM_BACK] = vt_menu_item_back
};
		
void vt_win_menu_on_item_select(struct vt_win *win, 
						  struct vt_menu_state * menu, 
						  int code)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];

	if (page->action != NULL)
		page->action(win, VT_MENU_SET_SELECT, code, menu->data);
}


void vt_win_menu_on_input(struct vt_win *win, 
						  struct vt_menu_state * menu, 
						  int c)
{
	int lvl = menu->lvl;
	const struct vt_menu_page * page = menu->page[lvl];
	const struct vt_menu_item * item = page->items;
	struct vt_item_flags_bmp * flags = __page_flags(page);
	int sel = menu->sel[lvl];
	int max = page->nitems - 1;

	(void)flags;

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
		if (item[sel].kind == VT_MENU_ITEM_SWITCH) {
			item[sel].f_switch(win, VT_SWITCH_RST, menu->data);
		} else {
			vt_menu_item_back(win, menu, &item[sel]);
		}
		break;

	case ' ': 
	case VT_CURSOR_RIGHT:
		if (item[sel].kind == VT_MENU_ITEM_SUBMENU)
			select_lut[item[sel].kind](win, menu, &item[sel]);
		else if (item[sel].kind == VT_MENU_ITEM_SWITCH) {
				item[sel].f_switch(win, VT_SWITCH_SET, menu->data);
			}
		break;

	case VT_CURSOR_UP:
		if ((sel = __menu_select_up(page, sel - 1)) >= 0) {
			menu->sel[lvl] = sel;
			vt_win_refresh(win);
			vt_msg_post(win, VT_MENU_ITEM_SELECT, sel); 
		}
		break;

	case VT_CURSOR_DOWN:
		if ((sel = __menu_select_down(page, sel + 1)) >= 0) {
			menu->sel[lvl] = sel;
			vt_win_refresh(win);
			vt_msg_post(win, VT_MENU_ITEM_SELECT, sel); 
		}
		break;

	case VT_PAGE_UP:
		if ((sel = __menu_select_down(page, 0)) >= 0) {
			menu->sel[lvl] = sel;
			vt_win_refresh(win);
			vt_msg_post(win, VT_MENU_ITEM_SELECT, sel); 
		}
		break;

	case VT_PAGE_DOWN:
		if ((sel = __menu_select_up(page, max)) >= 0) {
			menu->sel[lvl] = sel;
			vt_win_refresh(win);
			vt_msg_post(win, VT_MENU_ITEM_SELECT, sel); 
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
		vt_win_menu_refresh(win, menu);
		break;

	case VT_MENU_ITEM_SELECT:
		vt_win_menu_on_item_select(win, menu, arg);
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
										const struct vt_menu_page * page,
										void * data)
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
	menu.data = data;

	
	/* Static sanity check: */
	_Static_assert (sizeof(struct vt_menu_state) <= 32,
					"vt_menu_state too large");

	DBG("VtCreate: data=%08x size=%d", (uintptr_t)menu.data, sizeof(menu));

	win = vt_win_create(&def, vt_menu_msg_handler, NULL);

	vt_win_store(VT_WIN(win), &menu, sizeof(menu));

	return (struct vt_win_menu *)win;
}


