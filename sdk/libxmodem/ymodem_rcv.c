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
 * @file ymodem_rcv.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#include <sys/param.h>
#include <stdlib.h>
#include <errno.h>
#include <thinkos.h>

#include <xmodem.h>
#include <crc.h>

#include <sys/dcclog.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define YMODEM_RCV_TMOUT_MS 2000

int ymodem_rcv_init(struct ymodem_rcv * ry, const struct comm_dev * comm, 
					unsigned int mode)
{
	if ((ry == NULL) || (comm == NULL) || (mode > XMODEM_RCV_CRC))
		return -EINVAL;

	ry->comm = comm;

	ry->fsize = 1024 * 1024;
	ry->pktno = 0;
	ry->crc_mode = (mode == XMODEM_RCV_CRC) ? 1 : 0;
	ry->sync = ry->crc_mode ? 'C' : NAK;
	ry->xmodem = 0;
	ry->retry = 30;
	ry->fsize = 1024 * 1024;
	ry->count = 0;
	ry->data_len = 0;
	ry->data_pos = 0;

	DCC_LOG1(LOG_TRACE, "%s mode", ry->crc_mode ? "CRC" : "CKSUM");

	return 0;
}

int ymodem_rcv_cancel(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;

	pkt[0] = CAN;
	pkt[1] = CAN;

	ry->comm->op.send(ry->comm->arg, pkt, 2);

	return 0;
}

#if 0
static int ymodem_rcv_pkt(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;
	unsigned char * cp;
	int ret = 0;
	int cnt = 0;
	int nseq;
	int seq;
	int rem;

	for (;;) {

		if ((ret = ry->comm->op.send(ry->comm->arg, &ry->sync, 1)) < 0) {
			return ret;
		}

		for (;;) {
			int c;

			ret = ry->comm->op.recv(ry->comm->arg, pkt, 
									1, YMODEM_RCV_TMOUT_MS);

			if (ret == THINKOS_ETIMEDOUT)
				goto timeout;

			if (ret < 0)
				return ret;

			c = pkt[0];

			if (c == STX) {
				cnt = 1024;
				break;
			}

			if (c == SOH) {
				cnt = 128;
				break;
			}

			if (c == EOT) {
				/* end of transmission */
				pkt[0] = ACK;
				if ((ret = ry->comm->op.send(ry->comm->arg, pkt, 1)) < 0)
					return ret;

				return 0;
			}
		}

		rem = cnt + ((ry->mode) ? 4 : 3);
		cp = pkt + 1;


		/* receive the packet */
		while (rem) {

			ret = ry->comm->op.recv(ry->comm->arg, cp, rem, 500);
			if (ret == THINKOS_ETIMEDOUT)
				goto timeout;
			if (ret < 0)
				return ret;

			rem -= ret;
			cp += ret;
		}

		/* sequence */
		seq = pkt[1];
		/* inverse sequence */
		nseq = pkt[2];

		if (seq != ((~nseq) & 0xff)) {
			goto error;
		}

		cp = &pkt[3];

		if (ry->mode) {
			unsigned short crc = 0;
			unsigned short cmp;
			int i;

			for (i = 0; i < cnt; ++i)
				crc = CRC16CCITT(crc, cp[i]);

			cmp = (unsigned short)cp[i] << 8 | cp[i + 1];

			if (cmp != crc) {
				goto error;
			}

		} else {
			unsigned char cks = 0;
			int i;

			for (i = 0; i < cnt; ++i)
				cks += cp[i];

			if (cp[i] != cks)
				goto error;
		}


		if (seq == ((ry->pktno - 1) & 0xff)) {
			/* retransmission */
			ry->sync = ACK;
			continue;
		}

		if (seq != ry->pktno) {
			goto error;
		}

		ry->pktno = (ry->pktno + 1) & 0xff;
		ry->retry = 10;
		ry->sync = ACK;
		ry->data_len = cnt;
		ry->data_pos = 0;

		return cnt;

error:
		/* flush */
		while (ry->comm->op.recv(ry->comm->arg, pkt, 1024, 200) > 0);
		ry->sync = NAK;

timeout:

		if ((--ry->retry) == 0) {
			/* too many errors */
			ret = -1;
			break;
		}
	}


	pkt[0] = CAN;
	pkt[1] = CAN;
	pkt[2] = CAN;

	ry->comm->op.send(ry->comm->arg, pkt, 3);

	return ret;
}
#endif

