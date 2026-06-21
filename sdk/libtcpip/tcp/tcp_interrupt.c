/* 
 * Copyright(c) 2004-2012 BORESTE (www.boreste.com). All Rights Reserved.
 *
 * This file is part of the libtcpip.
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
 * @file tcp_interrupt.c
 * @brief Interrupts a system call (tcp_recv()) 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __USE_SYS_TCP__
#include <sys/tcp.h>

int tcp_interrupt(struct tcp_pcb * __tp)
{
	if (__tp == NULL) {
		DCC_LOG(LOG_WARNING, "NULL pointer");
		return -EINVAL;
	}

	tcpip_net_lock();

#ifdef ENABLE_SANITY_CHECK
	if (pcb_find((struct pcb *)__tp, &__tcp__.active) < 0) {
		DCC_LOG1(LOG_ERROR, "<%05x> pcb_find()", (int)__tp);
		tcpip_net_unlock();
		return -EBADF;
	}
#endif

	if ((__tp->t_state == TCPS_CLOSED)) {
		DCC_LOG(LOG_WARNING, "closed!");
		tcpip_net_unlock();
		return -EBADF;
	}

	if ((__tp->t_state == TCPS_TIME_WAIT) ||
		(__tp->t_state == TCPS_CLOSING) || 
		(__tp->t_state == TCPS_LAST_ACK)) {
		tcpip_net_unlock();
		return 0;
	}

	__tp->t_flags |= TF_INTERRUPTED;

	DCC_LOG2(LOG_MSG, "<%05x> interrupt signal [%d]", (int)__tp, __tp->t_cond);

	thinkos_cond_signal(__tp->t_cond); 
			 
	tcpip_net_unlock();

	return 0;
}

