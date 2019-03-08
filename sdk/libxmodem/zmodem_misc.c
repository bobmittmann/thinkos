
#include "xyzmodem-i.h"

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

char * __zm_ch(int ch)
{
	static char str[16];
	char * f;

	switch(ch) {
		case TIMEOUT:	
			f = "TIMEOUT";
			/* FALLTHROUGH */

		case ZRQINIT:
			f = "ZRQINIT";
			break;
		case ZRINIT:
			f = "ZRINIT";
			break;
		case ZSINIT:
			f = "ZSINIT";
			break;
		case ZACK:
			f = "ZACK";
			break;
		case ZFILE:
			f = "ZFILE";
			break;
		case ZSKIP:
			f = "ZSKIP";
			break;
		case ZCRC:
			f = "ZCRC";
			break;
		case ZNAK:
			f = "ZNAK";
			break;
		case ZABORT:
			f = "ZABORT";
			break;
		case ZFIN:
			f = "ZFIN";
			break;
		case ZRPOS:
			f = "ZRPOS";
			break;
		case ZDATA:
			f = "ZDATA";
			break;
		case ZEOF:
			f = "ZEOF";
			break;
		case ZPAD:
			f = "ZPAD";
			break;
		case ZCAN:
			f = "ZCAN";
			break;
		case ZDLE:
			f = "ZDLE";
			break;
		case ZDLEE:
			f = "ZDLEE";
			break;
		case ZBIN:
			f = "ZBIN";
			break;
		case ZHEX:
			f = "ZHEX";
			break;
		case ZBIN32:
			f = "ZBIN32";
			break;
		case ZRESC:
			f = "ZRESC";
			break;
		case ZCRCE:
			f = "ZCRCE";
			break;
		case ZCRCG:
			f = "ZCRCG";
			break;
		case ZCRCQ:
			f = "ZCRCQ";
			break;
		case ZCRCW:
			f = "ZCRCW";
			break;
		default:
			if (ch >= ' ' && ch <= '~')
				sprintf(str, "'%c' (%02xh)",(char)ch,(char)ch);
			else
				sprintf(str, "%u (%02xh)",(char)ch,(char)ch);
			f = str;
	}

	return f; 
}

char * __zm_frm(int frm)
{
	static char str[25];
	char * f;

	if (frm == TIMEOUT)
		return "TIMEOUT";

	if (frm == INVHDR)
		return "Invalid Header";

	if (frm & BADSUBPKT)
		strcpy(str, "BAD ");
	else
		str[0] = '\0';

	switch (frm & ~BADSUBPKT) {
		case ZRQINIT:
			f = "ZRQINIT";
			break;
		case ZRINIT:
			f = "ZRINIT";
			break;
		case ZSINIT:
			f = "ZSINIT";
			break;
		case ZACK:
			f = "ZACK";
			break;
		case ZFILE:
			f = "ZFILE";
			break;
		case ZSKIP:
			f = "ZSKIP";
			break;
		case ZNAK:
			f = "ZNAK";
			break;
		case ZABORT:
			f = "ZABORT";
			break;
		case ZFIN:
			f = "ZFIN";
			break;
		case ZRPOS:
			f = "ZRPOS";
			break;
		case ZDATA:
			f = "ZDATA";
			break;
		case ZEOF:
			f = "ZEOF";
			break;
		case ZFERR:
			f = "ZFERR";
			break;
		case ZCRC:
			f = "ZCRC";
			break;
		case ZCHALLENGE:
			f = "ZCHALLENGE";
			break;
		case ZCOMPL:
			f = "ZCOMPL";
			break;
		case ZCAN:
			f = "ZCAN";
			break;
		case ZFREECNT:
			f = "ZFREECNT";
			break;
		case ZCOMMAND:
			f = "ZCOMMAND";
			break;	
		case ZSTDERR:
			f = "ZSTDERR";
			break;		
		default: 
			sprintf(str,"Unknown (%08X)", frm);
			f = str;
			break;
	}

	strcat(str, f);

	return str; 
}

