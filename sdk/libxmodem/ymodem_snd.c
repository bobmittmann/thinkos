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
#include <string.h>

#include <xmodem.h>
#include <crc.h>
#include <errno.h>

#define TRACE_LEVEL TRACE_LVL_INF
#include <trace.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define SUB 0x1a 

#define YMODEM_SND_INIT_TMO_MS 1000
#define YMODEM_SND_TMO_MS 50
#define YMODEM_SND_EOT_TMO_MS 2000

enum {
	YMODEM_SND_IDLE = 0,
	YMODEM_SND_CRC = 1,
	YMODEM_SND_CKS = 2,
	YMODEM_SND_START = 3
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
	sy->ack = 1;
	sy->state = YMODEM_SND_IDLE;
	sy->tmout_ms = YMODEM_SND_INIT_TMO_MS;

	/* flush */
	while (sy->comm->op.recv(sy->comm->arg, sy->pkt.data, 1024, 100) > 0); 

	return 0;
}

int ymodem_snd_flush(struct ymodem_snd * sy, unsigned int tmo_ms)
{
	uint8_t buf[16];
	int n;

	/* flush */
	while ((n = sy->comm->op.recv(sy->comm->arg, buf, 16, tmo_ms)) > 0)  {
		INFX("SY", buf, n);
	}

	return 0;
}

int ymodem_snd_cancel(struct ymodem_snd * sy)
{
	uint8_t buf[4];
	int ret;


	buf[0] = CAN;

	ret = sy->comm->op.send(sy->comm->arg, buf, 1);

	sy->data_max = (sy->mode == XMODEM_SND_1K) ? 1024 : 128;
	sy->data_len = 0;
	sy->seq = 1;
	sy->ack = 1;
	sy->state = YMODEM_SND_IDLE;
	sy->tmout_ms = YMODEM_SND_INIT_TMO_MS;

	return ret;
}

static int ymodem_send_pkt(struct ymodem_snd * sy, 
						   uint8_t * data, int data_len)
{
	uint8_t * pkt = data - 3; 
	uint8_t * cp;
	uint8_t * fcs;
	int tot_len;
	int ret;
	int c;

	if (sy->state == YMODEM_SND_IDLE)
		return -EINVAL;

	if ((signed char)(sy->seq - sy->ack) >= 1) {
		// Wait for ACK
		if ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
									 1, sy->tmout_ms)) <= 0) {
			ERRS("YS: too many retries!");
			return ret;
		}

		c = *pkt;

		if (c == CAN) {
			ERRS("YS: CAN");
			ret = -ECANCELED;
			goto error;
		}

		if (c == ACK) {
			DBGS("YS: ACK");
			sy->ack++;
		} else if (sy->state == YMODEM_SND_START) {
			if (c == 'C') {
				INFS("YS: CRC mode ...");
				sy->state = YMODEM_SND_CRC;
			} else if (c == NAK) {
				INFS("YS: CKS mode ...");
				sy->state = YMODEM_SND_CKS;
			}
		} else if (c == NAK) {
			WARNS("YS: NAK");
			ret = -EBADMSG;
			goto error;
		} else { 
			INF("YS: %02x ??", c);
			ret = -EBADMSG;
			goto error;
		}
	}

	if (data_len == 1024) {
		pkt[0] = STX;
		DBGS("YS: STX");
	} else {
		pkt[0] = SOH;
		DBGS("YS: SOH");
	} 

	pkt[1] = sy->seq;
	pkt[2] = ~sy->seq;
	cp = &pkt[3];

	if ((sy->state == YMODEM_SND_CRC) || 
		(sy->state == YMODEM_SND_START)) {
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

	YAP("YS: tot_len=%d", tot_len);

	// Send packet 
	if ((ret = sy->comm->op.send(sy->comm->arg, pkt, tot_len)) < 0) {
		INFS("YS: console_write() failed!..");
		return ret;
	}

	sy->seq++;

	return 0;

error:
	/* flush */
	while (sy->comm->op.recv(sy->comm->arg, sy->pkt.data, 1024, 200) > 0); 

	return ret;
}

