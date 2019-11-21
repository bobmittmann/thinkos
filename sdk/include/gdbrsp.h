/* 
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


/** 
 * @file gdbrsp.h
 * @brief GDB Remote Serial Protocol library
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __GDB_H__
#define __GDB_H__

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

struct gdbrsp_comm_op {
	int (* send)(void *, const void *, size_t);
	int (* recv)(void *, void *, size_t);
	int (* flush)(void *);
	int (* drain)(void *);
	int (* settmo)(void *, unsigned int);
};

struct gdbrsp_target_op {
	int (* init)(void *);
	int (* mem_write)(void * arg, uint32_t addr, 
					  const void * ptr, unsigned int len);
	int (* mem_read)(void * arg, uint32_t addr, void * ptr, unsigned int len);
	int (* file_read)(void * arg, const char * name, char * dst, 
					  unsigned int offs, unsigned int size);
	int (* cpu_halt)(void * arg);
	int (* cpu_continue)(void * arg);
	int (* cpu_goto)(void * arg, uint32_t addr, int opt);
	int (* cpu_run)(void * arg, uint32_t addr, int opt);
	int (* cpu_reset)(void * arg);
	int (* app_exec)(void * arg);
	int (* thread_getnext)(void * arg, int thread_id);
	int (* thread_active)(void * arg);
	int (* thread_break_id)(void * arg);
	int (* thread_any)(void * arg);
	bool (* thread_isalive)(void * arg, int thread_id);
	int (* thread_register_get)(void * arg, int thread_id, int reg, uint64_t * val);
	int (* thread_register_set)(void * arg, unsigned int thread_id, int reg, uint64_t val);
	int (* thread_goto)(void * arg, unsigned int thread_id, uint32_t addr);
	int (* thread_step_req)(void * arg, unsigned int thread_id);
	int (* thread_continue)(void * arg, unsigned int thread_id);
	int (* thread_info)(void * arg, unsigned int gdb_thread_id, char * buf);
	int (* breakpoint_clear_all)(void * arg);
	int (* watchpoint_clear_all)(void * arg);
	int (* breakpoint_set)(void * arg, uint32_t addr, unsigned int size);
	int (* breakpoint_clear)(void * arg, uint32_t addr, unsigned int size);
	int (* watchpoint_set)(void * arg, uint32_t addr, unsigned int size, 
						  unsigned int opt);
	int (* watchpoint_clear)(void * arg, uint32_t addr, unsigned int size);
};

/* Opaque target structure */
struct gdbrsp_target;

/* Opaque communication structure */
struct gdbrsp_comm;

/* Opaque agent structure */
struct gdbrsp_agent;

#define GDB_RSP_QUIT -0x80000000

enum gdbrsp_retcode {
	GDBRSP_OK = 0
};

#ifdef __cplusplus
extern "C" {
#endif

struct gdbrsp * gdbrsp_alloc(void);

struct gdbrsp_comm * gdbrsp_comm_getinstance(void);
struct gdbrsp_target * gdbrsp_target_getinstance(void);
struct gdbrsp_agent * gdbrsp_agent_getinstance(void);

/** @brief initialize a target strcuture 
 *
 * @param target 
 * @param op
 * @param arg
 * @return #GDBRSP_OK on success...
 */
int gdbrsp_target_init(struct gdbrsp_target * target,
                       const struct gdbrsp_target_op * op, 
                       void * arg);

/** @brief initialize a communication interface strcuture 
 *
 * @param comm
 * @param op
 * @param arg
 * @return #GDBRSP_OK on success...
 */
int gdbrsp_comm_init(struct gdbrsp_comm * comm, 
                     const struct gdbrsp_comm_op * op, 
                     void * arg);

/** @brief initialize an gent strcuture 
 *
 * @param comm
 * @param op
 * @param arg
 * @return #GDBRSP_OK on success...
 */
int gdbrsp_agent_init(struct gdbrsp_agent * agent, 
                      struct gdbrsp_comm * comm, 
                      struct gdbrsp_target * target);

int gdbrsp_pkt_recv(struct gdbrsp_comm * comm, char * pkt, int max);

/* Console */
struct file * gdbrsp_fopen(struct gdbrsp_agent * gdb);
int gdbrsp_fclose(struct file * f);

/* Event handlers */
int gdbrsp_on_halt(struct gdbrsp_agent *, int signal);

int gdbrsp_on_breakpoint(struct gdbrsp_agent * gdb, char * pkt);

int gdbrsp_on_step(struct gdbrsp_agent * gdb, char * pkt);

int gdbrsp_on_break(struct gdbrsp_agent * gdb, char * pkt);

int gdbrsp_on_fault(struct gdbrsp_agent * gdb, char * pkt);

int gdbrsp_pkt_input(struct gdbrsp_agent * gdb, char * pkt, unsigned int len);

int gdbrsp_monitor_write(struct gdbrsp_agent * gdb, const void * buf, 
						  unsigned int cnt);

int gdbrsp_puts(struct gdbrsp_agent * gdb, const char * msg);

int gdbrsp_error(struct gdbrsp_agent * gdb, unsigned int err);

//void gdb_stub_task(struct dmon_comm * comm);

#ifdef __cplusplus
}
#endif	

#endif /* __GDB_H__ */