int __zm_zdlecpy(struct zmodem* zm, void * dst, 
						  const void * src, unsigned int len)
{
	uint8_t * buf = (uint8_t *)src;
	uint8_t * cp = (uint8_t *)dst;
	unsigned int cnt = 0;
	unsigned int i;

	for (i = 0; i < len; ++i) {
		int c;

		c = buf[i];

		switch (c) {
		case DLE:
		case DLE | 0x80:          /* even if high-bit set */
		case XON:
		case XON | 0x80:
		case XOFF:
		case XOFF | 0x80:
		case ZDLE:
			*cp++ = ZDLE;
			*cp++ = ZDLE | (uint8_t)(c ^ 0x40);
			break;
		case CR:
		case CR | 0x80:
			if (zm->escape_ctrl_chars && (zm->last_sent & 0x7f) == '@') {
				*cp++ = ZDLE;
				*cp++ = ZDLE | (uint8_t)(c ^ 0x40);
			} else
				*cp++ = c;
			break;
			/*		case TELNET_IAC:
					if (zm->escape_telnet_iac) {
					int ret;
					if ((ret=__zm_snd_raw(zm, ZDLE)) != 0)
					return ret;
					return __zm_snd_raw(zm, ZRUB1);
					}
					break; */
		default:
			if (zm->escape_ctrl_chars && (c & 0x60) == 0) {
				*cp++ = ZDLE;
				*cp++ = ZDLE | (uint8_t)(c ^ 0x40);
			} else
				*cp++ = c;
		}
	}

	cnt = cp - (uint8_t *)dst;

	return cnt; 
}


void __zm_flush(struct zmodem* zm)
{
	uint8_t buf[4];

	while (zm->comm->op.recv(zm->comm->arg, buf, 4, 100) > 0);
}

/*
 * rx_raw ; receive a single byte from the line.
 * reads as many are available and then processes them one at a time
 * check the data stream for 5 consecutive can characters;
 * and if you see them abort. this saves a lot of clutter in
 * the rest of the code; even though it is a very strange place
 * for an exit. (but that was wat session abort was all about.)
 */

int __zm_rcv_raw(struct zmodem* zm)
{
	uint8_t buf[4];
	int c;

	if (zm->comm->op.recv(zm->comm->arg, buf, 1, zm->recv_timeout) < 0) {
		WARN("__zm_rcv_raw: timeout");
		return TIMEOUT;
	}
	
	c = buf[0];

	if (c == CAN) {
		zm->n_cans++;
		if (zm->n_cans == 5) {
			zm->cancelled = true;
			WARN("__zm_rcv_raw: cancelled remotely");
		}
	} else {
		zm->n_cans = 0;
	}

	return c;
}

int __zm_rcv_raw_tmo(struct zmodem* zm, unsigned int tmo)
{
	uint8_t buf[4];
	int c;

	if (zm->comm->op.recv(zm->comm->arg, buf, 1, tmo) < 0) {
		YAPS("__zm_rcv_raw: timeout");
		return TIMEOUT;
	}
	
	c = buf[0];

	if (c == CAN) {
		zm->n_cans++;
		if (zm->n_cans == 5) {
			zm->cancelled = true;
			WARN("__zm_rcv_raw: cancelled remotely");
		}
	} else {
		zm->n_cans = 0;
	}

	return c;
}


/*
 * rx; receive a single byte undoing any escaping at the
 * sending site. this bit looks like a mess. sorry for that
 * but there seems to be no other way without incurring a lot
 * of overhead. at least like this the path for a normal character
 * is relatively short.
 */

