
#include "xyzmodem-i.h"

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>


static int __zm_pack_data32(struct zmodem* zm, uint8_t subpkt_type, 
						   uint8_t * ptr, unsigned int len)
{
	uint8_t * buf = zm->pkt.raw;
	uint8_t trail[8];
	uint32_t crc;
	int n;

	DBG("__zm_pack_data32: %s (%u bytes)", __zm_ch(subpkt_type), len);

	crc = 0xffffffffl;
	crc = crc32(crc, ptr, len);
	crc = CRC32(crc, subpkt_type);
	crc = ~crc;

	trail[0] = (crc >> 0 ) & 0xff;
	trail[1] = (crc >> 8 ) & 0xff;
	trail[2] = (crc >> 16) & 0xff;
	trail[3] = (crc >> 24) & 0xff;

	n = __zm_zdlecpy(zm, buf, ptr, len);
	buf[n++] = ZDLE;
	buf[n++] = subpkt_type;
	n += __zm_zdlecpy(zm, &buf[n], trail, 4);

#if 0
	if (subpkt_type == ZCRCW)
		buf[n++]= XON;
#endif

	return n;
}

static int __zm_pack_data16(struct zmodem* zm, uint8_t subpkt_type,
							uint8_t * ptr, unsigned int len)
{
	uint8_t * buf = zm->pkt.raw;
	uint8_t trail[4];
	uint16_t crc;
	int n;

	crc = crc16ccitt(0, ptr, len);
	crc = CRC16CCITT(crc, subpkt_type);
	trail[0] = (crc >> 8) & 0xff;
	trail[1] = (crc >> 0) & 0xff;

	n = __zm_zdlecpy(zm, buf, ptr, len);
	buf[n++] = ZDLE;
	buf[n++] = subpkt_type;
	n += __zm_zdlecpy(zm, &buf[n], trail, 2);

	DBG("__zm_pack_data16: %s (%u bytes) CRC=%04x", 
		__zm_ch(subpkt_type), len, crc);

#if 1
	if (subpkt_type == ZCRCW)
		buf[n++]= XON;
#endif

	return n;
}

/*
 * send a data subpacket using crc 16 or crc 32 as desired by the receiver
 */

static int __zm_pack_data(struct zmodem* zm, uint8_t subpkt_type, 
						  void * ptr, unsigned int len)
{
	int ret;

	if (!zm->want_fcs_16 && zm->can_fcs_32)
		ret = __zm_pack_data32(zm, subpkt_type, ptr, len);
	else
		ret = __zm_pack_data16(zm, subpkt_type, ptr, len);

	return ret;
}

int __zm_snd_zfin(struct zmodem* zm)
{
	unsigned char zfin_header[] = { ZFIN, 0, 0, 0, 0 };

	return __zm_snd_hex_hdr(zm, zfin_header);
}

int zmodem_abort_receive(struct zmodem* zm)
{
	WARNS("aborting receive");
	return __zm_snd_pos_hdr(zm, ZABORT, 0, /* hex? */ true);
}

int __zm_snd_znak(struct zmodem* zm)
{
	return __zm_snd_pos_hdr(zm, ZNAK, zm->ack_file_pos, /* hex? */ true);
}

int __zm_snd_zskip(struct zmodem* zm)
{
	return __zm_snd_pos_hdr(zm, ZSKIP, 0l, /* hex? */ true);
}

int __zm_snd_zeof(struct zmodem* zm)
{
	return __zm_snd_pos_hdr(zm, ZEOF, zm->file_size, /* hex? */ true);
}

/*
 * receive routines for each of the six different styles of header.
 * each of these leaves zm->rxd.hdr_len set to 0 if the end ret is
 * not a valid header.
 */

