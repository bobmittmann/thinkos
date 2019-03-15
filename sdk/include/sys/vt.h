/* 
 * File:	 sys/vt.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __SYS_VT_H__
#define __SYS_VT_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <vt100.h>

/* -------------------------------------------------------------------------
 * Opaque User Interface Window */
struct vt_win;

enum vt_color {
	VT_COLOR_BLACK   = 0,
	VT_COLOR_RED     = 1,
	VT_COLOR_GREEN   = 2,
	VT_COLOR_YELLOW  = 3,
	VT_COLOR_BLUE    = 4,
	VT_COLOR_MAGENTA = 5,
	VT_COLOR_CYAN    = 6,
	VT_COLOR_WHITE   = 7
};

enum vt_attr {
	VT_ATTR_NORMAL    = 0,
	VT_ATTR_BRIGHT    = 1,
	VT_ATTR_DIM       = 2,
	VT_ATTR_UNDERLINE = 3,
	VT_ATTR_BLINK     = 4,
	VT_ATTR_REVERSE   = 5,
	VT_ATTR_HIDDEN    = 6
};

enum vt_msg {
	VT_QUIT = 0,
	VT_TIMEOUT = 1,
	VT_CHAR_RECV = 2,
	VT_WIN_REFRESH = 16,
	VT_WIN_DESTROY = 17,
	VT_WIN_CREATE = 18
};

struct vt_pos {
	uint8_t x;
	uint8_t y;
};

struct vt_size {
	uint8_t w;
	uint8_t h;
};

struct vt_win_def {
	struct vt_win * parent;
	struct vt_pos pos;
	struct vt_size size;
	uint8_t fg_color;
	uint8_t bg_color;
	uint8_t attr;
	void * data;
	void (* msg_handler)(struct vt_win * win, enum vt_msg msg, 
						  uint32_t arg, void * data);
};

struct vt_screen_def {
	uint8_t fg_color;
	uint8_t bg_color;
	uint8_t attr;
	struct vt_size size;
	void * data;
	void (* msg_handler)(struct vt_win * win, enum vt_msg msg, 
						  uint32_t arg, void * data);
};


#ifdef __cplusplus
extern "C" {
#endif

void vt_init(void);
void vt_clrscr(void);
void vt_refresh(void);
void vt_quit(int retcode);

int vt_screen_init(const struct vt_screen_def * def);
int vt_screen_open(void);
int vt_screen_close(void);
struct vt_size vt_screen_size(void);

struct vt_win * vt_win_alloc(void);
int vt_win_free(struct vt_win * win);

int vt_win_init(struct vt_win * win, int pos_x, int pos_y, 
				int width, int height);

int vt_win_move(struct vt_win * win, int pos_x, int pos_y);
int vt_win_resize(struct vt_win * win, int width, int height);
struct vt_size vt_win_size(struct vt_win * win);
struct vt_pos vt_win_pos(struct vt_win * win);

int vt_win_hide(struct vt_win * win);
int vt_win_show(struct vt_win * win);
int vt_win_refresh(struct vt_win * win);
void vt_win_clear(struct vt_win * win);
int vt_win_open(struct vt_win * win);
int vt_win_close(struct vt_win * win);

int vt_bg_color_set(struct vt_win * win, enum vt_color);
int vt_fg_color_set(struct vt_win * win, enum vt_color);
int vt_font_attr_set(struct vt_win * win, enum vt_attr);

int vt_write(struct vt_win * win, const void * buf, unsigned int len); 

int __attribute__((format(__printf__, 2, 3))) vt_printf(struct vt_win * win, const char * fmt, ...);

int vt_cursor_move(struct vt_win * win, int x, int y);

int vt_cursor_hide(struct vt_win * win);

void vt_cursor_home(struct vt_win * win);

int vt_cursor_show(struct vt_win * win);

int vt_hbar(struct vt_win * win, unsigned int y); 

int vt_getc(unsigned int tmo);

FILE * vt_console_fopen(struct vt_win * win);

int vt_win_init_ext(struct vt_win * win, const struct vt_win_def * def);

void vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t data);

enum vt_msg vt_msg_wait(struct vt_win ** win, uintptr_t * data);
int vt_msg_dispatch(struct vt_win * win, enum vt_msg msg, uintptr_t data);

void vt_default_msg_handler(struct vt_win * win, enum vt_msg msg, 
							uint32_t arg, void * data);

int vt_default_msg_loop(void);

struct vt_win * vt_win_create(const struct vt_win_def * def);

char * vt_freadline(FILE *f, char * buf, unsigned int max);

#ifdef __cplusplus
}
#endif	

#endif /* __SYS_VT_H__ */

