/*
   Copyright(C) 2017 Robinson Mittmann.
  
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

#include "clock-i.h"

/* -------------------------------------------------------------------------
 * Pool of resources
 * ------------------------------------------------------------------------- */

static struct clock clock_pool[HTTPD_POOL_SIZE];

struct clock * clock_alloc(void)
{
	struct clock * clock;
	int i;

	for (i = 0; i < HTTPD_POOL_SIZE; ++i) {
		clk = &clock_pool[i];
//		if (!clk->auth) {
//			clk->auth = clock_def_authlst;
			return clk;
//		}
	}

	return clk;
}

