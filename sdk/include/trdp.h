/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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
 * @file trdp.h
 * @brief ThinkOS Remote Debug Protocol
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __TRDP_H__
#define __TRDP_H__

#include <stdint.h>


/*
   ThirdLnk pakcet layout
 */

enum thirdrpc_op {
	TRDRPC_SUSPEND       = 1,
	TRDRPC_RESUME        = 2,
	TRDRPC_EXEC          = 3,
	TRDRPC_REBOOT        = 4,
	TRDRPC_KERNELINFO    = 5,
	TRDRPC_MEM_LOCK      = 6,
	TRDRPC_MEM_UNLOCK    = 7,
	TRDRPC_MEM_ERASE     = 8,
	TRDRPC_MEM_READ      = 9,
	TRDRPC_MEM_WRITE     = 10,
	TRDRPC_MEM_SEEK      = 11,
	TRDRPC_MEM_CRC32     = 12,
	TRDRPC_EXCEPTINFO    = 13,
	TRDRPC_THREADINFO    = 14,
	TRDRPC_AUTH_INIT     = 65,
	TRDRPC_AUTH_DESC     = 67,
	TRDRPC_AUTH_REQ      = 69,
	TRDRPC_AUTH_RSA1024  = 71,
};

#define TRDLNK_MTU (512 + 4)

struct trdp_credentials {
};

struct trdp_auth_init {
	uint32_t session;
	uint32_t agent;
	int64_t time;
};

struct trdp_auth_desc {
	uint32_t session;
	uint32_t agent;
	uint8_t method[4];
};

struct trdp_auth_req {
	uint32_t session;
	uint32_t sequence;
	uint8_t method;
	uint32_t key[4];
};

struct trdp_auth_rsa1024 {
	uint32_t session;
	uint32_t sequence;
	uint32_t blk[];
};

#ifdef __cplusplus
extern "C" {
#endif

void erd_stub_task(struct dmon_comm * comm);

#ifdef __cplusplus
}
#endif	

#endif /* __TRDP_H__ */

