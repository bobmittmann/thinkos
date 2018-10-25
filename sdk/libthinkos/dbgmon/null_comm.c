/* 
 * File:	 usb-cdc.c
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

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <sys/serial.h>
#include <string.h>
#include <stdbool.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

#if (THINKOS_ENABLE_MONITOR)

int null_comm_send(const void * arg, const void * buf, unsigned int len)
{
	return 0;
}

int null_comm_recv(const void * arg, void * buf, unsigned int len)
{
	return 0;
}

int null_comm_connect(const void * arg)
{
	return 0;
}

bool null_comm_isconnected(const void * arg)
{
	return false;
}

void null_comm_rxflowctrl(const void * arg, bool stop)
{
}

static const struct dbgmon_comm_op null_comm_op = {
	.send = null_comm_send,
	.recv = null_comm_recv,
	.connect = null_comm_connect,
	.isconnected = null_comm_isconnected
};

static const struct dbgmon_comm null_comm_instance = {
	.dev = (void *)0,
	.op = &null_comm_op,
};

const struct dbgmon_comm * null_comm_getinstance(void)
{
	return &null_comm_instance;
}

#endif