bool __zm_rcv_bin16_hdr(struct zmodem* zm)
{
	int c;
	int n;
	unsigned short int crc;
	unsigned short int rxd_crc;

	DBGS("recv_bin16_header");

	crc = 0;

	for(n=0;n<HDRLEN;n++) {
		c = __zm_getc(zm);
		if(c == TIMEOUT) {
			WARNS("recv_bin16_header: timeout");
			return(false);
		}
		crc = __crc16ccitt(crc, c);
		zm->rxd.hdr[n] = c;
	}

	rxd_crc  = __zm_getc(zm) << 8;
	rxd_crc |= __zm_getc(zm);

	if(rxd_crc != crc) {
		WARN("crc16 error: 0x%hx, expected: 0x%04x", rxd_crc, crc);
		return(false);
	}
	DBG("CRC16 ok: 0x%04x", crc);

	zm->rxd.hdr_len = 5;

	return(true);
}

void __zm_rcv_hex_hdr(struct zmodem* zm)
{
	int c;
	int i;
	unsigned short int crc = 0;
	unsigned short int rxd_crc;

	for (i = 0; i < HDRLEN; i++) {
		c = __zm_rcv_hex(zm);
		if (c == TIMEOUT) {
			return;
		}
		crc = __crc16ccitt(crc, c);

		zm->rxd.hdr[i] = c;
	}

	/*
	 * receive the crc
	 */

	if ((c = __zm_rcv_hex(zm)) == TIMEOUT)
		return;

	rxd_crc = c << 8;

	if ((c = __zm_rcv_hex(zm)) == TIMEOUT)
		return;

	rxd_crc |= c;

	if (rxd_crc == crc) {
		YAP("CRC16 ok: 0x%04x", crc);
		zm->rxd.hdr_len = 5;
	} else {
		WARN("CRC16 error: 0x%04x, expected: 0x%04x", rxd_crc, crc);
	}

	/*
	 * drop the end of line sequence after a hex header
	 */
	c = __zm_getc(zm);
	if (c == '\r') {
		/*
		 * both are expected with cr
		 */
		__zm_getc(zm);	/* drop lf */
	}
}

bool __zm_rcv_bin32_hdr(struct zmodem* zm)
{
	int c;
	int n;
	unsigned long crc;
	unsigned long rxd_crc;

	DBGS("recv_bin32_header");

	crc = 0xffffffffl;

	for(n=0;n<HDRLEN;n++) {
		c = __zm_getc(zm);
		if(c == TIMEOUT) {
			return(true);
		}
		crc = CRC32(crc, c);
		zm->rxd.hdr[n] = c;
	}

	crc = ~crc;

	rxd_crc  = __zm_getc(zm);
	rxd_crc |= __zm_getc(zm) << 8;
	rxd_crc |= __zm_getc(zm) << 16;
	rxd_crc |= __zm_getc(zm) << 24;

	if (rxd_crc != crc) {
		WARN("crc32 error (%08lx, expected: %08lx)" ,rxd_crc, crc);
		return(false);
	}
	DBG("good crc32: %08lx", crc);

	zm->rxd.hdr_len = 5;
	return(true);
}

/*
 * receive any style header
 * if the errors flag is set than whenever an invalid header packet is
 * received invhdr will be returned. otherwise we wait for a good header
 * also; a flag (receive_32bit_data) will be set to indicate whether data
 * packets following this header will have 16 or 32 bit data attached.
 * variable headers are not implemented.
 */

