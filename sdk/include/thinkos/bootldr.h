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
struct dbgmon_app_desc {
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
	char desc[38];
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
			uint8_t minor;
			uint8_t major;
			uint16_t build;
		} ver;
	} sw;

	struct {
		uint8_t cnt;
		union {
			struct {
				const struct mem_desc * flash;
				const struct mem_desc * ram;
				const struct mem_desc * periph;
			};
			const struct mem_desc * lst[3];
		};
	} memory;

	struct dbgmon_app_desc application;

	int (* init)(void);
	void (* softreset)(void);
	void (* upgrade)(const struct dbgmon_comm *);
	const struct dbgmon_comm * (* dbgmon_comm_init)(void);

	int (* preboot_task)(void * );
	/* ThinkOS task: monitor will run this task by request */
	int (* configure_task)(void *);
	/* ThinkOS task: monitor will run this task by request */
	int (* selftest_task)(void *);
	/* ThinkOS task: monitor will run this task if fails to load 
	   the application */
	int (* default_task)(void *);

};

/* Board description instance */
extern const struct thinkos_board this_board;

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

static inline void dbgmon_req_app_stop(void) {
	dbgmon_signal(DBGMON_APP_STOP);
}

static inline void dbgmon_req_app_resume(void) {
	dbgmon_signal(DBGMON_APP_RESUME);
}

static inline void dbgmon_req_app_term(void) {
	dbgmon_soft_reset();
	dbgmon_signal(DBGMON_APP_TERM);
}

static inline void dbgmon_req_app_erase(void) {
	dbgmon_soft_reset();
	dbgmon_signal(DBGMON_APP_ERASE);
}

static inline void dbgmon_req_app_exec(void) {
	dbgmon_soft_reset();
	dbgmon_signal(DBGMON_APP_EXEC);
}

static inline void dbgmon_req_app_upload(void) {
	dbgmon_soft_reset();
	dbgmon_signal(DBGMON_APP_UPLOAD);
}

extern const struct thinkos_flash_desc board_flash_desc;
extern struct thinkos_flash_drv board_flash_drv;

#ifdef __cplusplus
extern "C" {
#endif

int dbgmon_ymodem_rcv_init(struct ymodem_rcv * rx, bool crc_mode, bool xmodem);

int dbgmon_ymodem_rcv_pkt(const struct dbgmon_comm * comm, 
						struct ymodem_rcv * rx);

void dbgmon_console_io_task(const struct dbgmon_comm * comm);

void dbgmon_print_thread(const struct dbgmon_comm * comm, 
					   unsigned int thread_id);

void dbgmon_print_context(const struct dbgmon_comm * comm, 
						const struct thinkos_context * ctx, 
						uint32_t sp);

void dbgmon_print_exception(const struct dbgmon_comm * comm, 
						  struct thinkos_except * xcpt);

int dbgmon_print_osinfo(const struct dbgmon_comm * comm);

void dbgmon_print_alloc(const struct dbgmon_comm * comm);

void dbgmon_print_stack_usage(const struct dbgmon_comm * comm);

void dbgmon_thread_exec(void (* func)(void *), void * arg);

bool dbgmon_app_erase(const struct dbgmon_comm * comm, 
					  uint32_t addr, unsigned int size);

int dbgmon_ymodem_flash(const struct dbgmon_comm * comm,
						uint32_t addr, unsigned int size);

bool dbgmon_app_suspend(void);

bool dbgmon_app_continue(void);

bool dbgmon_app_exec(const struct dbgmon_app_desc * desc, bool paused);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_BOOTLDR_H__ */

