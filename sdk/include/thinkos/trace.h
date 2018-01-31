/* 
 * File:	 /thinkos/trace.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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

#ifndef __THINKOS_TRACE_H__
#define __THINKOS_TRACE_H__

#ifndef __THINKOS_TRACE__
#error "Never use <thinkos/trace.h> directly; include <thinkos.h> instead."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#include <trace.h>

#ifndef TRACE_RING_SIZE
#define TRACE_RING_SIZE 1024
#endif

#ifndef TRACE_STRING_MAX
#define TRACE_STRING_MAX 64
#endif

enum krn_trace_lvl {
	KNONE  = 0,
	KPANIC = 17,
	KXCPT  = 18,
	KERR   = 19,
	KWARN  = 20,
	KINF   = 21,
	KDBG   = 22,
	KYAP   = 23
};

#ifndef KRN_TRACE_LVL
 #ifdef DEBUG
  #define ENABLE_KRN_TRACE
  #ifdef DEBUG_LEVEL
   #define KRN_TRACE_LVL DEBUG_LEVEL
  #elif DEBUG == 0
   #define KRN_TRACE_LVL KNONE
  #elif DEBUG == 2
   #define KRN_TRACE_LVL KERR
  #elif DEBUG == 3
   #define KRN_TRACE_LVL KWARN
  #elif DEBUG == 4
   #define KRN_TRACE_LVL KINF
  #elif DEBUG == 5
   #define KRN_TRACE_LVL KDBG
  #else
   #define KRN_TRACE_LVL KYAP
  #endif
 #else
  #define KRN_TRACE_LVL KNONE
 #endif
#else
 #ifndef ENABLE_KRN_TRACE
  #define ENABLE_KRN_TRACE
 #endif
#endif

#ifdef ENABLE_KRN_TRACE

#define TRACE(__LVL, __MSG) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace ( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__MSG);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; })); }} while (0)

#define TRACE1(__LVL, __FMT, __A) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace1( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A)); }} while (0)

#define TRACE2(__LVL, __FMT, __A, __B) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace2( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), \
	(int)(__B)); }} while (0)

#define TRACE3(__LVL, __FMT, __A, __B, __C) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace3( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), \
		(int)(__B), (int)(__C)); }} while (0)

#define TRACE4(__LVL, __FMT, __A, __B, __C, __D) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace4( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D)); }} while (0)

#define TRACE5(__LVL, __FMT, __A, __B, __C, __D, __E) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace5( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D), (int)(__E)); }} while (0)

#define TRACE6(__LVL, __FMT, __A, __B, __C, __D, __E, __F) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace6( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D), (int)(__E), (int)(__F)); }} while (0)

#define TRACE7(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace7( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D), (int)(__E), (int)(__F), (int)(__G)); \
	}} while (0)

#define TRACE8(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G, __H) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace8( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D), (int)(__E), (int)(__F), (int)(__G), \
		(int)(__H)); }} while (0)

#define TRACE9(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G, __H, __I) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace9( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D), (int)(__E), (int)(__F), (int)(__G), \
		(int)(__H), (int)(__I)); }} while (0)

#define TRACE10(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G, __H, \
				  __I, __J) \
	do { if (__LVL <= LOG_LEVEL)  { thinkos_trace10( ({ \
	static const char _f[] __attribute__ ((section(".dccdata"))) = __FILE__;\
	static const char _m[] __attribute__ ((section(".dccdata"))) = (__FMT);\
	static const struct trace_ref __attribute__((section(".dcclog"))) \
	__ref = { _f, __LINE__, __LVL, 0, __FUNCTION__, _m }; \
	(struct trace_ref *)&__ref; }), (int)(__A), (int)(__B), \
		(int)(__C), (int)(__D), (int)(__E), (int)(__F), (int)(__G), \
		(int)(__H), (int)(__I), (int)(__J)); }} while (0)

#else

#define TRACE_INIT()
#define TRACE_CONNECT()
#define TRACE(__LVL, __MSG)
#define TRACE1(__LVL, __FMT, __A)
#define TRACE2(__LVL, __FMT, __A, __B)
#define TRACE3(__LVL, __FMT, __A, __B, __C)
#define TRACE4(__LVL, __FMT, __A, __B, __C, __D)
#define TRACE5(__LVL, __FMT, __A, __B, __C, __D, __E)
#define TRACE6(__LVL, __FMT, __A, __B, __C, __D, __E, __F)
#define TRACE7(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G)
#define TRACE8(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G, __H)
#define TRACE9(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G, __H, __I)
#define TRACE10(__LVL, __FMT, __A, __B, __C, __D, __E, __F, __G, \
				  __H, __I, __J)

#endif

/* To avoid integer conversion, float point numbers should be wrapped
   by this macro */
#define __FLOAT(F)	({ union { float f; uint32_t u; } a; a.f = (F); a.u;})

struct trace_ring {
	uint64_t tm;
	volatile uint32_t head;
	volatile uint32_t tail;
	volatile uint32_t print_pos;
	volatile uint32_t print_tm;
	struct {
		union {
			const struct trace_ref * ref;
			uint32_t ts;
			uint32_t val;
		};
	} buf[TRACE_RING_SIZE];
};

extern struct trace_ring trace_ring;

#ifdef __cplusplus
extern "C" {
#endif

void thinkos_trace_svc(int32_t arg[], int self);

void thinkos_trace_ctl_svc(int32_t arg[], int self);

/* Internal kernel trace functions */

void __thinkos_trace0(const struct trace_ref * __entry);

void __thinkos_trace1(const struct trace_ref * __entry, int __a);

void __thinkos_trace2(const struct trace_ref * __entry, int __a, int __b);

void __thinkos_trace3(const struct trace_ref * __entry, int __a, int __b, 
					  int __c);

void __thinkos_trace4(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d);

void __thinkos_trace5(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d, int e);

void __thinkos_trace6(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d, int __e, int __f);

void __thinkos_trace7(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g);

void __thinkos_trace8(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g, int __h);

void __thinkos_trace9(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g, int __h,
			 int __i);

void __thinkos_trace10(const struct trace_ref * __entry, int __a, int __b, 
			 int __c, int __d, int __e, int __f, int __g, int __h,
			 int __i, int __j);

void __thinkos_trace_drain(void);

void __thinkos_trace_flush(void);

int __thinkos_trace_try_send(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_TRACE_H__ */

