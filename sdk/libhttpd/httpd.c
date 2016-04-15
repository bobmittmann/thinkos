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
 * @file httpd.c
 * @brief HTTPD library
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "httpd-i.h"

/*
 * HTTP server control structure
 */

struct httpd {
	struct tcp_pcb * tp;
	/* server root directory */
	const struct httpddir * dir;
	/* authentication data */
	const struct httpdauth * auth;
};

int httpd_accept(struct httpd * httpd, struct httpctl * ctl)
{
	struct tcp_pcb * tp;

	DCC_LOG1(LOG_INFO, "httpd=%p.", httpd);

	if ((tp = tcp_accept(httpd->tp)) == NULL) {
		DCC_LOG(LOG_ERROR, "tcp_accept().");
		return -1;
	}

	memset(ctl, 0, sizeof(struct httpctl));

	DCC_LOG1(LOG_INFO, "ctl=%p accepted.", ctl);
	ctl->tp = tp;

	return 0;
}

const char httpd_index_obj[] = "index.html";

const struct httpdobj * httpd_obj_lookup(struct httpd * httpd, struct httpctl * ctl)
{
	char path[HTTP_URI_MAX_LEN];
	const struct httpdobj * obj;
	const struct httpddir * dir;
	char * dst;
	char * src;
	char * sep;
	char * qry;
	char * oid;
	int c;

	dst = path;
	sep = path;
	oid = ctl->uri;

	qry = NULL;
	for (src = oid; (c = *src) != '\0'; ++src) {
		if (c == '/') {
			oid = src + 1;
			sep = dst;
		}
		if (c == '?') {
			*src = '\0';
			qry = src + 1;
			break;
		}
		*dst++ = c;
	}
	*sep = '\0';

	if (path[0] == '\0') {
		path[0] = '/';
		path[1] = '\0';
	}

	DCC_LOGSTR(LOG_INFO, "path=\"%s\"", path);

	for (dir = httpd->dir; (dir->path != NULL); ++dir) {
		if (strcmp(dir->path, path) == 0) {
			break;
		};
	}
	
	if (dir->path == NULL) {
		DCC_LOG(LOG_WARNING, "404 File Not Found (invalid path)");
		http_404(ctl);
		return NULL;
	}

	if (*oid == '\0')
		oid = (char *)httpd_index_obj;

	DCC_LOGSTR(LOG_INFO, "obj=\"%s\"", oid);

	for (obj = dir->objlst; (obj->oid != NULL); ++obj) {
		if (strcmp(obj->oid, oid) == 0) {
			break;
		};
	}

	if (obj->oid == NULL) {
		DCC_LOG(LOG_WARNING, "404 File Not Found (invalid OID)");
		http_404(ctl);
		return NULL;
	}

	if (qry != NULL) {
		int n;

		n = http_decode_uri_query(qry, 10000, ctl->qrylst, 
								  HTTP_QUERY_LST_MAX);
		if (n < 0) {
			DCC_LOG(LOG_WARNING, "400 Invalid Request");
			http_400(ctl);
			return NULL;
		}
		ctl->qrycnt = n;
	}

	return obj;
}

/* ---------------------------------------------------------------------------
 * HTTP server
 * ---------------------------------------------------------------------------
 */

const struct httpdauth httpd_def_authlst[] = {
	{ .uid = 0, .lvl = 0, .name = "", .passwd = "" }
};

//int http_parser_test(void);

int httpd_init(struct httpd * httpd,
							 int port, int backlog,
							 const struct httpddir dirlst[],
							 const struct httpdauth authlst[])
{
	struct tcp_pcb * tp;

//	http_parser_test();

	if (httpd == NULL) {
		DCC_LOG(LOG_ERROR, "Invalid parameter!");
		return -1;
	}

	if ((tp = tcp_alloc()) == NULL) {
		DCC_LOG(LOG_ERROR, "Can't alloc TCP PCB!");
		return -2;
	}

	tcp_bind(tp, INADDR_ANY, htons(port));

	if (tcp_listen(tp, backlog) != 0) {
		DCC_LOG(LOG_ERROR, "Can't register the TCP listener!");
		return -3;
	}

	httpd->tp = tp;
	httpd->dir = dirlst;

	if (authlst == NULL)
		httpd->auth = httpd_def_authlst;
	else
		httpd->auth = authlst;

	return 0;
}

/* -------------------------------------------------------------------------
 * Pool of resources
 * ------------------------------------------------------------------------- */

static struct httpd httpd_pool[HTTPD_POOL_SIZE];

struct httpd * httpd_alloc(void)
{
	struct httpd * httpd;
	int i;

	for (i = 0; i < HTTPD_POOL_SIZE; ++i) {
		httpd = &httpd_pool[i];
		if (!httpd->auth) {
			httpd->auth = httpd_def_authlst;
			return httpd;
		}
	}

	return httpd;
}


