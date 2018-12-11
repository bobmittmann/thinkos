#include "vt-i.h"

/* -------------------------------------------------------------------------
 * */

void __vt_win_open(struct vt_win * win)
{
	char s[128];
	char * cp;
	int n;

	win->open = 1;
	cp = s;
	cp += __vt_save(cp);
	cp += __vt_insert_off(cp);
	cp += __vt_set_bg_color(cp, win->bg_color);
	cp += __vt_set_fg_color(cp, win->fg_color);
	cp += __vt_set_font_mode(cp, win->attr);
	if (win->cursor_hide)
		cp += __vt_cursor_hide(cp);
	else
		cp += __vt_cursor_show(cp);
	cp += __vt_set_scroll(cp, win->pos.y, win->pos.y + win->size.h);
	cp += __vt_move_to(cp, win->pos.x + win->cursor.x, 
					   win->pos.y + win->cursor.y);
	n = cp - s;
	__vt_console_write(s, n);
}

void __vt_win_close(struct vt_win * win)
{
	char s[64];
	char * cp;
	int n;

	cp = s;
	cp += __vt_restore(cp);
	n = cp - s;
	__vt_console_write(s, n);

	win->open = 0;
}

int __vt_win_write(struct vt_win * win, 
				   const void * buf, unsigned int len) 
{
	const char scroll[] = "\n";
	char s[12];
	char * cp;
	int rem;
	int cnt;
	int x;
	int y;

	x = win->cursor.x;
	y = win->cursor.y;
	cp = (char *)buf;
	rem = len;
	cnt = 0;

	if (win->size.w == 0 || win->size.h == 0) {
		/* window is closed (0) in at least one dimension */
		return cnt;
	}

	while (rem > 0) {
		int k;
		int w;
		int n;
		int i;

		w = win->size.w - x;
		if (w == 0) {
			if (y < win->size.h) {
				y++;
			} else {
				__vt_console_write(scroll, sizeof(scroll) - 1);
			}
			x = 0;
			w = win->size.w;
			k = __vt_move_to(s, win->pos.x, win->pos.y + y);
			__vt_console_write(s, k);
		}

		/* search for next '\n' or '\r' */
		n = MIN(rem, w);
		for (i = 0; (i < n) && (cp[i] != '\n') && (cp[i] != '\r'); ++i);

		__vt_console_write(cp, i);

		if (cp[i] == '\n') {
			if (y < win->size.h) {
				y++;
			} else {
				__vt_console_write(scroll, sizeof(scroll) - 1);
			}

			x = 0;
			k = __vt_move_to(s, win->pos.x, win->pos.y + y);
			__vt_console_write(s, k);
			/* skip '\n' */
			i++;
		} else if (cp[i] == '\r') {
			x = 0;
			k = __vt_move_to(s, win->pos.x, win->pos.y + y);
			__vt_console_write(s, k);
			/* skip '\r' */
			i++;
		} else {
			x += i;
		}
		rem -= i;
		cnt += i;
		cp += i;
	};

	win->cursor.x = x;
	win->cursor.y = y;

	return cnt;
}

/* -------------------------------------------------------------------------
 * */

int vt_win_open(struct vt_win * win)
{
	__vt_lock();
	__vt_win_open(win);
	return 0;
}

int vt_win_close(struct vt_win * win)
{
	__vt_win_close(win);
	__vt_unlock();
	return 0;
}

int __vt_win_drain(void * dev)
{
	while (thinkos_console_drain() != 0);
	return 0;
}

static const struct fileop __win_fops = {
	.write = (void *)__vt_win_write,
	.read = (void *)NULL,
	.flush = (void *)__vt_win_drain,
	.close = (void *)NULL
};

int vt_printf(struct vt_win * win, const char * fmt, ...)
{
	struct file f = {
		.data = (void *)win,
		.op = &__win_fops 
	};
	va_list ap;
	int cnt;

	assert(win != NULL);
	assert(fmt != NULL);

	va_start(ap, fmt);
	cnt = vfprintf(&f, fmt, ap);
	va_end(ap);

	return cnt;
}

