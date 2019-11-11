/* 
 * File:	 xflash.c
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

#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <stdint.h>
#include <stdbool.h>
#include <crc.h>
#include "xflash.h"

#include <sys/dcclog.h>

#ifndef XMODEM_CHECKSUM
#define XMODEM_CHECKSUM     0
#endif

#ifndef XMODEM_FALLBACK
#define XMODEM_FALLBACK     0
#endif

#ifndef XMODEM_CRC_CHECK
#define XMODEM_CRC_CHECK    0
#endif

#ifndef XMODEM_SEQUENCE_CHECK
#define XMODEM_SEQUENCE_CHECK 0
#endif

#ifndef XFLASH_MAGIC
#define XFLASH_MAGIC        1
#endif

#ifndef XFLASH_VERBOSE
#define XFLASH_VERBOSE      0
#endif

#ifndef XFLASH_DEBUG        
#define XFLASH_DEBUG        0
#endif

#define CDC_TX_EP 2
#define CDC_RX_EP 1

void __attribute__((noreturn)) reset(void);
void delay(unsigned int msec);

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18

#define XMODEM_RCV_TMOUT_MS 2000

struct ymodem_rcv {
	unsigned int pktno;
	unsigned int fsize;
	unsigned int count;

#if XMODEM_CHECKSUM
	bool crc_mode;
#endif
#if XMODEM_FALLBACK
	bool xmodem;
#endif
	unsigned char sync;
	unsigned char retry;

	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};

static int ymodem_rcv_init(struct ymodem_rcv * rx)
{
	rx->pktno = 0;
#if XMODEM_CHECKSUM
	rx->crc_mode = true;
	rx->sync = rx->crc_mode ? 'C' : NAK;
#else
	rx->sync = 'C';
#endif
#if XMODEM_FALLBACK
	rx->xmodem = false;
#endif
	rx->retry = 30;
	rx->fsize = 1024 * 1024;
	rx->count = 0;

	return 0;
}

static int usb_ymodem_rcv_cancel(struct ymodem_rcv * rx)
{
	unsigned char * pkt = rx->pkt.hdr;

	DCC_LOG(LOG_WARNING, "CANCEL!");

	pkt[0] = CAN;
	pkt[1] = CAN;

	return usb_send(CDC_TX_EP, pkt, 2);
}

static int usb_ymodem_rcv_pkt(struct ymodem_rcv * rx)
{
	unsigned char * pkt = rx->pkt.hdr;
	unsigned char * cp;
	int ret = 0;
	int cnt = 0;
	int seq;
#if XMODEM_SEQUENCE_CHECK
	int nseq;
#endif
	int rem;
	int pos;
	int i;

	for (;;) {
		if ((ret = usb_send(CDC_TX_EP, &rx->sync, 1)) < 0) {
			return ret;
		}

		rem = 0;
		pos = 0;
		for (;;) {
			int c;

			if (rem == 0) {
				ret = usb_recv(CDC_RX_EP, pkt, 128, 2000);
				DCC_LOG1(LOG_TRACE, "usb_recv() ret=%d)", ret);

				if (ret <= 0)
					goto timeout;

				pos = 0;
				rem = ret;
			}

			c = pkt[pos];
			pos++;
			rem--;

			if (c == STX) {
				cnt = 1024;
				break;
			}

			if (c == SOH) {
				cnt = 128;
				break;
			}

			if (c == CAN) {
				return -1;
			}

			if (c == EOT) {
				/* end of transmission */
#if XMODEM_CHECKSUM
				rx->sync = rx->crc_mode ? 'C' : NAK;
#else
				rx->sync = 'C';
#endif
				rx->pktno = 0;
				pkt[0] = ACK;
				usb_send(CDC_TX_EP, pkt, 1);
				return 0;
			}
		}

		cp = pkt + 1;
		for (i = 0; i < rem; ++i)
			cp[i] = pkt[pos + i];
		cp += rem;

		rem = cnt + 4 - rem;

		/* receive the packet */
		while (rem) {
			ret = usb_recv(CDC_RX_EP, cp, rem, 500);
			DCC_LOG1(LOG_TRACE, "usb_recv() ret=%d)", ret);
			if (ret < 0)
				goto timeout;

			rem -= ret;
			cp += ret;
		}

		/* sequence */
		seq = pkt[1];
#if XMODEM_SEQUENCE_CHECK
		/* inverse sequence */
		nseq = pkt[2];

		if (seq != ((~nseq) & 0xff)) {
			goto error;
		}
#endif
		cp = &pkt[3];

#if XMODEM_CHECKSUM
		if (rx->crc_mode) 
