/* 
 * File:	 /thinkos/dbgmon.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

#ifndef THINKOS_ENABLE_RESET_RAM_VECTORS
  #ifdef CM3_RAM_VECTORS
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 1
  #else
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 0
  #endif
#endif

#ifndef THINKOS_DBGMON_STACK_SIZE
#define THINKOS_DBGMON_STACK_SIZE (960 + 16)
#endif

#ifndef THINKOS_DBGMON_ENABLE_IRQ_MGMT
#define THINKOS_DBGMON_ENABLE_IRQ_MGMT 1 
#endif

#ifndef THINKOS_DBGMON_ENABLE_RST_VEC
#define THINKOS_DBGMON_ENABLE_RST_VEC CM3_RAM_VECTORS 
#endif


#include <sys/usb-dev.h>

/* ----------------------------------------------------------------------------
 *  Debug/Monitor events
 * ----------------------------------------------------------------------------
 */

enum dbgmon_event {
	/* Debug monitor internal reset */
	DBGMON_RESET           = 0,
	/* ThinkOS power on startup indication */
	DBGMON_STARTUP         = 1,
	/* ThinkOS idle indication */
	DBGMON_IDLE            = 2,
	/* ThinkOS exception */
	DBGMON_EXCEPT          = 3,
	/* Board reset request */
	DBGMON_SOFTRST         = 4,
	/* Debug timer expiry indication */
	DBGMON_ALARM           = 5,
	/* ThinkOS Thread step break */
	DBGMON_THREAD_STEP     = 6,
	/* ThinkOS Thread fault break */
	DBGMON_THREAD_FAULT    = 7,
	/* ThinkOS Thread create */
	DBGMON_THREAD_CREATE   = 8,
	/* ThinkOS Thread teminate */
	DBGMON_THREAD_TERMINATE = 9,
	/* ThinkOS Thread breakpoint */
	DBGMON_BREAKPOINT      = 10,
	/* Debug Communication data received pending */
	DBGMON_COMM_RCV        = 11, 
	/* Debug Communication end of transfer */
	DBGMON_COMM_EOT        = 12,
	/* Debug Communication control signal */
	DBGMON_COMM_CTL        = 13,
	/* User console RX pipe data pending */
	DBGMON_RX_PIPE         = 14,
	/* User console TX pipe not full */
	DBGMON_TX_PIPE         = 15,

	/* ThinkOS application stop request */
	DBGMON_APP_STOP        = 18,
	/* ThinkOS application resume request */
	DBGMON_APP_RESUME      = 19,
	/* ThinkOS application terminate request */
	DBGMON_APP_TERM        = 20,
	/* ThinkOS application erase request */
	DBGMON_APP_ERASE       = 21,
	/* ThinkOS application 2pload request */
	DBGMON_APP_UPLOAD      = 22,
	/* ThinkOS application exec request */
	DBGMON_APP_EXEC        = 23,
	/* User/bootloader extension events 0 to 7 */
	DBGMON_USER_EVENT0     = 24,
	DBGMON_USER_EVENT1     = 25,
	DBGMON_USER_EVENT2     = 26,
	DBGMON_USER_EVENT3     = 27,
	DBGMON_USER_EVENT4     = 28,
	DBGMON_USER_EVENT5     = 29,
	DBGMON_USER_EVENT6     = 30,
	DBGMON_USER_EVENT7     = 31
};

#define SIG_SET(SIGSET, SIG) SIGSET |= (1 << (SIG))
#define SIG_CLR(SIGSET, SIG) SIGSET &= ~(1 << (SIG))
#define SIG_ISSET(SIGSET, SIG) (SIGSET & (1 << (SIG)))
#define SIG_ZERO(SIGSET) SIGSET = 0

/* ----------------------------------------------------------------------------
 *  Debug/Monitor Memory Description 
 * ----------------------------------------------------------------------------
 */
#define SZ_128   7
#define SZ_256   8
#define SZ_1K   10
#define SZ_2K   11
#define SZ_4K   12
#define SZ_8K   13
#define SZ_16K  14
#define SZ_32K  15
#define SZ_64K  16
#define SZ_128K 17
#define SZ_256K 18
#define SZ_512K 19
#define SZ_1M   20
#define SZ_2M   21
#define SZ_4M   22
#define SZ_8M   23
#define SZ_16M  24
#define SZ_32M  25
#define SZ_64M  26
#define SZ_128M 27
#define SZ_256M 28
#define SZ_512M 29
#define SZ_1G   30
#define SZ_2G   31

/* Memory flags */
#define M_XC  (1 << 7) /* Execution */
#define M_DM  (1 << 6) /* DMA  */
#define M_PE  (1 << 5) /* Peripheral  */
#define M_NV  (1 << 4) /* Non volatile  */
#define M_EX  (1 << 3) /* External  */
#define M_RW  (0 << 2) /*   */
#define M_RO  (1 << 2) /* Read Only  */
#define M_64  (3 << 0) /* 64 bit alignement  */
#define M_32  (2 << 0) /* 32 bits alignement  */
#define M_16  (1 << 0) /* 16 bits alignment */
#define M_8   (0 << 0) /* 8 bits alignment/access */

