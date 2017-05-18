#ifndef __WWW_H__
#define __WWW_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tcpip/httpd.h>

#ifdef __cplusplus
extern "C" {
#endif

int ajax_reply_error(struct httpctl * http, 
					 int code, const char * msg);

int ajax_reply_ok(struct httpctl * http);

#ifdef __cplusplus
}
#endif

#endif



