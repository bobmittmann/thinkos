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
 * @file tcpip/httpd.h
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __HTTPD_H__
#define __HTTPD_H__

#include <stdint.h>
#include <tcpip/tcp.h>


enum http_oid {
	OBJ_VOID = 0,
	OBJ_STATIC_HTML,
	OBJ_STATIC_TEXT,
	OBJ_STATIC_CSS,
	OBJ_STATIC_JS,
	OBJ_STATIC_JSON,
	OBJ_STATIC_XML,
	OBJ_STATIC_PNG,
	OBJ_STATIC_JPEG,
	OBJ_STATIC_GIF,
	OBJ_STATIC_HTML_GZ,
	OBJ_STATIC_TEXT_GZ,
	OBJ_STATIC_CSS_GZ,
	OBJ_STATIC_JS_GZ,
	OBJ_CODE_CGI,
	OBJ_CODE_JS,
};

enum http_method {
	HTTP_UNKNOWN = 0,
	HTTP_OPTIONS = 1,
	HTTP_GET = 2,
	HTTP_HEAD = 3,
	HTTP_POST = 4,
	HTTP_PUT = 5,
	HTTP_DELETE = 6,
	HTTP_TRACE = 7,
	HTTP_CONNECT = 8
};

enum mime_type {
    TEXT_HTML              = OBJ_STATIC_HTML,
    TEXT_PLAIN             = OBJ_STATIC_TEXT,
    TEXT_CSS               = OBJ_STATIC_CSS,
	APPLICATION_JAVASCRIPT = OBJ_STATIC_JS,
    APPLICATION_JSON       = OBJ_STATIC_JSON,
    APPLICATION_XML        = OBJ_STATIC_XML,
    IMAGE_PNG              = OBJ_STATIC_PNG,
    IMAGE_JPEG             = OBJ_STATIC_JPEG,
    IMAGE_GIF              = OBJ_STATIC_GIF,
    MULTIPART_FORM_DATA    = 10,
    APPLICATION_X_WWW_FORM_URLENCODED = 11
};


/*
 * HTTP server opaque control structure
 */

struct httpd;

/*
 * HTTP server object descriptor structure
 */
struct httpdobj {
	const char * oid;
	uint8_t typ;
	uint8_t lvl;
	uint32_t len;
	const void * ptr;
};

/*
 * HTTP server object directory structure
 */

struct httpddir {
	const char * path;
	const struct httpdobj * objlst;
};


#define HTTPDAUTH_NAME_MAX 12
#define HTTPDAUTH_PASSWD_MAX 12

struct httpdauth {
	uint8_t uid;
	uint8_t lvl;
	char name[HTTPDAUTH_NAME_MAX + 1];
	char passwd[HTTPDAUTH_PASSWD_MAX + 1];
};

/* 
 * HTTP connection control structure
 */
struct httpctl;

typedef int (* httpd_cgi_t)(struct httpctl ctl);

typedef int (* __httpd_cgi_t)(struct tcp_pcb * tp, char * opt, 
							int content_type, int content_len); 

/* 
	Ex:
	int _form_cgi_(struct tcp_pcb * tp, char * nm[], char * val[], int n);
*/	
typedef int (* httpd_form_cgi_t)(struct tcp_pcb * tp, char ** name, 
								 char ** value, int count);


/* 
	Ex:
	int _upload_cgi_(struct tcp_pcb * tp, char * buf, int len, int offs);
 */
typedef int (* httpd_file_cgi_t)(struct tcp_pcb * tp, char * buf, 
								 int len, int offs);

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Server
 * ------------------------------------------------------------------------- */

struct httpd * httpd_alloc(void);

int httpd_free(struct httpd * httpd);

int httpd_init(struct httpd * httpd, int port, int backlog,
		const struct httpddir dirlst[],
		const struct httpdauth authlst[]);


int httpd_stop(struct httpd * httpd);

int httpd_accept(struct httpd * httpd, struct httpctl * ctl);

const struct httpdobj * httpd_obj_lookup(struct httpd * httpd, struct httpctl * ctl);

/* -------------------------------------------------------------------------
 * Connections
 * ------------------------------------------------------------------------- */

struct httpctl * httpctl_alloc(void);

void httpctl_free(struct httpctl * ctl);

const char * http_uri_get(struct httpctl * http);

int http_method_get(struct httpctl * http);

int http_close(struct httpctl * ctl);

int http_recv_request(struct httpctl * ctl);

int http_recv(struct httpctl * ctl, void * buf, unsigned int len);

int http_multipart_recv(struct httpctl * ctl, void * buf, unsigned int len);

int http_content_recv(struct httpctl * ctl, void * buf, unsigned int len);

int http_send(struct httpctl * ctl, const void * buf, unsigned int len);

int http_get(struct httpctl * ctl, const struct httpdobj * obj);

int http_post(struct httpctl * ctl, const struct httpdobj * obj);

char * http_query_lookup(struct httpctl * ctl, char * key);

/* 200 OK */
int http_200(struct httpctl * __ctl, unsigned int __type);

/* 400 Bad Request */
int http_400(struct httpctl * __ctl);

/* 401 Unauthorized */
int http_401(struct httpctl * __ctl);
int http_401_auth(struct httpctl * __ctl);

/* 402 Payment Required */

/* 403 Forbidden */
int http_403(struct httpctl * __ctl);

/* 404 Not Found */
int http_404(struct httpctl * __ctl);

/* 405 Method Not Allowed */
int http_405(struct httpctl * __ctl);

/* 406 Not Acceptable */

/* 407 Proxy Authentication Required */

/* 408 Request Timeout */
int http_408(struct httpctl * __ctl);

/* 409 Conflict */
int http_409(struct httpctl * __ctl);

/* 410 Gone */
int http_410(struct httpctl * __ctl);

/* 411 Length Required */
int http_411(struct httpctl * __ctl);

/* 412 Precondition Failed */

/* 413 Request Entity Too Large */
int http_413(struct httpctl * __ctl);

/* 414 Request-URI Too Long */
int http_414(struct httpctl * __ctl);

/* 415 Unsupported Media Type */
int http_415(struct httpctl * __ctl);

/* 416 Requested Range Not Satisfiable */

/* 417 Expectation Failed */

/* 500 Internal Server Error */
int http_500(struct httpctl * __ctl);

/* 501 Not Implemented */
int http_501(struct httpctl * __ctl);

/* 502 Bad Gateway */

/* 503 Service Unavailable */
int http_503(struct httpctl * __ctl);

/* 504 Gateway Timeout */

/* 505 HTTP Version Not Supported */
int http_505(struct httpctl * __ctl);


#ifdef __cplusplus
}
#endif

#endif /* __HTTPD_H__ */

