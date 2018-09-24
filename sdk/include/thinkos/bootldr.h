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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

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
	uint32_t start_addr; /* Application memory block start address */
	uint32_t block_size; /* Size of the memory block in bytes */
	uint16_t crc32_offs; /* Position of the CRC32 word in the memory block */
	uint16_t filesize_offs;  /* Position of file size in the memory block */
	const struct magic_blk * magic; /* File identification descriptor */
};



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

#define BLK_RW  (0 << 7)
#define BLK_RO  (1 << 7)

/* Memory block descriptor */
struct blk_desc {
	uint32_t ref;
	uint8_t  opt;
	uint8_t  siz;
	uint16_t cnt;
};

struct mem_desc {
	char name[8];
	struct blk_desc blk[];
};

/* Board description */
struct thinkos_board {
	char name[18];

	struct {
		uint8_t minor;
		uint8_t major;
	} hw_ver;

	struct {
		uint8_t minor;
		uint8_t major;
		uint16_t build;
	} sw_ver;

	struct {
		const struct mem_desc * ram;
		const struct mem_desc * flash;
	} memory;

	struct dbgmon_app_desc application;

	int (* init)(void);
	void (* softreset)(void);
	bool (* autoboot)(unsigned int tick);
	bool (* configure)(const struct dbgmon_comm *);
	void (* upgrade)(const struct dbgmon_comm *);
	void (* selftest)(const struct dbgmon_comm *);
	void (* on_appload)(void);
	void (* on_error)(int code);
	void (* on_comm_init)(void);
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

#ifdef __cplusplus
extern "C" {
#endif

int dmon_ymodem_rcv_init(struct ymodem_rcv * rx, bool crc_mode, bool xmodem);

int dmon_ymodem_rcv_pkt(const struct dbgmon_comm * comm, 
						struct ymodem_rcv * rx);

void dmon_console_io_task(const struct dbgmon_comm * comm);

void dmon_print_thread(const struct dbgmon_comm * comm, 
					   unsigned int thread_id);

void dmon_print_context(const struct dbgmon_comm * comm, 
						const struct thinkos_context * ctx, 
						uint32_t sp);

void dmon_print_exception(const struct dbgmon_comm * comm, 
						  struct thinkos_except * xcpt);

int dmon_print_osinfo(const struct dbgmon_comm * comm);

void dmon_print_alloc(const struct dbgmon_comm * comm);

void dmon_print_stack_usage(const struct dbgmon_comm * comm);

void dmon_thread_exec(void (* func)(void *), void * arg);

bool dmon_app_exec(uint32_t addr, bool paused);

bool dmon_app_erase(const struct dbgmon_comm * comm, 
					uint32_t addr, unsigned int size);

int dmon_ymodem_flash(const struct dbgmon_comm * comm,
					  uint32_t addr, unsigned int size);

bool dmon_app_suspend(void);

bool dmon_app_continue(void);

bool dbgmon_app_exec(const struct dbgmon_app_desc * desc, bool paused);


#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_BOOTLDR_H__ */

