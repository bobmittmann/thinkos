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

uint32_t encoder_decode(struct encoder * enc, uint32_t code)
{
	code = code & 3;

	if ((enc->code & 3) == 3) {
		if ((code == 1) && (enc->val > enc->min)) {
				enc->val--;
		} else if ((code == 0) && (enc->val < enc->max)) {
				enc->val++;
		}
	}

	enc->code = (enc->code << 2) | code;

	return enc->val;
}

uint32_t encoder_val(struct encoder * enc)
{
	return enc->val;
}


