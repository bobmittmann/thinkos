
#include "xyzmodem-i.h"

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

/*
 * receive a data subpacket as dictated by the last received header.
 * return 2 with correct packet and end of frame
 * return 1 with correct packet frame continues
 * return 0 with incorrect frame.
 * return timeout with a timeout
 * if an acknowledgement is requested it is generated automatically
 * here. 
 */

/*
 * data subpacket reception
 */

int __zm_rcv_data32(struct zmodem* zm, unsigned char * p, 
					   unsigned maxlen, unsigned* l)
{
	int c;
	unsigned long rxd_crc;
	unsigned long crc;
	int subpkt_type;

	DBG("recv_data32");

	crc = 0xffffffffl;

	do {
		c = __zm_getc(zm);

		if (c == TIMEOUT) {
			return TIMEOUT;
		}
		if (c < 0x100 && *l < maxlen) {
			crc = CRC32(crc, c);
			*p++ = c;
			(*l)++;
			continue;
		}
	} while(c < 0x100);

	subpkt_type = c & 0xff;

	crc = CRC32(crc, subpkt_type);

	crc = ~crc;

	rxd_crc  = __zm_getc(zm);
	rxd_crc |= __zm_getc(zm) << 8;
	rxd_crc |= __zm_getc(zm) << 16;
	rxd_crc |= __zm_getc(zm) << 24;

	if(rxd_crc != crc) {
		WARN("crc32 error (%08lx, expected: %08lx) bytes=%u, subpkt-type=%s",
			 rxd_crc, crc, *l, __zm_ch(subpkt_type));
		return false;
	}
	DBG("good crc32: %08lx (bytes=%u, subpkt-type=%s)", crc, *l, 
		__zm_ch(subpkt_type));

	zm->ack_file_pos += *l;

	return subpkt_type;
}

int __zm_rcv_data16(struct zmodem* zm, register unsigned char* p, 
					   unsigned maxlen, unsigned* l)
{
	int c;
	int subpkt_type;
 	unsigned short crc;
	unsigned short rxd_crc;

	DBG("recv_data16");

	crc = 0;

	do {
		c = __zm_getc(zm);

		if (c == TIMEOUT) {
			return TIMEOUT;
		}
		if (c < 0x100 && *l < maxlen) {
			crc = CRC16CCITT(crc, c);
			*p++ = c;
			(*l)++;
		}
	} while (c < 0x100);

	subpkt_type = c & 0xff;

	crc = CRC16CCITT(crc, subpkt_type);

	rxd_crc  = __zm_getc(zm) << 8;
	rxd_crc |= __zm_getc(zm);

	if (rxd_crc != crc) {
		WARN("crc16 error (%04hx, expected: %04hx) bytes=%d", rxd_crc, 
			 crc, *l);
		return false;
	}
	DBG("good crc16: %04hx (bytes=%d)", crc, *l);

	zm->ack_file_pos += *l;

	return subpkt_type;
}


int __zm_rcv_data(struct zmodem* zm, unsigned char* p, 
					 size_t maxlen, unsigned* l, bool ack)
{
	int subpkt_type;
	unsigned n=0;

	if(l == NULL)
		l = &n;

	DBG("recv_data (%u-bit)", zm->receive_32bit_data ? 32:16);

	/*
	 * receive the right type of frame
	 */

	*l = 0;

	if (zm->receive_32bit_data) {
		subpkt_type = __zm_rcv_data32(zm, p, maxlen, l);
	} else {	
		subpkt_type = __zm_rcv_data16(zm, p, maxlen, l);
	}

	if ((subpkt_type == 0) || (subpkt_type == TIMEOUT))
		return subpkt_type;
	
	DBG("recv_data received subpacket-type: %s", __zm_ch(subpkt_type));

	switch (subpkt_type)  {
		/*
		 * frame continues non-stop
		 */
		case ZCRCG:
			return FRAMEOK;
		/*
		 * frame ends
		 */
		case ZCRCE:
			return ENDOFFRAME;
		/*
 		 * frame continues; zack expected
		 */
		case ZCRCQ:		
			if (ack)
				__zm_snd_ack(zm, zm->ack_file_pos);
			return FRAMEOK;
		/*
		 * frame ends; zack expected
		 */
		case ZCRCW:
			if (ack)
				__zm_snd_ack(zm, zm->ack_file_pos);
			return ENDOFFRAME;
	}

	WARN("invalid subpacket-type: %s", __zm_ch(subpkt_type));

	return 0;
}


bool __zm_rcv_subpacket(struct zmodem* zm, bool ack)
{
	int type;

	type = __zm_rcv_data(zm,zm->rx_data_subpacket,
						  sizeof(zm->rx_data_subpacket), NULL, ack);
	if(type!=FRAMEOK && type!=ENDOFFRAME) {
		__zm_snd_nak(zm);
		return false;
	}

	return true;
}