int __zm_getc(struct zmodem* zm)
{
	int c;

	/*
	 * outer loop for ever so for sure something valid
	 * will come in; a timeout will occur or a session abort
	 * will be received.
	 */

	while (zm->connected) {
		for (;;) {

			if ((c = __zm_rcv_raw(zm)) < 0)
				return c;
	
			switch (c) {
				case ZDLE:
					break;
				case XON:
				case XON | 0x80:
				case XOFF:
				case XOFF | 0x80:
					continue;			
				default:
					/*
	 				 * if all control characters should be escaped and 
					 * this one wasnt then its spurious and should be dropped.
					 */
					if (zm->escape_ctrl_chars && (c & 0x60) == 0) {
						continue;
					}
					/*
					 * normal character; return it.
					 */
					return c;
			}
			break;
		}
	
		/*
	 	 * ZDLE encoded sequence or session abort.
		 * (or something illegal; then back to the top)
		 */

		for (;;) {
			if ((c = __zm_rcv_raw(zm)) < 0)
				return c;

			if (c == XON || c == (XON|0x80) || 
			   c == XOFF || c == (XOFF|0x80) || c == ZDLE) {
				/*
				 * these can be dropped.
				 */
				continue;
			}

			switch (c) {
				/*
				 * these four are really nasty.
				 * for convenience we just change them into 
				 * special characters by setting a bit outside the
				 * first 8. that way they can be recognized and still
				 * be processed as characters by the rest of the code.
				 */
				case ZCRCE:
				case ZCRCG:
				case ZCRCQ:
				case ZCRCW:
					DBG("rx: encoding data subpacket type: %s", __zm_ch(c));
					return (c | ZDLEESC);
				case ZRUB0:
					return 0x7f;
				case ZRUB1:
					return 0xff;
				default:
					if (zm->escape_ctrl_chars && (c & 0x60) == 0) {
						/*
						 * a not escaped control character; probably
						 * something from a network. just drop it.
						 */
						continue;
					}
					/*
					 * legitimate escape sequence.
					 * rebuild the orignal and return it.
					 */
					if ((c & 0x60) == 0x40) {
						return c ^ 0x40;
					}

					WARN("rx: illegal sequence: ZDLE %s", __zm_ch(c));
					break;
			}
			break;
		} 
	}

	/*
	 * not reached.
	 */

	return 0;
}

static int __zm_rcv_nibble(struct zmodem* zm) 
{
	int c;

	c = __zm_getc(zm);

	if (c == TIMEOUT) {
		return c;
	}

	if (c > '9') {
		if(c < 'a' || c > 'f') {
			/*
			 * illegal hex; different than expected.
			 * we might as well time out.
			 */
			return TIMEOUT;
		}

		c -= 'a' - 10;
	} else {
		if(c < '0') {
			/*
			 * illegal hex; different than expected.
			 * we might as well time out.
			 */
			return TIMEOUT;
		}
		c -= '0';
	}

	return c;
}

int __zm_rcv_hex(struct zmodem* zm)
{
	int n1;
	int n0;
	int ret;

	n1 = __zm_rcv_nibble(zm);

	if (n1 == TIMEOUT) {
		return n1;
	}

	n0 = __zm_rcv_nibble(zm);

	if (n0 == TIMEOUT) {
		return n0;
	}

	ret = (n1 << 4) | n0;

	YAP("__zm_rcv_hex: 0x%02x", ret);

	return ret;
}

bool __zm_data_waiting(struct zmodem* zm, unsigned int timeout)
{
	uint8_t buf[4];

	if (zm->comm->op.recv(zm->comm->arg, buf, 0, timeout) < 0) {
		DBGS("__zm_data_waiting: no data.");
		return false;
	}

	DBGS("__zm_data_waiting: some data :) .");
	return true;
}

/*
 * read bytes as long as rdchk indicates that
 * more data is available.
 */

/* 
 * transmit a character. 
 */

int __zm_putc(struct zmodem* zm, unsigned char ch)
{
	uint8_t buf[2];
	int	ret;

	buf[0] = ch;

	if ((ret = zm->comm->op.send(zm->comm->arg, buf, 1)) < 0) {
		ERR("send_raw SEND ERROR: %d", ret);
		return ret;
	}

	zm->last_sent = ch;

	return ret;
}

int __zm_snd_raw(struct zmodem* zm, const void * buf, unsigned int len)
{
	uint8_t * cp = (uint8_t *)buf;
	int	ret;

	if ((ret = zm->comm->op.send(zm->comm->arg, cp, len)) < 0) {
		INFS("TX: console_write() failed!..");
	} else {
		zm->last_sent = cp[len - 1];
		YAPX("RAW", cp, len);
	}


	return ret;
}

extern const char __hextab[];

#if 0
static int __zm_snd_hex(struct zmodem* zm, uint8_t val)
{
	uint8_t buf[4];

	buf[0] = __hextab[val >> 4];
	buf[1] = __hextab[val & 0xf];

	return __zm_snd_raw(zm, buf, 2);
}

static int __zm_snd_padded_zdle(struct zmodem* zm)
{
	uint8_t buf[4];

	buf[0] = ZPAD;
	buf[1] = ZPAD;
	buf[2] = ZDLE;

	return __zm_snd_raw(zm, buf, 3);
}

static int __zm_snd_preamble(struct zmodem* zm, uint8_t c)
{
	uint8_t buf[4];

	buf[0] = ZPAD;
	buf[1] = ZPAD;
	buf[2] = ZDLE;
	buf[3] = c;

	return __zm_snd_raw(zm, buf, 4);
}
#endif



