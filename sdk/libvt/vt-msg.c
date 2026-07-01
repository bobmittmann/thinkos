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
 * Local functions
 */

void __vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t arg) 
{
	uint32_t head;
	int slot;

	/* FIXME: check boundaries */
	head = __sys_vt.queue.head;
	slot = head % VT_MSG_QUEUE_SIZE;
	__sys_vt.queue.msg[slot] = msg;
	__sys_vt.queue.idx[slot] = __vt_idx_by_win(win);
	__sys_vt.queue.arg[slot] = arg;
	__sys_vt.queue.head = head + 1;

	if (head == __sys_vt.queue.tail)
		thinkos_console_io_break(CONSOLE_IO_RD);
}

static int __vt_msg_dispatch(struct vt_win * win, 
							 enum vt_msg msg, uintptr_t arg)
{
	switch (msg) {
	case VT_WIN_REFRESH:
	case VT_WIN_DRAW:
		if (!win->visible)
			return 0;
		if (win->child != 0) {
			struct vt_win * child;

			child = __vt_win_by_idx(win->child);
			__vt_msg_post(child, msg, arg);
			while (child->sibiling != 0) {
				child = __vt_win_by_idx(child->sibiling);
				__vt_msg_post(child, msg, arg);
			}
		}
		break;
	default:
		break;
	}

	__vt_unlock();
	win->msg_handler(win, msg, arg, win->data);
	return __vt_lock();
}

/* -------------------------------------------------------------------------
 * Message handler
 */

void vt_default_msg_handler(struct vt_win * win, enum vt_msg msg, 
							uint32_t arg, void * data)
{
	struct vt_ctx * ctx;

	switch (msg) {

	case VT_WIN_DRAW:
		ctx = vt_win_ctx_open(win);
		vt_clear(ctx);
		vt_ctx_close(ctx);
		break;

	case VT_TIMEOUT:
		if (__vt_lock() >= 0) {
			if (win->child != 0) {
				struct vt_win * child;

				child = __vt_win_by_idx(win->child);
				__vt_msg_post(child, msg, arg);
				while (child->sibiling != 0) {
					child = __vt_win_by_idx(child->sibiling);
					__vt_msg_post(child, msg, arg);
				}

			}
			__vt_unlock();
		}
	default:
		break;
	}
}


/* -------------------------------------------------------------------------
 * Message loop 
 */

int vt_default_msg_loop(unsigned int itv_ms)
{
	struct vt_console * con = &__sys_vt.con;
	uint32_t tmo_clk;
	uint32_t tail;
	enum vt_msg msg;
	uintptr_t arg;
	int connected;
	int ret;

	if ((ret = __vt_lock()) < 0)
		return ret;

	tail = __sys_vt.queue.tail;
	tmo_clk = thinkos_clock() + itv_ms;
	connected = console_is_connected();
	for(;;) {
		char buf[8];
		struct vt_win * win;
		int32_t tmo_ms;
		uint32_t clk ;

		if (tail != __sys_vt.queue.head) {
			int slot;
			int idx;

			/* process queued messages */
			slot = tail % VT_MSG_QUEUE_SIZE;
			msg = __sys_vt.queue.msg[slot];
			idx = __sys_vt.queue.idx[slot];
			arg = __sys_vt.queue.arg[slot];
			win = __vt_win_by_idx(idx);
			tail++;
			__sys_vt.queue.tail = tail;
			__vt_msg_dispatch(win, msg, arg);
			if  (msg == VT_QUIT) {
				ret = (int)arg;
				break;
			}
			continue;
		}

		clk = thinkos_clock();
		if ((tmo_ms = (int32_t)(tmo_clk - clk)) <= 0) {
			/* if the system was blocked, it will post 
			   as many messages as needed to catch up, 
			   honouring the clock rate */
			arg = tmo_clk;
			win = __vt_win_root();
			msg = VT_TIMEOUT;
			tmo_clk += itv_ms;
			__vt_msg_post(win, msg, arg);
			continue;
		} 
	
		__vt_unlock();
		ret = thinkos_console_timedread(buf, sizeof(buf), tmo_ms);
		__vt_lock();

		if (ret > 0) {
			int i;
			for (i = 0; i < ret; ++i) {
				int c = buf[i];
				if ((c = __vt_console_decode(con, c)) > 0) {
					switch (c) {

					case VT_CURSOR_POS: {
						struct vt_pos pos;

						pos.x = con->val[1];
						pos.y = con->val[0];
						YAP("VtLoop: pos = (%d, %d)", pos.x, pos.y);
						__sys_vt.cursor = pos;
					}
						break;

					case VT_TERM_TYPE: {
						int type = con->val[0];
						YAP("VtLoop: term = %d", type);
						__sys_vt.term_type = type;
					}
						break;

					default:
						arg = c;
						win = __vt_sys_win_focused();
						msg = VT_CHAR_RECV;
						__vt_msg_post(win, msg, arg);
					}
				}
			}
		} else {
			int flag;
			flag = console_is_connected();
			if (flag != connected) {
				connected = flag;
				DBGS("VtLoop: VT_TERM_STATUS");
				arg = flag;
				win = __vt_win_root();
				msg = VT_TERM_STATUS;
				__vt_msg_post(win, msg, arg);
			} 
			
			if (ret == THINKOS_ETIMEDOUT) {
				DBGS("VtLoop: timeout...");
			} else if (ret == THINKOS_EAGAIN){
				DBGS("VtLoop: again");
			} else if (ret == THINKOS_EINTR) {
				YAPS("VtLoop: intr");
			} else {
				YAP("VtLoop: read: %d", ret);
			}
		}

	} 

	__vt_unlock();

	return ret;
}


int vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t arg) 
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {
		 __vt_msg_post(win, msg, arg); 
		__vt_unlock();
	}

	return ret;
}