int __zm_rcv_hdr_raw(struct zmodem* zm, int errors)
{
	int c;
	int	frame_type;

	zm->rxd.hdr_len = 0;

	do {
		do {
			if ((c = __zm_rcv_raw(zm)) < 0)
				return c;

			if (zm->cancelled)
				return ZCAN;
		} while (c != ZPAD);

		if ((c = __zm_rcv_raw(zm)) < 0)
			return c;

		if (c == ZPAD) {
			if ((c = __zm_rcv_raw(zm)) < 0)
				return c;
		}

		/*
		 * spurious zpad check
		 */

		if (c != ZDLE) {
			WARN("recv_header_raw: expected ZDLE, received: %s", __zm_ch(c));
			continue;
		}

		YAPS("__zm_rcv_hdr_raw: ZDLE");

		/*
		 * now read the header style
		 */

		c = __zm_getc(zm);

		if (c == TIMEOUT) {
			WARNS("__zm_rcv_hdr_raw: timeout!");
			return c;
		}

		switch (c) {
			case ZBIN:
				__zm_rcv_bin16_hdr(zm);
				zm->receive_32bit_data = false;
				break;
			case ZHEX:
				YAPS("__zm_rcv_hdr_raw: ZHEX");
				__zm_rcv_hex_hdr(zm);
				zm->receive_32bit_data = false;
				break;
			case ZBIN32:
				__zm_rcv_bin32_hdr(zm);
				zm->receive_32bit_data = true;
				break;
			default:
				/*
				 * unrecognized header style
				 */
				ERR("recv_header_raw: unrecognized header style: %s",
					__zm_ch(c));
				if (errors) {
					return INVHDR;
				}

				continue;
		}

		if (errors && zm->rxd.hdr_len == 0) {
			return INVHDR;
		}


	} while (zm->rxd.hdr_len == 0 && !zm->cancelled);

	if (zm->cancelled)
		return ZCAN;

	frame_type = zm->rxd.hdr[FTYPE];

	zm->rxd.hdr_pos = zm->rxd.hdr[ZP0] | (zm->rxd.hdr[ZP1] << 8) |
				(zm->rxd.hdr[ZP2] << 16) | (zm->rxd.hdr[ZP3] << 24);

	switch(frame_type) {
		case ZCRC:
			INFS("__zm_rcv_hdr_raw: ZCRC");
			zm->crc_request = zm->rxd.hdr_pos;
			break;
		case ZDATA:
			INFS("__zm_rcv_hdr_raw: ZDATA");
			zm->ack_file_pos = zm->rxd.hdr_pos;
			break;
		case ZFILE:
			zm->ack_file_pos = 0;
			ERRS("__zm_rcv_hdr_raw: ZFILE not implemented!");
#if 0
			if (!__zm_rcv_subpacket(zm,/* ack? */false))
				frame_type |= BADSUBPKT;
#endif
			break;
		case ZSINIT:
			ERRS("__zm_rcv_hdr_raw: ZSINIT not implemented!");
			break;

		case ZCOMMAND:
			ERRS("__zm_rcv_hdr_raw: ZCOMMAND not implemented!");
#if 0
			if (!__zm_rcv_subpacket(zm,/* ack? */true))
				frame_type |= BADSUBPKT;
#endif
			break;
		case ZFREECNT:
			ERRS("__zm_rcv_hdr_raw: ZFREECNT not implemented!");
#if 0
			__zm_snd_pos_hdr(zm, zack, 
							 getfreediskspace(".",1), /* hex? */ true);
#endif
			break;
		default:
			YAP("__zm_rcv_hdr_raw: frame_type=%s", __zm_frm(frame_type));
	}

	return frame_type;
}


int __zm_rcv_hdr(struct zmodem* zm)
{
	int ret;
	
	ret = __zm_rcv_hdr_raw(zm, 0);

	if (ret == TIMEOUT) {
		WARNS("__zm_rcv_hdr: timeout!");
	} else if(ret == INVHDR) {
		WARNS("__zm_rcv_hdr: invalid header!");
	} else if (ret == ZCAN) {
		WARNS("__zm_rcv_hdr: canceled!");
		/* FIXME: it may be reduntant */
		zm->cancelled = true;
	} else {
		DBG("__zm_rcv_hdr: %s (pos=%u)", __zm_frm(ret), zm->rxd.hdr_pos);
	}

	return ret;
}

#define int_to_bool(x) ((x) ? true : false)

void zmodem_parse_zrinit(struct zmodem* zm)
{
	zm->can_full_duplex = int_to_bool(zm->rxd.hdr[ZF0] & ZF0_CANFDX);
	zm->can_overlap_io	= int_to_bool(zm->rxd.hdr[ZF0] & ZF0_CANOVIO);
	zm->can_break		= int_to_bool(zm->rxd.hdr[ZF0] & ZF0_CANBRK);
	zm->can_fcs_32		= int_to_bool(zm->rxd.hdr[ZF0] & ZF0_CANFC32);
	zm->escape_ctrl_chars = int_to_bool(zm->rxd.hdr[ZF0] & ZF0_ESCCTL);
	zm->escape_8th_bit	= int_to_bool(zm->rxd.hdr[ZF0] & ZF0_ESC8);

	INF("ZRINIT (0x%02x):"
		"%s-duplex, %s overlap i/o, crc-%d, escape: %s %s",
		zm->rxd.hdr[ZF0],
		zm->can_full_duplex ? "full" : "half",
		zm->can_overlap_io ? "can" : "cannot",
		zm->can_fcs_32 ? 32 : 16,
		zm->escape_ctrl_chars ? "all" : "normal", 
		zm->escape_8th_bit ? "8th bit": ""	
		);

	if ((zm->recv_bufsize = (zm->rxd.hdr[ZP0] | zm->rxd.hdr[ZP1] << 8)) != 0)
		INF("recv_bufsize=%d", zm->recv_bufsize);
}