#endif
#if XMODEM_CRC_CHECK
		{
			unsigned short crc = 0;
			unsigned short cmp;
			int i;

			for (i = 0; i < cnt; ++i)
				crc = CRC16CCITT(crc, cp[i]);

			cmp = (unsigned short)cp[i] << 8 | cp[i + 1];

			if (cmp != crc) {
				goto error;
			}

		} 
#endif
#if XMODEM_CHECKSUM
		else {
			unsigned char cks = 0;
			int i;

			for (i = 0; i < cnt; ++i)
				cks += cp[i];

			if (cp[i] != cks) {
				goto error;
			}
		}
#endif

		if (seq == ((rx->pktno - 1) & 0xff)) {
			/* retransmission */
			DCC_LOG(LOG_WARNING, "retransmission!");
			continue;
		}

		DCC_LOG2(LOG_TRACE, "seq=%d pktno=%d", seq, rx->pktno);

#if XMODEM_SEQUENCE_CHECK
		if (seq != (rx->pktno & 0xff)) {
#if XMODEM_FALLBACK
			if ((rx->pktno == 0) && (seq == 1)) {
				rx->pktno++;
				/* Fallback to XMODEM */
				rx->xmodem = true;
			} else 
#endif
			{
				goto error;
			}
		}
#endif
		/* YModem first packet ... */
		if (rx->pktno == 0) {
			pkt[0] = ACK;
			usb_send(CDC_TX_EP, pkt, 1);
		} else {
			rx->retry = 10;
			rx->sync = ACK;
			if ((rx->count + cnt) > rx->fsize)
				cnt = rx->fsize - rx->count;
			rx->count += cnt;
		}

		rx->pktno++;

		return cnt;

#if XMODEM_SEQUENCE_CHECK || XMODEM_CRC_CHECK
error:
		/* flush */
		while (usb_recv(CDC_RX_EP, pkt, 1024, 100) > 0);
		ret = -1;
		break;
#endif

timeout:
		if ((--rx->retry) == 0) {
			/* too many errors */
			DCC_LOG(LOG_WARNING, "too many errors!");
			ret = -1;
			break;
		}
	}


	return usb_ymodem_rcv_cancel(rx);
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

#if XFLASH_MAGIC
#define MAGIC_REC_MAX 16

static int magic_match(struct magic * magic, int pos, uint8_t * buf, int len)
{
	int sz = magic->hdr.cnt * sizeof(struct magic_rec);
	int k;
	int j;

	/* check whether the magic record is in this
	   data block */
	if (magic->hdr.pos < pos) {
		return -1;
	}

	if ((magic->hdr.pos + sz) > (pos + len)) {
		return -2;
	}

	k = magic->hdr.pos - pos;
	for (j = 0; j < magic->hdr.cnt; ++j) {
		uint32_t data;

		data = buf[k] + (buf[k + 1] << 8) + 
			(buf[k + 2] << 16) + (buf[k + 3] << 24);

		if ((data & magic->rec[j].mask) != magic->rec[j].comp) {
			return -(3 + j);
		}

		k += sizeof(uint32_t);
	}	

	return 0;
}
#endif

#if XFLASH_VERBOSE
static const char s_err[] = "\r\nErr: ";
static const char s_invalid[] = "\r\nInvalid file!";
static const char s_erase[] = "\r\nFlash erase error!";
static const char s_program[] = "\r\nFlash program error!";
static const char s_ymodem[] = "\r\nYmodem (^X to cancel)... ";
static const char s_ok[] = "\r\nOK.";
#define PUTS(STR) usb_send(CDC_TX_EP, STR, sizeof(STR) - 1)
#else
static const char s_ymodem[] = {'\r', '\n', 'Y', 'm', 'o', 'd', 'e', 'm', 
	'.', '.', '.' };
static const char s_ok[] = {'\r', '\n', 'O', 'K'};
#define PUTS(STR) usb_send(CDC_TX_EP, STR, sizeof(STR))
#endif

static const char err_code[] = { '0', '1', '2', '3', '4', '5', '6' };

#define ERROR(ERR) usb_send(CDC_TX_EP, s_err, sizeof(s_err) - 1); \
	usb_send(CDC_TX_EP, &err_code[ERR], 1);

#if 0
int __attribute__((noreturn)) yflash(uint32_t blk_offs, unsigned int blk_size, 
		   const struct magic * magic, unsigned int opt)
