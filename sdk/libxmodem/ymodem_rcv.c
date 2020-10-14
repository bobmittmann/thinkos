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

	ry->pktno = 0;
	ry->crc_mode = (mode == XMODEM_RCV_CRC) ? 1 : 0;
	ry->xmodem = 0;
	ry->sync = ry->crc_mode ? 'C' : NAK;
	ry->retry = 30;
	ry->fsize = 1024 * 1024;
	ry->count = 0;
	ry->data_len = 0;
	ry->data_pos = 0;

	/* State initialized */
	ry->state = YR_IDL;

	DCC_LOG1(LOG_TRACE, "[IDLE] %s mode", ry->crc_mode ? "CRC" : "CKSUM");

	return 0;
}

int ymodem_rcv_cancel(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;

	pkt[0] = CAN;
	pkt[1] = CAN;

	DCC_LOG(LOG_TRACE, " --> CAN");
	ry->comm->op.send(ry->comm->arg, pkt, 2);

	return 0;
}

static int ymodem_rcv_pkt(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;
	unsigned char * cp;
	unsigned int nseq;
	unsigned int seq;
	int ret = 0;
	int cnt = 0;
	int rem;
	int len;

	if (ry->state < YR_IDL) {
		DCC_LOG1(LOG_ERROR, "invalid state %d", ry->state);
		return -EPERM;
	}

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

			if (ret < 0) {
				DCC_LOG1(LOG_ERROR, "comm receive error, ret=%d", ret);
				return ret;
			}	

			c = pkt[0];

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
				ry->state = YR_EOT;
				pkt[0] = ACK;
				DCC_LOG(LOG_TRACE, "[EOT] --> ACK");
				ry->comm->op.send(ry->comm->arg, pkt, 1);
				return 0;
			}
		}

		len = cnt + ((ry->crc_mode) ? 5 : 4);
		cp = pkt + 1;
		rem = len - 1;

		DCC_LOG1(LOG_TRACE, "%d remaining bytes...", rem);
		/* receive the packet */
		while (rem) {
			ret = ry->comm->op.recv(ry->comm->arg, cp, rem, 500);

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

		if (ry->crc_mode) {
			unsigned int crc = 0;
			unsigned int cmp;
			int i;

			for (i = 0; i < cnt; ++i) {
				crc = CRC16CCITT(crc, cp[i]);
			}

			cmp = ((unsigned int)cp[i] << 8) | cp[i + 1];

			if (cmp != crc) {
				DCC_LOG2(LOG_ERROR, "CRC error! %04x!=%04x", cmp, crc);
				goto error;
			}

			DCC_LOG2(LOG_TRACE, "CRC OK, seq=%d pktno=%d", seq, ry->pktno);
		} else {
			unsigned char cks = 0;
			int i;

			for (i = 0; i < cnt; ++i)
				cks += cp[i];

			if (cp[i] != cks) {
				DCC_LOG(LOG_ERROR, "checksum error!");
				goto error;
			}
		
			DCC_LOG2(LOG_TRACE, "CKS OK, seq=%d pktno=%d", seq, ry->pktno);
		}


        if (seq == ((ry->pktno - 1) & 0xff)) {
            /* retransmission!! */
            if ((seq == 0) && (ry->pktno == 1) && (ry->xmodem == 0)) {
                DCC_LOG(LOG_WARNING, "Ymodem restart..." );
                ry->pktno = 0;
            } else {
                DCC_LOG2(LOG_WARNING, "pktno=%d count=%d rxmit ..." ,
                         ry->pktno, ry->count);
                continue;
            }
        }

		if (seq != (ry->pktno & 0xff)) {
			if ((ry->pktno == 0) && (seq == 1)) {
				ry->pktno++;
				/* Fallback to XMODEM */
				ry->xmodem = 1;
				DCC_LOG(LOG_TRACE, "Xmodem fallback");
			} else {
				DCC_LOG(LOG_ERROR, "sequence error!");
				goto error;
			}
		}

		if (ry->pktno == 0) {
			/* YModem first packet ... */
			DCC_LOG(LOG_TRACE, "[HDR] --> ACK");
			pkt[0] = ACK;
			ry->comm->op.send(ry->comm->arg, pkt, 1);
		} else {
			ry->retry = 10;
			ry->sync = ACK;
			if ((ry->count + cnt) > ry->fsize)
				cnt = ry->fsize - ry->count;
			ry->count += cnt;
			ry->state = YR_DAT;
			DCC_LOG(LOG_TRACE, "[DAT] --> ACK");
		}

		ry->pktno++;

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
            if ((ry->count == ry->fsize) && (ry->xmodem == 0)) {
                DCC_LOG(LOG_TRACE, "[EOT] transfer complete!!");
				ry->state = YR_EOT;
                return 0;
            }
			/* too many errors */
			DCC_LOG(LOG_WARNING, "[ERR] too many errors!");
			ry->state = YR_ERR;
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

static int ymodem_rcv_decode(void * pkt, int len, char * fname)
{
	unsigned int fsize;
	char * cp;
	int ret;
	int i;

	cp = (char *)pkt;
	if (*cp == '\0') {
		DCC_LOG(LOG_WARNING, "file name is empty!");
		ret = 0;
	} else {

		DCC_LOGSTR(LOG_TRACE, "fname='%s'", cp);
		if (fname != NULL) { 
			for (i = 0; (cp[i] != '\0') & (i < len); ++i)
				fname[i] = cp[i];
			fname[i] = '\0';
		} else {
			for (i = 0; (cp[i] != '\0') & (i < len); ++i);
		}
		for (; (cp[i] == '\0') & (i < len); ++i);

		if (i < len) {
			fsize = dec2int(&cp[i]);
			if (fsize == 0) {
				DCC_LOG(LOG_WARNING, "file size is zero!");
			} else {
				DCC_LOG1(LOG_TRACE, "fsize=%d", fsize);
			}
			ret = fsize;
		} else {
			DCC_LOG(LOG_WARNING, "no file size info!");
			ret = -1;
		}
	}

	return ret;
}

int ymodem_rcv_loop(struct ymodem_rcv * ry, void * data, int len)
{
	unsigned char * dst = (unsigned char *)data;
	int rem;
	int ret;

	if ((dst == NULL) || (len <= 0)) {
		return -EINVAL;
	}

	for (;;) {
		if ((rem = (ry->data_len - ry->data_pos)) > 0) {
			unsigned char * src;
			int cnt;
			int i;

			cnt = MIN(rem, len);
			src = &ry->pkt.data[ry->data_pos];

			for (i = 0; i < cnt; ++i)
				dst[i] = src[i];

			ry->data_pos += cnt;

			ret = cnt;
			break;
		}

		if ((ret = ymodem_rcv_pkt(ry)) <= 0)
			break;

		if (!ry->xmodem && (ry->pktno == 1)) {
			if ((ret = ymodem_rcv_decode(ry->pkt.data, 
										 len, NULL)) > 0) {
				ry->fsize = ret;
				ry->data_len = 0;
				ry->data_pos = 0;
				ret = 0;
				break;
			}
		} else {
			ry->data_len = ret;
			ry->data_pos = 0;
		}
	} 

	return ret;
}

int ymodem_rcv_start(struct ymodem_rcv * ry, char * fname, 
					 unsigned int * pfsize)
{
	int ret;

	if (ry->state < YR_IDL) {
		DCC_LOG1(LOG_ERROR, "invalid state %d", ry->state);
		return -EPERM;
	}

	while ((ret = ymodem_rcv_pkt(ry)) > 0) { 

		if (ry->xmodem)
			return ret;

		if (ry->pktno == 1) {
			int len = ret;

			if ((ret = ymodem_rcv_decode(ry->pkt.data, len, fname)) > 0) {
				ry->fsize = ret;
				if (pfsize != NULL)
					*pfsize = ret;

				DCC_LOG1(LOG_TRACE, "fsize=%d", ret);
			}

		} else {
			DCC_LOG1(LOG_WARNING, "pktno=%d!!!!", ry->pktno);
			ry->data_len = ret;
			ry->data_pos = 0;
		}

		if (ry->state == YR_DAT) {
			ret = 0;
			break;
		}
	}

	return ret;
}


