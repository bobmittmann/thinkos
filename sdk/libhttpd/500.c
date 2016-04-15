/*
 * Copyright(c) 2004-2014 BORESTE (www.boreste.com). All Rights Reserved.
 *
 * This file is part of the libhttpd.
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
 * @file 500.c
 * @brief HTTPD library
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "httpd-i.h"

static const char http_msg_500[] = HTTPD_MSG(500, "Internal Server Error", "");

int http_500(struct httpctl * __ctl)
{
	return tcp_send(__ctl->tp, http_msg_500, sizeof(http_msg_500) - 1,
					TCP_SEND_NOCOPY);
}

