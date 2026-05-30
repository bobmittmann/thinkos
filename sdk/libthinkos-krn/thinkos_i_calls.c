/* 
 * thinkos_semaphore.c
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif


#if (THINKOS_SEMAPHORE_MAX) > 0
void thinkos_krn_sem_post_i(int sem);
#else
void thinkos_krn_sem_post_i(int sem) 
{
}
#endif /* THINKOS_SEM_MAX > 0 */

#if (THINKOS_EVENT_MAX) > 0
void thinkos_krn_ev_raise_i(int set, int ev);
#else
void thinkos_krn_ev_raise_i(int set, int ev)
{
}
#endif

#if (THINKOS_FLAG_MAX) > 0
void thinkos_krn_flag_give_i(int flag);
#else
void thinkos_krn_flag_give_i(int flag) 
{
}
#endif

#if (THINKOS_GATE_MAX) > 0
void thinkos_krn_gate_open_i(int gate);
#else
void thinkos_krn_gate_open_i(int gate)
{
}
#endif

/* From Interrupt Call table */
const struct thinkos_i_call_tab cm3_except7_isr = {
	.sem_post = thinkos_krn_sem_post_i,
	.ev_raise = thinkos_krn_ev_raise_i,
	.flag_give = thinkos_krn_flag_give_i,
	.gate_open = thinkos_krn_gate_open_i
};

//const uintptr_t __attribute__((alias("thinkos_krn_i_call_tab")));