int __zm_snd_hex_hdr(struct zmodem* zm, uint8_t hdr[])
{
	uint8_t buf[ZMAXHLEN * 2];
	uint8_t type;
	uint16_t crc;
	uint8_t * cp;
	int n;
	int i;

	type = hdr[0];
	cp = buf;

	crc = crc16ccitt(0, hdr, HDRLEN);

	*cp++ = ZPAD;
	*cp++ = ZPAD;
	*cp++ = ZDLE;
	*cp++ = ZHEX;

	for (i = 0; i < HDRLEN; i++) {
		*cp++ = __hextab[(hdr[i] >> 4) & 0x0f];
		*cp++ = __hextab[hdr[i] & 0xf];
	}

	*cp++ = __hextab[(crc >> 12) & 0x0f];
	*cp++ = __hextab[(crc >> 8) & 0x0f];
	*cp++ = __hextab[(crc >> 4) & 0x0f];
	*cp++ = __hextab[(crc >> 0) & 0x0f];

	/*
	 * end of line sequence
	 */

	*cp++ = '\r';
	*cp++ = '\n'; /* fdsz sends 0x8a instead of 0x0a */

	if (type != ZACK && type != ZFIN)
		*cp++ = XON;

	n = cp - buf;

	return __zm_snd_raw(zm, buf, n);
}

static int __zm_snd_bin32_hdr(struct zmodem* zm, uint8_t hdr[])
{
	uint8_t buf[ZMAXHLEN * 2];
	uint32_t crc;
	uint8_t * cp;
	int n;
	int i;

	INFS("__zm_snd_bin32_hdr: ...");

	cp = buf;

	crc = 0xffffffffl;
	crc = crc32(crc, hdr, HDRLEN);
	crc = ~crc;

	*cp++ = ZPAD;
	*cp++ = ZPAD;
	*cp++ = ZDLE;
	*cp++ = ZBIN32;

	for (i = 0; i < HDRLEN; i++) {
		*cp++ = hdr[i];
	}

	*cp++ = (crc >> 24) & 0xff;
	*cp++ = (crc >> 16) & 0xff;
	*cp++ = (crc >> 8) & 0xff;
	*cp++ = (crc >> 0) & 0xff;

	n = cp - buf;

	return __zm_snd_raw(zm, buf, n);
}

static int __zm_snd_bin16_hdr(struct zmodem* zm, uint8_t hdr[])
{
	uint8_t buf[ZMAXHLEN * 2];
	uint8_t trail[4];
	uint16_t crc;
	uint8_t * cp;
	int n;

	YAP("__zm_snd_bin16_hdr: HDRLEN=%d", HDRLEN);

	cp = buf;
	crc = crc16ccitt(0, hdr, HDRLEN);
	trail[0] = (crc >> 8) & 0xff;
	trail[1] = (crc >> 0) & 0xff;

	*cp++ = ZPAD;
	*cp++ = ZPAD;
	*cp++ = ZDLE;
	*cp++ = ZBIN;

	cp += __zm_zdlecpy(zm, cp, hdr, HDRLEN);
	cp += __zm_zdlecpy(zm, cp, trail, 2);

	n = cp - buf;

	YAP("__zm_snd_bin16_hdr: n=%u", n);

	return __zm_snd_raw(zm, buf, n);
}

int __zm_snd_pos_hdr(struct zmodem* zm, int type, uint32_t pos, bool hex) 
{
	uint8_t hdr[5];
	int ret;

	hdr[0]   = type;
	hdr[ZP0] = (uint8_t) (pos        & 0xff);
	hdr[ZP1] = (uint8_t)((pos >>  8) & 0xff);
	hdr[ZP2] = (uint8_t)((pos >> 16) & 0xff);
	hdr[ZP3] = (uint8_t)((pos >> 24) & 0xff);

	if (hex)
		ret = __zm_snd_hex_hdr(zm, hdr);
	else if (zm->can_fcs_32 && !zm->want_fcs_16)
		ret = __zm_snd_bin32_hdr(zm, hdr);
	else
		ret = __zm_snd_bin16_hdr(zm, hdr);

	return ret;
}

int __zm_snd_ack(struct zmodem* zm, long pos)
{
	return __zm_snd_pos_hdr(zm, ZACK, pos, /* hex? */ true);
}