/* Memory block descriptor */
struct blk_desc {
	char tag[8];
	uint32_t ref;
	uint8_t  opt;
	uint8_t  siz;
	uint16_t cnt;
};

/* Memory region/type descriptor */
struct mem_desc {
	char tag[8];
	uint8_t cnt; /* number of entries in the block list */
	struct blk_desc blk[]; /* sorted block list */
};

/* ----------------------------------------------------------------------------
 *  Debug/Monitor communication interface
 * ----------------------------------------------------------------------------
 */

struct dbgmon_comm_op {
	int (*send)(const void * dev, const void * buf, unsigned int len);
	int (*recv)(const void * dev, void * buf, unsigned int len);
	int (* connect)(const void * dev);
	bool (* isconnected)(const void * dev);
	void (* rxflowctrl)(const void * dev, bool stop);
};

struct dbgmon_comm {
	const void * dev;
	const struct dbgmon_comm_op * op;
};

static inline int dbgmon_comm_send(const struct dbgmon_comm * comm, 
								   const void * buf, unsigned int len) {
	return comm->op->send(comm->dev, buf, len);
}

static inline int dbgmon_comm_recv(const struct dbgmon_comm * comm,
								   void * buf, unsigned int len) {
	return comm->op->recv(comm->dev, buf, len);
}

static inline int dbgmon_comm_connect(const struct dbgmon_comm * comm) {
	return comm->op->connect(comm->dev);
}

static inline bool dbgmon_comm_isconnected(const struct dbgmon_comm * comm) {
	return comm->op->isconnected(comm->dev);
}

static inline void dbgmon_comm_rxflowctrl(const struct dbgmon_comm * comm, 
									   bool stop) {
	comm->op->rxflowctrl(comm->dev, stop);
}


#ifdef __cplusplus
extern "C" {
#endif

void thinkos_dbgmon_svc(int32_t arg[], int self);

void dbgmon_reset(void);

void __attribute__((noreturn)) 
	dbgmon_exec(void (* task) (const struct dbgmon_comm *, void *), 
				void * param);

int dbgmon_thread_create(void (* func)(void *), void * arg, 
						 const struct thinkos_thread_inf * inf);

int dbgmon_thread_resume(int thread_id);

void dbgmon_unmask(int sig);

void dbgmon_mask(int sig);

void dbgmon_clear(int sig);

void dbgmon_signal(int sig); 

void __dbgmon_idle_hook(void);

uint32_t dbgmon_select(uint32_t watch);

int dbgmon_sched_select(uint32_t evmask);

int dbgmon_wait(int sig);

int dbgmon_expect(int sig);

bool dbgmon_is_set(int sig);

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

int __attribute__((format (__printf__, 2, 3))) 
	dbgmon_printf(const struct dbgmon_comm * comm, const char *fmt, ... );

int dbgmon_putc(int c, const struct dbgmon_comm * comm);

int dbgmon_puts(const char * s, const struct dbgmon_comm * comm);

int dbgmon_gets(char * s, int size, const struct dbgmon_comm * comm);

int dbgmon_getc(const struct dbgmon_comm * comm);

int dbgmon_scanf(const struct dbgmon_comm * comm, const char *fmt, ... );

void dbgmon_hexdump(const struct dbgmon_comm * comm, 
					const struct mem_desc * mem,
					uint32_t addr, unsigned int size);


/* Safe read and write operations to avoid faults in the debugger */

bool dbgmon_mem_wr32(const struct mem_desc * mem, 
					 uint32_t addr, uint32_t val);

bool dbgmon_mem_rd32(const struct mem_desc * mem, 
					 uint32_t addr, uint32_t * val);

bool dbgmon_mem_rd64(const struct mem_desc * mem, 
					   uint32_t addr, uint64_t * val);

bool dbgmon_mem_wr64(const struct mem_desc * mem, 
					 uint32_t addr, uint64_t val);

int dbgmon_mem_read(const struct mem_desc * mem, uint32_t addr, 
					void * ptr, unsigned int len);

bool dbgmon_mem_belong(const struct mem_desc * mem, uint32_t addr);

const struct mem_desc * dbgmon_mem_lookup(const struct mem_desc * const lst[], 
										  unsigned int cnt, uint32_t addr);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor communication interface
 * ----------------------------------------------------------------------------
 */

const struct dbgmon_comm * usb_comm_init(const usb_dev_t * usb);
const struct dbgmon_comm * usb_comm_getinstance(void);

const struct dbgmon_comm * custom_comm_init(void);
const struct dbgmon_comm * custom_comm_getinstance(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_DBGMON_H__ */