int vt_write(struct vt_win * win, const void * buf, unsigned int len) 
{
	int cnt;

	assert(win != NULL);
	assert(buf != NULL);

	cnt = __vt_win_write(win, buf, len); 

	return cnt;
}

const char __hbar[] = "----------------------------------------"; 

int vt_hbar(struct vt_win * win, unsigned int y) 
{
	char s[16];
	char * cp;
	int rem;

	assert(win != NULL);

	cp = s;
	cp += __vt_move_to(cp, win->pos.x, win->pos.y + y);
	__vt_console_write(s, cp - s);

	rem = win->size.w;
	while (rem) {
		int n;
		n = MIN(rem, sizeof(__hbar) - 1);
		__vt_console_write(__hbar, n);
		rem -= n;
	}

	return 0;
}

int vt_cursor_move(struct vt_win * win, int x, int y)
{
	assert(win != NULL);

	if (x > win->size.w)
		x = win->size.w;
	if (y > win->size.h)
		y = win->size.h;

	win->cursor.x = x;
	win->cursor.y = y;

	if (win->open) {
		char s[16];
		int n = n = __vt_move_to(s, win->pos.x + x, win->pos.y + y);
		__vt_console_write(s, n);
	}
	return 0;
}

void vt_cursor_home(struct vt_win * win)
{
	assert(win != NULL);

	win->cursor.x = 0;
	win->cursor.y = 0;

	if (win->open) {
		char s[32];
		int n = __vt_move_to(s, win->pos.x, win->pos.y);
		__vt_console_write(s, n);
	}
}

int vt_bg_color_set(struct vt_win * win, enum vt_color color)
{
	assert(win != NULL);

	win->bg_color = color;
	if (win->open) {
		char s[16];
		int n = __vt_set_bg_color(s, win->bg_color);
		__vt_console_write(s, n);
	}
	return 0;
}

int vt_fg_color_set(struct vt_win * win, enum vt_color color)
{
	assert(win != NULL);

	win->fg_color = color;
	if (win->open) {
		char s[8];
		int n = __vt_set_bg_color(s, win->fg_color);
		__vt_console_write(s, n);
	}
	return 0;
}

int vt_font_attr_set(struct vt_win * win, enum vt_attr attr)
{
	assert(win != NULL);

	win->attr = attr;
	if (win->open) {
		char s[8];
		int n = __vt_set_font_mode(s, win->attr);
		__vt_console_write(s, n);
	}
	return 0;
}

int vt_cursor_hide(struct vt_win * win)
{
	assert(win != NULL);

	win->cursor_hide = 1;
	if (win->open) {
		char s[8];
		int n = __vt_cursor_hide(s);
		__vt_console_write(s, n);
	}
	return 0;
}

int vt_cursor_show(struct vt_win * win)
{
	assert(win != NULL);

	win->cursor_hide = 0;
	if (win->open) {
		char s[8];
		int n = __vt_cursor_show(s);
		__vt_console_write(s, n);
	}
	return 0;
}

const char __vt_clrln[] = "                            "
	"                                                  "
	"                                                  "; 

void vt_win_clear(struct vt_win * win)
{
	char s[32];
	char * cp;
	int n;
	int i;

	for (i = 0; i < win->size.h; ++i) {
		n = __vt_move_to(s, win->pos.x, win->pos.y + i);
		__vt_console_write(s, n);

		n = win->size.w;
		cp = (char *)__vt_clrln;
		__vt_console_write(cp, n);
	}
}

struct vt_win * vt_win_alloc(void) 
{
	union vt_mem_blk * blk;
	
	if ((blk = __vt_alloc()) == NULL) {
	}

	return &blk->win;
}

int vt_win_free(struct vt_win * win) 
{
	return __vt_free(win);
}

int vt_win_refresh(struct vt_win * win) 
{
	return __vt_free(win);

}