#endif
int __attribute__((noreturn)) yflash(uint32_t blk_offs, unsigned int blk_size, 
		   const struct magic * magic)
{
#if XFLASH_MAGIC
	struct {
		struct magic_hdr hdr;
		struct magic_rec rec[MAGIC_REC_MAX];
	} magic_buf;
	int i;
#endif
	unsigned int cnt;
	struct ymodem_rcv ry;
	uint32_t offs;
	int ret;

#if XFLASH_DEBUG
	usb_send(CDC_TX_EP, "\r\noffs ", 7);
	usb_send_hex(CDC_TX_EP, blk_offs);
	usb_send(CDC_TX_EP, "size ", 7);
	usb_send_hex(CDC_TX_EP, blk_size);
#endif

#if XFLASH_MAGIC
	if (magic != 0) {
		/* copy magic check block */
		cnt = magic->hdr.cnt > MAGIC_REC_MAX ? MAGIC_REC_MAX : magic->hdr.cnt;
		for (i = 0; i < cnt; ++i) {
			magic_buf.rec[i] = magic->rec[i];
#if XFLASH_DEBUG
			usb_send(CDC_TX_EP, "magic ", 8);
			usb_send_hex(CDC_TX_EP, magic_buf.rec[i].comp);
#endif
		}	
		magic_buf.hdr.cnt = cnt;
		magic_buf.hdr.pos = magic->hdr.pos;
	} else {
		magic_buf.hdr.cnt = 0;
		magic_buf.hdr.pos = 0;
	}
#endif

#if XFLASH_DEBUG
	usb_send(CDC_TX_EP, "\r\n", 2);
	usb_drain(CDC_TX_EP);
#endif

	DCC_LOG(LOG_TRACE, "flash_unlock()");
#if 0
	if (opt & XFLASH_OPT_BYPASS) {
		PUTS(s_ok);
		usb_drain(CDC_TX_EP);
		return 55;
	}
#endif
	do {
		PUTS(s_ymodem);
		ymodem_rcv_init(&ry);
		ry.fsize = blk_size;
		offs = blk_offs;
		cnt = 0;

		flash_unlock();

		while ((ret = usb_ymodem_rcv_pkt(&ry)) >= 0) {
			int len;

			DCC_LOG1(LOG_TRACE, "usb_ymodem_rcv_pkt() ret=%d)", ret);
#if XMODEM_FALLBACK
			if ((ret == 0) && (ry.xmodem)) {
				LOG_CODE(1);
				break;
			}
#endif
			len = ret;

			if (ry.pktno == 1) {
				char * cp;
				int fsize;
				cp = (char *)ry.pkt.data;
				while (*cp != '\0')
					cp++;
				cp++; /* skip null */
				fsize = dec2int(cp);
				if (fsize == 0) {
					DCC_LOG(LOG_WARNING, "empty file!");
					break;
				}
				ry.fsize = fsize;

				DCC_LOG1(LOG_TRACE, "file size=%d", fsize);

			} else {
#if XFLASH_MAGIC
				if (ry.pktno == 2) {
					if ((ret = magic_match((struct magic *)&magic_buf, 
									 cnt, ry.pkt.data, len)) < 0) {
						DCC_LOG(LOG_WARNING, "invalid file magic!");
#if XFLASH_VERBOSE
						usb_ymodem_rcv_cancel(&ry);
						delay(1000);
						PUTS(s_invalid);
#if XFLASH_DEBUG
						ERROR(-ret);
						usb_hex_dump(CDC_TX_EP, ry.pkt.data, len);
#endif
#endif
						break;
					}
				}
#endif 

				DCC_LOG2(LOG_TRACE, "flash_erase(offs=0x%06x, len=%d)", 
						 offs, len);

				if ((ret = flash_erase(offs, len)) < 0) {
					DCC_LOG(LOG_WARNING, "flash_erase() failed!");
#if XFLASH_VERBOSE
					usb_ymodem_rcv_cancel(&ry);
					delay(1000);
					PUTS(s_erase);
#endif
					break;
				}
	
				DCC_LOG(LOG_TRACE, "flash_write()");
				if ((ret = flash_write(offs, ry.pkt.data, len)) < 0) {
					DCC_LOG(LOG_WARNING, "flash_write() failed!");
#if XFLASH_VERBOSE
					usb_ymodem_rcv_cancel(&ry);
					delay(1000);
					PUTS(s_program);
#endif
					break;
				}
				offs += len;
				cnt += len;
			}
		}

//		if (opt & XFLASH_OPT_RET_ERR) { 
//			return ret;
//		}
	} while ((ret < 0) || (cnt == 0));

	PUTS(s_ok);

	usb_drain(CDC_TX_EP);

//	if (opt & XFLASH_OPT_RESET) { 
		delay(3000);
		reset();
//	}

//	return 0;
}


