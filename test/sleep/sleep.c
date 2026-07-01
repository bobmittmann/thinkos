/* 
 * File:	 sem_test.c
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <thinkos.h>
#include <sys/console.h>
#include <sys/null.h>
#include <sys/tty.h>

int task(void * arg)
{
	/* set the production enable flag to start production */
	do {
		thinkos_sleep(1);
	} while (1);

	return 0;
};

#define SIZEOF_STACK 512
uint32_t stack[31][SIZEOF_STACK/4];

int main(int argc, char ** argv)
{
	int i;

	for (i = 0; i < 31; ++i) {
		thinkos_thread_create(C_TASK(task), NULL, stack[i], SIZEOF_STACK);
	}

	return task(NULL);
}

