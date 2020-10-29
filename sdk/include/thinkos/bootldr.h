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

#include <thinkos/board.h>

/* File identification magic block 

   This block is used to guess the type of a memory block or file
   based on a pattarn located somewhere inside the file.
 
 */
struct magic_blk {
	struct {
		uint16_t pos; /* Position of the pattern in bytes */
		uint16_t cnt; /* Number of record entries */
	} hdr;
	/* Pattern records */
	struct {
	    uint32_t mask; /* Bitmask */
		uint32_t comp; /* Compare value */
	} rec[];
};

/* application block descriptor */
struct monitor_app_desc {
	char tag[8];
	uint32_t start_addr; /* Application memory block start address */
	uint32_t block_size; /* Size of the memory block in bytes */
	uint16_t crc32_offs; /* Position of the CRC32 word in the memory block */
	uint16_t filesize_offs;  /* Position of file size in the memory block */
	const struct magic_blk * magic; /* File identification descriptor */
};

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

	void (* upgrade)(const struct monitor_comm *);
	const struct monitor_comm * (* monitor_comm_init)(void);
	struct monitor_app_desc application;
	void * arg;

	/* System initialization - runs once when the system powers up */
	int (* init)(void);
	/* Soft reset */
	void (* softreset)(void);

	int (* configure_task)(void *);
	int (* selftest_task)(void *);
	/* ThinkOS task: monitor will run this task prior to run
	 the application. It should implement configuration
	 application integrity check etc... 
	 The return code indicates wether is ok or not to call the
	 application task. */
	int (* preboot_task)(void *);
	/* ThinkOS task: application loader . */
	int (* app_task)(void *);
	/* ThinkOS task: monitor will run this task if the application
	   returns.  */
	int (* default_task)(void *);

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



/* FIXME: Not quite sure why this is here!!!! */
struct ymodem_rcv {
	unsigned int pktno;
	unsigned int fsize;
	unsigned int count;

	unsigned char crc_mode;
	unsigned char xmodem;
	unsigned char sync;
	unsigned char retry;

	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};

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

#ifdef __cplusplus
extern "C" {
#endif

void __attribute((noreturn)) thinkos_boot(const struct thinkos_board * board);

int monitor_ymodem_rcv_init(struct ymodem_rcv * rx, bool crc_mode, bool xmodem);

int monitor_ymodem_rcv_pkt(const struct monitor_comm * comm, 
						struct ymodem_rcv * rx);

void monitor_console_io_task(const struct monitor_comm * comm);

int monitor_ymodem_flash(const struct monitor_comm * comm,
						uint32_t addr, unsigned int size);

bool monitor_app_suspend(void);

bool monitor_app_continue(void);

bool monitor_app_exec(const struct monitor_app_desc * desc, bool paused);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_BOOTLDR_H__ */

