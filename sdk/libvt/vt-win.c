/* parse-markup: reST */
  
/* 
 * vt-win.c
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
 * @file vt-win.c
 * @brief VT API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "vt-i.h"
#include <sys/null.h>

#define TRACE_LEVEL TRACE_LVL_NONE
#include <trace.h>

/* -------------------------------------------------------------------------
 * Windowing API functions
 */

struct vt_win * vt_win_alloc(void) 
{
	struct vt_win * win = NULL;
	union vt_mem_blk * blk;
	
	if (__vt_lock() >= 0) {
		if ((blk = __vt_alloc()) == NULL) {
			win = &blk->win;
		}
		__vt_unlock();
	}

	return win;
}

int vt_win_free(struct vt_win * win) 
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {
		if (win->data_alloc) {
			__vt_free(win->data);
		}
		ret = __vt_free(win);
		__vt_unlock();
	}

	return ret;
}

void vt_win_render(struct vt_win * win)
{
	if (__vt_lock() >= 0) {
		__vt_msg_post(win, VT_WIN_DRAW, 0);
		__vt_unlock();
	}
}

void vt_win_refresh(struct vt_win * win) 
{
	if (__vt_lock() >= 0) {
		__vt_msg_post(win, VT_WIN_REFRESH, 0);
		__vt_unlock();
	}
}

struct vt_pos vt_win_pos(struct vt_win * win)
{
	struct vt_win * parent = __vt_win_by_idx(win->parent);
	struct vt_pos pos;

	pos.x = win->pos.x - parent->pos.x;
	pos.y = win->pos.y - parent->pos.y;

	return pos;
}

struct vt_win * vt_win_parent(struct vt_win * win)
{
	struct vt_win * parent = __vt_win_by_idx(win->parent);

	return parent;
}

struct vt_size vt_win_size(struct vt_win * win)
{
	return win->size;
}

#if 0
void vt_win_set_data(struct vt_win * win, void * val)
{
	uint32_t * src = (uint32_t *)val;
	uint32_t * dst = (uint32_t *)win->data;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}
#endif

ssize_t vt_win_store(struct vt_win * win, const void * dat, size_t len)
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {
		if (win->data == NULL) {
			if (win->data_alloc == 0) {
				union vt_mem_blk * blk;

				blk = __vt_alloc();
				win->data = blk;
				win->data_alloc = 1;
			} 
			len = (len > sizeof(union vt_mem_blk)) ? 
				sizeof(union vt_mem_blk) : len;
		} else { 
			if (win->data_alloc) {
				len = (len > sizeof(union vt_mem_blk)) ? 
					sizeof(union vt_mem_blk) : len;
			} 
		}
		memcpy(win->data, dat, len);
		__vt_unlock();
		ret = len;
	}

	return ret;
}

ssize_t vt_win_recall(struct vt_win * win, void * dat, size_t len)
{
	if (win->data == NULL)
		return 0;

	if (win->data_alloc) {
		len = (len > sizeof(union vt_mem_blk)) ? 
			sizeof(union vt_mem_blk) : len;
	} 

	memcpy(dat, win->data, len);

	return len;
}


void vt_win_show(struct vt_win * win)
{
	if (__vt_lock() >= 0) {
		if (!win->visible) {
			win->visible = true;
			__vt_msg_post(win, VT_WIN_DRAW, 0);
		}
		__vt_unlock();
	}
}

void vt_win_focus(struct vt_win * win)
{
	struct vt_win * w;
	struct vt_win * p;

	if (__vt_lock() >= 0) {
		if (!win->has_focus) {

			w = __vt_win_root();
			w->has_focus = false;
		
			p = w;
			w = __vt_win_child(p);
			while (w != NULL) {
				w->has_focus = false;
				w = __vt_win_sibiling(w);
			}

			win->has_focus = 1;
/*			win->cursor_hide = 0; */
			__vt_sys_cache_focused(win);
			if (win->visible) {
				__vt_msg_post(win, VT_WIN_DRAW, 0);
			}
		}	
	
		__vt_unlock();
	}
}