static int ymodem_rcv_pkt(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;
	unsigned char * cp;
	int ret = 0;
	int cnt = 0;
	int seq;
	int nseq;
	int rem;
	int pos;
	int i;

	for (;;) {
		if ((ret = ry->comm->op.send(ry->comm->arg, &ry->sync, 1)) < 0) {
			return ret;
		}

		rem = 0;
		pos = 0;
		for (;;) {
			int c;

			if (rem == 0) {
				ret = ry->comm->op.recv(ry->comm->arg, pkt, 
										128, YMODEM_RCV_TMOUT_MS);
				DCC_LOG1(LOG_TRACE, "comm.recv()=%d", ret);

				if (ret == THINKOS_ETIMEDOUT)
					goto timeout;

				if (ret < 0) {
					DCC_LOG1(LOG_ERROR, "comm receive error, ret=%d", ret);
					return ret;
				}	

				pos = 0;
				rem = ret;
			}

			c = pkt[pos];
			pos++;
			rem--;

			if (c == STX) {
				DCC_LOG(LOG_TRACE, "STX");
				cnt = 1024;
				break;
			}

			if (c == SOH) {
				DCC_LOG(LOG_TRACE, "SOH");
				cnt = 128;
				break;
			}

			if (c == CAN) {
				DCC_LOG(LOG_TRACE, "CAN");
				return -1;
			}

			if (c == EOT) {
				DCC_LOG(LOG_TRACE, "EOT");
				/* end of transmission */
				ry->sync = ry->crc_mode ? 'C' : NAK;
				ry->pktno = 0;
				pkt[0] = ACK;
				ry->comm->op.send(ry->comm->arg, pkt, 1);
				return 0;
			}
		}

		cp = pkt + 1;
		for (i = 0; i < rem; ++i)
			cp[i] = pkt[pos + i];
		cp += rem;

		rem = cnt + 4 - rem;
		DCC_LOG1(LOG_TRACE, "%d remaining bytes...", rem);

		/* receive the packet */
		while (rem) {
			ret = ry->comm->op.recv(ry->comm->arg, cp, rem, 500);
			DCC_LOG1(LOG_TRACE, "comm.recv()=%d", ret);

			if (ret == THINKOS_ETIMEDOUT)
				goto timeout;

			if (ret < 0) {
				DCC_LOG1(LOG_ERROR, "comm receive error, ret=%d", ret);
				return ret;
			}	

			rem -= ret;
			cp += ret;
		}

		/* sequence */
		seq = pkt[1];
		/* inverse sequence */
		nseq = pkt[2];

		if (seq != ((~nseq) & 0xff)) {
			DCC_LOG(LOG_ERROR, "sequence mismatch!");
			goto error;
		}
		cp = &pkt[3];

		if (ry->crc_mode) 
		{
			unsigned int crc = 0;
			unsigned int cmp;
			int i;

			for (i = 0; i < cnt; ++i) {
				crc = CRC16CCITT(crc, cp[i]);
				DCC_LOG1(LOG_TRACE, "%02x", cp[i]);
			}

			cmp = ((unsigned int)cp[i] << 8) | cp[i + 1];

			if (cmp != crc) {
				DCC_LOG2(LOG_ERROR, "CRC error! %04x!=%04x", cmp, crc);
				goto error;
			}

		} else {
			unsigned char cks = 0;
			int i;

			for (i = 0; i < cnt; ++i)
				cks += cp[i];

			if (cp[i] != cks) {
				DCC_LOG(LOG_ERROR, "checksum error!");
				goto error;
			}
		}

		if (seq == ((ry->pktno - 1) & 0xff)) {
			/* retransmission */
			DCC_LOG(LOG_WARNING, "retransmission!");
			continue;
		}

		DCC_LOG2(LOG_TRACE, "seq=%d pktno=%d", seq, ry->pktno);

		if (seq != (ry->pktno & 0xff)) {
			if ((ry->pktno == 0) && (seq == 1)) {
				ry->pktno++;
				/* Fallback to XMODEM */
				ry->xmodem = 1;
			} else {
				DCC_LOG(LOG_ERROR, "sequence error!");
				goto error;
			}
		}

		/* YModem first packet ... */
		if (ry->pktno == 0) {
			pkt[0] = ACK;
			ry->comm->op.send(ry->comm->arg, pkt, 1);
		} else {
			ry->retry = 10;
			ry->sync = ACK;
			if ((ry->count + cnt) > ry->fsize)
				cnt = ry->fsize - ry->count;
			ry->count += cnt;
		}

		ry->pktno++;
		ry->data_len = cnt;
		ry->data_pos = 0;

		return cnt;

error:
		/* flush */
		while (ry->comm->op.recv(ry->comm->arg, pkt, 1024, 200) > 0);
		ry->sync = NAK;
		ret = -1;
		break;

timeout:
		DCC_LOG(LOG_TRACE, "timeout...");

		if ((--ry->retry) == 0) {
			/* too many errors */
			DCC_LOG(LOG_WARNING, "too many errors!");
			ret = -1;
			break;
		}
	}

	ymodem_rcv_cancel(ry);

	return ret;
}

unsigned long dec2int(const char * __s)
{
	unsigned long val = 0;
	char * cp = (char *)__s;
	char c;

	while ((c = *cp) != '\0') {
		if ((c < '0') || (c > '9'))
			break;
		val = val * 10;
		val += c - '0';
		cp++;
	}

	return val;
}

int ymodem_rcv_loop(struct ymodem_rcv * ry, void * data, int len)
{
	unsigned char * dst = (unsigned char *)data;
	int rem;
	int ret;

	if ((dst == NULL) || (len <= 0)) {
		return -EINVAL;
	}

	do {
		if ((rem = (ry->data_len - ry->data_pos)) > 0) {
			unsigned char * src;
			int n;
			int i;

			n = MIN(rem, len);
			src = &ry->pkt.data[ry->data_pos];

			for (i = 0; i < n; ++i)
				dst[i] = src[i];

			ry->data_pos += n;

			DCC_LOG1(LOG_TRACE, "pending data %d", n);

			return n;
		}

		ret = ymodem_rcv_pkt(ry);

		if (ret > 0 && !ry->xmodem && ry->pktno == 1) {
			char * cp;
			int fsize;
			cp = (char *)ry->pkt.data;
			while (*cp != '\0')
				cp++;
			cp++; /* skip null */
			fsize = dec2int(cp);
			if (fsize == 0) {
				DCC_LOG(LOG_WARNING, "empty file!");
				break;
			}
			ry->fsize = fsize;

			DCC_LOG1(LOG_TRACE, "file size=%d", fsize);
		}

	} while (ret >= 0);

	return ret;
}

