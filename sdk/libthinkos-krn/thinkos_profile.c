/* 
 * thinkos_profile.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>

/* -------------------------------------------------------------------------- 
 * ThinkOS profile block
 * --------------------------------------------------------------------------*/

const struct thinkos_profile thinkos_profile = {
	.header = {
		.size = sizeof(struct thinkos_profile),
		.version = THINKOS_PROFILE_VERSION,
		.reserved = 0
	},

	.kernel = {
		.version = {
			.major = THINKOS_KERNEL_VERSION_MAJOR,
			.minor = THINKOS_KERNEL_VERSION_MINOR,
			.reserved = 0,
		}
	},


};

