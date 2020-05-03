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
 * @file gdb-rsp.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#include <stdlib.h>
#include <stdbool.h>

#include <sys/dcclog.h>
#include <sys/stm32f.h>
#include <sys/param.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#include <thinkos.h>

/* -------------------------------------------------------------------------
 * Flash memory auxiliary functions 
 * ------------------------------------------------------------------------- */

int thinkos_krn_mem_flash_req(struct thinkos_flash_drv * drv, 
							  struct flash_op_req * req)
{
	int ret;

	while (req->opc != THINKOS_FLASH_MEM_NOP) {
		__idle_hook_req(IDLE_HOOK_FLASH_MEM);

		if ((ret = dbgmon_expect(DBGMON_FLASH_DRV)) < 0) {
			DCC_LOG(LOG_WARNING, "dbgmon_expect()!");
			return ret;
		}
	}

	return req->ret;
}

int dbgmon_flash_write(uint32_t addr, const void * buf, size_t size)
{
	const struct mem_desc * mem = board_flash_desc.mem;
	struct thinkos_flash_drv * drv = &board_flash_drv;
	struct flash_op_req * req = &drv->krn_req;
	off_t offs;
	int ret;


	offs = addr - mem->base;
	(void)offs;

	req->buf = (void *)buf;
	req->size = size;
	req->opc = THINKOS_FLASH_MEM_WRITE;

	ret = thinkos_krn_mem_flash_req(drv, req);

	return ret;
}

int dbgmon_flash_erase(uint32_t addr, size_t size)
{
	const struct mem_desc * mem = board_flash_desc.mem;
	struct thinkos_flash_drv * drv = &board_flash_drv;
	struct flash_op_req * req = &drv->krn_req;
	off_t offs;
	int ret;

	offs = addr - mem->base;
	(void)offs;

	req->size = size;
	req->opc = THINKOS_FLASH_MEM_ERASE;

	ret = thinkos_krn_mem_flash_req(drv, req);

	return ret;
}
