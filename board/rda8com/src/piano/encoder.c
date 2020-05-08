/* 
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the usb-serial converter.
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

/** 
 * @file encoder.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#include "encoder.h"
#include <stdio.h>

void encoder_init(struct encoder * enc, uint32_t val,
				  uint32_t min, uint32_t max)
{
	enc->code = 0;
	enc->min = min;
	enc->max = max;

	if (max < min)
		max = min;

	if (val > max)
		val = max;

	if (val < min)
		val = min;

	enc->val = val;
}

uint32_t bit2hex(uint32_t code)
{
	uint32_t ret;


	ret =  (code <<  (0 - 0)) & (1 << 0);
	ret += (code <<  (4 - 1)) & (1 << 4);
	ret += (code <<  (8 - 2)) & (1 << 8);
	ret += (code << (12 - 3)) & (1 << 12);
	ret += (code << (16 - 4)) & (1 << 16);
	ret += (code << (20 - 5)) & (1 << 20);
	ret += (code << (24 - 6)) & (1 << 24);
	ret += (code << (28 - 7)) & (1 << 28);

	return ret;
}

uint32_t encoder_decode(struct encoder * enc, uint32_t code)
{
	uint32_t prev = enc->code;

	code = code & 3;

	switch (prev & 0x0f) {
	case 0x0c:
		if (enc->val > enc->min) {
			enc->val--;
		}
		break;
	case 0x03:
		if (enc->val < enc->max) {
				enc->val++;
		}
		break;
	}

	enc->code = (enc->code << 2) | code;
	//printf("code: %08x --> %08x diff %02x\n", bit2hex(prev), 
	//	   bit2hex(enc->code), bit2hex(diff));

	return enc->val;
}

uint32_t encoder_val(struct encoder * enc)
{
	return enc->val;
}


