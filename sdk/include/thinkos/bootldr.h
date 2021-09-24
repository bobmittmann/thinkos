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

#ifndef __THINKOS_BOOTLDR_H__
#define __THINKOS_BOOTLDR_H__

#ifndef __THINKOS_BOOTLDR__
#error "Never use <thinkos/bootldr.h> directly; include <thinkos.h> instead."
#endif 

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_APP__
#include <thinkos/app.h>

#include <thinkos/board.h>

/* Board description */
struct thinkos_board {
	char name[16];
	char desc[32];
	struct {
		char tag[12];
		struct {
			uint8_t minor;
			uint8_t major;
		} ver;
	} hw;

	struct {
		char tag[10];
		struct {
			uint16_t build;
			uint8_t minor;
			uint8_t major;
		} ver;
	} sw;

	void (* on_softreset)(void);
	int (* on_break)(const struct monitor_comm *);

	const struct thinkos_mem_map * memory;
};

/* Boot options */
/* Enble the debug monitor comm port initialization, Ex. USB */
#define BOOT_OPT_DBGCOMM    (1 << 0)
/* Enable console over COMM port */
#define BOOT_OPT_CONSOLE    (1 << 1)
/* Enable loading the debug monitor process */
#define BOOT_OPT_MONITOR    (1 << 2)
/* Call the board configuration function */
#define BOOT_OPT_CONFIG     (1 << 3)
/* Call the board selftest function */
#define BOOT_OPT_SELFTEST   (1 << 4)
/* Try to run the application */
#define BOOT_OPT_APPRUN     (1 << 5)

static inline void monitor_req_app_stop(void) {
	monitor_signal(MONITOR_APP_STOP);
}

static inline void monitor_req_app_resume(void) {
	monitor_signal(MONITOR_APP_RESUME);
}

static inline void monitor_req_app_term(void) {
	monitor_soft_reset();
	monitor_signal(MONITOR_APP_TERM);
}

static inline void monitor_req_app_erase(void) {
	monitor_soft_reset();
	monitor_signal(MONITOR_APP_ERASE);
}

static inline void monitor_req_app_exec(void) {
	monitor_soft_reset();
	monitor_signal(MONITOR_APP_EXEC);
}

static inline void monitor_req_app_upload(void) {
	monitor_soft_reset();
	monitor_signal(MONITOR_APP_UPLOAD);
}

extern const struct thinkos_flash_desc board_flash_desc;
extern struct thinkos_flash_drv board_flash_drv;

struct btl_shell_env;

typedef int(* btl_cmd_callback_t)(struct btl_shell_env * env, int argc, 
								  char * argv[]);

#ifdef __cplusplus
extern "C" {
#endif

int blt_cmd_lookup(struct btl_shell_env * env, const char * str);

int btl_console_shell(struct btl_shell_env * env);

int btl_shell_env_init(struct btl_shell_env * env);

struct btl_shell_env * btl_shell_env_getinstance(void);





void standby_monitor_task(const struct monitor_comm * comm, void * arg);

void __attribute((noreturn)) thinkos_boot(const struct thinkos_board * board,
	void (monitor)(const struct monitor_comm *, void *));

bool monitor_app_suspend(void);

bool monitor_app_continue(void);

bool monitor_app_exec(const struct monitor_comm * comm);

int thinkos_krn_app_start(struct thinkos_rt * krn, unsigned int thread_idx,
						  uintptr_t addr);
#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_BOOTLDR_H__ */

