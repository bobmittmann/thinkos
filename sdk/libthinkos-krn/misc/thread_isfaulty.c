/* 
 * thread_isfaulty.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include "thinkos_krn-i.h"

#if (THINKOS_ENABLE_THREAD_FAULT)
bool __thinkos_thread_isfaulty(unsigned int th)
{
	struct thinkos_rt * krn = &thinkos_rt;

	if (th > THINKOS_THREADS_MAX)
		return false;

	if (!__thread_ctx_is_valid(krn, th))
		return false;

	return __thread_fault_get(krn, th);
}
#endif

