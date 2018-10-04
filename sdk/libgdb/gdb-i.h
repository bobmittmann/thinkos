#ifndef __GDB_I_H__
#define __GDB_I_H__

#include <audio.h>
#include <string.h>
#include <sys/param.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>


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
 * @file gdb-i.h
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <stdlib.h>
#include <stdbool.h>
#include <sys/dcclog.h>
#include <sys/stm32f.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <gdb.h>

#include "signals.h"

#ifndef GDB_ENABLE_MEMWRITE
#define GDB_ENABLE_MEMWRITE 0
#endif

#define THREAD_ID_OFFS 100
#define THREAD_ID_ALL -1
#define THREAD_ID_ANY 0
#define THREAD_ID_NONE -2
#define THREAD_ID_IDLE (THINKOS_THREAD_IDLE + THREAD_ID_OFFS) 

#ifndef RSP_BUFFER_LEN
#if THINKOS_ENABLE_FPU
#define RSP_BUFFER_LEN (512 + 0)
#else
#define RSP_BUFFER_LEN 512
#endif
#endif

#ifndef GDB_ENABLE_NOACK_MODE
#define GDB_ENABLE_NOACK_MODE 1
#endif

#ifndef GDB_ENABLE_NOSTOP_MODE
#define GDB_ENABLE_NOSTOP_MODE 0
#endif

#ifndef GDB_ENABLE_VCONT
#define GDB_ENABLE_VCONT 1
#endif

#ifndef GDB_ENABLE_VFLASH
#define GDB_ENABLE_VFLASH 0
#endif

#ifndef GDB_ENABLE_MULTIPROCESS
#define GDB_ENABLE_MULTIPROCESS 0
#endif

#ifndef GDB_ENABLE_QXFER_FEATURES
#define GDB_ENABLE_QXFER_FEATURES 1
#endif

#ifndef GDB_ENABLE_QXFER_MEMORY_MAP
#define GDB_ENABLE_QXFER_MEMORY_MAP 1
#endif

#ifndef GDB_ENABLE_COSMETIC
#define GDB_ENABLE_COSMETIC 0
#endif

#ifndef GDB_ENABLE_RXMIT
#define GDB_ENABLE_RXMIT 0
#endif

#define CTRL_B 0x02
#define CTRL_C 0x03
#define CTRL_D 0x04
#define CTRL_E 0x05
#define CTRL_F 0x06
#define CTRL_G 0x07
#define CTRL_H 0x08
#define CTRL_I 0x09
#define CTRL_J 0x0a
#define CTRL_K 0x0b
#define CTRL_L 0x0c
#define CTRL_M 0x0d /* CR */
#define CTRL_N 0x0e
#define CTRL_O 0x0f
#define CTRL_P 0x10
#define CTRL_Q 0x11
#define CTRL_R 0x12
#define CTRL_S 0x13
#define CTRL_T 0x14
#define CTRL_U 0x15
#define CTRL_V 0x16
#define CTRL_W 0x17
#define CTRL_X 0x18
#define CTRL_Y 0x19
#define CTRL_Z 0x1a

struct gdb_rspd {
	uint8_t noack_mode    : 1;
	uint8_t nonstop_mode  : 1;
	uint8_t stopped       : 1;
	uint8_t active_app    : 1;
	uint8_t session_valid : 1;
	uint8_t last_signal;
#if GDB_ENABLE_MULTIPROCESS
	uint16_t pid;
#endif
	struct {
		int8_t g; 
		int8_t c;
	} thread_id;
#if GDB_ENABLE_RXMIT
	struct {
		char * pkt;
		uint16_t len;
	} tx;
#endif
	struct dbgmon_comm * comm;
};

enum gdb_error_code {
	GDB_ERR_THREAD_IS_DEAD = 1,
	GDB_ERR_REGISTER_NOT_KNOWN = 2,
	GDB_ERR_REGISTER_SET_FAIL = 3,
	GDB_ERR_MEMORY_READ_FAIL = 4,
	GDB_ERR_BREAKPOINT_SET_FAIL = 5,
	GDB_ERR_WATCHPOINT_SET_FAIL = 6,
	GDB_ERR_STEP_REQUEST_FAIL = 7,
	GDB_ERR_APP_EXEC_FAIL = 8
};

#ifdef __cplusplus
extern "C" {
#endif

int uint2dec(char * s, unsigned int val);
unsigned long long hex2ll_be(const char * __s, char ** __endp);
unsigned long hex2int(const char * __s, char ** __endp);
bool prefix(const char * __s, const char * __prefix);
int char2hex(char * pkt, int c);
int str2str(char * pkt, const char * s);
int str2hex(char * pkt, const char * s);
int bin2hex(char * pkt, const void * buf, int len);
int int2str2hex(char * pkt, unsigned int val);
int uint2hex(char * s, unsigned int val);
int long2hex_be(char * pkt, unsigned long val);
int longlong2hex_be(char * s, unsigned long long val);
int hex2char(char * hex);
extern const char __hextab[];

int thread_getnext(int thread_id);
int thread_active(void);
int thread_step_id(void);
int thread_break_id(void);
int thread_any(void);
bool thread_isalive(int thread_id);
int thread_register_set(unsigned int thread_id, int reg, uint64_t val);
int thread_register_get(int gdb_thread_id, int reg, uint64_t * val);
int thread_goto(unsigned int thread_id, uint32_t addr);
int thread_step_req(unsigned int thread_id);
int thread_continue(unsigned int thread_id);
int thread_info(unsigned int gdb_thread_id, char * buf);

int target_mem_erase(uint32_t addr, unsigned int len);

int target_mem_write(uint32_t addr, const void * ptr, unsigned int len);

int target_mem_read(uint32_t addr, void * ptr, unsigned int len);

int target_file_read(const char * name, char * dst, 
					 unsigned int offs, unsigned int size);

void target_halt(void);
bool target_continue(void);
int target_goto(uint32_t addr, int opt);


#ifdef __cplusplus
}
#endif

#endif /* __GDB_I_H__ */

