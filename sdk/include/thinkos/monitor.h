/* 
 * File:	 /thinkos/monitor.h
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

#ifndef __THINKOS_MONITOR_H__
#define __THINKOS_MONITOR_H__

#ifndef __THINKOS_MONITOR__
#error "Never use <thinkos/monitor.h> directly; include <thinkos.h> instead."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>

#include <thinkos.h>

#include <sys/memory.h>

#ifndef THINKOS_ENABLE_RESET_RAM_VECTORS
  #ifdef CM3_RAM_VECTORS
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 1
  #else
    #define THINKOS_ENABLE_RESET_RAM_VECTORS 0
  #endif
#endif

#ifndef THINKOS_MONITOR_ENABLE_RST_VEC
#define THINKOS_MONITOR_ENABLE_RST_VEC CM3_RAM_VECTORS 
#endif

/* Mark for debug/monitor breakpoint numbers. */
#define MONITOR_BKPT_ON_THREAD_CREATE 64
#define MONITOR_BKPT_ON_THREAD_TERMINATE 65

#define __BKPT(__NO) asm volatile ("bkpt %0\n" : : "I" __NO)

/* ----------------------------------------------------------------------------
 *  Debug/Monitor events
 * ----------------------------------------------------------------------------
 */

enum monitor_event {
	/* Debug monitor internal reset */
	MONITOR_RESET           = 0,
	/* ThinkOS power on startup indication */
	MONITOR_STARTUP         = 1,
	/* ThinkOS idle indication */
	MONITOR_IDLE            = 2,
	/* Board reset request */
	MONITOR_SOFTRST         = 3,
	/* ThinkOS kernel fault */
	MONITOR_KRN_EXCEPT      = 4,
	/* Debug timer expiry indication */
	MONITOR_ALARM           = 5,
	/* ThinkOS Thread step break */
	MONITOR_THREAD_STEP     = 6,
	/* ThinkOS Thread error */
	MONITOR_THREAD_FAULT    = 7,
	/* ThinkOS Thread create */
	MONITOR_THREAD_CREATE   = 8,
	/* ThinkOS Thread teminate */
	MONITOR_THREAD_TERMINATE = 9,
	/* ThinkOS Thread breakpoint */
	MONITOR_BREAKPOINT      = 10,
	/* Debug Communication data received pending */
	MONITOR_COMM_RCV        = 11, 
	/* Debug Communication end of transfer */
	MONITOR_COMM_EOT        = 12,
	/* Debug Communication control signal */
	MONITOR_COMM_CTL        = 13,
	/* User console RX pipe data pending */
	MONITOR_RX_PIPE         = 14,
	/* User console TX pipe not empty */
	MONITOR_TX_PIPE         = 15,

	/* ThinkOS application stop request */
	MONITOR_APP_STOP        = 18,
	/* ThinkOS application resume request */
	MONITOR_APP_RESUME      = 19,
	/* ThinkOS application terminate request */
	MONITOR_APP_TERM        = 20,
	/* ThinkOS application erase request */
	MONITOR_APP_ERASE       = 21,
	/* ThinkOS application 2pload request */
	MONITOR_APP_UPLOAD      = 22,
	/* ThinkOS application exec request */
	MONITOR_APP_EXEC        = 23,
	/* User/bootloader extension events 0 to 7 */
	MONITOR_USER_EVENT0     = 24,
	MONITOR_USER_EVENT1     = 25,
	MONITOR_USER_EVENT2     = 26,
	MONITOR_USER_EVENT3     = 27,
	MONITOR_USER_EVENT4     = 28,
	MONITOR_USER_EVENT5     = 29,
	MONITOR_USER_EVENT6     = 30,
	MONITOR_FLASH_DRV       = 31,
	/*  */
	MONITOR_NONE            = 32
};

#define SIG_SET(SIGSET, SIG) SIGSET |= (1 << (SIG))
#define SIG_CLR(SIGSET, SIG) SIGSET &= ~(1 << (SIG))
#define SIG_ISSET(SIGSET, SIG) (SIGSET & (1 << (SIG)))
#define SIG_ZERO(SIGSET) SIGSET = 0

