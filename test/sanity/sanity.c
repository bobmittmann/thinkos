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

extern const char * zarathustra[];

volatile uint64_t buffer; /* production buffer */

int sem_empty; /* semaphore to signal an empty buffer */
int sem_full; /* semaphore to signal a full buffer */

int prod_count;
volatile bool prod_done; /* production control flag */
volatile bool cons_done; /* consumer control flag */

int producer_task(void * arg)
{
	uint64_t y;
	unsigned int i = 0;
	uint64_t x0 = 0;
	uint64_t x1 = 0;
	uint64_t x2 = 0;
	const char * line;
	int j;

	prod_done = false;

	printf(" %s(): [%d] started...\n", __func__, thinkos_thread_self());
	thinkos_sleep(100);

	j = 0;
	for (i = 0; i < prod_count; i++) {
		/* let's spend some time thinking */
		thinkos_sleep(500);


		line = zarathustra[j++];
		if (line == NULL) {
			j = 0;
			line = zarathustra[j++];
		}

		x2 = strlen(line);

		/* working */
		if (i == 0)
			y = 0;
		else if (i == 1)
			y = 1;
		else
			y = x1 + x0 + x2;

		x0 = x1;
		x1 = y;

		/* waiting for room to insert a new item */
		thinkos_sem_wait(sem_empty);

		/* insert the produced item in the buffer */
		buffer = y;

		/* signal a full buffer */
		thinkos_sem_post(sem_full);
	}

	prod_done = true;

	return i;
}

int consumer_task(void * arg)
{
	int i = 0;

	cons_done = false;

	printf(" %s(): [%d] started...\n", __func__, thinkos_thread_self());
	thinkos_sleep(100);

	/* set the production enable flag to start production */
	do {
		printf(" %3d ", i);
		/* wait for an item to be produced */
		while (thinkos_sem_timedwait(sem_full, 50) == THINKOS_ETIMEDOUT) {
			printf(".");
		}

		/* unload the buffer */
		printf(" %016llx %llu\n", buffer, buffer);
		i++;
		/* signal the empty buffer */
		thinkos_sem_post(sem_empty);
	} while (!prod_done);

	/* get the last produced item, if any */
	if (thinkos_sem_timedwait(sem_full, 0) == 0) {
		printf(" %3d ", i);
		printf(" %016llx %llu\n", buffer, buffer);
		i++;
		thinkos_sem_post(sem_empty);
	}

	cons_done = true;

	return i;
};

THINKOS_DEFINE_STACK(producer_stack, 512);
THINKOS_DEFINE_STACK(consumer_stack, 512);

//uint32_t producer_stack[512];
//uint32_t consumer_stack[128];

void semaphore_test(void)
{
	int producer_th;
	int consumer_th;
#if 0
	int producer_ret;
	int consumer_ret;
#endif

	/* allocate the empty signal semaphores */
	/* initialize the empty as 1 so we can insert an item immediately. */
	sem_empty = thinkos_sem_alloc(1); 

	/* allocate the full signal semaphores */
	/* initialize the full as 0 as we don't have produced anything yet. */
	sem_full = thinkos_sem_alloc(0); 

	/* create the producer thread */
	producer_th = thinkos_thread_create(C_TASK(producer_task), NULL, 
			producer_stack, sizeof(producer_stack));

	/* create the consuer thread */
	consumer_th = thinkos_thread_create(C_TASK(consumer_task), NULL, 
			consumer_stack, sizeof(consumer_stack));

	printf(" * Empty semaphore: %d\n", sem_empty);
	printf(" * Full semaphore: %d\n", sem_full);
	printf(" * Producer thread: %d\n", producer_th);
	printf(" * Consumer thread: %d\n", consumer_th);
	printf("\n");
	thinkos_sleep(100);

	/* number of items to be produced */
	prod_count = 100;

	while (!prod_done) {
		thinkos_sleep(100);
	}

	while (!cons_done) {
		thinkos_sleep(100);
	}
#if 0
	/* wait for the production thread to finish */
	producer_ret = thinkos_join(producer_th);
	/* wait for the consumer thread to finish */
	consumer_ret = thinkos_join(consumer_th);

	printf(" * Production return = %d\n", producer_ret);
	printf(" * Consumer return = %d\n", consumer_ret);
#endif

	/* release the semaphores */
	thinkos_sem_free(sem_empty);
	thinkos_sem_free(sem_full);

	printf("\n");
};

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

int main(int argc, char ** argv)
{
	int i;
	
	/* Initialize the stdin, stdout and stderr */
	stdio_init();

	/* Print a useful information message */
	printf("\n");
	printf("---------------------------------------------------------\n");
	printf(" ThinkOS - Sanity test\n");
	printf("\n");
	printf(" Semaphore example\n");
	printf("---------------------------------------------------------\n");
	printf("\n");

	/* Run the test */
	semaphore_test();

	printf(" Returning... ");

	for (i = 10; i > 0; --i) {
		thinkos_sleep(10000);
		printf("%d ", i);
	}

	return 0;
}

