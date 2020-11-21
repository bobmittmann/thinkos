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

#include <thinkos.h>
#include <sys/memory.h>

/* ----------------------------------------------------------------------------
 *  Debug/Monitor events
 * ----------------------------------------------------------------------------
 */

enum monitor_event {
	/* Debug monitor internal reset */
	MONITOR_RESET           = 0,
	/* ThinkOS kernel fault */
	MONITOR_KRN_FAULT       = 1,
	/* ThinkOS kernel reset indication */
	MONITOR_KRN_ABORT       = 2,
	/* ThinkOS idle indication */
	MONITOR_IDLE            = 3,
	/* Board reset request */
	MONITOR_SOFTRST         = 4,
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
	/* ThinkOS Thread break (stopre	quest, breakpoint.. ) */
	MONITOR_THREAD_BREAK    = 10,
	/* Debug Communication break signal */
	MONITOR_COMM_BRK        = 11, 
	/* Debug Communication data received pending */
	MONITOR_COMM_RCV        = 12, 
	/* Debug Communication end 3f transfer */
	MONITOR_COMM_EOT        = 14,
	/* Debug Communication control signal */
	MONITOR_COMM_CTL        = 15,
	/* User console RX pipe data pending */
	MONITOR_RX_PIPE         = 16,
	/* User console TX pipe not empty */
	MONITOR_TX_PIPE         = 17,

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
	uint8_t ctrl;
	uint32_t pc;
	uint32_t sp;
	struct thinkos_context * ctx;
};

struct monitor_context {
	/* scheduler saved context */
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	/* automatic saved context - exception frame */
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;
};

struct monitor_swap {
	uint32_t xpsr;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t lr;
};

/* ----------------------------------------------------------------------------
 *  Debug/Monitor communication interface
 * ----------------------------------------------------------------------------
 */

/* Status bits */
#define COMM_ST_BREAK_REQ (1 << 0)
#define COMM_ST_CONNECTED (1 << 1)

/* Control operations */
enum monitor_comm_ctrl {
	COMM_CTRL_STATUS_GET = 0,
	COMM_CTRL_CONNECT = 1,
	COMM_CTRL_DISCONNECT = 2,
	COMM_CTRL_BREAK_ACK = 3
};

struct monitor_comm_op {
	int (*send)(const void * dev, const void * buf, unsigned int len);
	int (*recv)(const void * dev, void * buf, unsigned int len);
	int (*ctrl)(const void * dev, unsigned int opc);
};

struct monitor_comm {
	const void * dev;
	const struct monitor_comm_op * op;
};

extern uint32_t thinkos_monitor_stack[THINKOS_MONITOR_STACK_SIZE / 4];
extern const uint16_t thinkos_monitor_stack_size;

#ifdef __cplusplus
extern "C" {
#endif

static inline int monitor_comm_send(const struct monitor_comm * comm, 
								   const void * buf, unsigned int len) {
	return comm->op->send(comm->dev, buf, len);
}

static inline int monitor_comm_recv(const struct monitor_comm * comm,
								   void * buf, unsigned int len) {
	return comm->op->recv(comm->dev, buf, len);
}

static inline int monitor_comm_connect(const struct monitor_comm * comm) {
	return comm->op->ctrl(comm->dev, COMM_CTRL_CONNECT);
}

static inline int monitor_comm_disconnect(const struct monitor_comm * comm) {
	return comm->op->ctrl(comm->dev, COMM_CTRL_DISCONNECT);
}

static inline int monitor_comm_break_ack(const struct monitor_comm * comm) {
	return comm->op->ctrl(comm->dev, COMM_CTRL_BREAK_ACK);
}

static inline int monitor_comm_status_get(const struct monitor_comm * comm) {
	return comm->op->ctrl(comm->dev, COMM_CTRL_STATUS_GET);
}

static inline bool monitor_comm_isconnected(const struct monitor_comm * comm) {
	return (comm->op->ctrl(comm->dev, COMM_CTRL_STATUS_GET) & 
	        COMM_ST_CONNECTED) ? true : false;
}

void thinkos_monitor_svc(int32_t arg[], int self);

void thinkos_krn_monitor_init(const struct monitor_comm * comm, 
                     void (* task)(const struct monitor_comm *, void *),
					 void * param);

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
	monitor_signal(MONITOR_THREAD_CREATE);
}