int zmodem_get_zfin(struct zmodem* zm)
{
	int type;

	__zm_snd_zfin(zm);
	do {
		type = __zm_rcv_hdr(zm);
	} while(type != ZFIN && type != TIMEOUT && zm->connected);
	
	/*
	 * these os are formally required; but they don't do a thing
	 * unfortunately many programs require them to exit 
	 * (both programs already sent a zfin so why bother ?)
	 */

	if(type != TIMEOUT) {
		__zm_putc(zm, 'o');
		__zm_putc(zm, 'o');
	}

	return 0;
}



static int __zm_snd_zrqinit(struct zmodem* zm)
{
	uint8_t zrqinit_hdr[] = { ZRQINIT, 0, 0, 0, 0 };

	return __zm_snd_hex_hdr(zm, zrqinit_hdr);
}

int zmodem_snd_start(struct zmodem * zm, const char * fname, 
					 unsigned int fsize)
{
	uint8_t zfile_frame[] = { ZFILE, 0, 0, 0, 0 };
	int raw_len;
	uint8_t * raw_buf;
	int retry;
	int type;
	int len;
	char * cp;
	char * opt;

	if (zm->block_size == 0)
		zm->block_size = ZBLOCKLEN;	

	if (zm->block_size < 128)
		zm->block_size = 128;	

	if (zm->block_size > Z_MAX_SUBPKT_LEN)
		zm->block_size = Z_MAX_SUBPKT_LEN;

	if (zm->max_block_size < zm->block_size)
		zm->max_block_size = zm->block_size;

	if (zm->max_block_size > Z_MAX_SUBPKT_LEN)
		zm->max_block_size = Z_MAX_SUBPKT_LEN;

	zm->file_skipped = false;
	zm->data_len = 0;

	for (zm->errors = 0; zm->errors <= zm->max_errors && 
		 !zm->cancelled && zm->connected; ++zm->errors) {

		INF("sending zrqinit (%u of %u)", zm->errors + 1, zm->max_errors + 1);

		__zm_snd_zrqinit(zm);

		type = __zm_rcv_hdr(zm);

		if (type == ZRINIT) {
			zmodem_parse_zrinit(zm);
			break;
		}

		WARN("send_file: received header type %s", __zm_frm(type));
	}

	if (zm->errors >= zm->max_errors || zm->cancelled)
		return -1;

	zm->file_size = fsize;
	zm->file_pos = 0;

	/*
	 * set conversion option
	 * (not used; always binary)
	 */

	zfile_frame[ZF0] = ZF0_ZCBIN;

	/*
	 * management option
	 */

	if (zm->management_protect) {
		zfile_frame[ZF1] = ZF1_ZMPROT;		
		DBGS("send_file: protecting destination");
	} else if (zm->management_clobber) {
		zfile_frame[ZF1] = ZF1_ZMCLOB;
		DBGS("send_file: overwriting destination");
	} else if (zm->management_newer) {
		zfile_frame[ZF1] = ZF1_ZMNEW;
		DBGS("send_file: overwriting destination if newer");
	} else
		zfile_frame[ZF1] = ZF1_ZMCRC;

	/*
	 * transport options
	 * (just plain normal transfer)
	 */

	zfile_frame[ZF2] = ZF2_ZTNOR;

	/*
	 * extended options
	 */

	zfile_frame[ZF3] = 0;

	/*
	 * now build the data subpacket with the file name and lots of other
	 * useful information.
	 */

	/*
	 * first enter the name and a 0
	 */

	cp = (char *)zm->pkt.data;

	strcpy(cp, fname);

	opt = cp + strlen(fname) + 1;
	cp = opt;

	cp += sprintf(cp, "%u %u %u %d %u %u %d"
			,zm->file_size
			,1549405971     /* time */
			,0				/* file mode */
			,0				/* serial number */
			,0              /* files remaining */
			,zm->file_size  /* bytes remaining */
			,0				/* file type */
		   );

	cp++;

	len = cp - (char *)zm->pkt.data;

	DBG("start: fname: '%s'", zm->pkt.data);
	DBG("start: options: '%s'", opt);
	DBG("start: %d", zm->file_size);

	raw_buf = zm->pkt.raw;
	raw_len = __zm_pack_data(zm, ZCRCW, zm->pkt.data, len);

		/*
		 * send the header and the data
		 */

		__zm_snd_bin_hdr(zm, zfile_frame);
		__zm_snd_raw(zm, raw_buf, raw_len);
	retry = 0;
	do {
		if (++retry >= zm->max_errors) {
			INFS("start: too many attempts!");
			return -1;
		}


		/*
		 * wait for anything but an zack packet
		 */

		do {
			type = __zm_rcv_hdr(zm);
			if (zm->cancelled)
				return -1;
			if (!zm->connected)
				return -1;
		} while (type == ZACK);

		INF("start: type='%s'", __zm_frm(type));

		if (type == ZSKIP) {
			zm->file_skipped = true;
			WARNS("file skipped by receiver");
			return -1;
		}

		if (type == ZCRC) {
			ERRS("ZCRC not implemented for stream...");

			if (zm->crc_request == 0)
				INFS("receiver requested crc of entire file");
			else
				INF("receiver requested crc of first %d bytes", 
					zm->crc_request);

			/* Fake CRC */
			__zm_snd_pos_hdr(zm, ZCRC, 0x1111, true);

			type = __zm_rcv_hdr(zm);

		}

		/* FIXME: ??? */
		if (type == ZRINIT) {
			zmodem_parse_zrinit(zm);
		}
	
	} while (type != ZRPOS);

	if (zm->rxd.hdr_pos && zm->rxd.hdr_pos <= zm->file_size) {
		zm->file_pos = zm->rxd.hdr_pos;
		INF("starting transfer at offset: %u (resume)", zm->file_pos);
	}

	return 0;
}

