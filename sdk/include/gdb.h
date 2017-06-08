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
 * @file gdb.h
 * @brief ThinkOS libgdb
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __GDB_H__
#define __GDB_H__

#include <stdint.h>

struct gdb_comm_op {
	int (* send)(void *, const void *, unsigned int);
	int (* recv)(void *, void *, unsigned int);
	int (* flush)(void *);
	int (* drain)(void *);
	int (* settmo)(void *, unsigned int);
	int (* init)(void *);
};

struct gdb_target_op {
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
	int (* thread_register_get)(void * arg, int thread_id, 
								int reg, uint32_t * val);
	int (* thread_register_set)(void * arg, unsigned int thread_id, 
								int reg, uint32_t val);
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


struct gdb_rsp;

extern struct gdb_rsp gdb_rsp_singleton;

#ifdef __cplusplus
extern "C" {
#endif

struct gdb_rsp * gdb_rsp_alloc(void);

int gdb_rsp_init(struct gdb_rsp * gdb, void * buf, int len);

int gdb_rsp_target_init(struct gdb_rsp * gdb, const struct gdb_target_op * op, 
					  void * arg);

int gdb_rsp_comm_init(struct gdb_rsp * gdb, const struct gdb_comm_op * op, 
					  void * arg);

void gdb_rsp_comm_loop(struct gdb_rsp * gdb);

/* Console */
struct file * gdb_rsp_fopen(struct gdb_rsp * gdb);
int gdb_rsp_fclose(struct file * f);

/* Event handlers */
int gdb_rsp_on_halt(struct gdb_rsp *, int signal);

int gdb_rsp_on_step(struct gdb_rsp * gdb);

int gdb_rsp_on_breakpoint(struct gdb_rsp * gdb);

int gdb_rsp_on_fault(struct gdb_rsp * gdb);

int gdb_rsp_monitor_write(struct gdb_rsp * gdb, const void * buf, 
						  unsigned int cnt);

int gdb_rsp_puts(struct gdb_rsp * gdb, const char * msg);

int gdb_rsp_error(struct gdb_rsp * gdb, unsigned int err);

//void gdb_stub_task(struct dmon_comm * comm);

#ifdef __cplusplus
}
#endif	

#endif /* __GDB_H__ */

