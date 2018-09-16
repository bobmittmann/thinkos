/* 
 * File:	 usb-cdc.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

/** 
 * @file monitor.c
 * @brief YARD-ICE debug monitor
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <ascii.h>
#include "version.h"

#define APPLICATION_BLOCK_OFFS 0x00020000
#define APPLICATION_BLOCK_SIZE (384 * 1024)
#define APPLICATION_START_ADDR (0x08000000 + APPLICATION_BLOCK_OFFS)

int app_main(int argc, char *argv[]);

const char * const argv[] = { "thinkos_app" };

void __attribute__((noreturn)) board_app_task(void * param)
{
//	int argc = 1;

	console_write(NULL, "\r\nThinkOS app loader...\r\n\r\n", 27);

	for (;;) {
//		app_main(argc, (char **)argv);
		thinkos_sleep(500);
	}
}

extern int __heap_end;
const void * heap_end = &__heap_end; 
extern uint32_t _stack;
extern const struct thinkos_thread_inf thinkos_main_inf;

void board_init(void);
void board_app_task(void * arg);

extern const uint8_t otg_xflash_pic[];
extern const unsigned int sizeof_otg_xflash_pic;

struct magic_hdr {
	uint16_t pos;
	uint16_t cnt;
};

struct magic_rec {
	uint32_t mask;
	uint32_t comp;
};

struct magic {
	struct magic_hdr hdr;
	struct magic_rec rec[];
};

static const char s_version[] = "ThinkOS " VERSION_NUM "\r\n";

static const char s_help[] = 
" ^O - OS Info\r\n"
" ^P - Pause app\r\n"
" ^R - Upload FPGA\r\n"
" ^V - Help\r\n"
" ^W - Wipe App\r\n"
" ^Y - Upload YARD-ICE\r\n"
" ^Z - Restart\r\n"
" ^\\ - Upload ThinkOS\r\n"
;

static const char s_hr[] = 
"\r\n----\r\n";

//static const char s_error[] = "Error!\r\n";
static const char s_confirm[] = "Confirm [y]?";

static int yflash(uint32_t blk_offs, uint32_t blk_size,
		   const struct magic * magic)
{
	uint32_t * yflash_code = (uint32_t *)(0x20001000);
	int (* yflash_ram)(uint32_t, uint32_t, const struct magic *) = 
		((void *)yflash_code) + 1;
//	unsigned int pri;
	int ret;

//	pri = cm3_primask_get();
	cm3_primask_set(1);

	__thinkos_memcpy(yflash_code, otg_xflash_pic, sizeof_otg_xflash_pic);
	ret = yflash_ram(blk_offs, blk_size, magic);

//	cm3_primask_set(pri);

	return ret;
}

static const struct magic app_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 3
	},
	.rec = {
		{  0xffffffff, 0x0a0de004 },
		{  0xffffffff, 0x6e696854 },
		{  0xffffffff, 0x00534f6b }
	}
};

static int app_yflash(void)
{
	return yflash(APPLICATION_BLOCK_OFFS, APPLICATION_BLOCK_SIZE, &app_magic);
}


static const struct magic bootloader_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 2
	},
	.rec = {
		{  0xfffc0000, 0x20000000 },
		{  0xffff0000, 0x08000000 },
	}
};

static void bootloader_yflash(void)
{
	yflash(0, 32768, &bootloader_magic);
}

int __scan_stack(void * stack, unsigned int size);

extern int app_loader(void * arg);

static void print_osinfo(struct dbgmon_comm * comm)
{
	struct thinkos_rt * rt = &thinkos_rt;
	uint32_t cycbuf[THINKOS_THREADS_MAX + 2];
	uint32_t cyccnt;
	int32_t delta;
	uint32_t cycdiv;
	uint32_t busy;
	uint32_t cycsum = 0;
	uint32_t cycbusy;
	uint32_t idle;
	const char * tag;
	int i;

	cyccnt = CM3_DWT->cyccnt;
	delta = cyccnt - thinkos_rt.cycref;
	/* update the reference */
	thinkos_rt.cycref = cyccnt;
	/* update active thread's cycle counter */
	thinkos_rt.cyccnt[thinkos_rt.active] += delta; 
	/* copy the thread counters to a buffer */
	__thinkos_memcpy32(cycbuf, rt->cyccnt, sizeof(cycbuf));
	/* reset cycle counters */
	__thinkos_memset32(rt->cyccnt, 0, sizeof(cycbuf));

	dbgmon_printf(comm, s_hr);
	cycsum = 0;
	for (i = 0; i < THINKOS_THREADS_MAX; ++i)
		cycsum += cycbuf[i];
	cycbusy = cycsum;
	cycsum += cycbuf[THINKOS_CYCCNT_IDLE];

	cycdiv = (cycsum + 500) / 1000;
	busy = (cycbusy + cycdiv / 2) / cycdiv;
	idle = 1000 - busy;
	dbgmon_printf(comm, "CPU: %d.%d%% busy, %d.%d%% idle\r\n", 
			 busy / 10, busy % 10, idle / 10, idle % 10);

	dbgmon_printf(comm, " Th     Tag       SP       LR       PC  WQ TmW");
	dbgmon_printf(comm, " CPU %% ");
	dbgmon_printf(comm, " Locks");
	dbgmon_printf(comm, "\r\n");

	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		if (rt->ctx[i] != NULL) {
			int j;
			/* Internal thread ids start form 0 whereas user
			   thread numbers start form one ... */
			tag = (rt->th_inf[i] != NULL) ? rt->th_inf[i]->tag : "...";
			dbgmon_printf(comm, "%3d %7s %08x %08x %08x %3d %s", i + 1, tag,
					 (uint32_t)rt->ctx[i], rt->ctx[i]->lr, rt->ctx[i]->pc, 
					 rt->th_stat[i] >> 1, rt->th_stat[i] & 1 ? "Yes" : " No");

			busy = (cycbuf[i] + cycdiv / 2) / cycdiv;
			dbgmon_printf(comm, " %3d.%d", busy / 10, busy % 10);

			for (j = 0; j < THINKOS_MUTEX_MAX ; ++j) {
				if (rt->lock[j] == i)
					dbgmon_printf(comm, " %d", j + THINKOS_MUTEX_BASE);
			}
			dbgmon_printf(comm, "\r\n");
		}
	}
}