struct monitor_thread_inf {
	int8_t thread_id;
	uint8_t errno;
	uint32_t pc;
	uint32_t sp;
	struct thinkos_context * ctx;
};

/* ----------------------------------------------------------------------------
 *  Debug/Monitor communication interface
 * ----------------------------------------------------------------------------
 */

struct monitor_comm_op {
	int (*send)(const void * dev, const void * buf, unsigned int len);
	int (*recv)(const void * dev, void * buf, unsigned int len);
	int (* connect)(const void * dev);
	bool (* isconnected)(const void * dev);
};

struct monitor_comm {
	const void * dev;
	const struct monitor_comm_op * op;
};

static inline int monitor_comm_send(const struct monitor_comm * comm, 
								   const void * buf, unsigned int len) {
	return comm->op->send(comm->dev, buf, len);
}

static inline int monitor_comm_recv(const struct monitor_comm * comm,
								   void * buf, unsigned int len) {
	return comm->op->recv(comm->dev, buf, len);
}

static inline int monitor_comm_connect(const struct monitor_comm * comm) {
	return comm->op->connect(comm->dev);
}

static inline bool monitor_comm_isconnected(const struct monitor_comm * comm) {
	return comm->op->isconnected(comm->dev);
}

#ifdef __cplusplus
extern "C" {
#endif

void thinkos_monitor_svc(int32_t arg[], int self);

void thinkos_monitor_reset(void);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor events/signals 
 * ----------------------------------------------------------------------------
 */
void monitor_unmask(int sig);

void monitor_mask(int sig);

void monitor_clear(int sig);

void monitor_signal(int sig); 

int monitor_select(uint32_t evmask);

int monitor_expect(int sig);

bool monitor_is_set(int sig);

int monitor_wait_idle(void);

void monitor_soft_reset(void);

void __monitor_idle_hook(void);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor alarm/timer 
 * ----------------------------------------------------------------------------
 */

int monitor_sleep(unsigned int ms);

void monitor_alarm(unsigned int ms);

void monitor_alarm_stop(void);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor watchpoint/breakpoint API
 * ----------------------------------------------------------------------------
 */
bool monitor_breakpoint_set(uint32_t addr, uint32_t size);

bool monitor_breakpoint_clear(uint32_t addr, uint32_t size);

void monitor_breakpoint_clear_all(void);

bool monitor_watchpoint_set(uint32_t addr, uint32_t size, int access);

bool monitor_watchpoint_clear(uint32_t addr, uint32_t size);

void monitor_watchpoint_clear_all(void);

bool monitor_breakpoint_disable(uint32_t addr);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor thread API
 * ----------------------------------------------------------------------------
 */
static inline void __monitor_signal_thread_create(int thread_id) {
	register int r0 asm("r0") = (int)thread_id;
	asm volatile ("udf %0 \n" : : "I" (MONITOR_BKPT_ON_THREAD_CREATE), 
				  "r"(r0) : );
}

static inline void __monitor_signal_thread_terminate(int thread_id, int code) {
	register int r0 asm("r0") = (int)thread_id;
	register int r1 asm("r1") = (int)code;
	asm volatile ("udf %0 \n" : : "I" (MONITOR_BKPT_ON_THREAD_TERMINATE), 
				  "r"(r0), "r"(r1) );
}

/* flags the monitor schedule */
static inline void __attribute__((always_inline)) __thinkos_monitor_setpend(void) {
	struct cm3_scb * scb = CM3_SCB;
	/* rise a pending systick interrupt */
	scb->icsr = SCB_ICSR_PENDSTSET;
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

int monitor_thread_terminate_get(int * code);

int monitor_thread_inf_get(unsigned int id, struct monitor_thread_inf * inf);

int monitor_thread_break_get(void);
void monitor_thread_break_clr(void);

int monitor_thread_step_get(void);
void monitor_thread_step_clr(void);
int monitor_thread_step(unsigned int id, bool block);

int monitor_thread_last_fault_get(uint32_t * addr);

int monitor_thread_create(int (* func)(void *), void * arg, 
                         const struct thinkos_thread_inf * inf);

void monitor_thread_resume(int thread_id);

void monitor_thread_destroy(int thread_id);

/* ??? */
void __attribute__((noreturn)) monitor_exec(void (* task) 
   (const struct monitor_comm *, void *), void * param);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor memory API
 * ----------------------------------------------------------------------------
 */

/* Safe read and write operations to avoid faults in the debugger */

bool monitor_mem_wr32(const struct mem_desc * mem, 
                     uint32_t addr, uint32_t val);

bool monitor_mem_rd32(const struct mem_desc * mem, 
                     uint32_t addr, uint32_t * val);

bool monitor_mem_rd64(const struct mem_desc * mem, 
                     uint32_t addr, uint64_t * val);

bool monitor_mem_wr64(const struct mem_desc * mem, 
                     uint32_t addr, uint64_t val);

int monitor_mem_read(const struct mem_desc * mem, uint32_t addr, 
                    void * ptr, unsigned int len);

bool monitor_mem_belong(const struct mem_desc * mem, uint32_t addr);

const struct mem_desc * monitor_mem_lookup(const struct mem_desc * const lst[], 
                                          unsigned int cnt, uint32_t addr);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor FLASH memory
 * ----------------------------------------------------------------------------
 */
int monitor_flash_write(uint32_t addr, const void * buf, size_t len);

int monitor_flash_erase(uint32_t addr, size_t size);

int monitor_flash_open(const char * tag);

int monitor_flash_close(void);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor communication interface
 * ----------------------------------------------------------------------------
 */

#include <sys/usb-dev.h>

const struct monitor_comm * usb_comm_init(const usb_dev_t * usb);
const struct monitor_comm * usb_comm_getinstance(void);

const struct monitor_comm * custom_comm_init(void);
const struct monitor_comm * custom_comm_getinstance(void);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor COMM IO API
 * ----------------------------------------------------------------------------
 */

/* Minimalistic printf style output formatter */
int __attribute__((format (__printf__, 2, 3))) 
	monitor_printf(const struct monitor_comm * comm, const char *fmt, ... );

/* Sends a single ASCII character */
int monitor_putc(int c, const struct monitor_comm * comm);

/* Sends a NULL terminated string */
int monitor_puts(const char * s, const struct monitor_comm * comm);

/* Formats and sends a 32bits signed value into decimal */
void monitor_comm_send_int(int32_t val, unsigned int width, 
                          const struct monitor_comm * comm);

/* Formats and sends a 32bits unsigned value into decimal */
void monitor_comm_send_uint(uint32_t val, unsigned int width, 
                           const struct monitor_comm * comm); 

/* Formats and sends a C string */
void monitor_comm_send_str(const char * s, unsigned int width, 
		const struct monitor_comm * comm);

/* Formats and sends a blanks */
void monitor_comm_send_blanks(unsigned int width, 
		const struct monitor_comm * comm);

/* Formats 32bits unsigned value into hexadecimal and sends */
void monitor_comm_send_hex(uint32_t val, unsigned int width, 
		const struct monitor_comm * comm);

/* Returns a string */
int monitor_gets(char * s, int size, const struct monitor_comm * comm);

/* Returns a single character */
int monitor_getc(const struct monitor_comm * comm);

/* Minimalistic scanf style input formatter */
int monitor_scanf(const struct monitor_comm * comm, const char *fmt, ... );

/* Formats and sends a memory buffer formatted as hexadecimal */
void monitor_hexdump(const struct monitor_comm * comm, 
                    const struct mem_desc * mem,
                    uint32_t addr, unsigned int size);

void monitor_print_context(const struct monitor_comm * comm, 
                          const struct thinkos_context * ctx, 
                          uint32_t sp);

void monitor_print_exception(const struct monitor_comm * comm, 
                            struct thinkos_except * xcpt);

void monitor_print_profile(const struct monitor_comm * comm, 
                          const struct thinkos_profile * p);

void monitor_print_alloc(const struct monitor_comm * comm);

void __thinkos_monitor_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_MONITOR_H__ */
