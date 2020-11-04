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
#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>
#include <vt100.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#include <sys/dcclog.h>

#if (THINKOS_ENABLE_KRNSVC)

void thinkos_krnsvc1(int32_t * arg, int self)
{
}

void thinkos_krnsvc2(int32_t * arg, int self)
{
}

void thinkos_krnsvc_isr(uint32_t arg[], uint32_t opc)
{
	switch (opc) {
	case THINKOS_KRNSVC_SIGNAL:
		DCC_LOG1(LOG_WARNING, "Signal: ", arg[0]);
		break;

	case THINKOS_KRNSVC_ERROR:
		{
			int32_t err = arg[0];
			DCC_LOG2(LOG_ERROR, VT_PSH VT_FBK VT_BYW
					 "/!\\ Error %d [%s] /!\\" VT_POP, 
					 err, thinkos_err_name_lut[err]);
		}
		break;

	case THINKOS_KRNSVC_THREAD_START:
		DCC_LOG(LOG_WARNING, "Thread create hook!!!");
		break;

	case THINKOS_KRNSVC_THREAD_STOP:
		DCC_LOG(LOG_WARNING, "Thread terminate hook!!!");
		break;

	default:
		DCC_LOG1(LOG_ERROR, "Invalid kernel service %d!!!", opc);
		break;
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#pragma GCC diagnostic pop

#endif /* THINKOS_ENABLE_KRNSVC */

