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
 * @file trace.h
 * @brief Real-time trace
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

/* Trace options */
enum trace_level {
	TRACE_LVL_NONE  = 0,
	TRACE_LVL_ALERT = 1,
	TRACE_LVL_CRIT  = 2,
	TRACE_LVL_ERR   = 3,
	TRACE_LVL_WARN  = 4,
	TRACE_LVL_NOTE  = 5,
	TRACE_LVL_INF   = 6,
	TRACE_LVL_DBG   = 7,
	TRACE_LVL_YAP   = 8
};

/* Trace flags (options) */
#define TRACE_XXD 1
#define TRACE_AD  2

/* Trace reference */
struct trace_ref {
	unsigned char lvl;   /* Trace level */
	unsigned char opt;   /* Options */
	unsigned short line; /* Source file line number */
	const char * func;   /* Function name */
	const char * fmt;    /* Formatting string */
};

/* Trace entry */
struct trace_entry {
	const struct trace_ref * ref; /* Pointer to a trace entry reference */
	uint32_t dt; /* Time elapsed since last trance in the buffer [microsseconds] */
	uint64_t tm; /* Absolute time of this trace entry */
	uint32_t idx; /* Trace entry sequential number */
};

#ifndef TRACE_LEVEL
 #ifdef DEBUG
  #define ENABLE_TRACE
  #ifdef DEBUG_LEVEL
   #define TRACE_LEVEL DEBUG_LEVEL
  #elif DEBUG == 0
   #define TRACE_LEVEL TRACE_LVL_NONE
  #elif DEBUG == 2
   #define TRACE_LEVEL TRACE_LVL_ERR
  #elif DEBUG == 3
   #define TRACE_LEVEL TRACE_LVL_WARN
  #elif DEBUG == 4
   #define TRACE_LEVEL TRACE_LVL_INF
  #elif DEBUG == 5
   #define TRACE_LEVEL TRACE_LVL_DBG
  #else
   #define TRACE_LEVEL TRACE_LVL_YAP
  #endif
 #else
  #define TRACE_LEVEL TRACE_LVL_NONE
 #endif
#else
 #ifndef ENABLE_TRACE
  #define ENABLE_TRACE
 #endif
#endif

/* Trace optons */
#define TRACE_FLUSH     (1 << 0)
#define TRACE_COUNT     (1 << 1)
#define TRACE_ABSTIME   (1 << 2)
#define TRACE_ALL       (1 << 3)
#define TRACE_UNSAFE    (1 << 4)
#define TRACE_FUNC      (1 << 5)

#ifdef ENABLE_TRACE

static inline void __attribute__((always_inline,format (__printf__, 1, 2))) 
	tracef_chk(const char * __fmt, ... ) {}

#define TRACE_REF(__LVL, __OPT, __STR) __extension__({static const struct trace_ref \
	__attribute__((section(".rodata.trace"))) __ref__ = { .lvl = (__LVL), \
	.opt = (__OPT), .line = __LINE__, .func = __func__, .fmt = __STR };\
	&__ref__;})


#define YAP(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_YAP, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define DBG(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_DBG, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define INF(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_INF, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define WARN(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_WARN, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define ERR(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_ERR, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)


#define YAP_I(__FMT, ...) do { tracef_chk( __fmt, , ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_YAP, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef_i(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define DBG_I(__FMT, ...) do { tracef_chk( __fmt, , ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_DBG, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef_i(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define INF_I(__FMT, ...) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_INF, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef_chk( __fmt, , ## __VA_ARGS__); \
		tracef_i(&__ref, ## __VA_ARGS__); \
		}} while (0)

#define WARN_I(__FMT, ...) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_WARN, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef_chk( __fmt, , ## __VA_ARGS__); \
		tracef_i(&__ref, ## __VA_ARGS__); \
		}} while (0)

#define ERR_I(__FMT, ...) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_ERR, .opt=0, \
		  .func=__func__, .fmt=__FMT}; \
		tracef_chk( __fmt, , ## __VA_ARGS__); \
		tracef_i(&__ref, ## __VA_ARGS__); \
		}} while (0)


#define YAPS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_YAP, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace(&__ref); \
		}} while (0)

#define DBGS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_DBG, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace(&__ref); \
		}} while (0)

#define INFS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_INF, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace(&__ref); \
		}} while (0)

#define WARNS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_WARN, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace(&__ref); \
		}} while (0)

#define ERRS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_ERR, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace(&__ref); \
		}} while (0)


#define YAPX(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		tracex(TRACE_REF(TRACE_LVL_YAP, TRACE_XXD, __STR), __BUF, __LEN); \
		}} while (0)

