#ifndef __WWW_H__
#define __WWW_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tcpip/httpd.h>

enum ajax_error {
	AJAX_OK,
	AJAX_ERR_SIMRPC_OPEN_FAILED = 1,
	AJAX_ERR_SIMRPC_ATTR_SET_FAILED = 2,
};

/*---------------------------------------------------------------------------
  Common HTML macros
  ---------------------------------------------------------------------------*/

#define HTML_FOOTER "<hr>\r\n"\
	"<a href=\"https://https://github.com/bobmittmann/yard-ice\">"\
	"<img src=\"thinkos47x20.png\">"\
	"</a> - Cortex-M Operating System - \r\n"\
	"<i><a href=\"https://https://github.com/bobmittmann/yard-ice\">"\
	"YARD-ICE</a></i><br>\r\n"\
	"&copy; Copyright 2013-2015, Bob Mittmann<br>\r\n"\
	"</body>\r\n</html>\r\n"

#define DOCTYPE_HTML "<!DOCTYPE html PUBLIC " \
	"\"-//W3C//DTD XHTML 1.0 Strict//EN\" "\
	"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\r\n"\
	"<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n"

#define SERVER "Server: " HTTPD_SERVER_NAME "\r\n"

#define META_COPY "<meta name=\"copyright\" "\
	"content=\"Copyright (c) Bob Mittmann 2014-2015\"/>\r\n"

#define META_HTTP "<meta http-equiv=\"content-type\" "\
	"content=\"text/html; charset=utf-8\"/>\r\n"

#define LINK_MODALBOX "<script type=\"text/javascript\" "\
		"src=\"/lib/modalbox.js.gz\"></script>\r\n"

#define LINK_ZEPTO "<script type=\"text/javascript\" "\
		"src=\"/lib/zepto.js.gz\"></script>\r\n"

#define LINK_ICON "<link rel=\"shortcut icon\" "\
	"href=\"thinkos16x16.png\"/>\r\n"

#define LINK_CSS "<link rel=\"stylesheet\" href=\"/style.css\" "\
	"type=\"text/css\" media=\"screen\"/>\r\n"

#define HTML_MAX 2048 /* HTML formatting string buffer length */

extern const char footer_html[];
extern const uint16_t sizeof_footer_html;

extern struct httpdobj www_root[];
extern struct httpdobj www_ipcfg[];
extern struct httpdobj www_lib[];
extern struct httpdobj www_img[];


/*---------------------------------------------------------------------------
  Automatically generated code
  ---------------------------------------------------------------------------*/


/* --- bin2hex.py: file size --- */

#define SIZEOF_PA75_MONITOR_HTML 957

#define SIZEOF_PA75_STATUS_HTML 2076

#define SIZEOF_LIBPA75_JS 1706

#define SIZEOF_ZEPTO_MIN_JS 9380


/* --- bin.hex.py: file content --- */

extern const uint8_t pa75_monitor_html[];

extern const uint8_t pa75_status_html[];

extern const uint8_t libpa75_js[];

extern const uint8_t zepto_min_js[];

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif










