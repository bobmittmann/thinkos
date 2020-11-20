/* 
 * File:	 thinkos_trace.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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

#define __THINKOS_TRACE__
#include <thinkos/trace.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Os\")")
#endif
#include <thinkos.h>

#include <sys/param.h>
#include <stdbool.h>

#if 0
int thinkos_trace(const struct trace_ref * ref);
int thinkos_trace_open(void);
int thinkos_trace_close(int id);
int thinkos_trace_read(int id, uint32_t * buf, unsigned int len);
int thinkos_trace_flush(int id);
int thinkos_trace_getfirst(int id, struct trace_entry * entry);
int thinkos_trace_getnext(int id, struct trace_entry * entry);
#endif

#if (THINKOS_ENABLE_TRACE)

struct trace_ring trace_ring;

/* ---------------------------------------------------------------------------
 * Kernel level trace calls
 * ---------------------------------------------------------------------------
 */

void __thinkos_trace0(const struct trace_ref * __ref)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(2)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace1(const struct trace_ref * __ref, int __a)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(3)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace2(const struct trace_ref * __ref, int __a, int __b)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(4)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace3(const struct trace_ref * __ref, int __a, int __b, 
					int __c)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(5)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace4(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(6)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace5(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d, int __e)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(7)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __e;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace6(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d, int __e, int __f)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(8)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __e;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __f;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace7(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(9)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __e;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __f;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __g;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace8(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g, int __h)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(10)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __e;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __f;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __g;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __h;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace9(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g, int __h,
			 int __i)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(11)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __e;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __f;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __g;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __h;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __i;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace10(const struct trace_ref * __ref, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g, int __h,
			 int __i, int __j)
{
    unsigned int pri = cm3_primask_get();
    unsigned int head;

    cm3_primask_set(1);
    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(12)) {
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = __ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = CM3_DWT->cyccnt;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __a;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __b;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __c;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __d;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __e;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __f;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __g;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __h;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __i;
		trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = __j;
        trace_ring.head = head;
    }
    cm3_primask_set(pri);
}

void __thinkos_trace_drain(void)
{
	uint32_t tail = trace_ring.tail;
    uint32_t head = trace_ring.head;

	while ((int32_t)(head - tail) > 0) {
		uint32_t val = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].val;
		uint32_t demcr; 
		do {
			demcr = CM3_DCB->demcr;
		} while (demcr & DCB_DEMCR_MON_REQ);
		CM3_DCB->dcrdr = val;
		CM3_DCB->demcr = demcr | DCB_DEMCR_MON_REQ;
		asm volatile ("isb\n" :  :  : );
	}
	trace_ring.tail = tail;
}

int __thinkos_trace_try_send(void)
{
	uint32_t tail = trace_ring.tail;
    uint32_t head = trace_ring.head;

	if ((int32_t)(head - tail) > 0) {
		uint32_t val = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].val;
		uint32_t demcr = CM3_DCB->demcr;
		if (demcr & DCB_DEMCR_MON_REQ)
			return -1;
		CM3_DCB->dcrdr = val;
		CM3_DCB->demcr = demcr | DCB_DEMCR_MON_REQ;
		asm volatile ("isb\n" :  :  : );
		trace_ring.tail = tail;
		return 1;
	} 

	return 0;
}

void __thinkos_trace_flush(void)
{
    unsigned int pri = cm3_primask_get();
	uint32_t head;

    cm3_primask_set(1);
    head = trace_ring.head;
	/* set the new tail */
	trace_ring.tail = head;
    cm3_primask_set(pri);
}

/* ---------------------------------------------------------------------------
 * User level trace service 
 * ---------------------------------------------------------------------------
 */

void thinkos_trace_svc(int32_t * arg, int self)
{
	const struct trace_ref * ref = (const struct trace_ref *)arg[0];
    uint32_t * buf = (uint32_t *)arg[1];
    unsigned int cnt = arg[2];
    unsigned int head;
    unsigned int pri;
    uint32_t now;


#if THINKOS_ENABLE_ARG_CHECK
	if (ref == NULL) {
		__THINKOS_ERROR(THINKOS_ERR_TRACE_ENTRY_NULL);
		arg[0] = THINKOS_EINVAL;
		return;
	}
	if (buf == NULL) {
		__THINKOS_ERROR(THINKOS_ERR_TRACE_ENTRY_NULL);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* Use the cycle counter as a timestamp */
	now = CM3_DWT->cyccnt;

	/* FIXME: interrupt disabe */
    pri = cm3_primask_get();
    cm3_primask_set(1);

    head = trace_ring.head;
    if ((TRACE_RING_SIZE + trace_ring.tail - head) >= (unsigned int)(cnt + 2)) {
        int i;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ref = ref;
        trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].ts = now;
        for (i = 0; i < cnt; ++i)
            trace_ring.buf[head++ & (TRACE_RING_SIZE - 1)].val = buf[i];
        trace_ring.head = head;
    }

    cm3_primask_set(pri);
}

void thinkos_trace_ctl_svc(int32_t * arg, int self)
{
	unsigned int req = arg[0];
	
	switch (req) {
	case THINKOS_TRACE_OPEN:
		/* TODO: not implemented yet ... */
		arg[0] = THINKOS_EINVAL;
		break;

	case THINKOS_TRACE_CLOSE:
		/* TODO: not implemented yet ... */
		arg[0] = THINKOS_EINVAL;
		break;

	case THINKOS_TRACE_READ:
		/* TODO: not implemented yet ... */
		arg[0] = THINKOS_EINVAL;
		break;

	case THINKOS_TRACE_FLUSH:
		/* TODO: not implemented yet ... */
		arg[0] = THINKOS_EINVAL;
		break;

	case THINKOS_TRACE_GETFIRST:
		/* TODO: not implemented yet ... */
		arg[0] = THINKOS_EINVAL;
		break;

	case THINKOS_TRACE_GETNEXT:
		/* TODO: not implemented yet ... */
		arg[0] = THINKOS_EINVAL;
		break;

	default:
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

const char thinkos_trace_nm[] = "TRC";

#endif /* THINKOS_ENABLE_TRACE */

