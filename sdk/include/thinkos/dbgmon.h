/* 
 * File:	 board.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(c) 2003-2006 BORESTE (www.boreste.com). All Rights Reserved.
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

#ifndef __THINKOS_DBGMON_H__
#define __THINKOS_DBGMON_H__

#ifndef __THINKOS_DBGMON__
#error "Never use <thinkos/dbgmon.h> directly; include <thinkos.h> instead."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>

#ifndef THINKOS_ENABLE_RESET_RAM_VECTORS
  #ifdef CM3_RAM_VECTORS
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 1
  #else
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 0
  #endif
#endif

#include <sys/usb-dev.h>

enum dbgmon_event {
	DBGMON_COMM_RCV     = 0,
	DBGMON_COMM_EOT     = 1,
	DBGMON_COMM_CTL     = 2,

	DBGMON_RX_PIPE      = 3,
	DBGMON_TX_PIPE      = 4,
	DBGMON_ALARM        = 5,

	DBGMON_THREAD_STEP  = 8,
	DBGMON_THREAD_FAULT = 12,
	DBGMON_BREAKPOINT   = 16,
	DBGMON_SOFTRST      = 27,
	DBGMON_IRQ_STEP     = 28,
	DBGMON_IDLE         = 29,
	DBGMON_EXCEPT       = 30,
	DBGMON_RESET        = 31
};

struct dmon_comm;


#ifdef __cplusplus
extern "C" {
#endif

void thinkos_dbgmon_init(void * comm, void (* task)(struct dmon_comm * ));

void dbgmon_reset(void);

void __attribute__((noreturn)) dbgmon_exec(void (* task)(struct dmon_comm *));

void dbgmon_unmask(int sig);

void dbgmon_mask(int sig);

void dbgmon_clear(int sig);

void dbgmon_signal(int sig); 

void dbgmon_signal_idle(void);

uint32_t dbgmon_select(uint32_t watch);

int dbgmon_wait(int sig);

int dbgmon_expect(int sig);

int dbgmon_sleep(unsigned int ms);

void dbgmon_alarm(unsigned int ms);

void dbgmon_alarm_stop(void);

int dbgmon_wait_idle(void);

void dbgmon_soft_reset(void);

bool dmon_breakpoint_set(uint32_t addr, uint32_t size);

bool dmon_breakpoint_clear(uint32_t addr, uint32_t size);

void dmon_breakpoint_clear_all(void);

bool dmon_watchpoint_set(uint32_t addr, uint32_t size, int access);

bool dmon_watchpoint_clear(uint32_t addr, uint32_t size);

void dmon_watchpoint_clear_all(void);

int dmon_thread_step(unsigned int id, bool block);

int dmon_comm_send(struct dmon_comm * comm, 
				   const void * buf, unsigned int len);

int dmon_comm_recv(struct dmon_comm * comm, void * buf, unsigned int len);

int dmon_comm_connect(struct dmon_comm * comm);

bool dmon_comm_isconnected(struct dmon_comm * comm);

void dmon_comm_rxflowctrl(struct dmon_comm * comm, bool en);

struct dmon_comm * usb_comm_init(const usb_dev_t * usb);

struct dmon_comm * usb_comm_getinstance(void);

int dmprintf(struct dmon_comm * comm, const char *fmt, ... );

int dmputc(int c, struct dmon_comm * comm);

int dmputs(const char * s, struct dmon_comm * comm);

int dmgets(char * s, int size, struct dmon_comm * comm);

int dmgetc(struct dmon_comm * comm);

int dmscanf(struct dmon_comm * comm, const char *fmt, ... );

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_DBGMON_H__ */