void vt_win_hide(struct vt_win * win)
{
	if (__vt_lock() >= 0) {
			if (win->visible) {
				struct vt_win *wroot = __vt_win_root();
				win->visible = false;
				__vt_msg_post(wroot, VT_WIN_DRAW, 0);
			}
		__vt_unlock();
	}
}


int __vt_win_init(struct vt_win * win, 
				  const struct vt_win_def * def,
				  vt_msg_handler_t msg_handler,
				  void * data)
{
	struct vt_win * parent;
	struct vt_win * child;
	int x0;
	int y0;
	int x1;
	int y1;

	/* Update tree */
	if ((parent = def->parent) == NULL)
			parent = __vt_win_root();
	win->parent = __vt_idx_by_win(parent);

	if (parent->child == 0)
		parent->child = __vt_idx_by_win(win);
	else {
		child = __vt_win_by_idx(parent->child);
		while (child->sibiling != 0)
			child = __vt_win_by_idx(child->sibiling);
		child->sibiling = __vt_idx_by_win(win);
	}
	win->child = 0;
	win->sibiling = 0;

	x0 = parent->pos.x + def->pos.x;
	y0 = parent->pos.y + def->pos.y;
	if (x0 > (parent->pos.x + parent->size.w))
		x0 = parent->pos.x + parent->size.w;
	if (y0 > (parent->pos.y + parent->size.h))
		y0 = parent->pos.y + parent->size.h;

	x1 = x0 + def->size.w;
	if (x1 > (parent->pos.x + parent->size.w))
		x1 = parent->pos.x + parent->size.w;
	y1 = y0 + def->size.h;
	if (y1 > (parent->pos.y + parent->size.h))
		y1 = parent->pos.y + parent->size.h;

	win->pos.x = x0;
	win->pos.y = y0;
	win->size.w = x1 - x0;
	win->size.h = y1 - y0;

	win->attr = def->attr;

	win->cursor.x = 0;
	win->cursor.y = 0;
	win->cursor_hide = 1;
	win->visible = 0;
	win->has_focus = 0;
	win->data_alloc = 0;

	win->msg_handler = (msg_handler == NULL) ? vt_default_msg_handler :
		msg_handler;
	win->data = data;

	return 0;
}

int vt_win_init(struct vt_win * win, 
				const struct vt_win_def * def,
				vt_msg_handler_t msg_handler,
				void * data)
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {
		__vt_win_init(win, def, msg_handler, data);
		__vt_unlock();
	}

	return ret;
}

struct vt_win * vt_win_create(const struct vt_win_def * def,
				vt_msg_handler_t msg_handler,
				void * data)
{
	struct vt_win * win = NULL;

	if (__vt_lock() >= 0) {
		union vt_mem_blk * blk;

		if ((blk = __vt_alloc()) != NULL) {
			win = &blk->win;
			memset(win, 0, sizeof(struct vt_win));

			__vt_win_init(win, def, msg_handler, data);
			__vt_msg_post(win, VT_WIN_CREATE, 0);
		}

		__vt_unlock();
	}

	return win;
}



/* -------------------------------------------------------------------------
 * Miscellaneous
 */

int vt_query_term(void)
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {
		__vt_console_write(VT100_QUERY_DEVICE_CODE, 
						   sizeof(VT100_QUERY_DEVICE_CODE));
		__vt_unlock();
	}

	return ret;
}



/* -------------------------------------------------------------------------
 * VT console stream (file)
 */

static int __vt_win_fwrite(struct vt_win * win, 
						   const void * buf, unsigned int len) 
{
	struct vt_ctx * ctx = __vt_ctx();
	int cnt;
	int ret;

	if ((ret = __vt_lock()) < 0) {
		WARNS("__vt_lock failed!" );
		return ret;
	}

	if (win->visible) {
		__vt_ctx_prepare(ctx, win);

		cnt = __vt_ctx_write(ctx, buf, len); 

		__vt_ctx_save(ctx, win);
	} else {
		cnt = len;
	}

	__vt_unlock();

	return cnt;
}

