/* 
 * File:	 vt-i.h
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

#ifndef __VT_I_H__
#define __VT_I_H__

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>
#include <assert.h>
#include <string.h>
#include <sys/console.h>
#include <thinkos.h>
#include <sys/vt.h>
#include <vt100.h>

#define IN_BS      '\x8'
#define IN_TN_BS     0x7F /* TELNET back space */
#define IN_EOL      '\r'
#define IN_SKIP     '\3'
#define IN_EOF      '\x1A'
#define IN_ESC      '\033'


#define OUT_CURSOR_LEFT     "\x8"
#define OUT_BS              "\x8 \x8"
#define OUT_SKIP            "^C"
#define OUT_BEL             "\7"

#define MODE_RAW 0
#define MODE_ESC 1
#define MODE_ESC_VAL1 2
#define MODE_ESC_VAL2 3
#define MODE_ESC_O 4

/* -------------------------------------------------------------------------
 * VT Window */
struct vt_win {
	uint8_t parent;
	uint8_t sibiling;
	uint8_t child;
	uint8_t fg_color:4;
	uint8_t bg_color:4;

	struct vt_pos pos;
	struct vt_size size;

	struct vt_pos cursor;

	uint16_t attr: 6;
	uint16_t cursor_hide: 1;
	uint16_t open: 1;
	uint16_t visible: 1;

	void (* msg_handler)(struct vt_win * win, enum vt_msg msg, uintptr_t arg, 
						  void * data);
	void * data;
};

union vt_mem_blk {
	struct {
		union vt_mem_blk * next;
	};
	struct vt_win win;
};

struct vt_console {
	uint8_t mode;
	uint16_t ctrl;
	uint32_t val;
};

#ifndef VT_WIN_POOL_SIZE
#define VT_WIN_POOL_SIZE  16
#endif

#ifndef VT_MSG_QUEUE_SIZE
#define VT_MSG_QUEUE_SIZE 16
#endif

struct sys_vt_rt {
	uint8_t mutex;
	uint8_t msg[VT_MSG_QUEUE_SIZE];
	uint8_t idx[VT_MSG_QUEUE_SIZE];
	uintptr_t arg[VT_MSG_QUEUE_SIZE];
	struct {
		volatile uint32_t head;
		volatile uint32_t tail;
	} queue;
	struct vt_console con;
	struct {
		uint8_t max;
		uint8_t used;
		struct {
			union vt_mem_blk * first;
			union vt_mem_blk * last;
		} free;
		union vt_mem_blk blk[VT_WIN_POOL_SIZE];
	} alloc;
};

extern struct sys_vt_rt __sys_vt;

inline static union vt_mem_blk * __vt_blk_by_idx(unsigned int idx) {
	return &__sys_vt.alloc.blk[idx];
}

inline static unsigned int __vt_idx_by_blk(union vt_mem_blk * blk) {
	return blk <= __sys_vt.alloc.blk ? 0 : blk - __sys_vt.alloc.blk;
}

inline static struct vt_win * __vt_win_by_idx(unsigned int idx) {
	return &__vt_blk_by_idx(idx)->win;
}

inline static unsigned int __vt_idx_by_win(struct vt_win * win) {
	union vt_mem_blk * blk = (union vt_mem_blk *)win;
	return __vt_idx_by_blk(blk);
}

inline static struct vt_win * __vt_win_root(void) {
	return &__vt_blk_by_idx(0)->win;
}

inline static void __vt_lock(void) {
	thinkos_mutex_lock(__sys_vt.mutex);
}

inline static void __vt_unlock(void) {
	thinkos_mutex_unlock(__sys_vt.mutex);
}

#ifdef __cplusplus
extern "C" {
#endif

int __vt_move_to(char * s, int x, int y);

union vt_mem_blk * __vt_alloc(void);

int __vt_free(void * __p);

void __vt_console_write(const char * s, unsigned int len);

void __vt_puts(const char * s);

int __vt_strcpyn(char * dst, const char * src, unsigned int len);

int __vt_set_scroll(char * s, int y0, int y1);

int __vt_set_bg_color(char * s, int color);

int __vt_set_fg_color(char * s, int color);

int __vt_set_font_mode(char * s, int mode);

int __vt_move_to(char * s, int x, int y);

int __vt_insert_on(char * s);

int __vt_insert_off(char * s);

int __vt_insert_line(char * s);

int __vt_delete_line(char * s);

int __vt_delete_chars(char * s, int n);

int __vt_clrscr(char * s);

int __vt_clreol(char * s);

int __vt_save(char * s);

int __vt_restore(char * s);

int __vt_get_cursor_pos(struct vt_pos * pos);

int __vt_cursor_up(char * s);

int __vt_cursor_down(char * s);

int __vt_cursor_right(char * s);

int __vt_cursor_left(char * s);

int __vt_cursor_show(char * s);

int __vt_cursor_hide(char * s);

int __vt_getc(void);

int __vt_reset(char * s);

int __vt_win_drain(void * dev);

void __vt_win_open(struct vt_win * win);

void __vt_win_close(struct vt_win * win);

int __vt_win_write(struct vt_win * win, 
				   const void * buf, unsigned int len);

void __vt_lock(void);

void __vt_unlock(void);

int __vt_console_decode(struct vt_console * con, int c);

#ifdef __cplusplus
}
#endif	

#endif /* __VT_I_H__ */