static int __zm_snd_blk(struct zmodem* zm, uint8_t * buf, unsigned long len)
{
	unsigned int raw_len;
	uint8_t * raw_buf;
	unsigned long pos;
	uint8_t type;
			  

	pos = zm->file_pos;

	type = ZCRCG;

#if 0
	if (!zm->can_overlap_io || zm->no_streaming) {
		WARNS("__zm_snd_blk: ZCRCW");
		type = ZCRCW;
		return -1;
	}

	if (zm->file_pos >= zm->file_size || len == 0)	 {
		WARNS("__zm_snd_blk: ZCRCE");
		type = ZCRCE;
		return -1;
	}
#endif

	raw_buf = zm->pkt.raw;
	raw_len = __zm_pack_data(zm, type, buf, len);

	__zm_snd_pos_hdr(zm, ZDATA, pos, /* hex? */ false);

	if (__zm_snd_raw(zm, raw_buf, raw_len) < 0) {
		ERRS("__zm_snd_blk: TIMEOUT");
		return TIMEOUT;
	}

	zm->file_pos = pos + len;

	INFS("__zm_snd_blk: data sent...");

	for (;;) {
		int type;
		int c;
		
		//c = __zm_rcv_raw_tmo(zm, zm->consecutive_errors ? 1000 : 0);
		c = __zm_rcv_raw_tmo(zm, 0);

		if (c == TIMEOUT)
			break;

		YAPS("back-channel traffic detected:");

		if (c == ZPAD) {
			type = __zm_rcv_hdr(zm);
			if (type != TIMEOUT && type != ZACK) {
				return type;
			}
		} else {
			YAP("received: %s",__zm_ch(c));
		}
	}

	if (zm->cancelled)
		return ZCAN;

	zm->consecutive_errors = 0;

	return ZACK;
}