int __zm_snd_nak(struct zmodem* zm)
{
	return __zm_snd_pos_hdr(zm, ZNAK, 0, /* hex? */ true);
}

int __zm_snd_bin_hdr(struct zmodem* zm, uint8_t hdr[])
{
	int ret;

	if (zm->can_fcs_32 && !zm->want_fcs_16)
		ret = __zm_snd_bin32_hdr(zm, hdr);
	else
		ret = __zm_snd_bin16_hdr(zm, hdr);

	return ret;
}

#if 0

int ___zm_snd_blk(struct zmodem* zm, uint8_t * buf, 
				 unsigned long len, unsigned int * sent)
{
	unsigned buf_sent = 0;
	unsigned long pos;
	uint8_t type;
			  
	if (sent != NULL)
		*sent = 0;

	pos = zm->file_pos;
	__zm_snd_pos_hdr(zm, ZDATA, pos, /* hex? */ false);

	while (zm->connected) {
		unsigned int raw_len;
		uint8_t * raw_buf;
		unsigned int n;

		n = MIN(zm->block_size, len);
		DBG("__zm_snd_blk: n=%d", n);
		zm->file_pos += n;

		type = ZCRCG;

		/** zmodem.doc:
			zcrcw data subpackets expect a response before the next frame is sent.
			if the receiver does not indicate overlapped i/o capability with the
			canovio bit, or sets a buffer size, the sender uses the 
			zcrcw to allow
			the receiver to write its buffer before sending more data.
		***/
		if (!zm->can_overlap_io || zm->no_streaming 
			|| (zm->recv_bufsize && ((buf_sent + n) >= zm->recv_bufsize))) {
			WARNS("__zm_snd_blk: ZCRCW");
			type = ZCRCW;
			buf_sent = 0;
		}

		if (zm->file_pos >= zm->file_size || n==0)	 {
			WARNS("__zm_snd_blk: ZCRCE");
			type = ZCRCE;
		}

		raw_buf = zm->pkt.raw;
		raw_len = __zm_pack_data(zm, type, buf, len);

		if (__zm_snd_raw(zm, raw_buf, raw_len) < 0)
			return TIMEOUT;

		if (type == ZCRCW || type == ZCRCE) {	
			int ack;
			DBG("sent end-of-frame (%s sub-packet)", __zm_ch(type));

			if (type == ZCRCW) {	/* zack expected */

				DBG("waiting for zack");
				while (zm->connected) {
					if ((ack = __zm_rcv_hdr(zm)) != ZACK)
						return ack;

					if (zm->cancelled)
						return ZCAN;

					if (zm->rxd.hdr_pos == zm->file_pos)
						break;

					WARN("zack for incorrect offset (%u vs %u)",
						 zm->rxd.hdr_pos, zm->file_pos);
				} 

			}
		}

		if (sent!=NULL)
			*sent += n;

		buf_sent += n;

		if (zm->file_pos >= zm->file_size || n == 0)	 {
			WARNS("__zm_snd_blk: ZCRCE");
			type = ZCRCE;
		}

		if (n == 0) {
			ERR("send_from: read error");
			return ZACK;
		}

		/* 
		 * characters from the other side
		 * check out that header
		 */

		while (__zm_data_waiting(zm, zm->consecutive_errors ? 1000:0) 
			&& !zm->cancelled && zm->connected) {
			int type;
			int c;
			DBG("back-channel traffic detected:");

			if ((c = __zm_rcv_raw(zm)) < 0)
				return(c);

			if (c == ZPAD) {
				type = __zm_rcv_hdr(zm);
				if(type != TIMEOUT && type != ZACK) {
					return type;
				}
			} else
				DBG("received: %s",__zm_ch(c));
		}

		if (zm->cancelled)
			return ZCAN;

		zm->consecutive_errors = 0;

		if(zm->block_size < zm->max_block_size) {
			zm->block_size*=2;
			if(zm->block_size > zm->max_block_size)
				zm->block_size = zm->max_block_size;
		}

		if (type == ZCRCW || type == ZCRCE)	/* end-of-frame */
			__zm_snd_pos_hdr(zm, ZDATA, zm->file_pos, /* hex? */ false);
	}

	ERR("send_from: returning unexpectedly!");

	/*
	 * end of file reached.
	 * should receive something... so fake zack
	 */

	return ZACK;
}

#endif

