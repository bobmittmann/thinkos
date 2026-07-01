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
 * @file xmodem_snd.c
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

#define XMODEM_SND_INIT_TMO_MS 1000
#define XMODEM_SND_TMO_MS 16

#define TRACE_LEVEL TRACE_LVL_ERR
//#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

enum {
	XMODEM_SND_IDLE = 0,
	XMODEM_SND_CRC = 1,
	XMODEM_SND_CKS = 2
};

int xmodem_snd_init(struct xmodem_snd * sx, 
					const struct comm_dev * comm, unsigned int mode)
{
	if ((sx == NULL) || (comm == NULL) || (mode > XMODEM_SND_1K))
		return -EINVAL;

	sx->comm = comm;
	sx->mode = mode;
	sx->data_max = (sx->mode == XMODEM_SND_1K) ? 1024 : 128;
	sx->data_len = 0;
	sx->seq = 1;
	sx->ack = 1;
	sx->state = XMODEM_SND_IDLE;
	sx->tmout_ms = XMODEM_SND_INIT_TMO_MS;

	/* flush */
	while (sx->comm->op.recv(sx->comm->arg, sx->pkt.data, 1024, 200) > 0); 


	return 0;
}

int xmodem_snd_cancel(struct xmodem_snd * sx)
{
	uint8_t buf[4];
	int ret;


	buf[0] = CAN;

	ret = sx->comm->op.send(sx->comm->arg, buf, 1);

	sx->data_max = (sx->mode == XMODEM_SND_1K) ? 1024 : 128;
	sx->data_len = 0;
	sx->seq = 1;
	sx->ack = 1;
	sx->state = XMODEM_SND_IDLE;


	return ret;
}

int xmodem_snd_start(struct xmodem_snd * sx)
{
	uint8_t pkt[8];
	int retry = 0;
	int ret;
	int c;

	while (sx->comm->op.recv(sx->comm->arg, sx->pkt.data, 1024, 200) > 0); 

	sx->data_max = (sx->mode == XMODEM_SND_1K) ? 1024 : 128;
	sx->data_len = 0;
	sx->seq = 1;
	sx->ack = 1;
	sx->state = XMODEM_SND_IDLE;

	INFS("RX: IDLE...");

	for (;;) {

		// Wait for NAK or 'C'
		if ((ret = sx->comm->op.recv(sx->comm->arg, pkt, 
									 1, sx->tmout_ms)) <= 0) {
			/*
			   if (ret == -THINKOS_ETIMEDOUT) {
			   INFS("RX: ETIMEDOUT...");
			   if (++retry < 20) 
			   continue;
			   } else {
			   INF("RX: ret=%d", ret);
			   } */
			if (++retry < 20) 
				continue;
			return ret;
		}

		c = *pkt;

		if (c == CAN) {
			ERRS("RX: CAN");
			return -ECANCELED;
		}

		if (c == 'C') {
			INFS("RX: CRC mode");
			sx->state = XMODEM_SND_CRC;
			break;
		}

		if (c == NAK) {
			INFS("RX: CKS mode");
			sx->state = XMODEM_SND_CKS;
			break;
		}

		INF("RX: '%02x'", c);

	}

	sx->tmout_ms = XMODEM_SND_TMO_MS;

	return 0;
}

