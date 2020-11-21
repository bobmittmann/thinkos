/* 
 * File:	 __thread_krn_wq_get.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

#include "thinkos_krn-i.h"

int __thinkos_krn_thread_wq_get(struct thinkos_rt * krn, 
								unsigned int thread_idx)
{
#if !(THINKOS_ENABLE_THREAD_STAT)
	int i;
#endif
	int wq;

#if (THINKOS_ENABLE_SANITY_CHECK)
	if ((thread_idx >= THINKOS_THREADS_MAX) || 
		!__thread_ctx_is_valid(krn, thread_idx)) {
		return -1;
	}
#endif

#if (THINKOS_ENABLE_THREAD_STAT)
	wq = krn->th_stat[thread_idx] >> 1;
#else
	for (i = 0; i < (THINKOS_WQ_CNT); ++i) {
		if (krn->wq_lst[i] & (1 << thread_idx))
			break;
	}
	if (i == (THINKOS_WQ_CNT))
		return -1; /* not found */
	wq = i;
#endif /* THINKOS_ENABLE_THREAD_STAT */

	return wq;
}