static void __app_exec(void)
{
	int thread_id = 0;

	DCC_LOG(LOG_TRACE, "__thinkos_thread_abort()");
	__thinkos_thread_abort(thread_id);

	DCC_LOG(LOG_TRACE, "__thinkos_thread_init()");
	__thinkos_thread_init(thread_id, (uintptr_t)&_stack, board_app_task, 
						  (void *)NULL);

	__thinkos_thread_inf_set(thread_id, &thinkos_main_inf);

	DCC_LOG(LOG_TRACE, "__thinkos_thread_resume()");
	__thinkos_thread_resume(thread_id);

	DCC_LOG(LOG_TRACE, "__thinkos_defer_sched()");
	__thinkos_defer_sched();
}

static void pause_all(void)
{
	unsigned int wq;
	unsigned int irq;


	/* clear all bits on all queues */
	for (wq = 0; wq < THINKOS_WQ_LST_END; ++wq) 
		thinkos_rt.wq_lst[wq] = 0;

//	thinkos_rt.wq_ready = 1 << (THINKOS_THREADS_MAX);

	for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
		if (thinkos_rt.irq_th[irq] != THINKOS_THREAD_IDLE)
			cm3_irq_disable(irq);
	}

	__thinkos_defer_sched();
}

static bool monitor_process_input(struct dbgmon_comm * comm, int c)
{
	switch (c) {
	case CTRL_FS:
		dbgmon_puts(s_confirm, comm);
		if (dbgmon_getc(comm) == 'y')
			bootloader_yflash();
		break;
	case CTRL_O:
		print_osinfo(comm);
		break;
	case CTRL_V:
		dbgmon_puts(s_hr, comm);
		dbgmon_puts(s_version, comm);
		dbgmon_puts(s_help, comm);
		break;
	case CTRL_P:
		dbgmon_puts("^P\r\n", comm);
		pause_all();
		break;
	case CTRL_Y:
		dbgmon_puts(s_confirm, comm);
		if (dbgmon_getc(comm) == 'y') {
			app_yflash();
		} else {
			dbgmon_puts("\r\n", comm);
		}

		break;

	case CTRL_Z:
		dbgmon_puts("^Z\r\n", comm);
		dbgmon_soft_reset();
		/* Request app exec */
		dbgmon_signal(DBGMON_APP_EXEC); 
		break;

	default:
		return false;
	}

	return true;
}

/*
   Dafault Monitor Task
 */
#define MONITOR_AUTOBOOT 1
#define MONITOR_SHELL 2

