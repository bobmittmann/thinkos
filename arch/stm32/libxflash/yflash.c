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

#ifndef XFLASH_MAGIC
#define XFLASH_MAGIC        0
#endif

#ifndef XFLASH_VERBOSE
#define XFLASH_VERBOSE      1
#endif

#ifndef XFLASH_DEBUG        
#define XFLASH_DEBUG        1
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
	unsigned int pktno;
    unsigned int count;
	unsigned int fsize;
    unsigned int data_len;

	int state;
	bool crc_mode;
	bool xmodem;
	unsigned char sync;
	unsigned char retry;

	struct { 
		unsigned char hdr[3];
		unsigned char data[1024];
		unsigned char fcs[2];
	} pkt;
};

static void __ymodem_rcv_reset(struct ymodem_rcv * ry, unsigned int fsizemax)
{
    ry->pktno = 0;
    ry->sync = ry->crc_mode ? 'C' : NAK;
    ry->fsize = fsizemax;
    ry->retry = 30;
    ry->count = 0;
    ry->data_len = 0;
    ry->state = YR_IDL;
}

static int ymodem_rcv_init(struct ymodem_rcv * ry)
{
	ry->crc_mode = true;
	ry->xmodem = false;

	__ymodem_rcv_reset(ry, 0);
	return 0;
}

static void usb_ymodem_rcv_flush(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.data;

	while (usb_recv(CDC_RX_EP, pkt, 1024, 100) > 0);

}

#if 1
static int usb_ymodem_rcv_cancel(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;

	DCC_LOG(LOG_WARNING, "CANCEL!");
	usb_ymodem_rcv_flush(ry);

	pkt[0] = CAN;
	pkt[1] = CAN;

	return usb_send(CDC_TX_EP, pkt, 2);
}
#endif

static int usb_ymodem_rcv_pkt(struct ymodem_rcv * ry)
{
	unsigned char * pkt = ry->pkt.hdr;
	unsigned char * cp;
	int ret = 0;
	int cnt = 0;
	int seq;
	int nseq;
	int rem;
	int pos;
	int len;
	int i;

    if (ry->state < YR_IDL) {
        DCC_LOG1(LOG_ERROR, "invalid state %d", ry->state);
        return -1;
    }

	for (;;) {

		if ((ret = usb_send(CDC_TX_EP, &ry->sync, 1)) < 0) {
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
				ry->sync = ACK;
                ry->state = YR_CAN;
				return -1;
			}

			if (c == EOT) {
				/* end of transmission */
                ry->sync = ACK;
                ry->state = YR_EOT;
				return 1;
			}
		}

		cp = pkt + 1;
		for (i = 0; i < rem; ++i)
			cp[i] = pkt[pos + i];
		cp += rem;
		
		len = cnt + ((ry->crc_mode) ? 5 : 4);
		rem = len -  1;

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
		/* inverse sequence */
		nseq = pkt[2];

		if (seq != ((~nseq) & 0xff)) {
			goto error;
		}
		cp = &pkt[3];

		if (ry->crc_mode) 
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

		} else {
			unsigned char cks = 0;
			int i;

			for (i = 0; i < cnt; ++i)
				cks += cp[i];

			if (cp[i] != cks) {
				goto error;
			}
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

		DCC_LOG2(LOG_TRACE, "seq=%d pktno=%d", seq, ry->pktno);

		if (seq != (ry->pktno & 0xff)) {
			if ((ry->pktno == 0) && (seq == 1)) {
				ry->pktno++;
				/* Fallback to XMODEM */
				ry->xmodem = true;
			} else {
				goto error;
			}
		}

		/* YModem first packet ... */
      if ((ry->pktno == 0) && (!ry->xmodem) && (ry->state < YR_HDR)) {
			ry->sync = ACK;
			ry->state = YR_HDR;
		} else {
			ry->retry = 2;
			ry->sync = ACK;
			ry->state = YR_DAT;
			if ((ry->count + cnt) > ry->fsize)
				cnt = ry->fsize - ry->count;
			ry->count += cnt;
		}

		ry->pktno++;

		return cnt;

timeout:
        if (ry->state == YR_EOT) {
            ry->sync = CAN;
            ret = 0;
            break;
        }

        if (ry->state == YR_HDR) {
            ry->sync = ry->crc_mode ? 'C' : NAK;
            ret = 1;
            break;
        }

        if ((ry->count == ry->fsize) && (ry->xmodem == false)) {
            ry->sync = CAN;
            ry->state = YR_ERR;
            ret = -1;
            break;
        }

error:
        if ((--ry->retry) == 0) {
            /* too many errors */
            DCC_LOG(LOG_WARNING, "[ERR] too many errors!");
            ry->sync = CAN;
            ry->state = YR_ERR;
            ret = -1;
            break;
        }
	}

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