#define DBGX(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		tracex(TRACE_REF(TRACE_LVL_DBG, TRACE_XXD, __STR), __BUF, __LEN); \
		}} while (0)

#define INFX(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		tracex(TRACE_REF(TRACE_LVL_INF, TRACE_XXD, __STR), __BUF, __LEN); \
		}} while (0)

#define WARNX(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		tracex(TRACE_REF(TRACE_LVL_WARN, TRACE_XXD, __STR), __BUF, __LEN); \
		}} while (0)

#define ERRX(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		tracex(TRACE_REF(TRACE_LVL_ERR, TRACE_XXD, __STR), __BUF, __LEN); \
		}} while (0)


#define YAPA(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		tracex(TRACE_REF(TRACE_LVL_YAP, TRACE_AD, __STR), __BUF, __LEN); \
		}} while (0)

#define DBGA(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		tracex(TRACE_REF(TRACE_LVL_DBG, TRACE_AD, __STR), __BUF, __LEN); \
		}} while (0)

#define INFA(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		tracex(TRACE_REF(TRACE_LVL_INF, TRACE_AD, __STR), __BUF, __LEN); \
		}} while (0)

#define WARNA(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		tracex(TRACE_REF(TRACE_LVL_WARN, TRACE_AD, __STR), __BUF, __LEN); \
		}} while (0)

#define ERRA(__STR, __BUF, __LEN) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		tracex(TRACE_REF(TRACE_LVL_ERR, TRACE_AD, __STR), __BUF, __LEN); \
		}} while (0)


#define YAPS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_YAP, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace_I(&__ref); \
		}} while (0)

#define DBGS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_DBG, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace_I(&__ref); \
		}} while (0)

#define INFS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_INF, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace_i(&__ref); \
		}} while (0)

#define WARNS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_WARN, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace_i(&__ref); \
		}} while (0)

#define ERRS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ .line=__LINE__, .lvl=TRACE_LVL_ERR, .opt=0, \
		  .func=__func__, .fmt=__STR}; \
		trace_i(&__ref); \
		}} while (0)

#else

#define DBG(__FMT, ...)
#define INF(__FMT, ...)
#define WARN(__FMT, ...)
#define ERR(__FMT, ...)
#define YAP(__FMT, ...)

#define DBG_I(__FMT, ...)
#define INF_I(__FMT, ...)
#define WARN_I(__FMT, ...)
#define ERR_I(__FMT, ...)
#define YAP_I(__FMT, ...)

#define DBGS(__STR)
#define INFS(__STR)
#define WARNS(__STR)
#define ERRS(__STR)
#define YAPS(__STR)

#define DBGX(__STR, __BUF, __LEN)
#define INFX(__STR, __BUF, __LEN)
#define WARNX(__STR, __BUF, __LEN)
#define ERRX(__STR, __BUF, __LEN)
#define YAPX(__STR, __BUF, __LEN)

#define DBGS_I(__STR)
#define INFS_I(__STR)
#define WARNS_I(__STR)
#define ERRS_I(__STR)
#define YAPS_I(__STR)

#endif

extern const char * const trace_lvl_nm[];

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * Real-Time Trace
 * ----------------------------------------------------------------------
 */

uint32_t trace_timestamp(void);

uint32_t trace_ts2us(uint32_t tm);

void trace_init(void);

int32_t trace_dt(uint32_t * prev);

//__attribute__((format (__printf__, 1, 2)))

void tracef(const struct trace_ref * ref, ... );

void trace(const struct trace_ref * ref);

void tracef_i(const struct trace_ref * ref, ... );

void trace_i(const struct trace_ref * ref);

int trace_tm2timeval(struct timeval * tv, uint64_t tm);

int trace_ts2timeval(struct timeval * tv, uint32_t ts);

void tracex(const struct trace_ref * ref, const void * buf, size_t len);

/* ----------------------------------------------------------------------
 * Trace decode
 * ----------------------------------------------------------------------
 */

int trace_fprint(FILE * f, unsigned int opt);

int trace_getfirst(struct trace_entry * entry, char * s, int max);

int trace_getnext(struct trace_entry * entry, char * s, int max);

void trace_flush(struct trace_entry * entry);

int trace_tail(struct trace_entry * entry);

/* ----------------------------------------------------------------------
 * Trace kernel level decode (no locking)
 * ----------------------------------------------------------------------
 */

int trace_krn_getfirst(struct trace_entry * entry, char * s, int len);

int trace_krn_getnext(struct trace_entry * entry, char * s, int len);

int trace_krn_tail(struct trace_entry * entry);

#ifdef __cplusplus
}
#endif	

#endif /* __TRACE_H__ */