void __attribute__((noreturn)) monitor_task(struct dbgmon_comm * comm, void * param)
{
	uint32_t sigmask;
	uint32_t sigset;
	uint8_t buf[1];
	uint8_t * ptr;
	int cnt;
	int len;
	uint32_t flags = (uint32_t)param;

	/* unmask events */
	sigmask =  (1 << DBGMON_STARTUP) | (1 << DBGMON_SOFTRST) | 
		(1 << DBGMON_COMM_RCV) | (1 << DBGMON_TX_PIPE) | 
		(1 << DBGMON_RX_PIPE) | (1 << DBGMON_APP_EXEC) |
		(1 << DBGMON_COMM_CTL);
	DCC_LOG1(LOG_TRACE, "unmasking events=%08x", sigmask);

	if (!(flags & MONITOR_AUTOBOOT)) {
		dbgmon_puts(s_hr, comm);
		dbgmon_puts(s_version, comm);
	}

	if (flags & MONITOR_AUTOBOOT) {
		DCC_LOG(LOG_TRACE, "Autoboot...");
		dbgmon_soft_reset();
		dbgmon_signal(DBGMON_APP_EXEC); 
	}

	DCC_LOG(LOG_TRACE, "Main loop...");
	for(;;) {
		sigset = dbgmon_select(sigmask);
		DCC_LOG1(LOG_MSG, "sigset=%08x", sigset);

		if (sigset & (1 << DBGMON_COMM_CTL)) {
			DCC_LOG1(LOG_TRACE, "Comm Ctl, sigset=%08x", sigset);
			dbgmon_clear(DBGMON_COMM_CTL);
//			if (!dbgmon_comm_isconnected(comm))	
//				dbgmon_reset();
		}

		if (sigset & (1 << DBGMON_SOFTRST)) {
			dbgmon_clear(DBGMON_SOFTRST);
			board_init();
		}

		if (sigset & (1 << DBGMON_STARTUP)) {
			dbgmon_clear(DBGMON_STARTUP);
		}

		if (sigset & (1 << DBGMON_APP_EXEC)) {
			DCC_LOG(LOG_TRACE, "/!\\ APP_EXEC signal !");
			dbgmon_clear(DBGMON_APP_EXEC);
			__app_exec();
		}

		if (sigset & (1 << DBGMON_COMM_RCV)) {
			DCC_LOG(LOG_TRACE, "DBGMON_COMM_RCV siganl");
			if (flags & MONITOR_SHELL) { 
				/* receive from the COMM driver one bye at the time */
				if (dbgmon_comm_recv(comm, buf, 1) > 0) {
					int c = buf[0];
					DCC_LOG1(LOG_INFO, "Comm recv. %c", c);
					/* process the input character */
					if (!monitor_process_input(comm, c)) {
						/* if the character was not consumed by the monitor 
						   insert into the console pipe */
						/* get a pointer to the head of the pipe.
						   __console_rx_pipe_ptr() will return the number of 
						   consecutive spaces in the buffer. We need only one. */
						if (__console_rx_pipe_ptr(&ptr) > 0) {
							/* copy the character into the RX fifo */
							ptr[0] = c;
							/* commit the fifo head */
							__console_rx_pipe_commit(1);
						}
					}
				} 
			} else {
				if ((cnt = __console_rx_pipe_ptr(&ptr)) > 0) {
					DCC_LOG1(LOG_INFO, "Comm recv. rx_pipe.free=%d", cnt);
					if ((len = dbgmon_comm_recv(comm, ptr, cnt)) > 0)
						__console_rx_pipe_commit(len); 
				} else {
					DCC_LOG(LOG_INFO, "Comm recv. Masking DMON_COMM_RCV!");
					sigmask &= ~(1 << DBGMON_COMM_RCV);
				}
			}
		}

		if (sigset & (1 << DBGMON_RX_PIPE)) {
			if ((cnt = __console_rx_pipe_ptr(&ptr)) > 0) {
				DCC_LOG1(LOG_INFO, "RX Pipe. rx_pipe.free=%d. "
						 "Unmaksing DMON_COMM_RCV!", cnt);
				sigmask |= (1 << DBGMON_COMM_RCV);
			} else {
				DCC_LOG(LOG_INFO, "RX Pipe empty!!!");
			}
			dbgmon_clear(DBGMON_RX_PIPE);
		}


		if (sigset & (1 << DBGMON_TX_PIPE)) {
			DCC_LOG(LOG_MSG, "TX Pipe.");
			if ((cnt = __console_tx_pipe_ptr(&ptr)) > 0) {
				DCC_LOG1(LOG_INFO, "TX Pipe, %d pending chars.", cnt);
				cnt = dbgmon_comm_send(comm, ptr, cnt);
				__console_tx_pipe_commit(cnt); 
			} else {
				DCC_LOG(LOG_INFO, "TX Pipe empty!!!");
				dbgmon_clear(DBGMON_TX_PIPE);
			}
		}
	}
}
