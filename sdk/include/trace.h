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

enum trace_level {
	TRACE_LVL_NONE = 0,
	TRACE_LVL_ERR = 1,
	TRACE_LVL_WARN = 2,
	TRACE_LVL_INF = 3,
	TRACE_LVL_DBG = 4,
	TRACE_LVL_YAP = 5
};

/* Trace reference */
struct trace_ref {
	unsigned char lvl;   /* Trace level */
	unsigned char opt;   /* Options (unused) */
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

void inline __attribute__((always_inline,format (__printf__, 1, 2))) 
	tracef_chk(const char * __fmt, ... ) {};

#define YAP(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_YAP, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define DBG(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_DBG, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define INF(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_INF, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define WARN(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_WARN, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define ERR(__FMT, ...) do { tracef_chk(__FMT, ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_ERR, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef(&__ref, ## __VA_ARGS__); \
	}} while (0)


#define YAP_I(__FMT, ...) do { tracef_chk( __fmt, , ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_YAP, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef_i(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define DBG_I(__FMT, ...) do { tracef_chk( __fmt, , ## __VA_ARGS__); \
	if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_DBG, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef_i(&__ref, ## __VA_ARGS__); \
	}} while (0)

#define INF_I(__FMT, ...) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_INF, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef_chk( __fmt, , ## __VA_ARGS__); \
		tracef_i(&__ref, ## __VA_ARGS__); \
		}} while (0)

#define WARN_I(__FMT, ...) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_WARN, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef_chk( __fmt, , ## __VA_ARGS__); \
		tracef_i(&__ref, ## __VA_ARGS__); \
		}} while (0)

#define ERR_I(__FMT, ...) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_ERR, 0, __LINE__, __FUNCTION__, __FMT}; \
		tracef_chk( __fmt, , ## __VA_ARGS__); \
		tracef_i(&__ref, ## __VA_ARGS__); \
		}} while (0)


#define YAPS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_YAP, 0, __LINE__, __FUNCTION__, __STR}; \
		trace(&__ref); \
		}} while (0)

#define DBGS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_DBG, 0, __LINE__, __FUNCTION__, __STR}; \
		trace(&__ref); \
		}} while (0)

#define INFS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_INF, 0, __LINE__, __FUNCTION__, __STR}; \
		trace(&__ref); \
		}} while (0)

#define WARNS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_WARN, 0, __LINE__, __FUNCTION__, __STR}; \
		trace(&__ref); \
		}} while (0)

#define ERRS(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_ERR, 0, __LINE__, __FUNCTION__, __STR}; \
		trace(&__ref); \
		}} while (0)

#define YAPS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_YAP)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_YAP, 0, __LINE__, __FUNCTION__, __STR}; \
		trace_I(&__ref); \
		}} while (0)

#define DBGS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_DBG)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_DBG, 0, __LINE__, __FUNCTION__, __STR}; \
		trace_I(&__ref); \
		}} while (0)

#define INFS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_INF)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_INF, 0, __LINE__, __FUNCTION__, __STR}; \
		trace_i(&__ref); \
		}} while (0)

#define WARNS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_WARN)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_WARN, 0, __LINE__, __FUNCTION__, __STR}; \
		trace_i(&__ref); \
		}} while (0)

#define ERRS_I(__STR) do { if (TRACE_LEVEL >= TRACE_LVL_ERR)  { \
		static const struct trace_ref __ref = \
		{ TRACE_LVL_ERR, 0, __LINE__, __FUNCTION__, __STR}; \
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

/* ----------------------------------------------------------------------
 * Trace decode
 * ----------------------------------------------------------------------
 */

int trace_fprint(FILE * f, unsigned int opt);

int trace_getfirst(struct trace_entry * entry, char * s, int max);

int trace_getnext(struct trace_entry * entry, char * s, int max);

void trace_flush(struct trace_entry * entry);

int trace_tail(struct trace_entry * entry);

#ifdef __cplusplus
}
#endif	

#endif /* __TRACE_H__ */
