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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
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

#if (THINKOS_ENABLE_FLASH_MEM)
static int monitor_flash_req(struct thinkos_flash_drv * drv, 
							const struct thinkos_flash_desc * desc,
							struct flash_op_req * req)
{
	int ret;

	while (req->opc != THINKOS_FLASH_MEM_NOP) {

		DCC_LOG(LOG_TRACE, "FLASH_DRV tasklet request");
		__idle_hook_req(IDLE_HOOK_FLASH_MEM);

		if ((ret = monitor_expect(MONITOR_FLASH_DRV)) < 0) {
			DCC_LOG(LOG_WARNING, "monitor_expect()!");
			return ret;
		}
	}

	ret = req->ret;
	DCC_LOG1(LOG_TRACE, "ret=%d", ret);

	return ret;
}
#else
static int monitor_flash_req(struct thinkos_flash_drv * drv, 
							  const struct thinkos_flash_desc * desc,
							  struct flash_op_req * req) {
	return thinkos_flash_drv_req(drv, desc,req);
}
#endif

int monitor_flash_write(uint32_t addr, const void * buf, size_t size)
{



	struct thinkos_flash_drv * drv = &board_flash_drv;
	const struct thinkos_flash_desc * desc = &board_flash_desc;
	const struct mem_desc * mem = desc->mem;
	struct flash_op_req * req = &drv->krn_req;

	DCC_LOG2(LOG_TRACE, "addr=0x%08x size=%d", addr, size);

	req->offset = addr - (mem->base + drv->partition.offset);
	req->size = size;
	req->buf = (void *)buf;
	req->opc = THINKOS_FLASH_MEM_WRITE;

	return monitor_flash_req(drv, desc, req);
}

int monitor_flash_erase(uint32_t addr, size_t size)
{
	struct thinkos_flash_drv * drv = &board_flash_drv;
	const struct thinkos_flash_desc * desc = &board_flash_desc;
	const struct mem_desc * mem = desc->mem;
	struct flash_op_req * req = &drv->krn_req;

	DCC_LOG2(LOG_TRACE, "addr=0x%08x size=%d", addr, size);

	req->offset = addr - (mem->base + drv->partition.offset);
	req->size = size;
	req->opc = THINKOS_FLASH_MEM_ERASE;

	return monitor_flash_req(drv, desc, req);
}

int monitor_flash_open(const char * tag)
{
	struct thinkos_flash_drv * drv = &board_flash_drv;
	const struct thinkos_flash_desc * desc = &board_flash_desc;
	struct flash_op_req * req = &drv->krn_req;
	int ret;

	req->tag = tag;
	req->opc = THINKOS_FLASH_MEM_OPEN;

	ret = monitor_flash_req(drv, desc, req);

	if (ret >= 0) {
		/* store key for subsequent requests */
		req->key = ret;
	}

	return ret;
}

int monitor_flash_close(void)
{
	struct thinkos_flash_drv * drv = &board_flash_drv;
	const struct thinkos_flash_desc * desc = &board_flash_desc;
	struct flash_op_req * req = &drv->krn_req;

	req->opc = THINKOS_FLASH_MEM_CLOSE;

	return monitor_flash_req(drv, desc, req);
}