int ymodem_snd_start(struct ymodem_snd * sy, const char * fname, 
					 unsigned int fsize)
{
	char * buf = (char *)sy->pkt.data;
	unsigned int dlen;
	unsigned int dmax;
	uint8_t pkt[8];
	int retry;
	char * cp;
	int ret;
//	int i;
	int c;

	retry = 0;
	sy->data_len = 0;
	sy->seq = 0;
	sy->ack = 0;
	sy->tmout_ms = YMODEM_SND_INIT_TMO_MS;

	while (sy->state == YMODEM_SND_IDLE) {
		INFS("YS: IDLE...");

		/* Wait for NAK or 'C' */
		if ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
									 1, sy->tmout_ms)) <= 0) {
			/*
			   if (ret == -THINKOS_ETIMEDOUT) {
			   INFS("YS: ETIMEDOUT...");
			   if (++retry < 20) 
			   continue;
			   } else {
			   INF("YS: ret=%d", ret);
			   } */
			if (++retry < 20) 
				continue;
			return ret;
		}

		c = *pkt;

		if (c == CAN) {
			ERRS("YS: CAN");
			return -ECANCELED;
		}

		if (c == 'C') {
			INFS("YS: CRC mode");
			sy->state = YMODEM_SND_START;
			break;
		}

		if (c == NAK) {
			INFS("YS: CKS mode");
			sy->state = YMODEM_SND_CKS;
			break;
		}

		ERR("YS.Start: received '%02x' ???", c);
	}

	buf = (char *)sy->pkt.data;
	cp = buf;
	strcpy(cp, fname);
	cp += strlen(cp) + 1;
	cp += sprintf(cp, "%d ", fsize);
	dlen = cp - buf + 1;

	INF("YS.Start: fname=\"%s\" fsize=%d", fname, fsize);
	DBG("YS.Start: dlen=%d", dlen);

	if (dlen > 1024) {
		ERR("YS.Start: dlen=%d > 1024 !!!!", dlen);
		return -1;
	} 

	dmax = (dlen > 128) ? 1024 : 128;

	/* padding */
//	for (i = 0; dmax - dlen; ++i)
//			cp[i] = '\0';

	if ((ret = ymodem_send_pkt(sy, (uint8_t *)buf, dmax)) < 0) {
		ERRS("YS.SendPkt failed...");
		return ret;
	}

	sy->tmout_ms = YMODEM_SND_TMO_MS;

	sy->data_len = 0;
	sy->fsize = fsize;
	sy->count = 0;

	return 0;
}

int ymodem_snd_loop(struct ymodem_snd * sy, const void * data, int len)
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

		dst = &sy->pkt.data[sy->data_len];
		free = sy->data_max - sy->data_len;
		n = MIN(rem, free);

		for (i = 0; i < n; ++i)
			dst[i] = src[i];

		src += n;
		rem -= n;
		sy->data_len += n;

		if (sy->data_len == sy->data_max) {
			int ret;

			if ((ret = ymodem_send_pkt(sy, sy->pkt.data, sy->data_len)) < 0) {
				ERRS("YS.SendPkt failed...");
				return ret;
			}

			sy->count += sy->data_len;
			sy->data_len = 0;
		}
	}

	return len;
}

static int sy_wait_ack(struct ymodem_snd * sy)
{
	uint8_t buf[4];
	uint8_t pkt[4];
	int ret;

	while ((int8_t)(sy->seq - sy->ack) > 0) {
		DBG("YS: seq=%d ack=%d ", sy->seq, sy->ack);
		/* Wait for ACK */
		while ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
									 1, sy->tmout_ms)) == 0) {
			WARNS("YS: ret==0");
		}
		
		if (ret < 0) {
			ERRS("YS: ack wait timeout!");
			goto cancel;
//			break;
		} else {
			int c = *pkt;
			if (c == ACK) {
				DBGS("YS: ACK");
				sy->ack++;
			} else if (c == NAK) {
				WARNS("YS: NAK");
				break;
			} else {
				WARN("YS: %02x", c);
			}
		}
	}

	return 0;


