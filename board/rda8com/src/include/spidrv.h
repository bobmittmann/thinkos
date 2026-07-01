/* 
 * File:	 spidrv.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2020 Bob Mittmann. All Rights Reserved.
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

#ifndef __SPIDRV_H__
#define __SPIDRV_H__

#ifdef __cplusplus
extern "C" {
#endif

void spidrv_master_init(unsigned int freq);

void spidrv_wr(int dat);

int spidrv_rd(void);

int spidrv_xfr16(int data);

uint32_t spidrv_xfer(uint32_t data);

#ifdef __cplusplus
}
#endif
#endif /* __SPIDRV_H__ */

