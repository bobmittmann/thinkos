/* 
 * File:	 stdio.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
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


#include <thinkos.h>
#include <sys/file.h>
#include <sys/ice-comm.h>
#include <arch/cortex-m3.h>
#include <sys/dcclog.h>

static const struct dcc_trace_entry __attribute__((section(".dcclog"))) 
comm_log_entry = { 
	.file = "COMM", 
	.line = 0,
	.level = LOG_COMM,
	.opt = LOG_OPT_STR, 
	.function = "",
	.msg = "'%s'"
};

int comm_write(void * dev, const void * buf, unsigned int len) 
{
	const char * str = (const char *)buf;
	struct ice_comm_blk * comm = ICE_COMM_BLK;
	unsigned int head;
	int fm = cm3_faultmask_get(); /* save fault mask */
	uint32_t val;
	int i;
	
	cm3_cpsid_f(); /* disable interrupts and faults */
	if (comm->dbg != DBG_CONNECTED) {
		if (comm->dbg == DBG_SYNC)
			comm->dev = DEV_CONNECTED;
		goto ret;
	}
	head = comm->tx_head;
	while ((16 - ((head - comm->tx_tail) & 0xffff)) < 1);
	comm->tx_buf.u32[head++ & 0xf] = (int)&comm_log_entry;
	i = 0;
	do {
		val = 0;
		if (i < len) {
			val = str[i++];
			if (i < len) {
				val |= str[i++] << 8;
				if (i < len) {
					val |= str[i++] << 16;
					if (i < len) {
						val |= str[i++] << 24;
					}
				}
			}
		}
		while ((16 - ((head - comm->tx_tail) & 0xffff)) < 1);
		comm->tx_buf.u32[head++ & 0xf] = val;
		comm->tx_head = head;
	} while (i < len);	
	if ((i & 0x3) == 0) {
		while ((16 - ((head - comm->tx_tail) & 0xffff)) < 1);
		comm->tx_buf.u32[head++ & 0xf] = 0;
		comm->tx_head = head;
	}
ret:
	cm3_faultmask_set(fm);  /* restore fault mask */

	return len;
}

int comm_read(void * dev, void * buf, unsigned int len, unsigned int msec) 
{
	return 0;
}

int comm_drain(void * dev)
{
	return 0;
}

int comm_close(void * dev)
{
	return 0;
}

const struct fileop comm_fops = {
	.write = (void *)comm_write,
	.read = (void *)comm_read,
	.flush = (void *)comm_drain,
	.close = (void *)comm_close
};

const struct file comm_file = {
	.data = 0, 
	.op = &comm_fops 
};

struct file * ice_comm_fopen(void)
{
	struct file * f;
	f = (struct file *)&comm_file;
	return f;
}

