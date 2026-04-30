/* 
 * Copyright(C) 2012-2014 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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
 * @file xmodem.h
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdint.h>
#include <stdbool.h>


struct comm_ops {
	int  (* send)(void *, const void *, unsigned int);
	int  (* recv)(void *, void *, unsigned int, unsigned int);
};

struct comm_dev {
	void * arg;
	struct comm_ops op;
};

enum {
	XMODEM_RCV_CKS = 0,
	XMODEM_RCV_CRC = 1,
};

struct xmodem_rcv {
	const struct comm_dev * comm;

	signed char mode;
	unsigned char sync;
	unsigned char pktno;
	unsigned char retry;

	unsigned short data_len;
	unsigned short data_pos;
	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};

enum {
	XMODEM_SND_STD = 0,
	XMODEM_SND_1K = 1
};
/*
struct xmodem_snd {
	const struct comm_dev * comm;

	volatile uint8_t seq;
	volatile uint8_t ack;
	volatile uint8_t state;
	uint8_t mode;

	uint16_t err;
	uint16_t tmout_ms;

	struct { 
		uint8_t hdr[3];
		uint8_t data[1024];
		uint8_t fcs[2];
		uint16_t free;
	} pkt;
};
*/

struct xmodem_snd {
	const struct comm_dev * comm;

	unsigned char seq;
	unsigned char ack;
	unsigned char state;
	unsigned char mode;
	unsigned short data_len;
	unsigned short data_max;
	unsigned int tmout_ms;

	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};


#define YMODEM_FNAME_LEN_MAX 120

enum {
	YMODEM_RCV_CKS = 0,
	YMODEM_RCV_CRC = 1,
};

enum yr_state {
	YR_CAN = -2,
	YR_ERR = -1,
	YR_UND = 0,
	YR_IDL = 1,
	YR_EOT = 2,
	YR_HDR = 3,
	YR_DAT = 4,
};

struct ymodem_rcv {
	const struct comm_dev * comm;

	unsigned int pktno;
	unsigned int fsize;
	unsigned int count;

	char state;
	char crc_mode;
	char xmodem;
	unsigned char sync;
	unsigned char retry;

	unsigned short data_len;
	unsigned short data_pos;
	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};

struct ymodem_snd {
	const struct comm_dev * comm;

	unsigned int pktno;
	unsigned int fsize;
	unsigned int count;

	unsigned char seq;
	unsigned char ack;
	unsigned char state;
	unsigned char mode;
	unsigned short data_len;
	unsigned short data_max;
	unsigned short data_pos;
	unsigned int tmout_ms;

	char crc_mode;
	char xmodem;
	unsigned char sync;
	unsigned char retry;

	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};

#define ZMODEM_RXD_BUF_LEN 64

#define ZMODEM_HDR_LEN 16
#define ZMODEM_PKT_LEN_MAX 1024

struct zmodem {
	const struct comm_dev * comm;

	int16_t     type; /* last received status */

	uint8_t     last_sent;

	uint16_t	recv_bufsize;	/* Receiver specified buffer size */
	uint16_t	crc_request;
	uint16_t	errors;
	uint16_t	consecutive_errors;

	/* Configuration */
	uint16_t	recv_timeout;
	uint16_t	max_errors;
	uint16_t	block_size;
	uint16_t	max_block_size;

	struct {
		/*
		 * receiver capability flags
		 * extracted from the ZRINIT frame as received
		 */
		uint32_t can_full_duplex : 1;
		uint32_t can_overlap_io : 1;
		uint32_t can_break : 1;
		uint32_t can_fcs_32 : 1;
		uint32_t want_fcs_16 : 1;
		uint32_t escape_ctrl_chars : 1;	
		uint32_t escape_8th_bit : 1;
		/*
		 * file management options.
		 * only one should be on
		 */
		uint32_t management_newer : 1;
		uint32_t management_clobber : 1;
		uint32_t management_protect : 1;