static inline void __monitor_signal_thread_terminate(int thread_id, int code) {
	monitor_signal(MONITOR_THREAD_TERMINATE);
}


void monitor_signal_thread_terminate(unsigned int thread_id, int code);

void monitor_signal_thread_fault(unsigned int thread_id, int32_t code);

int monitor_thread_break_get(int32_t * pcode);

int monitor_thread_terminate_get(int * code);

int monitor_thread_inf_get(unsigned int id, struct monitor_thread_inf * inf);



struct thinkos_context * monitor_thread_erro_get(uint8_t * thread_id, 
												 int8_t * code);

void monitor_thread_break_clr(void);

int monitor_thread_step_get(void);
void monitor_thread_step_clr(void);
int monitor_thread_step(unsigned int id, bool block);

int monitor_thread_last_fault_get(uint32_t * addr);

int monitor_thread_create(int (* func)(void *, unsigned int), void * arg);

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

bool monitor_mem_wr32(const struct thinkos_mem_desc * mem, 
                     uint32_t addr, uint32_t val);

bool monitor_mem_rd32(const struct thinkos_mem_desc * mem, 
                     uint32_t addr, uint32_t * val);

bool monitor_mem_rd64(const struct thinkos_mem_desc * mem, 
                     uint32_t addr, uint64_t * val);

bool monitor_mem_wr64(const struct thinkos_mem_desc * mem, 
                     uint32_t addr, uint64_t val);

int monitor_mem_read(const struct thinkos_mem_desc * mem, uint32_t addr, 
                    void * ptr, unsigned int len);

bool monitor_mem_belong(const struct thinkos_mem_desc * mem, uint32_t addr);

const struct thinkos_mem_desc * monitor_mem_lookup(
	const struct thinkos_mem_desc * const lst[], 
	unsigned int cnt, uint32_t addr);

/* ----------------------------------------------------------------------------
 *  Debug/Monitor FLASH memory
 * ----------------------------------------------------------------------------
 */
int monitor_flash_write(uint32_t addr, const void * buf, size_t len);

int monitor_flash_erase(uint32_t addr, size_t size);

int monitor_flash_open(const char * tag);

int monitor_flash_close(void);



int monitor_flash_ymodem_recv(const struct monitor_comm * comm,
							  const char * tag);

int monitor_flash_erase_all(const struct monitor_comm * comm,
							const char * tag);

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
                    const struct thinkos_mem_desc * mem,
                    uint32_t addr, unsigned int size);

void monitor_print_osinfo(const struct monitor_comm * comm, 
						  struct thinkos_rt * rt,
						  uint32_t cycref[]);

void monitor_print_thread(const struct monitor_comm * comm, 
						 unsigned int thread_id);

void monitor_print_context(const struct monitor_comm * comm, 
						  const struct thinkos_context * ctx, 
						  uint32_t sp, uint32_t ctrl);

void monitor_print_exception(const struct monitor_comm * comm, 
                            struct thinkos_except * xcpt);

void monitor_print_profile(const struct monitor_comm * comm, 
                          const struct thinkos_profile * p);

void monitor_print_alloc(const struct monitor_comm * comm);

void monitor_print_stack_usage(const struct monitor_comm * comm);

uint32_t __thinkos_monitor_isr(void);

void __thinkos_monitor_sched(struct thinkos_monitor * mon); 

/* ----------------------------------------------------------------------------
 * Default Monitor comm event handler
 * ----------------------------------------------------------------------------
 */
uint32_t monitor_on_comm_rcv(const struct monitor_comm * comm, 
							 uint32_t sigmask);

uint32_t monitor_on_comm_ctl(const struct monitor_comm * comm, 
							 uint32_t sigmask);

uint32_t monitor_on_tx_pipe(const struct monitor_comm * comm, 
							uint32_t sigmask);

uint32_t monitor_on_rx_pipe(const struct monitor_comm * comm, 
							uint32_t sigmask);

int monitor_thread_exec(const struct monitor_comm * comm, 
						int (* task)(void *, unsigned int), void * arg);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_MONITOR_H__ */

