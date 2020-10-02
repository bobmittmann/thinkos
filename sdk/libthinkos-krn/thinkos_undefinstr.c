/* 
 * thinkos_undefinstr.c
 *
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_ERROR__
#include <thinkos/error.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#include <sys/dcclog.h>

void thinkos_krn_svc1(int32_t * arg, int self)
{
}

void thinkos_krn_svc2(int32_t * arg, int self)
{
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

void thinkos_undefinstr_isr(uint32_t arg[], uint32_t opc)
{
	switch (opc) {
	case DBGMON_BKPT_ON_THREAD_CREATE:
		DCC_LOG(LOG_WARNING, "Thread create hook!!!");
		break;

	case DBGMON_BKPT_ON_THREAD_TERMINATE:
		DCC_LOG(LOG_WARNING, "Thread terminate hook!!!");
		break;

	case THINKOS_NO_ERROR ... THINKOS_ERR_MAX:
		DCC_LOG1(LOG_ERROR, "Kernel error %d!!!", opc);
		break;

	default:
		break;
	}
}

#pragma GCC diagnostic pop

