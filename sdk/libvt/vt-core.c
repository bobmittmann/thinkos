#include "vt-i.h"

struct sys_vt_rt __sys_vt;

/* -------------------------------------------------------------------------
 * */

union vt_mem_blk * __vt_alloc(void)
{
	union vt_mem_blk * p;

	thinkos_mutex_lock(__sys_vt.mutex);

	p = __sys_vt.alloc.free.first;

	if ((p = __sys_vt.alloc.free.first) != NULL) {
		if ((__sys_vt.alloc.free.first = p->next) == NULL)
			__sys_vt.alloc.free.last = 
				(union vt_mem_blk *)&__sys_vt.alloc.free.first;

		__sys_vt.alloc.used++;
	}

	thinkos_mutex_unlock(__sys_vt.mutex);

	return p;
}

int __vt_free(void * __p)
{
	union vt_mem_blk * p  = (union vt_mem_blk *)__p; 
	union vt_mem_blk * q;

	thinkos_mutex_lock(__sys_vt.mutex);

	q = __sys_vt.alloc.free.last;
	q->next = p;
	__sys_vt.alloc.free.last = p;
	p->next = NULL;
	__sys_vt.alloc.used--; /* update statistics */

	thinkos_mutex_unlock(__sys_vt.mutex);

	return 0;
}

void vt_init(void)
{
	union vt_mem_blk * p;
	union vt_mem_blk * q;
	int i;

	if (__sys_vt.mutex == 0) {
		__sys_vt.mutex = thinkos_mutex_alloc();
	}

	if (__sys_vt.sem == 0) {
		__sys_vt.sem = thinkos_sem_alloc(0);
	}

	__sys_vt.alloc.used = 0;
	__sys_vt.alloc.max = VT_WIN_POOL_SIZE;

	/* create a linked list of memory blocks */
	q = &__sys_vt.alloc.blk[1];
	p = (union vt_mem_blk *)&__sys_vt.alloc.free.first;
	for (i = 1; i < VT_WIN_POOL_SIZE; i++) {
		p->next = q;
		p = q;
		q++;
	}

	p->next = NULL;
	__sys_vt.alloc.free.last = p;
	 
	__sys_vt.queue.head = 0;
	__sys_vt.queue.tail = 0;

}

void vt_quit(int retcode)
{
	vt_msg_post(__vt_win_root(), VT_QUIT, (uintptr_t)retcode);
}

