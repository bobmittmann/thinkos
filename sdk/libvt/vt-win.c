#include "vt-i.h"
#include <sys/null.h>

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

/* -------------------------------------------------------------------------
 * Local functions
 */

static int __vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t arg) 
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

	return thinkos_console_io_break(CONSOLE_IO_RD);
}

/* -------------------------------------------------------------------------
 * API functions
 */

#if 0
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
#endif

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
		if (win->child != 0) {
			if (__vt_lock() >= 0) {
				struct vt_win * child;

				child = __vt_win_by_idx(win->child);
				vt_msg_post(child, msg, arg);
				while (child->sibiling != 0) {
					child = __vt_win_by_idx(child->sibiling);
					vt_msg_post(child, msg, arg);
				}

				__vt_unlock();
			}
		}
	default:
		break;
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


int vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t arg) 
{
	int ret;

	if ((ret = __vt_lock()) >= 0) {
		ret = __vt_msg_post(win, msg, arg); 
		__vt_unlock();
	}

	return ret;
}

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


int vt_msg_dispatch(struct vt_win * win, enum vt_msg msg, uintptr_t arg)
{
	switch (msg) {
	case VT_WIN_REFRESH:
	case VT_WIN_DRAW:
		if (!win->visible)
			return 0;
		if (win->child != 0) {
			struct vt_win * child;

			child = __vt_win_by_idx(win->child);
			vt_msg_post(child, msg, arg);
			while (child->sibiling != 0) {
				child = __vt_win_by_idx(child->sibiling);
				vt_msg_post(child, msg, arg);
			}
		}
		break;
	default:
		break;
	}

	win->msg_handler(win, msg, arg, win->data);

	return 0;
}


int vt_default_msg_loop(unsigned int itv_ms)
{
	uint32_t tmo_clk;
	uint32_t tail;
	enum vt_msg msg;
	uintptr_t arg;

	tail = __sys_vt.queue.tail;
	tmo_clk = thinkos_clock() + itv_ms;

	do {
		char buf[1];
		struct vt_win * win;
		int ret = 0;
		int slot;


		while (tail == __sys_vt.queue.head) {

			uint32_t clk = thinkos_clock();
			int32_t tmo_ms;
			if ((tmo_ms = (int32_t)(tmo_clk - clk)) < 0) {


				/* if the system was blocked, it will dispatch 
				   as many messages as needed to catch up, 
				   honouring the clock rate */
				arg = tmo_clk;
				win = __vt_win_root();
				msg = VT_TIMEOUT;
				vt_msg_dispatch(win, msg, arg);
				tmo_clk += itv_ms;
			} else if ((ret = thinkos_console_timedread(buf, 1, tmo_ms)) > 0) {
				int c;
				if ((c = __vt_console_decode(&__sys_vt.con, buf[0])) > 0) {
					arg = c;
					win = __vt_sys_win_focused();
					msg = VT_CHAR_RECV;
					vt_msg_dispatch(win, msg, arg);
				}
			} else {
			}
		}

		/* process queued messages */
		slot = tail % VT_MSG_QUEUE_SIZE;
		msg = __sys_vt.queue.msg[slot];
		win = __vt_win_by_idx(__sys_vt.queue.idx[slot]);
		arg = __sys_vt.queue.arg[slot];
		tail++;
		__sys_vt.queue.tail = tail;

		vt_msg_dispatch(win, msg, arg);

	} while (msg != VT_QUIT);

	return (int)arg;
}


/* -------------------------------------------------------------------------
 * VT console
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

