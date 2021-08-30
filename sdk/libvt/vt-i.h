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


#define VT_ATTR_FG_COLOR(_VT_COLOR_) (30 + (_VT_COLOR_))
#define VT_ATTR_BG_COLOR(_VT_COLOR_) (40 + (_VT_COLOR_))

/* -------------------------------------------------------------------------
 * VT Window */

struct vt_win {
	uint8_t parent;
	uint8_t sibiling;
	uint8_t child;
	struct {
		uint8_t visible: 1;
		uint8_t has_focus: 1;
		uint8_t cursor_hide: 1;
		uint8_t data_alloc: 1;
	};

	void (* msg_handler)(struct vt_win * win, enum vt_msg msg, 
						 uintptr_t arg, void * data);
	
	struct vt_attrs attr;
	struct vt_pos pos;
	
	struct vt_size size;
	struct vt_pos cursor;

	void * data;
};

union vt_mem_blk {
	struct {
		union vt_mem_blk * next;
	};
	struct vt_win win;
	struct {
		uint32_t data[5]; /* 20 bytes */
	};
};

/* ----------------------------------------------------------------------------
 * Console input decode 
 */

#define VT_CON_MODE_RAW      0
#define VT_CON_MODE_ESC      1
#define VT_CON_MODE_ESC_O    2
#define VT_CON_MODE_ESC_VAL  3

#define VT_CON_VAL_LST_SZ 2

struct vt_console {
	uint8_t mode;
	uint8_t idx;
	uint8_t val[VT_CON_VAL_LST_SZ];
};

typedef int (* vt_key_decode_t)(struct vt_console * con, int c);

extern const vt_key_decode_t __vt_con_decode_by_mode[];

static inline int __vt_console_decode(struct vt_console * con, int c) {
	return __vt_con_decode_by_mode[con->mode](con, c);
}

/* ----------------------------------------------------------------------------
 * Windows
 */

#ifndef VT_MEM_BLK_POOL_SIZE
#define VT_MEM_BLK_POOL_SIZE  16
#endif

#ifndef VT_MSG_QUEUE_SIZE
#define VT_MSG_QUEUE_SIZE 16
#endif

#define VT_STATE_ATTR_MSK 0x0000003f
#define VT_STATE_POS_MSK  0xffff0000

struct vt_state {
	union {
		uint32_t u32;	
		struct {
			uint32_t attr_bright: 1;
			uint32_t attr_dim: 1;
			uint32_t attr_hidden: 1;
			uint32_t attr_underline: 1;

			uint32_t attr_blink: 1;
			uint32_t attr_reverse: 1;

			uint32_t color_bg: 3;
			uint32_t color_fg: 3;

			uint32_t cursor_hide: 1;
			uint32_t insert_off: 1;
			uint32_t font_g1: 1;
			uint32_t res1: 1;

			uint32_t pos_x: 8;
			uint32_t pos_y: 8;
		};
	};
};

/* ----------------------------------------------------------------------------
 * Drawing context
 */

struct vt_ctx {
	struct vt_state loc; /* local/virtual/cached display state */ 
	struct vt_state rem; /* remote display state */ 
	struct vt_pos min;
	struct vt_pos max;
};

struct sys_vt_rt {
	uint8_t mutex;
	uint8_t sem;
	struct {
		uint8_t has_focus;
	} cache;
	struct {
		uint8_t msg[VT_MSG_QUEUE_SIZE];
		uint8_t idx[VT_MSG_QUEUE_SIZE];
		uintptr_t arg[VT_MSG_QUEUE_SIZE];
		volatile uint32_t head;
		volatile uint32_t tail;
	} queue;
	struct vt_console con;
	struct vt_ctx ctx;
	struct {
		uint8_t max;
		uint8_t used;
		struct {
			union vt_mem_blk * first;
			union vt_mem_blk * last;
		} free;
		union vt_mem_blk blk[VT_MEM_BLK_POOL_SIZE];
		/* block meta info */
		uint8_t meta[VT_MEM_BLK_POOL_SIZE];
	} alloc;
};

extern struct sys_vt_rt __sys_vt;


/* ----------------------------------------------------------------------------
 * Memory pool
 */

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

inline static struct vt_win * __vt_sys_win_focused(void) {
	return &__vt_blk_by_idx(__sys_vt.cache.has_focus)->win;
}

inline static void __vt_sys_cache_focused(struct vt_win * win) {
	__sys_vt.cache.has_focus = __vt_idx_by_win(win);
}

inline static struct vt_win * __vt_win_sibiling(struct vt_win * win) {
	return (win->sibiling == 0) ? NULL : __vt_win_by_idx(win->sibiling);
}

inline static struct vt_win * __vt_win_parent(struct vt_win * win) {
	return (win->parent == 0) ? NULL : __vt_win_by_idx(win->parent);
}

inline static struct vt_win * __vt_win_child(struct vt_win * win) {
	return (win->child == 0) ? NULL : __vt_win_by_idx(win->child);
}


inline static struct vt_ctx * __vt_ctx(void) {
	return &__sys_vt.ctx;
}

/* ----------------------------------------------------------------------------
 * screen lock 
 */

inline static int __vt_lock(void) {
	return thinkos_mutex_lock(__sys_vt.mutex);
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

int __vt_console_write(const char * s, unsigned int len);

int __vt_puts(const char * s);

int __vt_putc(int c);

int __vt_strcpyn(char * dst, const char * src, unsigned int len);

int __vt_set_scroll(char * s, int y0, int y1);


int __vt_set_attr_lst(char * s, uint8_t attr[], int len);

int __vt_attr_fg_color(char * s, int color);

int __vt_attr_bg_color(char * s, int color);

int __vt_attr_add(char * s, int attr);

int __vt_set_bg_color(char * s, int color);

int __vt_set_fg_color(char * s, int color);

int __vt_set_font_mode(char * s, int mode);

int __vt_set_attr(char * s, int attr);

int __vt_clear_attr(char * s);

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

void __vt_win_open(struct vt_win * win);

void __vt_win_close(struct vt_win * win);

int __vt_font_g0(char * s);

int __vt_font_g1(char * s);

int __vt_lock(void);

void __vt_unlock(void);

int __vt_console_decode(struct vt_console * con, int c);



void __vt_ctx_prepare(struct vt_ctx * ctx, struct vt_win * win);

void __vt_ctx_save(struct vt_ctx * ctx, struct vt_win * win);

/* prepare a context ... */
int __vt_ctx_close(struct vt_ctx * ctx);

int __vt_ctx_write(struct vt_ctx * ctx, const void * buf, unsigned int len);

#ifdef __cplusplus
}
#endif	

#endif /* __VT_I_H__ */

