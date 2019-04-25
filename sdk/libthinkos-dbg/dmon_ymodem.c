/* 
 * File:	 usb-cdc.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>
#include <sys/param.h>
#include <sys/stm32f.h>
#include <crc.h>
#include <vt100.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define XMODEM_RCV_TMOUT_MS 2000

int dmon_ymodem_rcv_init(struct ymodem_rcv * rx, bool crc_mode, bool xmodem)
{
	rx->pktno = 0;
	rx->crc_mode = crc_mode;
	rx->xmodem = xmodem;
	rx->sync = rx->crc_mode ? 'C' : NAK;
	rx->xmodem = 0;
	rx->retry = 30;
	rx->fsize = 1024 * 1024;
	rx->count = 0;

	return 0;
}

#if 0
static int dmon_ymodem_rcv_cancel(struct dbgmon_comm * comm, struct ymodem_rcv * rx)
{
	unsigned char * pkt = rx->pkt.hdr;

	pkt[0] = CAN;
	pkt[1] = CAN;
	pkt[2] = CAN;

	dbgmon_comm_send(comm, pkt, 3);

	return 0;
}
#endif

int dmon_ymodem_rcv_pkt(const struct dbgmon_comm * comm, 
						struct ymodem_rcv * rx)
{
	unsigned char * pkt = rx->pkt.hdr;
	unsigned char * cp;
	int ret = 0;
	int cnt = 0;
	unsigned int nseq;
	unsigned int seq;
	int rem;
	int len;

	for (;;) {

		dbgmon_alarm_stop();
		DCC_LOG1(LOG_TRACE, VT_PSH VT_FMG VT_REV " SYN=%02x " VT_POP, rx->sync);
		dbgmon_comm_send(comm, &rx->sync, 1);

		dbgmon_alarm(XMODEM_RCV_TMOUT_MS);

		for (;;) {
			int c;

			ret = dbgmon_comm_recv(comm, pkt, 1);
			if (ret < 0) {
				DCC_LOG(LOG_WARNING, VT_PSH VT_FMG VT_BRI "SYN timeout!" VT_POP);
				goto timeout;
			}

			c = pkt[0];

			if (c == STX) {
				DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_REV " STX " VT_POP);
				cnt = 1024;
				break;
			}

			if (c == SOH) {
				DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_REV " SOH " VT_POP);
				cnt = 128;
				break;
			}

			if (c == CAN) {
				DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_REV " CAN " VT_POP);
				dbgmon_alarm_stop();
				return -1;
			}

			if (c == EOT) {
				DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_REV " EOT " VT_POP);
				/* end of transmission */
				rx->sync = rx->crc_mode ? 'C' : NAK;
				rx->pktno = 0;
				pkt[0] = ACK;
				dbgmon_alarm_stop();
				dbgmon_comm_send(comm, pkt, 1);
				return 0;
			}
		}

		len = cnt + ((rx->crc_mode) ? 5 : 4);
		cp = pkt + 1;
		rem = len - 1;

		/* receive the packet */
		while (rem) {
			dbgmon_alarm(XMODEM_RCV_TMOUT_MS);
			ret = dbgmon_comm_recv(comm, cp, rem);
			if (ret < 0) {
				DCC_LOG2(LOG_WARNING, VT_PSH VT_FMG VT_BRI 
						 "timeout len=%d rem=%d" VT_POP, len, rem);
				goto timeout;
			}
			rem -= ret;
			cp += ret;
		}

		/* sequence */
		seq = pkt[1];
		/* inverse sequence */
		nseq = pkt[2];

		if (seq != ((~nseq) & 0xff)) {
			DCC_LOG1(LOG_WARNING, VT_PSH VT_FMG VT_BRI VT_REV
					 "invalid seq=%d..." VT_POP, seq);
			goto error;
		}

		cp = &pkt[3];

		if (rx->crc_mode) {
			unsigned short crc = 0;
			unsigned short cmp;
			int i;

			for (i = 0; i < cnt; ++i)
				crc = CRC16CCITT(crc, cp[i]);

			cmp = (unsigned short)cp[i] << 8 | cp[i + 1];

			if (cmp != crc) {
				DCC_LOG2(LOG_WARNING, VT_PSH VT_FMG VT_BRI VT_REV
						" /!\\ CRC error: %04x != %04x /!\\ " 
						VT_POP, crc, cmp);

				DCC_XXD(LOG_MSG, "PKT", pkt, len);
				goto error;
			}

		} else {
			unsigned char cks = 0;
			int i;

			for (i = 0; i < cnt; ++i)
				cks += cp[i];

			if (cp[i] != cks) {
				DCC_LOG(LOG_WARNING, VT_PSH VT_FMG VT_BRI VT_REV
						"wrong sum" VT_POP);
				goto error;
			}
		}

		if (seq == ((rx->pktno - 1) & 0xff)) {
			/* retransmission */
			if ((seq == 0) && (rx->pktno == 1) && (rx->xmodem == 0)) {
				DCC_LOG(LOG_WARNING, VT_PSH VT_FMG VT_BRI VT_UND
						"Ymodem restart..." VT_POP);
				rx->pktno = 0;
			} else {
				DCC_LOG2(LOG_WARNING, VT_PSH VT_FMG VT_BRI VT_UND
						 "pktno=%d count=%d rxmit ..." VT_POP, 
						 rx->pktno, rx->count);
				continue;
			}
		}

		if (seq != (rx->pktno & 0xff)) {
			if ((rx->pktno == 0) && (seq == 1)) {
				rx->pktno++;
				/* Fallback to XMODEM */
				rx->xmodem = 1;
				DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_BRI VT_UND
						"XMODEM..." VT_POP);
			} else {
				DCC_LOG(LOG_WARNING, VT_PSH VT_FMG VT_BRI VT_UND
						"wrong sequence" VT_POP);
				goto error;
			}
		}

		/* YModem first packet ... */
		if (rx->pktno == 0) {
			DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_REV " ACK " VT_POP);
			pkt[0] = ACK;
			dbgmon_comm_send(comm, pkt, 1);
		} else {
			rx->retry = 10;
			rx->sync = ACK;
			if ((rx->count + cnt) > rx->fsize)
				cnt = rx->fsize - rx->count;
			rx->count += cnt;
		}

		DCC_LOG2(LOG_TRACE, VT_PSH VT_FMG 
				 "pktno=%d count=%d" VT_POP, rx->pktno, rx->count);
		rx->pktno++;

		dbgmon_alarm_stop();
		return cnt;

error:
		/* flush */
		while (dbgmon_comm_recv(comm, pkt, 1024) > 0);
		ret = -1;
		break;

timeout:
		DCC_LOG(LOG_WARNING, "timeout!!");
		if ((--rx->retry) == 0) {
			if ((rx->count == rx->fsize) && (rx->xmodem == 0)) {
				DCC_LOG(LOG_TRACE, "transfer complete!!");
				return 0;
			}
			/* too many errors */
			ret = -1;
			break;
		}
	}

	pkt[0] = CAN;
	pkt[1] = CAN;

	dbgmon_comm_send(comm, pkt, 2);
	dbgmon_alarm_stop();

	return ret;
}

