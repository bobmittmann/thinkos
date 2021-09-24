/* parse-markup: reST */
  
/* 
 * vt.h
 *
 * Copyright(C) 2021 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the Vt library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/* 
 * @file vt.h
 * @brief VT API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __VT_H__
#define __VT_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <vt100.h>

/* -------------------------------------------------------------------------
 * Opaque User Interface Window */
struct vt_win;

#define VT_WIN(__WIN__) (struct vt_win *)(__WIN__)

/* -------------------------------------------------------------------------
 * User Interface Window handler */
typedef int vt_win_t;

enum vt_color_code {
	VT_COLOR_BLACK   = 0,
	VT_COLOR_RED     = 1,
	VT_COLOR_GREEN   = 2,
	VT_COLOR_YELLOW  = 3,
	VT_COLOR_BLUE    = 4,
	VT_COLOR_MAGENTA = 5,
	VT_COLOR_CYAN    = 6,
	VT_COLOR_WHITE   = 7
};

enum vt_attr_code {
	VT_ATTR_NORMAL    = 0,
	VT_ATTR_BRIGHT    = 1,
	VT_ATTR_DIM       = 2,
	VT_ATTR_UNDERLINE = 4,
	VT_ATTR_BLINK     = 5,
	VT_ATTR_REVERSE   = 7,
	VT_ATTR_HIDDEN    = 8
};

enum vt_msg {
	VT_QUIT = 0,
	VT_TIMEOUT = 1,
	VT_CHAR_RECV = 2,
	VT_WIN_REFRESH = 3,
	VT_WIN_DRAW = 4,
	VT_WIN_NCDRAW = 5,
	VT_WIN_CLEAR = 6,
	VT_WIN_DESTROY = 7,
	VT_WIN_CREATE = 8,
	VT_TERM_STATUS = 9,
	VT_MENU_ITEM_SELECT = 10,
	VT_MENU_OPEN = 11,
};

#define MK_VT_KEY(CODE)   (0x2000 + (CODE))
#define VT_CTRL           0x4000

#define VT_CURSOR_UP    MK_VT_KEY(0)
#define VT_CURSOR_DOWN  MK_VT_KEY(1)
#define VT_CURSOR_RIGHT MK_VT_KEY(2)
#define VT_CURSOR_LEFT  MK_VT_KEY(3)
#define VT_PAGE_UP      MK_VT_KEY(5)
#define VT_PAGE_DOWN    MK_VT_KEY(6)
#define VT_INSERT       MK_VT_KEY(7)
#define VT_DELETE       MK_VT_KEY(8)
#define VT_HOME         MK_VT_KEY(9)
#define VT_END          MK_VT_KEY(10)
#define VT_CURSOR_POS   MK_VT_KEY(11)
#define VT_TERM_TYPE    MK_VT_KEY(12)

#define VT_CTRL_CURSOR_UP    VT_CURSOR_UP + VT_CTRL 
#define VT_CTRL_CURSOR_DOWN  VT_CURSOR_DOWN + VT_CTRL   
#define VT_CTRL_CURSOR_RIGHT VT_CURSOR_RIGHT + VT_CTRL    
#define VT_CTRL_CURSOR_LEFT  VT_CURSOR_LEFT + VT_CTRL   
#define VT_CTRL_PAGE_UP      VT_PAGE_UP + VT_CTRL   
#define VT_CTRL_PAGE_DOWN    VT_PAGE_DOWN + VT_CTRL   


#define VT_POS(_X, _Y) __extension__({(struct vt_pos){ .x = _X, .y = _Y};})
#define VT_SIZE(_W, _H) __extension__({(struct vt_size){ .w = _W, .h = _H};})

struct vt_pos {
	union {
		uint16_t u16;
		struct {
			uint8_t x;
			uint8_t y;
		};
	};
};

struct vt_size {
	union {
		uint16_t u16;
		struct {
			uint8_t w;
			uint8_t h;
		};
	};
};

struct vt_rect {
	struct vt_pos pos;
	struct vt_size size;
};

struct vt_attrs {
	union {
		uint16_t u16;
		struct {
			uint8_t bg_color: 4;
			uint8_t fg_color: 4;
			uint8_t bright: 1;
			uint8_t dim: 1;
			uint8_t hidden: 1;
			uint8_t underline: 1;
			uint8_t blink: 1;
			uint8_t reverse: 1;
		};
	};
};

struct vt_win_def {
	struct vt_win * parent;
	struct vt_pos pos;
	struct vt_size size;
	struct vt_attrs attr;
};

struct vt_screen_def {
	struct vt_size size;
	struct vt_attrs attr;
};


typedef void (* vt_msg_handler_t)(struct vt_win *, enum vt_msg, 
								  uintptr_t, void *);

/* VT context */
struct vt_ctx;


