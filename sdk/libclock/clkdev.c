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

#include <stdlib.h>
#include <errno.h>
#include <sys/clkdev.h>
#include <arch/cortex-m3.h>

#ifdef CONFIG_H
#include "config.h"
#endif

#ifndef CLKDEV_MAX
#define CLKDEV_MAX 2
#endif

static struct clkdev __clkdev_pool[CLKDEV_MAX];

struct clkdev * clkdev_alloc(void * __clk, const struct clkdevop * __op)
{
	struct clkdev * dev;
	int i;

	if (__op == NULL)
		return NULL;

	for (i = 0; i < CLKDEV_MAX; ++i) {
		dev = &__clkdev_pool[i];
		while ((void *)__ldrex((uint32_t *)(&dev->op)) == NULL) {
			if (__strex((uint32_t *)(&dev->op), (uint32_t)__op) == 0) {
				dev->clk = __clk;
				return dev;
			}
		}
	}

	return NULL;
}

static int32_t clkdevop_dummy(void * clk)
{
	(void)clk;

	return 0;
}

/* Null clock device operations */
static const struct clkdevop null_clkdevop = {
	.tick = (void *)clkdevop_dummy,
	.get = (void *)clkdevop_dummy,
	.set = (void *)clkdevop_dummy,
	.step = (void *)clkdevop_dummy,
	.drift_comp = (void *)clkdevop_dummy,
	.drift_get = (void *)clkdevop_dummy
};

int clkdev_free(struct clkdev * __dev)
{
	if ((__dev < __clkdev_pool) || __dev >= (__clkdev_pool + CLKDEV_MAX)) {
		/* Not in the pool!! */
		return -EINVAL;
	}

	__dev->clk = NULL;
	__dev->op = &null_clkdevop;

	return 0;
}