		/* Status */
		uint32_t cancelled : 1;
		uint32_t connected : 1;

		uint32_t file_skipped : 1;
		uint32_t no_streaming : 1;
		uint32_t receive_32bit_data : 1;
	};

	union {
		uint8_t rx_data_subpacket[ZMODEM_PKT_LEN_MAX]; 
		struct {
			uint8_t raw[ZMODEM_PKT_LEN_MAX + 4];
			uint8_t data[ZMODEM_PKT_LEN_MAX + 4];
		} pkt; 
		
	};

	struct {
		uint8_t	buf[ZMODEM_RXD_BUF_LEN];
		uint8_t cnt; /* number of bytes in the receiving buffer */	
		uint8_t pos; /* current position in the buffer */
		uint8_t n_cans;
		
		uint8_t hdr_len;			/* last received header size */
		uint8_t	hdr[ZMODEM_HDR_LEN];	/* last received header */
		uint32_t hdr_pos;	/* last received header position value */
	} rxd;

	uint32_t data_len;

	uint32_t file_size;
	uint32_t file_pos;

	uint32_t ack_file_pos;	
	/* file position used in acknowledgement of correctly */
	/* received data subpackets */

};
#ifdef __cplusplus
extern "C" {
#endif

int xmodem_rcv_init(struct xmodem_rcv * rx, 
					const struct comm_dev * comm, 
					int mode);

int xmodem_rcv_loop(struct xmodem_rcv * rx, void * data, int len);

int xmodem_rcv_cancel(struct xmodem_rcv * rx);


int xmodem_snd_init(struct xmodem_snd * sx, const struct comm_dev * comm, 
					unsigned int mode);

//int xmodem_snd_loop(struct xmodem_snd * sx, const void * data, 
//					unsigned int len);

int xmodem_snd_loop(struct xmodem_snd * sx, const void * data, int len);

int xmodem_snd_cancel(struct xmodem_snd * sx);

int xmodem_snd_eot(struct xmodem_snd * sx);

int xmodem_snd_start(struct xmodem_snd * sx);


int ymodem_rcv_init(struct ymodem_rcv * ry, 
					const struct comm_dev * comm, 
					unsigned int mode);

int ymodem_rcv_start(struct ymodem_rcv * ry, char * fname, 
					 unsigned int * pfsize);

int ymodem_rcv_loop(struct ymodem_rcv * ry, void * data, int len);

int ymodem_rcv_cancel(struct ymodem_rcv * ry);

int ymodem_rcv_flush(struct ymodem_rcv * ry);


int ymodem_snd_init(struct ymodem_snd * sy, 
					const struct comm_dev * comm, unsigned int mode);

int ymodem_snd_start(struct ymodem_snd * sy, const char * fname, 
					 unsigned int fsize);

int ymodem_snd_cancel(struct ymodem_snd * sy);

int ymodem_snd_loop(struct ymodem_snd * sy, const void * data, int len);

int ymodem_snd_eot(struct ymodem_snd * sy);

int ymodem_snd_done(struct ymodem_snd * sy);

int ymodem_snd_flush(struct ymodem_snd * sy, unsigned int tmo_ms);

int zmodem_snd_init(struct zmodem * zm, const struct comm_dev * comm, 
					unsigned int mode);

int zmodem_snd_wait_connect(struct zmodem * zm);

int zmodem_snd_rz(struct zmodem * zm);

int zmodem_snd_start(struct zmodem * zm, const char * fname, 
					 unsigned int fsize);

int zmodem_snd_cancel(struct zmodem * zm);

int zmodem_snd_loop(struct zmodem* zm, const void * data, unsigned int len);

int zmodem_snd_eof(struct zmodem * zm);

int xmodem_snd_readback(struct xmodem_snd * sx);

int xmodem_snd_status(struct xmodem_snd * sx);


#ifdef __cplusplus
}
#endif

#endif /* __XMODEM_H__ */