#ifdef __cplusplus
extern "C" {
#endif

int vt_init(void);
int vt_reset(void);

void vt_clrscr(void);

void vt_screen_refresh(void);
void vt_screen_render(void);
void vt_quit(int retcode);

int vt_screen_init(const struct vt_screen_def * def,
				   vt_msg_handler_t msg_handler,
				   void * data);

int vt_screen_open(void);
int vt_screen_close(void);
void vt_screen_reset(void);
struct vt_size vt_screen_size(void);




struct vt_win * vt_win_alloc(void);

int vt_win_free(struct vt_win * win);

int vt_win_init(struct vt_win * win, 
				const struct vt_win_def * def,
				vt_msg_handler_t msg_handler,
				void * data);

struct vt_win * vt_win_create(const struct vt_win_def * def,
							  vt_msg_handler_t msg_handler,
							  void * data);


int vt_win_move(struct vt_win * win, int pos_x, int pos_y);
int vt_win_resize(struct vt_win * win, int width, int height);
struct vt_size vt_win_size(struct vt_win * win);
struct vt_pos vt_win_pos(struct vt_win * win);

void vt_win_hide(struct vt_win * win);
void vt_win_show(struct vt_win * win);
void vt_win_refresh(struct vt_win * win);
void vt_win_render(struct vt_win * win);
void vt_win_focus(struct vt_win * win);

int vt_win_puts(struct vt_win * win, const char * buf);

struct vt_win * vt_win_parent(struct vt_win * win);

#if 0
void vt_win_set_data(struct vt_win * win, void * val);
void vt_win_get_data(struct vt_win * win, void * val);
#endif

ssize_t vt_win_store(struct vt_win * win, const void * dat, size_t len);
ssize_t vt_win_recall(struct vt_win * win, void * dat, size_t len);

int vt_getc(unsigned int tmo);
int vt_getkey(unsigned int tmo);

char * vt_readline(char * buf, unsigned int max, unsigned int tmo);

FILE * vt_console_fopen(struct vt_win * win);

int vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t data);

enum vt_msg vt_msg_wait(struct vt_win ** win, uintptr_t * data);
enum vt_msg vt_msg_timedwait(struct vt_win ** win, uintptr_t * arg,
							unsigned int tmo_ms);
int vt_msg_dispatch(struct vt_win * win, enum vt_msg msg, uintptr_t data);

void vt_default_msg_handler(struct vt_win * win, enum vt_msg msg, 
							uint32_t arg, void * data);

int vt_default_msg_loop(unsigned int tmo_ms);


/* ------------------------------------------------------------------------- 
   Contexts ....
   Terminal Context
 */

/* get the context of the whole terminal */
struct vt_ctx * vt_root_ctx(void);
/* get the context of a rectangle inside the window */
struct vt_ctx * vt_win_rect_ctx(struct vt_win * win, struct vt_rect * rect);
/* get the context of the window frame */
struct vt_ctx * vt_win_frame_ctx(struct vt_win * win);

/* get and lock the context of the window */
struct vt_ctx * vt_win_ctx_open(struct vt_win * win);
/* unlock the context */
int vt_ctx_close(struct vt_ctx * ctx);

/* flush the terminal context buffer */
int vt_ctx_flush(struct vt_ctx * ctx);



/* print */
int vt_write(struct vt_ctx * ctx, const void * buf, unsigned int len);

int __attribute__((format(__printf__, 2, 3))) 
	vt_printf(struct vt_ctx * ctx, const char * fmt, ...);

int vt_putc(struct vt_ctx * ctx, int c);
int vt_puts(struct vt_ctx * ctx, const char * buf);

/* move */
void vt_move(struct vt_ctx * ctx, int x, int y);
void vt_cursor_home(struct vt_ctx * ctx);

/* move and print */
int __attribute__((format(__printf__, 4, 5))) 
	vt_mov_printf(struct vt_ctx * ctx, int x, int y, const char * fmt, ...);

int vt_mov_putc(struct vt_ctx * ctx, int x, int y, int c);
int vt_mov_puts(struct vt_ctx * ctx, int x, int y, const char * buf);


/* attributes ... */
void vt_cursor_hide(struct vt_ctx * ctx);
void vt_cursor_show(struct vt_ctx * ctx);

void vt_push(struct vt_ctx * ctx);
void vt_pop(struct vt_ctx * ctx);

void vt_bg_color_set(struct vt_ctx * ctx, enum vt_color_code);
void vt_fg_color_set(struct vt_ctx * ctx, enum vt_color_code);

void vt_attr_bright_set(struct vt_ctx * ctx);
void vt_attr_dim_set(struct vt_ctx * ctx);
void vt_attr_underline_set(struct vt_ctx * ctx);
void vt_attr_blink_set(struct vt_ctx * ctx);
void vt_attr_reverse_set(struct vt_ctx * ctx);
void vt_attr_hidden_set(struct vt_ctx * ctx);
void vt_attr_clear(struct vt_ctx * ctx);

void vt_font_g0(struct vt_ctx * ctx);
void vt_font_g1(struct vt_ctx * ctx);

/* semi-graphics */
void vt_clear(struct vt_ctx * ctx);

void vt_frame(struct vt_ctx *ctx, const char * title);

void vt_nc_frame(struct vt_ctx *ctx, const char * title);

int vt_hbar(struct vt_ctx * ctx, unsigned int y); 
 
/* Draw a frame with optional title */
void vt_rect_frame(struct vt_ctx *ctx, 
				   struct vt_rect rect, 
				   const char * title, bool hlight);

/* Draw a rectangle at position "pos" with size "size" */
void vt_rect(struct vt_ctx *ctx, struct vt_pos pos, struct vt_size size);

/* Draw an horizontal of size w starting at position (x, y)*/
int vt_hline(struct vt_ctx * ctx, struct vt_pos pos, int w);

/* Draw an horizontal line with T marks at each end */
void vt_hsplit(struct vt_ctx *ctx, int x, int y, int w);

/* Clear n positions in the current line starting at current position */
void vt_clear_ln(struct vt_ctx * ctx, unsigned int n);

struct vt_size vt_ctx_size(struct vt_ctx * ctx);

void vt_utf8_putc(struct vt_ctx * ctx, int c);

int vt_query_term(void);

#ifdef __cplusplus
}
#endif	

#endif /* __SYS_VT_H__ */

