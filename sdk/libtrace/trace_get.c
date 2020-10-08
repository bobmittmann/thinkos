/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file trace.c
 * @brief Real-time trace
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "trace-i.h"

int trace_krn_getfirst(struct trace_iterator * it)
{
	struct trace_entry * entry = &it->entry;
	uint32_t tail;
	uint32_t ts;
	int ret;

	tail = trace_ctl.tail;
	entry->dt = 0;

	if ((int32_t)(trace_ctl.head - tail) < 2) {
		entry->ref = NULL;
		entry->tm = trace_ctl.tm;
		entry->idx = tail;
		ret = -1;
	} else {
		entry->ref = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].ref;
		ts = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].ts;
		entry->tm = ts;
		it->ts = ts;
		entry->idx = tail;
		ret = 0;
	}

	return ret;
}

extern const struct trace_ref __trace_usr_start[];
extern const struct trace_ref __trace_usr_end;

int trace_krn_getnext(struct trace_iterator * it)
{
	struct trace_entry * entry = &it->entry;
	uint32_t tail;
	int ret;

	tail = trace_ctl.tail;
	if ((int32_t)(entry->idx - tail) > 0)
		tail = entry->idx;

	if ((int32_t)(trace_ctl.head - tail) >= 2) {
		const struct trace_ref  * ref;
		uint32_t ts;
		uint32_t dt;

		ref = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].ref;
		if ((ref >= __trace_usr_start) && (ref < &__trace_usr_end)) {
			entry->ref = ref;
			ts = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].ts;
			dt = ts - (entry->tm & 0xffffffff);
			it->ts = ts;
			entry->dt = dt;
			entry->tm += dt;
			entry->idx = tail;
			ret = 0;
		} else {
			ret = -2;
		}
	} else {
		ret = -1;
	}

	return ret;
}

void trace_krn_flush(struct trace_iterator * it)
{
	struct trace_entry * entry = &it->entry;
	uint32_t head;
	uint32_t tail;
	uint32_t ts;
	uint32_t dt;

	head = trace_ctl.head;
	tail = trace_ctl.tail;

	if ((int32_t)(head - tail) < 2) {
		ts = (trace_ctl.tm & 0xffffffff);
	} else {
		ts = trace_ring.buf[(tail + 1) & (TRACE_RING_SIZE - 1)].ts;
	}

	if (entry == NULL) {
		tail = head;
	} else {
		if ((int32_t)(head - entry->idx) < 0)
			entry->idx = head;
		tail = entry->idx;
	}

	/* update the time at tail */
	dt = ts - (trace_ctl.tm & 0xffffffff);
	trace_ctl.tm += dt;
	/* set the new tail */
	trace_ctl.tail = tail;
}

int trace_krn_tail(struct trace_iterator * it)
{
	struct trace_entry * entry = &it->entry;
	uint32_t tail;
	uint32_t ts;
	uint32_t dt;

	tail = trace_ctl.tail;

	entry->dt = 0;
	entry->idx = tail;

	if ((int32_t)(trace_ctl.head - tail) < 2) {
		entry->ref = NULL;
		entry->tm = trace_ctl.tm;
	} else {
		entry->ref = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].ref;
		ts = trace_ring.buf[tail++ & (TRACE_RING_SIZE - 1)].ts;
		dt = ts - (trace_ctl.tm & 0xffffffff);
		it->ts = ts;
		entry->tm = trace_ctl.tm + dt;
	}

	return 0;
}

void trace_flush(struct trace_iterator * it)
{
	thinkos_mutex_lock(trace_ctl.mutex);

	trace_krn_flush(it);

	thinkos_mutex_unlock(trace_ctl.mutex);
}

int trace_tail(struct trace_iterator * it)
{
	int ret;

	if (it == NULL)
		return -1;

	thinkos_mutex_lock(trace_ctl.mutex);

	ret = trace_krn_tail(it);

	thinkos_mutex_unlock(trace_ctl.mutex);

	return ret;
}

struct trace_entry * trace_getfirst(struct trace_iterator * it)
{
	struct trace_entry * entry;

	if (it == NULL)
		return NULL;

	thinkos_mutex_lock(trace_ctl.mutex);

	if (trace_krn_getfirst(it) < 0)
		entry = NULL;
	else
		entry = &it->entry;

	thinkos_mutex_unlock(trace_ctl.mutex);

	return entry;
}

struct trace_entry * trace_getnext(struct trace_iterator * it)
{
	struct trace_entry * entry; 

	if (it == NULL)
		return NULL;

	thinkos_mutex_lock(trace_ctl.mutex);

	if (trace_krn_getnext(it) < 0)
		entry = NULL;
	else
		entry = &it->entry;

	thinkos_mutex_unlock(trace_ctl.mutex);

	return entry;
}

