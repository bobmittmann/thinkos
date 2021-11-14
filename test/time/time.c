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

/* -------------------------------------------------------------------------
 * Stdio
 * ------------------------------------------------------------------------- */

void stdio_init(void)
{
	struct tty_dev *tty;
	FILE *f_raw;
	FILE *f_tty;
	int i;

	/* wait 5 seconds for the USB connection */
	for (i = 0; i < 50; ++i) {
		if (console_is_connected())
			break;
		thinkos_sleep(100);
	}

	f_raw = console_fopen();
	tty = tty_attach(f_raw);
	f_tty = tty_fopen(tty);
	(void)f_tty;

	/* initialize STDIO */
	stdout = f_tty;
	stdin = f_tty;
	stderr = null_fopen(0);
}

/* Convert from clock interval to float point */
#define CLK_FLOAT(X)     ((float)(uint64_t)(X) / 4294967296.)

int main(int argc, char ** argv)
{
	uint64_t time;
	int i;
	
	/* Initialize the stdin, stdout and stderr */
	stdio_init();

	/* Print a useful information message */
	printf("\n");
	printf("---------------------------------------------------------\n");
	printf(" ThinkOS - Sanity test\n");
	printf("\n");
	printf(" Date and time example\n");
	printf("---------------------------------------------------------\n");
	printf("\n");

	/* Run the test */
	for (i = 10; i > 0; --i) {
		thinkos_sleep(1000);
		time = thinkos_time_monotonic_get();
		printf("%d - %.10f\r\n", i, time);
	}

	return 0;
}

