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

struct ymodem_rcv {
	const struct comm_dev * comm;

	unsigned int pktno;
	unsigned int fsize;
	unsigned int count;

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

#define ZMODEM_HDR_LEN 16
#define ZMODEM_PKT_LEN_MAX 1024

struct zmodem {
	const struct comm_dev * comm;

	struct {
		uint8_t	hdr[ZMODEM_HDR_LEN];	/* last received header */
		uint8_t hdr_len;			/* last received header size */
		uint32_t hdr_pos;	/* last received header position value */
	} rxd;
	int type;	/* last received status */

	/*
	 * receiver capability flags
	 * extracted from the ZRINIT frame as received
	 */

	bool can_full_duplex;
	bool can_overlap_io;
	bool can_break;
	bool can_fcs_32;
	bool want_fcs_16;
	bool escape_ctrl_chars;	
	bool escape_8th_bit;

	/*
	 * file management options.
	 * only one should be on
	 */

	bool management_newer;
	bool management_clobber;
	bool management_protect;


	struct {
		uint8_t raw[ZMODEM_PKT_LEN_MAX + 4];
		uint8_t data[ZMODEM_PKT_LEN_MAX + 4];
	} pkt; 
	unsigned int data_len;

	union {
		uint8_t rx_data_subpacket[ZMODEM_PKT_LEN_MAX]; 
	};

	uint32_t file_size;
	uint32_t file_pos;

	bool receive_32bit_data;
	uint32_t ack_file_pos;	
	/* file position used in acknowledgement of correctly */
						/* received data subpackets */

	int last_sent;
	int n_cans;

	/* Status */
	bool		cancelled;
	bool		connected;
	bool		file_skipped;
	bool		no_streaming;
	uint16_t	recv_bufsize;	/* Receiver specified buffer size */
	uint16_t	crc_request;
	uint16_t	errors;
	uint16_t	consecutive_errors;

	/* Configuration */
	uint16_t	recv_timeout;
	uint16_t	max_errors;
	uint16_t	block_size;
	uint16_t	max_block_size;
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

int xmodem_snd_loop(struct xmodem_snd * sx, const void * data, int len);

int xmodem_snd_cancel(struct xmodem_snd * sx);

int xmodem_snd_eot(struct xmodem_snd * sx);



int ymodem_rcv_init(struct ymodem_rcv * ry, 
					const struct comm_dev * comm, 
					unsigned int mode);

int xmodem_snd_start(struct xmodem_snd * sx);

int ymodem_rcv_loop(struct ymodem_rcv * ry, void * data, int len);

int ymodem_rcv_cancel(struct ymodem_rcv * ry);





int ymodem_snd_init(struct ymodem_snd * sy, 
					const struct comm_dev * comm, unsigned int mode);

int ymodem_snd_start(struct ymodem_snd * sy, const char fname, 
					 unsigned int size);

int ymodem_snd_cancel(struct ymodem_snd * sy);

int ymodem_snd_loop(struct ymodem_snd * sy, const void * data, int len);

int ymodem_snd_eot(struct ymodem_snd * sy);



int zmodem_snd_init(struct zmodem * zm, const struct comm_dev * comm, 
					unsigned int mode);

int zmodem_snd_rz(struct zmodem * zm);

int zmodem_snd_start(struct zmodem * zm, const char * fname, 
					 unsigned int fsize);

int zmodem_snd_cancel(struct zmodem * zm);

int zmodem_snd_loop(struct zmodem * zm, const void * data, int len);

int zmodem_snd_eof(struct zmodem * zm);


#ifdef __cplusplus
}
#endif

#endif /* __XMODEM_H__ */

