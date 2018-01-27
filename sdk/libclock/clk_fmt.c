/*
   Copyright(C) 2011 Robinson Mittmann.
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

int uint2dec(char * s, unsigned int val);

/****************************************************************************
 * utility functions 
 ****************************************************************************/

char * fmt_clk(char * s, int64_t ts)
{
	bool neg = false;
	int32_t sec;
	int32_t us;
	int32_t min;
	int32_t hour;
	int32_t days;

	if (ts < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		ts = -ts;
	}

	us = ((ts & 0xffffffffLL) * 1000000 + (1LL << 31)) >> 32;
	sec = ts >> 32;
	min = sec / 60;
	sec -= min * 60;
	hour = min / 60;
	min -= hour * 60;
	days = hour / 24;
	hour -= days * 24;

	if ((days > 0) || (hour > 0)) {
		if (neg)
			hour = -hour;
		sprintf(s, "%3d:%02d:%02d.%03d", hour, min, sec, us / 1000);
	} else if (min > 0) {
		if (neg)
			min = -min;
		sprintf(s, "%02d:%02d.%03d", min, sec, us / 1000);
	} else if (sec > 0) {
		if (neg)
			sec = -sec;
		sprintf(s, "%02d.%06d", sec, us);
	} else if (neg)
		sprintf(s, "-.%06d", us);
	else
		sprintf(s, ".%06d", us);

	return s;
}


/* format a clock timestamp, milliseconds resolution  */
char * fmt_clk_ms(char * s, int64_t ts)
{
	bool neg = false;
	uint32_t sec;
	uint32_t ms;
	int min;
	int hour;
	int days;

	if (ts < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		ts = -ts;
	}

	ms = ((ts & 0xffffffffLL) * 1000 + (1LL << 31)) >> 32;
	sec = ts >> 32;
	min = sec / 60;
	sec -= min * 60;
	hour = min / 60;
	min -= hour * 60;
	days = hour / 24;
	hour -= days * 24;

	if (neg)
		hour = -hour;

	sprintf(s, "%3d:%02d:%02d.%03d", hour, min, sec, ms);

	return s;
}

/* format a clock timestamp, mircosseconds resolution  */
char * fmt_clk_us(char * s, int64_t ts)
{
	bool neg = false;
	uint32_t sec;
	uint32_t us;
	int min;
	int hour;
	int days;

	if (ts < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		ts = -ts;
	}

	us = ((ts & 0xffffffffLL) * 1000000 + (1LL << 31)) >> 32;
	sec = ts >> 32;
	min = sec / 60;
	sec -= min * 60;
	hour = min / 60;
	min -= hour * 60;
	days = hour / 24;
	hour -= days * 24;

	if (neg)
		hour = -hour;

	sprintf(s, "%3d:%02d:%02d.%06d", hour, min, sec, us);

	return s;
}

char * fmt_q31_3(char * s, int64_t x)
{
	bool neg = false;
	int64_t y;
	int32_t d;
	int32_t q;

	if (x < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		x = -x;
	}

	y = ((uint64_t)x * 1000LL + (1LL << 30) - 1) >> 31;
	q = y % 1000;
	d = y / 1000;
	if (neg)
		sprintf(s, "-%d.%03d", d, q);
	else
		sprintf(s, "%d.%03d", d, q);

	return s;
}

char * fmt_q31_6(char * s, int64_t x)
{
	bool neg = false;
	int64_t y;
	int32_t d;
	int32_t q;

	if (x < 0) {
		neg = true;
		x = -x;
	} 

	y = ((int64_t)x * 1000000LL + (1LL << 30) - 1) >> 31;
	q = y % 1000000;
	d = y / 1000000;
	if (neg)
		sprintf(s, "-%d.%06d", d, q);
	else
		sprintf(s, "%d.%06d", d, q);

	return s;
}

char * fmt_q31_9(char * s, int64_t x)
{
	bool neg = false;
	int64_t y;
	int32_t d;
	int32_t q;

	if (x < 0) {
		neg = true;
		x = -x;
	}

	y = ((int64_t)x * 1000000000LL + (1LL << (30)) - 1) >> 31;
	q = y % 1000000000;
	d = y / 1000000000;
	if (neg)
		sprintf(s, "-%d.%09d", d, q);
	else
		sprintf(s, "%d.%09d", d, q);
	return s;
}

char * fmt_clk_3(char * s, int64_t x)
{
	bool neg = false;
	int32_t dec;
	uint32_t frac;

	if (x < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		x = -x;
	}

	dec = x >> 32;
	x &= 0xffffffff;

	frac = ((uint64_t)x * 1000LL + (1LL << 31)) >> 32;

	if (neg)
		sprintf(s, "-%d.%03d", dec, frac);
	else
		sprintf(s, "%d.%03d", dec, frac);

	return s;
}

char * fmt_clk_6(char * s, int64_t x)
{
	bool neg = false;
	int32_t dec;
	uint32_t frac;

	if (x < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		x = -x;
	}

	dec = x >> 32;
	x &= 0xffffffff;

	frac = ((uint64_t)x * 1000000LL + (1LL << 31)) >> 32;

	if (neg)
		sprintf(s, "-%d.%06d", dec, frac);
	else
		sprintf(s, "%d.%06d", dec, frac);

	return s;
}

char * fmt_clk_9(char * s, int64_t x)
{
	bool neg = false;
	int32_t dec;
	uint32_t frac;

	if (x < 0) {
		/* negative timestamp. Get the absolute value and store the signal */
		neg = true;
		x = -x;
	}

	dec = x >> 32;
	x &= 0xffffffff;

	frac = ((uint64_t)x * 1000000000LL + (1LL << 31)) >> 32;

	if (neg)
		sprintf(s, "-%d.%09d", dec, frac);
	else
		sprintf(s, "%d.%09d", dec, frac);

	return s;
}

