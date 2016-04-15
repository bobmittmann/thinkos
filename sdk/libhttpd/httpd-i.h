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
 * @file httpd-i
 * @brief HTTPD library private header
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __HTTPD_I_H__
#define __HTTPD_I_H__

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <tcpip/tcp.h>
#include <tcpip/httpd.h>

#include <sys/dcclog.h>


#ifdef CONFIG_H
#include "config.h"
#endif

#ifndef HTTPD_SERVER_NAME
#define HTTPD_SERVER_NAME "ThinkOS Web Server"
#endif

#ifndef HTTPD_POOL_SIZE
#define HTTPD_POOL_SIZE 1
#endif

#ifndef HTTP_URI_MAX_LEN
#define HTTP_URI_MAX_LEN 255
#endif

#ifndef HTTP_QUERY_LST_MAX
#define HTTP_QUERY_LST_MAX 16
#endif

#ifndef HTTP_CTL_POOL_SIZE
#define HTTP_CTL_POOL_SIZE 1
#endif

#ifndef HTTP_RCVBUF_LEN
/* 'GET http://www.domain.xxx/somedir/subdir/file.html HTTP/1.1' CRLF */
#define HTTP_RCVBUF_LEN (HTTP_URI_MAX_LEN + 17)
#endif

/*
 * HTTP connection control structure
 */
struct httpctl {
//	struct httpd * httpd;
	struct tcp_pcb * tp;
	uint16_t version;
	uint8_t method;
	uint8_t auth;
	struct {
		uint8_t type;
		uint8_t bdry_len;
		uint32_t bdry_hash;
		uint32_t len;
		uint32_t pos;
	} content;
	struct {
		uint16_t head;
		uint16_t pos;
		uint16_t tail;
		uint32_t pat; /* search compare window */
		uint32_t buf[(HTTP_RCVBUF_LEN + 3) / 4];
	} rcvq; /* receive queue */
	uint8_t qrycnt;
	struct httpqry qrylst[HTTP_QUERY_LST_MAX];
	char * usr;
	char * pwd;
	char uri[HTTP_URI_MAX_LEN + 1];
};

/*
 * Preformatted HTML macros 
 */
#ifndef HTTPD_MSG_FOOTER
#define HTTPD_MSG_FOOTER "<hr>&copy; Copyrigth 2013-2014, Bob Mittmann" \
	"<br><b>ThinkOS</b> - Cortex-M3 Operating System - "\
	"<i><a href=\"https://code.google.com/p/yard-ice\">YARD-ICE</a></i>"\
	"</body></html>\r\n"
#endif

/*
 * Template for HTML content of HTTP return code messages
 */
#define HTTPD_MSG(CODE, INFO, MSG) "HTTP/1.1 " #CODE " " INFO "\r\n"\
	"Server: " HTTPD_SERVER_NAME "\r\n"\
	"Content-type: text/html;charset=ISO-8859-1\r\n\r\n"\
	"<html><head><title>" #CODE "</title></head>"\
	"<h2>" #CODE " - " INFO "</h2><p>" MSG "</p>"\
	HTTPD_MSG_FOOTER

/* Authentication */
#define HTTPAUTH_BASIC              0x77
#define HTTPAUTH_DIGEST             0x00


#ifdef __cplusplus
extern "C" {
#endif

int http_decode_uri_query(char * buf, int len, 
						  struct httpqry lst[], int max);

int http_multipart_boundary_lookup(struct httpctl * ctl);

int http_parse_header(struct httpctl * ctl);

int http_recv_queue_shift(struct httpctl * ctl);

#ifdef __cplusplus
}
#endif

#endif /* __HTTPD_I_H__ */

