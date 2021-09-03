#include "vt-i.h"

struct sys_vt_rt __sys_vt;

/* -------------------------------------------------------------------------
 * */

union vt_mem_blk * __vt_alloc(void)
{
	union vt_mem_blk * p;

	p = __sys_vt.alloc.free.first;

	if ((p = __sys_vt.alloc.free.first) != NULL) {
		if ((__sys_vt.alloc.free.first = p->next) == NULL)
			__sys_vt.alloc.free.last = 
				(union vt_mem_blk *)&__sys_vt.alloc.free.first;

		__sys_vt.alloc.used++;
	}

	return p;
}

int __vt_free(void * __p)
{
	union vt_mem_blk * p  = (union vt_mem_blk *)__p; 
	union vt_mem_blk * q;

	q = __sys_vt.alloc.free.last;
	q->next = p;
	__sys_vt.alloc.free.last = p;
	p->next = NULL;
	__sys_vt.alloc.used--; /* update statistics */

	return 0;
}

static int __vt_core_reset(void)
{
	union vt_mem_blk * p;
	union vt_mem_blk * q;
	int i;

	__sys_vt.alloc.used = 0;
	__sys_vt.alloc.max = VT_MEM_BLK_POOL_SIZE;

	/* create a linked list of memory blocks */
	q = &__sys_vt.alloc.blk[1];
	p = (union vt_mem_blk *)&__sys_vt.alloc.free.first;
	for (i = 1; i < VT_MEM_BLK_POOL_SIZE; i++) {
		p->next = q;
		p = q;
		q++;
	}

	p->next = NULL;
	__sys_vt.alloc.free.last = p;
	 
	__sys_vt.queue.head = 0;
	__sys_vt.queue.tail = 0;

	return thinkos_sem_init(__sys_vt.sem, 0);
}

union vt_mem_blk * __vt_blk_next(union vt_mem_blk * blk) 
{
	union vt_mem_blk * nxt = NULL;
	int idx;

	idx = __vt_idx_by_blk(blk);

	if (++idx < __sys_vt.alloc.used)
		nxt = __vt_blk_by_idx(idx);

	return nxt;
}

int vt_reset(void)
{
	int ret;

	if ((ret = thinkos_mutex_lock(__sys_vt.mutex)) < 0)
		return ret;

	ret = __vt_core_reset();

	thinkos_mutex_unlock(__sys_vt.mutex);

	return ret;
}


int vt_init(void)
{
	int ret;

	if (__sys_vt.mutex == 0) {
		if ((ret = thinkos_mutex_alloc()) < 0)
			return ret;
		__sys_vt.mutex = ret;
	}

	if (__sys_vt.sem == 0) {
		if ((ret = thinkos_sem_alloc(0)) < 0)
			return ret;
		__sys_vt.sem = ret;
	}

	thinkos_mutex_lock(__sys_vt.mutex);

	ret = __vt_core_reset();

	thinkos_mutex_unlock(__sys_vt.mutex);

	return ret;
}

void vt_quit(int retcode)
{
	vt_msg_post(__vt_win_root(), VT_QUIT, (uintptr_t)retcode);
}

