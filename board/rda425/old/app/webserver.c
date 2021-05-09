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
 * @file webserver.c
 * @brief YARD-ICE application main
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <thinkos.h>
#include <assert.h>

#include "www/www.h"

#undef DEBUG
#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LVL_INF
#include <trace.h>

struct httpddir httpd_dir[] = {
	{ .path = "/", .objlst = www_root },
//	{ .path = "/img", .objlst = www_img },
//	{ .path = "/ipcfg", .objlst = www_ipcfg },
	{ .path = NULL, .objlst = NULL }
};

int httpd_task(struct httpd * httpd)
{
	struct httpctl * http;
	const struct httpdobj * obj;

	INF("Webserver started (thread %d).", thinkos_thread_self());

	http = httpctl_alloc();
	assert(http != NULL);

	for (;;) {
		if (httpd_accept(httpd, http) < 0) {
			ERR("http_accept() failed!");
			thinkos_sleep(100);
			continue;
		}

		if (http_recv_request(http) >= 0) {
			DBG("URL=\"%s\"", http_uri_get(http));

			if ((obj = httpd_obj_lookup(httpd, http)) != NULL) {
				switch (http_method_get(http)) {
				case HTTP_GET:
					DBG("HTTP GET \"%s\"", obj->oid);
					http_get(http, obj);
					break;
				case HTTP_POST:
					DBG("HTTP POST \"%s\"", obj->oid);
					http_post(http, obj);
					break;
				}
			} else {
				WARN("HTTP 404 Not Found: \"%s\"", http_uri_get(http));
			}
		} else {
			WARN("HTTP 404 Not Found: \"%s\"", http_uri_get(http));
		}

		http_close(http);
	}

	return 0;
}

#define HTTPD_STACK_SIZE 4096
#define HTTPD_TASK_CNT 2

uint32_t httpd_stack[HTTPD_TASK_CNT][HTTPD_STACK_SIZE / 4] __attribute__((section (".ccm")));

const struct thinkos_thread_inf httpd_inf[HTTPD_TASK_CNT] = {
	{
		.stack_ptr = httpd_stack[0],
		.stack_size = HTTPD_STACK_SIZE,
		.priority = 32,
		.thread_id = 7,
		.paused = false,
		.tag = "HTTPD1"
	},
#if HTTPD_TASK_CNT > 1
	{
		.stack_ptr = httpd_stack[1],
		.stack_size = HTTPD_STACK_SIZE,
		.priority = 32,
		.thread_id = 8,
		.paused = false,
		.tag = "HTTPD2"
	},
#endif
#if HTTPD_TASK_CNT > 2
	{
		.stack_ptr = httpd_stack[2],
		.stack_size = HTTPD_STACK_SIZE,
		.priority = 32,
		.thread_id = 9,
		.paused = false,
		.tag = "HTTPD3"
	},
#endif
#if HTTPD_TASK_CNT > 3
	{
		.stack_ptr = httpd_stack[3],
		.stack_size = HTTPD_STACK_SIZE,
		.priority = 32,
		.thread_id = 10,
		.paused = false,
		.tag = "HTTPD4"
	},
#endif
#if HTTPD_TASK_CNT > 4
	{
		.stack_ptr = httpd_stack[5],
		.stack_size = HTTPD_STACK_SIZE,
		.priority = 32,
		.thread_id = 11,
		.paused = false,
		.tag = "HTTPD5"
	},
#endif
};

int webserver_start(void)
{
	struct httpd * httpd;
	int i;

	httpd = httpd_alloc();

	if (httpd_init(httpd, 80, 4, httpd_dir, NULL) < 0) {
		INF("Webserver initialization failed!");
		return -1;
	}

	for (i = 0; i < HTTPD_TASK_CNT; ++i) {
		thinkos_thread_create_inf((void *)httpd_task, httpd, &httpd_inf[i]);
	}

	return 0;
}