static int ymodem_rcv_decode(void * pkt, int len)
{
    char * cp = (char *)pkt;
    unsigned int fsize;
    int ret;
    int i;

	/* Skip file name */
	for (i = 0; (cp[i] != '\0') & (i < len); ++i);

	/* Skip zeros */
	for (; (cp[i] == '\0') & (i < len); ++i);

	if (i < len) {
		fsize = dec2int(&cp[i]);
		ret = fsize;
	} else {
		ret = 0;
	}

	return ret;
}

int usb_ymodem_rcv_start(struct ymodem_rcv * ry, unsigned int fsizemax)
{
    int ret;

    if (ry->xmodem)
        return 0;

    __ymodem_rcv_reset(ry, fsizemax);

    while ((ret = usb_ymodem_rcv_pkt(ry)) > 0) {
        int cnt = ret;

        if (cnt < 128) {
            /* if not a data packet ....} */
            continue;
        }

        if ((ry->pktno == 1) && (!ry->xmodem)) {
            /* YModem file metadata packet */
            if ((ret = ymodem_rcv_decode(ry->pkt.data, cnt)) > 0) {
                ry->fsize = ret;
                break;
            }
            if (ret == 0) {
                ry->state = YR_EOT;
            }
        } else {
            ry->count += cnt;
            ry->data_len = ret;
            break;
        }
    }

	usb_ymodem_rcv_flush(ry);

    return ret;
}




#if XFLASH_MAGIC
#define MAGIC_REC_MAX 16
/* pos = position in the file (usually 0) 
 *
 * */

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
const char s_err[] = "\r\nErr: ";
const char s_invalid[] = "\r\nInvalid file!";
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

const char err_code[] = { '0', '1', '2', '3', '4', '5', '6' };

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
		unsigned int cnt;
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
	ymodem_rcv_init(&ry);

	flash_unlock();

	do {
		PUTS(s_ymodem);
		offs = blk_offs;

		/* set the initial max file size */
		while ((ret = usb_ymodem_rcv_start(&ry, blk_size)) > 0) {
			int cnt;
			for (;;) {
				int len;

				if ((len = ry.data_len) > 0) {
					if ((ry.pktno == 2) || ((ry.pktno == 1) && (ry.xmodem))) {

#if XFLASH_MAGIC
						unsigned char * src = ry.pkt.data;
						if ((ret = magic_match((struct magic *)&magic_buf, 
											   offs, src, len)) < 0) {
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
						DCC_LOG2(LOG_TRACE, "flash_erase(offs=0x%06x, len=%d)", 
								 offs, len);
#endif 
						if ((ret = flash_erase(offs, len)) < 0) {
							DCC_LOG(LOG_WARNING, "flash_erase() failed!");
#if XFLASH_VERBOSE
							usb_ymodem_rcv_cancel(&ry);
							delay(1000);
							PUTS(s_erase);
#endif
							break;
						}
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
				}

				if ((ret = usb_ymodem_rcv_pkt(&ry)) <= 0) {
					break;
				}

				cnt = ret;
				if (cnt >= 128) {
					ry.data_len = ret;
				}
			}

			if (ret < 0) {
				break;
			}
		}
//		if (opt & XFLASH_OPT_RET_ERR) { 
//			return ret;
//		}
	} while ((ret < 0) || (offs == blk_offs));

	usb_ymodem_rcv_flush(&ry);

	PUTS(s_ok);

	usb_drain(CDC_TX_EP);

//	if (opt & XFLASH_OPT_RESET) { 
		delay(3000);
		reset();
//	}

//	return 0;
}