static int __vt_win_drain(struct vt_win * win)
{
	struct vt_ctx * ctx = __vt_ctx();
	int ret;

	if ((ret = __vt_lock()) < 0) {
		WARNS("__vt_lock failed!" );
		return ret;
	}

	__vt_ctx_prepare(ctx, win);

	while (thinkos_console_drain() != 0);

	__vt_ctx_close(ctx);

	__vt_unlock();

	return 0;
}

const struct fileop vt_win_fops = {
	.write = (int (*)(void *, const void *, size_t))__vt_win_fwrite,
	.read = (int (*)(void *, void *, size_t, unsigned int))null_read,
	.flush = (int (*)(void *))__vt_win_drain,
	.close = (int (*)(void *))null_close
};

struct file vt_console_file;

FILE * vt_console_fopen(struct vt_win * win)
{
	struct file * f = &vt_console_file;

	f->data = (void *)win;
	f->op = &vt_win_fops;

	return f;
}

#if 0

/* -------------------------------------------------------------------------
 * Obsolete / deprecated
 */


enum vt_msg vt_msg_wait(struct vt_win ** win, uintptr_t * arg)
{
	enum vt_msg msg;
	uint32_t tail;
	char buf[1];
	int tmo = 100;
	int ret = 0;
	int slot;

	tail = __sys_vt.queue.tail;
	while (tail == __sys_vt.queue.head) {
		if ((ret = thinkos_console_timedread(buf, 1, tmo)) < 0) {
			arg = NULL;
			*win = __vt_win_root();
			return VT_TIMEOUT;
		} else if (ret > 0) {
			int c;
			if ((c = __vt_console_decode(&__sys_vt.con, buf[0])) > 0) {
				*arg = c;
				*win = __vt_win_root();
				return VT_CHAR_RECV;
			}
		}
	}

	slot = tail % VT_MSG_QUEUE_SIZE;
	msg = __sys_vt.queue.msg[slot];
	*win  = __vt_win_by_idx(__sys_vt.queue.idx[slot]);
	*arg = __sys_vt.queue.arg[slot];
	__sys_vt.queue.tail = tail + 1;

	return msg;
}

enum vt_msg vt_msg_timedwait(struct vt_win ** win, uintptr_t * arg, 
							unsigned int tmo)
{
	enum vt_msg msg;
	uint32_t tail;
	char buf[1];
	int ret = 0;
	int slot;

	tail = __sys_vt.queue.tail;
	while (tail == __sys_vt.queue.head) {
		if ((ret = thinkos_console_timedread(buf, 1, tmo)) < 0) {
			arg = NULL;
			*win = __vt_win_root();
			return VT_TIMEOUT;
		} else if (ret > 0) {
			int c;

			if ((c = __vt_console_decode(&__sys_vt.con, buf[0])) > 0) {
				*arg = c;
				*win = __vt_win_root();
				return VT_CHAR_RECV;
			}
		}
	}

	slot = tail % VT_MSG_QUEUE_SIZE;
	msg = __sys_vt.queue.msg[slot];
	*win  = __vt_win_by_idx(__sys_vt.queue.idx[slot]);
	*arg = __sys_vt.queue.arg[slot];
	__sys_vt.queue.tail = tail + 1;

	return msg;
}

int vt_win_open(struct vt_win * win)
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {

		if (win->visible)
			__vt_win_open(win);
	}

	return ret;
}

int vt_win_close(struct vt_win * win)
{
	if (win->open)
		__vt_win_close(win);

	__vt_unlock();
	return 0;
}

void vt_win_set_data(struct vt_win * win, void * val)
{
	uint32_t * src = (uint32_t *)val;
	uint32_t * dst = (uint32_t *)win->data;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void vt_win_get_data(struct vt_win * win, void * val)
{
	uint32_t * src = (uint32_t *)win->data;
	uint32_t * dst = (uint32_t *)val;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}
#endif

