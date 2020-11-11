/* 
 * thinkos_type.c
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

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>
#include <thinkos.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

const struct {
	union {
		uint8_t wq[THINKOS_WQ_CNT]; /* placeholder */
		struct {	
			uint8_t wq_ready; /* ready threads queue */
#if (THINKOS_ENABLE_TIMESHARE)
			uint8_t wq_tmshare; /* Threads waiting for time share cycle */
#endif
#if (THINKOS_ENABLE_CLOCK)
			uint8_t wq_clock;
#endif
#if (THINKOS_MUTEX_MAX) > 0
			uint8_t wq_mutex[THINKOS_WQ_MUTEX_CNT];
#endif
#if (THINKOS_COND_MAX) > 0
			uint8_t wq_cond[THINKOS_WQ_COND_CNT];
#endif
#if (THINKOS_SEMAPHORE_MAX) > 0
			uint8_t wq_sem[THINKOS_WQ_SEMAPHORE_CNT];
#endif
#if (THINKOS_EVENT_MAX) > 0
			uint8_t wq_event[THINKOS_WQ_EVENT_CNT];
#endif
#if (THINKOS_FLAG_MAX) > 0
			uint8_t wq_flag[THINKOS_WQ_FLAG_CNT];
#endif
#if (THINKOS_GATE_MAX) > 0
			uint8_t wq_gate[THINKOS_WQ_GATE_CNT];
#endif
#if (THINKOS_ENABLE_JOIN)
			uint8_t wq_join[THINKOS_WQ_JOIN_CNT];
#endif
#if (THINKOS_ENABLE_CONSOLE)
			uint8_t wq_console_wr;
			uint8_t wq_console_rd;
#endif
#if (THINKOS_ENABLE_PAUSE)
			uint8_t wq_paused;
#endif
#if (THINKOS_ENABLE_JOIN)
			uint8_t wq_canceled; /* canceled threads wait queue */
#endif
#if (THINKOS_ENABLE_COMM)
			uint8_t wq_comm_send;
			uint8_t wq_comm_recv;
#endif
#if (THINKOS_IRQ_MAX) > 0
			uint8_t wq_irq;
#endif
#if (THINKOS_DMA_MAX) > 0
			uint8_t wq_dma;
#endif
#if (THINKOS_FLASH_MEM_MAX) > 0
			uint8_t wq_flash_mem[THINKOS_WQ_FLASH_MEM_CNT];
#endif
#if (THINKOS_ENABLE_THREAD_FAULT)
			uint8_t wq_fault;
#endif
		};
	};
} thinkos_obj_kind_lut = {
	.wq_ready = THINKOS_OBJ_READY,
#if THINKOS_ENABLE_TIMESHARE
	.wq_tmshare = THINKOS_OBJ_TMSHARE,
#endif
#if THINKOS_ENABLE_CLOCK
	.wq_clock = THINKOS_OBJ_CLOCK,
#endif
#if THINKOS_MUTEX_MAX > 0
	.wq_mutex = { [0 ... (THINKOS_MUTEX_MAX - 1)] = THINKOS_OBJ_MUTEX },
#endif 
#if THINKOS_COND_MAX > 0
	.wq_cond = { [0 ... (THINKOS_COND_MAX - 1)] = THINKOS_OBJ_COND },
#endif
#if THINKOS_SEMAPHORE_MAX > 0
	.wq_sem = { [0 ... (THINKOS_SEMAPHORE_MAX - 1)] = THINKOS_OBJ_SEMAPHORE },
#endif
#if THINKOS_EVENT_MAX > 0
	.wq_event = { [0 ... (THINKOS_EVENT_MAX - 1)] = THINKOS_OBJ_EVENT },
#endif
#if THINKOS_FLAG_MAX > 0
	.wq_flag = { [0 ... (THINKOS_FLAG_MAX - 1)] = THINKOS_OBJ_FLAG },
#endif
#if THINKOS_GATE_MAX > 0
	.wq_gate = { [0 ... (THINKOS_GATE_MAX - 1)] = THINKOS_OBJ_GATE },
#endif
#if THINKOS_ENABLE_JOIN
	.wq_join = { [0 ... (THINKOS_THREADS_MAX - 1)] = THINKOS_OBJ_JOIN },
#endif
#if THINKOS_ENABLE_CONSOLE
	.wq_console_wr = THINKOS_OBJ_CONWRITE,
	.wq_console_rd = THINKOS_OBJ_CONREAD,
#endif
#if THINKOS_ENABLE_PAUSE
	.wq_paused = THINKOS_OBJ_PAUSED,
#endif
#if THINKOS_ENABLE_JOIN
	.wq_canceled = THINKOS_OBJ_CANCELED,
#endif
#if THINKOS_ENABLE_COMM
	.wq_comm_send = THINKOS_OBJ_COMMSEND,
	.wq_comm_recv = THINKOS_OBJ_COMMRECV,
#endif
#if THINKOS_IRQ_MAX > 0
	.wq_irq = THINKOS_OBJ_IRQ,
#endif
#if THINKOS_DMA_MAX > 0
	.wq_irq = THINKOS_OBJ_DMA,
#endif
#if THINKOS_FLASH_MEM_MAX > 0
	.wq_flash_mem = { [0 ... (THINKOS_FLASH_MEM_MAX - 1)] = 
		THINKOS_OBJ_FLASH_MEM },
#endif
#if THINKOS_ENABLE_THREAD_FAULT
	.wq_fault = THINKOS_OBJ_FAULT
#endif
};

#pragma GCC diagnostic pop

