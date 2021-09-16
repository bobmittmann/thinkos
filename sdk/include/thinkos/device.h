/* 
 * File:	 thinkos/device.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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

#ifndef __THINKOS_DEVICE_H__
#define __THINKOS_DEVICE_H__

#ifndef __THINKOS_DEVICE__
#error "Never use <thinkos/device.h> directly; include <thinkos/kernel.h> instead."
#endif 

struct thinkos_dev;

/* Device  descriptor */
struct thinkos_dev_desc {
	union {
		char tag[8];
		uint64_t hash;
	};
	struct thinkos_dev * dev;
};

struct thinkos_dev_map {
	char tag[7];
	uint8_t cnt; /* Number of entries in the list of device descriptors */
	const struct thinkos_dev_desc * desc[]; /* Sorted list of descriptors */
};

#ifdef __cplusplus
extern "C" {
#endif


int __thinkos_krn_dev_init(struct thinkos_rt * krn, 
                            const struct thinkos_dev_map * map);

bool __krn_dev_lookup(struct thinkos_rt * krn, const char * tag, 
					  struct thinkos_dev * dev);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_DEVICE_H__ */