struct vt_pos vt_win_pos(struct vt_win * win)
{
	struct vt_win * parent = __vt_win_by_idx(win->parent);
	struct vt_pos pos;

	pos.x = win->pos.x - parent->pos.x;
	pos.y = win->pos.y - parent->pos.y;

	return pos;
}

struct vt_size vt_win_size(struct vt_win * win)
{
	return win->size;
}

void vt_default_msg_handler(struct vt_win * win, enum vt_msg msg, 
						 uint32_t arg, void * data)
{
	switch (msg) {
	case VT_WIN_CREATE:
		vt_win_open(win);
		vt_win_clear(win);
		vt_win_close(win);
		break;
	default:
		break;
	}
}

int vt_win_init_ext(struct vt_win * win, const struct vt_win_def * def)
{
	struct vt_win * parent;
	struct vt_win * child;
	int x0;
	int y0;
	int x1;
	int y1;

	/* Update tree */
	win->parent = __vt_idx_by_win(def->parent);
	parent = __vt_win_by_idx(win->parent);
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

	win->fg_color = def->fg_color;
	win->bg_color = def->bg_color;
	win->attr = def->attr;

	win->cursor.x = 0;
	win->cursor.y = 0;
	win->cursor_hide = 0;
	win->msg_handler = (def->msg_handler == NULL) ? vt_default_msg_handler :
		def->msg_handler;
	win->data = def->data;

	return 0;
}

struct vt_win * vt_win_create(const struct vt_win_def * def)
{
	union vt_mem_blk * blk;
	struct vt_win * win;
	
	if ((blk = __vt_alloc()) == NULL)
		return NULL;

	win = &blk->win;
	memset(win, 0, sizeof(struct vt_win));

	vt_win_init_ext(win, def);
	vt_msg_post(win, VT_WIN_CREATE, 0);

	return win;
}

void vt_msg_post(struct vt_win * win, enum vt_msg msg, uintptr_t arg)
{
	uint32_t head;
	int pos;

	__vt_lock();

	head = __sys_vt.queue.head;
	pos = head % VT_MSG_QUEUE_SIZE;
	__sys_vt.msg[pos] = msg;
	__sys_vt.idx[pos] = __vt_idx_by_win(win);
	__sys_vt.arg[pos] = arg;
	__sys_vt.queue.head = head + 1;

	thinkos_console_io_break(CONSOLE_IO_RD);

	__vt_unlock();
}

static int __vt_on_console_recv(int c)
{
	return c;	
}

enum vt_msg vt_msg_wait(struct vt_win ** win, uintptr_t * arg)
{
	enum vt_msg msg;
	uint32_t tail;
	char buf[1];
	int tmo = 100;
	int ret = 0;
	int pos;

	tail = __sys_vt.queue.tail;
	while (tail == __sys_vt.queue.head) {
		if ((ret = thinkos_console_timedread(buf, 1, tmo)) < 0) {
			arg = NULL;
			*win = __vt_win_root();
			return VT_TIMEOUT;
		} else if (ret > 0) {
			int c;
			if ((c = __vt_on_console_recv(buf[0])) > 0) {
				*arg = c;
				*win = __vt_win_root();
				return VT_CHAR_RECV;
			}
		}
	}

	pos = tail % VT_MSG_QUEUE_SIZE;
	msg = __sys_vt.msg[pos];
	*win  = __vt_win_by_idx(__sys_vt.idx[pos]);
	*arg = __sys_vt.arg[pos];
	__sys_vt.queue.tail = tail + 1;

	return msg;
}

int vt_msg_dispatch(struct vt_win * win, enum vt_msg msg, uintptr_t arg)
{
	switch (msg) {
	case VT_WIN_REFRESH:
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

int vt_default_msg_loop(void)
{
	struct vt_win * win;
	enum vt_msg msg;
	uintptr_t arg = 0;

	while ((msg = vt_msg_wait(&win, &arg)) != VT_QUIT) {
		vt_msg_dispatch(win, msg, arg);
	}

	return (int)arg;
}