cancel:
	WARNS("YS: CAN CAN CAN");
	buf[0] = CAN;
	buf[1] = CAN;
	buf[2] = CAN;
	ret = sy->comm->op.send(sy->comm->arg, buf, 3);

	return -1;
}

static int ymodem_send_char(struct ymodem_snd * sy, int c)
{
	uint8_t buf[4];
	int ret;

	buf[0] = c;

	sy->seq++;
	ret = sy->comm->op.send(sy->comm->arg, buf, 1);

	return ret;
}

int ymodem_snd_eot(struct ymodem_snd * sy)
{
	uint8_t * data;
	int data_len;
	int data_max;
	int ret;

	if ((data_len = sy->data_len) <= (1024 - 128))
		data_max = 128;
	else
		data_max =  sy->data_max;

	data = sy->pkt.data;

	DBG("ymodem_snd_eot: size=%d count=%d rem=%d...", 
		sy->fsize, sy->count, sy->fsize - sy->count);

	while (data_len > 0) {
		int len;
		int i;

		DBG("ymodem_snd_eot: rem=%d...", data_len);

		len = MIN(data_len, data_max);

		/* padding */
		for (i = len; i < data_max; ++i)
			data[i] = SUB;

		if ((ret = ymodem_send_pkt(sy, data, data_max)) < 0) {
			return ret;
		}

		data_len -= len;
		data += len;
	}

	sy_wait_ack(sy);

	INFS("YS: EOT");
	ymodem_send_char(sy, EOT);

	sy->tmout_ms = YMODEM_SND_EOT_TMO_MS;
	sy_wait_ack(sy);

	sy->state = YMODEM_SND_IDLE;
	sy->data_len = 0;
	sy->seq = 0;
	sy->ack = 0;

	return 0;
}

/* end of Batch Transmission Session */
int ymodem_snd_done(struct ymodem_snd * sy)
{
	unsigned int dmax;
	unsigned int i;
	uint8_t pkt[8];
	uint8_t * buf;
	int ret;

	for (;;) {
		int c;
		
		while ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
									 1, sy->tmout_ms)) == 0) {
				WARNS("YS: ret==0");
		}
		
		if (ret < 0) {
			WARNS("YS: EOT timeout!");
			return 0;
		} 
		
		c = *pkt;

		if (c == CAN) {
			ERRS("YS: CAN");
			return -ECANCELED;
		}

		if (c == 'C') {
			INFS("YS: CRC mode");
			sy->state = YMODEM_SND_START;
			break;
		}
	}

	/* NULL packet */
	dmax = 128;
	buf = (uint8_t *)sy->pkt.data;
	for (i = 0; i < dmax; ++i)
			buf[i] = '\0';

	INFS("YS: NULL");

	if ((ret = ymodem_send_pkt(sy, (uint8_t *)buf, dmax)) < 0) {
		ERRS("YS.SendPkt failed...");
		return ret;
	}

	INFS("YS: NULL sent...");

	while ((int8_t)(sy->seq - sy->ack) > 0) {
		// Wait for ACK
		while ((ret = sy->comm->op.recv(sy->comm->arg, pkt, 
									 1, sy->tmout_ms)) == 0) {
				WARNS("YS: ret==0");
		}
		
		if (ret < 0) {
			ERRS("YS: EOT timeout!");
			break;
		} else {
			int c = *pkt;
			if (c == ACK) {
				DBGS("YS: ACK");
				sy->ack++;
			} else if (c == NAK) {
				WARNS("YS: NAK");
			} else {
				WARN("YS: %02x", c);
			}
		}
	}

	INFS("YS: ret==0");
	sy->state = YMODEM_SND_IDLE;
	sy->data_len = 0;
	sy->seq = 0;
	sy->ack = 0;

	return 0;
}