static int xmodem_send_pkt(struct xmodem_snd * sx, 
						   uint8_t * data, int data_len)
{
	uint8_t * pkt = data - 3; 
	uint8_t * cp;
	uint8_t * fcs;
	int tot_len;
	int ret;
	int c;

	if (sx->state == XMODEM_SND_IDLE)
		return -EINVAL;

	if ((signed char)(sx->seq - sx->ack) >= 1) {
		// Wait for ACK
		if ((ret = sx->comm->op.recv(sx->comm->arg, pkt, 
									 1, sx->tmout_ms)) <= 0) {
			ERRS("RX: too many retries!");
			return ret;
		}

		c = *pkt;

		if (c == CAN) {
			INFS("RX: CAN");
			ret = -ECANCELED;
			goto error;
		}

		if (c == NAK) {
			WARNS("RX: NAK");
			ret = -EBADMSG;
			goto error;
		} 
		
		if (c != ACK) {
			INFS("RX: ??");
			ret = -EBADMSG;
			goto error;

		}

		INFS("RX: ACK");
		sx->ack++;
	}

	if (data_len == 1024) {
		pkt[0] = STX;
	} else {
		pkt[0] = SOH;
	} 

	pkt[1] = sx->seq;
	pkt[2] = ~sx->seq;
	cp = &pkt[3];

	if (sx->state == XMODEM_SND_CRC) {
		unsigned short crc = 0;
		int i;

		for (i = 0; i < data_len; ++i)
			crc = CRC16CCITT(crc, cp[i]);

		fcs = &cp[i];
		fcs[0] = crc >> 8;
		fcs[1] = crc & 0xff;
		tot_len = 3 + data_len + 2;
	} else {
		uint8_t cks = 0;
		int i;

		for (i = 0; i < data_len; ++i)
			cks += cp[i];

		fcs = &cp[i];
		fcs[0] = cks;
		tot_len = 3 + data_len + 1;
	}

	INF("SX: tot_len=%d", tot_len);

	// Send packet 
	if ((ret = sx->comm->op.send(sx->comm->arg, pkt, tot_len)) < 0) {
		INFS("SX: console_write() failed!..");
		return ret;
	}

	sx->seq++;


	return 0;

error:
	/* flush */
	while (sx->comm->op.recv(sx->comm->arg, sx->pkt.data, 1024, 200) > 0); 

	return ret;
}

int xmodem_snd_loop(struct xmodem_snd * sx, const void * data, int len)
{
	uint8_t * src = (uint8_t *)data;
	unsigned int rem;

	if ((src == NULL) || (len < 0))
		return -EINVAL;

	rem = len;
	while (rem > 0) {
		unsigned int free;
		uint8_t * dst;
		int n;
		int i;

		dst = &sx->pkt.data[sx->data_len];
		free = sx->data_max - sx->data_len;
		n = MIN(rem, free);

		for (i = 0; i < n; ++i)
			dst[i] = src[i];

		src += n;
		rem -= n;
		sx->data_len += n;

		if (sx->data_len == sx->data_max) {
			int ret;

			if ((ret = xmodem_send_pkt(sx, sx->pkt.data, sx->data_len)) < 0) {
				ERRS("XS.SendPkt failed...");
				return ret;
			}

			sx->data_len = 0;
		}
	}

	return len;
}

int xmodem_snd_eot(struct xmodem_snd * sx)
{
	uint8_t buf[4];
	uint8_t * data;
	int data_len;
	int data_max;
	int ret;


	if ((data_len = sx->data_len) <= (1024 - 128))
		data_max = 128;
	else
		data_max =  sx->data_max;

	data = sx->pkt.data;

	while (data_len > 0) {
		int len;
		int i;

		len = MIN(data_len, data_max);

		/* padding */
		for (i = len; i < data_max; ++i)
			data[i] = '\0';


		if ((ret = xmodem_send_pkt(sx, data, data_max)) < 0) {
			return ret;
		}

		data_len -= len;
		data += len;
	}


	buf[0] = EOT;
	buf[1] = EOT;
	buf[2] = EOT;
	ret = sx->comm->op.send(sx->comm->arg, buf, 3);

	sx->data_max = (sx->mode == XMODEM_SND_1K) ? 1024 : 128;
	sx->data_len = 0;
	sx->seq = 1;
	sx->state = XMODEM_SND_IDLE;

	/* flush */
	while (sx->comm->op.recv(sx->comm->arg, sx->pkt.data, 1024, 200) > 0); 
	
	return ret;
}