int zmodem_snd_loop(struct zmodem* zm, const void * data, unsigned int len)
{
	uint8_t * src = (uint8_t *)data;
	unsigned int cnt;
	int type;

	zm->errors = 0;
	zm->consecutive_errors = 0;

	YAP("loop: %u", len);

	if (src == NULL)
		return -EINVAL;

	cnt = 0;

	do {
		uint8_t * dst;
		unsigned int rem;
		int n;
		int i;

		dst = &zm->pkt.data[zm->data_len];
		rem = zm->max_block_size - zm->data_len;
		n = MIN(len, rem);

		for (i = 0; i < n; ++i)
			dst[i] = src[i];

		zm->data_len += n;

		if (zm->data_len == zm->max_block_size) {

			do {
				/*
				 * and start sending
				 */

				type = __zm_snd_blk(zm, zm->pkt.data, zm->data_len);

				if (!zm->connected)
					return -1;

				if (type == ZFERR || type == ZABORT || zm->cancelled)
					return -1;

				if (type == ZACK)	/* success */
					break;

				ERR("%s at offset: %u", __zm_ch(type), zm->file_pos);

#if 0
				if (zm->block_size == zm->max_block_size && 
					zm->max_block_size > ZBLOCKLEN)
					zm->max_block_size /= 2;

				if (zm->block_size > 128)
					zm->block_size /= 2; 

#endif
				zm->errors++;
				if (++zm->consecutive_errors > zm->max_errors)
					return -1;

				/*
				 * fetch pos from the ZRPOS header
				 */
				if (type == ZRPOS) {
					if (zm->rxd.hdr_pos <= zm->file_size) {
						if (zm->file_pos != zm->rxd.hdr_pos) {
							zm->file_pos = zm->rxd.hdr_pos;
							INF("Resuming transfer from offset: %u", 
								zm->file_pos);
						}
					} else
						WARN("Invalid ZRPOS offset: %u", zm->rxd.hdr_pos);
				}

			} while (type == ZRPOS || type == ZNAK || type == TIMEOUT);

			YAP("Finishing transfer: %s", __zm_ch(type));

			zm->data_len = 0;
		}

		src += n;
		cnt += n;
	} while (cnt < len);

	return cnt;
}

int zmodem_snd_eof(struct zmodem* zm)
{
	int ret = -1;
	
	if (zm->type == ZACK) {
		int attempts;

		/*
		 * file sent. send end of file frame
		 * and wait for zrinit. if it doesnt come then try again
		 */

		for (attempts = 0; attempts <= zm->max_errors && 
			 !zm->cancelled && zm->connected; ++attempts) {

			INF("Sending End-of-File (ZEOF) frame (%u of %u)",
				attempts + 1, zm->max_errors + 1);

			__zm_snd_zeof(zm);

			if (__zm_rcv_hdr(zm) == ZRINIT) {
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int zmodem_snd_rz(struct zmodem * zm)
{
	uint8_t buf[8];

	buf[0] = 'r';
	buf[1] = 'z';
	buf[2] = '\r';
	buf[3] = '\n';

	return __zm_snd_raw(zm, buf, 4);
}

int zmodem_snd_init(struct zmodem * zm, const struct comm_dev * comm, 
					unsigned int mode)
{
	if ((zm == NULL) || (comm == NULL))
		return -EINVAL;

	zm->comm = comm;

	zm->block_size = Z_MAX_SUBPKT_LEN;
	zm->max_block_size = Z_MAX_SUBPKT_LEN;
	zm->data_len = 0;
	zm->max_errors = 10;
	zm->file_skipped = false;
	zm->cancelled = false;
	zm->connected = true;
	zm->recv_timeout = 3; /* USB */
	zm->recv_bufsize = 0;
	zm->n_cans = 0;
	zm->management_protect = false;
	zm->management_clobber = true;
	zm->management_newer = false;
	zm->no_streaming = false;

	__zm_flush(zm);

	return 0;

}

