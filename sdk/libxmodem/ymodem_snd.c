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
 * @file ymodem_snd.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>

#include <xmodem.h>
#include <crc.h>
#include <errno.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define XMODEM_SND_TMOUT_MS 2000

enum {
	XMODEM_SND_IDLE = 0,
	XMODEM_SND_CRC = 1,
	XMODEM_SND_CKS = 2
};


int ymodem_snd_init(struct ymodem_snd * sy, 
					const struct comm_dev * comm, unsigned int mode)
{
	if ((sy == NULL) || (comm == NULL) || (mode > XMODEM_SND_1K))
		return -EINVAL;

	sy->comm = comm;
	sy->mode = mode;
	sy->data_max = (sy->mode == XMODEM_SND_1K) ? 1024 : 128;
	sy->data_len = 0;
	sy->seq = 1;
	sy->state = XMODEM_SND_IDLE;

	/* flush */
	while (sy->comm->op.recv(sy->comm->arg, sy->pkt.data, 1024, 200) > 0); 


	return 0;
}

int ymodem_snd_start(struct ymodem_snd * sy, const char fname, 
					 unsigned int size)
{
	return 0;
}


int ymodem_snd_cancel(struct ymodem_snd * sy)
{
	unsigned char buf[4];
	int ret;


	buf[0] = CAN;

	ret = sy->comm->op.send(sy->comm->arg, buf, 1);

	sy->data_max = (sy->mode == XMODEM_SND_1K) ? 1024 : 128;
	sy->data_len = 0;
	sy->seq = 1;
	sy->state = XMODEM_SND_IDLE;


	return ret;
}

static int ymodem_send_pkt(struct ymodem_snd * sy, 
						   unsigned char * data, int data_len)
{
	unsigned char * pkt = data - 3; 
	unsigned char * cp;
	unsigned char fcs[2];
	int retry = 0;
	int fcs_len;
	int ret;
	int c;


	if (sy->state == XMODEM_SND_IDLE) {

		for (;;) {

			// Wait for NAK or 'C'
			if ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
										 1, XMODEM_SND_TMOUT_MS)) < 0) {
				if (ret == ETIMEDOUT) {
					if (++retry < 20) 
						continue;
				}
				return ret;
			}
			c = *pkt;

			if (c == CAN) {
				return -ECANCELED;
			}

			if (c == 'C') {
				sy->state = XMODEM_SND_CRC;
				break;
			}

			if (c == NAK) {
				sy->state = XMODEM_SND_CKS;
				break;
			}

		}
	}

	if (data_len == 1024) {
		pkt[0] = STX;
	} else {
		pkt[0] = SOH;
	} 

	pkt[1] = sy->seq;
	pkt[2] = ~sy->seq;
	cp = &pkt[3];

	if (sy->state == XMODEM_SND_CRC) {
		unsigned short crc = 0;
		int i;

		for (i = 0; i < data_len; ++i)
			crc = CRC16CCITT(crc, cp[i]);

		fcs[0] = crc >> 8;
		fcs[1] = crc & 0xff;
		fcs_len = 2;
	} else {
		unsigned char cks = 0;
		int i;

		for (i = 0; i < data_len; ++i)
			cks += cp[i];

		fcs[0] = cks;
		fcs_len = 1;
	}

	for (;;) {


		// Send packet less FCS 
		if ((ret = sy->comm->op.send(sy->comm->arg, pkt, data_len + 3)) < 0) {
			return ret;
		}

		// Send FCS (checksum or CRC)
		if ((ret = sy->comm->op.send(sy->comm->arg, fcs, fcs_len)) < 0) {
			return ret;
		}

		// Wait for ACK
		if ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
									 1, XMODEM_SND_TMOUT_MS)) <= 0) {
			if (ret == ETIMEDOUT) {
				if (++retry < 10)
					continue;
			}
			return ret;
		}

		c = *pkt;

		if (c == ACK) {
			break;
		}

		if (c == CAN) {
			ret = -ECANCELED;
			goto error;
		}

		if (c != NAK) {
			ret = -EBADMSG;
			goto error;
		}


		if (++retry == 10) {
			ret = -ECANCELED;
			goto error;
		}
	}

	sy->seq++;

	return 0;

error:
	/* flush */
	while (sy->comm->op.recv(sy->comm->arg, sy->pkt.data, 1024, 200) > 0); 

	return ret;
}

int ymodem_snd_loop(struct ymodem_snd * sy, const void * data, int len)
{
	unsigned char * src = (unsigned char *)data;

	if ((src == NULL) || (len < 0))
		return -EINVAL;

	do {
		unsigned char * dst;
		int ret;
		int rem;
		int n;
		int i;

		dst = &sy->pkt.data[sy->data_len];
		rem = sy->data_max - sy->data_len;
		n = MIN(len, rem);

		for (i = 0; i < n; ++i)
			dst[i] = src[i];

		sy->data_len += n;

		if (sy->data_len == sy->data_max) {

			if ((ret = ymodem_send_pkt(sy, sy->pkt.data, sy->data_len)) < 0) {
				return ret;
			}

			sy->data_len = 0;
		}

		src += n;
		len -= n;
	} while (len);

	return 0;
}

int ymodem_snd_eot(struct ymodem_snd * sy)
{
	unsigned char buf[4];
	unsigned char * data;
	int data_len;
	int data_max;
	int ret;


	if ((data_len = sy->data_len) <= (1024 - 128))
		data_max = 128;
	else
		data_max =  sy->data_max;

	data = sy->pkt.data;

	while (data_len > 0) {
		int len;
		int i;

		len = MIN(data_len, data_max);

		/* padding */
		for (i = len; i < data_max; ++i)
			data[i] = '\0';


		if ((ret = ymodem_send_pkt(sy, data, data_max)) < 0) {
			return ret;
		}

		data_len -= len;
		data += len;
	}


	buf[0] = EOT;
	ret = sy->comm->op.send(sy->comm->arg, buf, 1);

	sy->data_max = (sy->mode == XMODEM_SND_1K) ? 1024 : 128;
	sy->data_len = 0;
	sy->seq = 1;
	sy->state = XMODEM_SND_IDLE;

	return ret;
}

