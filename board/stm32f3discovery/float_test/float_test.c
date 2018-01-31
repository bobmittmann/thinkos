/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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
 * @file compass.c
 * @brief magnetometer application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/stm32f.h>
#include <sys/console.h>
#include <stdlib.h>
#include <stdio.h>
#include <thinkos.h>
#include <assert.h>
#include <math.h>

struct oscillator {
	float r[2][2];
	float p;
	float q;
};

volatile float f[32];

void fpu_test(void)
{
	int i;

	for (i = 0; i < 32; ++i)
		f[i] = i;

	asm volatile ("vldmia %0, {s0-s31}\n"
				  "nop\n"
				  "nop\n"
				  "nop\n"
				  : : "r" (f));
}

void printf_test(void)
{
	printf("      '1.0' --> '%f'\n", 1.0);
	printf("      '1.1' --> '%f'\n", 1.1);
	printf("      '1.01' --> '%f'\n", 1.01);
	printf("      '1.1' --> '%8.8f'\n", 1.1);
	printf("     '-1.1' --> '%8.8f'\n", -1.1);
	printf("      '1.001' --> '%8.8f'\n", 1.001);
	printf("      '1.00001' --> '%8.8f'\n", 1.00001);
	printf("      '1.000001' --> '%8.8f'\n", 1.000001);
	printf("      '1.0000001' --> '%8.8f'\n", 1.0000001);
	printf("      '1.00000001' --> '%8.8f'\n", 1.00000001);
	printf("  '10000.00000001' --> '%.12f'\n", 10000.00000001);
	printf("  '10000.00000001' --> '%f'\n", 10000.00000001);

	printf("      '0.0' --> '%f'\n", 0.0);
	printf("      '0.0' --> '%.0f'\n", 0.0);
	printf("      '0.0' --> '%.1f'\n", 0.0);
	printf("      '0.0' --> '%.3f'\n", 0.0);

	printf("      '1.0' --> '%f'\n", 1.0);
	printf("      '1.0' --> '%.0f'\n", 1.0);
	printf("      '1.0' --> '%.1f'\n", 1.0);
	printf("      '1.0' --> '%.10f'\n", 1.0);

	printf("     '-1.0' --> '%f'\n", -1.0);
	printf("     '-1.0' --> '%.0f'\n", -1.0);
	printf("     '-1.0' --> '%.1f'\n", -1.0);
	printf("     '-1.0' --> '%.10f'\n", -1.0);

	printf("  'INFINITY' --> '%.0f'\n", (double)INFINITY);
	printf("  'NAN' --> '%.0f'\n", (double)NAN);

	printf("      '0.999999' --> '%.12f'\n", 0.999999);
	printf("      '0.5' --> '%.1f'\n", 0.5);
	printf("      '0.25' --> '%.1f'\n", 0.25);
	printf("      '0.75' --> '%.1f'\n", 0.75);
	printf("      '0.125' --> '%.1f'\n", 0.125);
	printf("      '0.9' --> '%.1f'\n", 0.9);
	printf("      '0.99' --> '%.1f'\n", 0.99);

	printf("      '0.5' --> '%.2f'\n", 0.5);
	printf("      '0.25' --> '%.2f'\n", 0.25);
	printf("      '0.75' --> '%.2f'\n", 0.75);
	printf("      '0.125' --> '%.2f'\n", 0.125);
	printf("      '0.99' --> '%.2f'\n", 0.99);
	printf("      '0.999' --> '%.2f'\n", 0.999);

	printf("      '0.5' --> '%.3f'\n", 0.5);
	printf("      '0.25' --> '%.3f'\n", 0.25);
	printf("      '0.125' --> '%.3f'\n", 0.125);
	printf("      '0.999' --> '%.3f'\n", 0.999);
	printf("      '0.9999' --> '%.3f'\n", 0.9999);

	printf("             '0.5'     --> '%.4f'\n", 0.5);
	printf("             '0.25'    --> '%.4f'\n", 0.25);
	printf("             '0.125'   --> '%.4f'\n", 0.125);
	printf("             '0.99999' --> '%.4f'\n", 0.99999);
	printf("             '1.1234'  --> '%.4f'\n", 1.1234);
	printf("             '2.3456'  --> '%.4f'\n", 2.3456);
	printf("            '12.3456'  --> '%.4f'\n", 12.3456);
	printf("           '123.4567'  --> '%.4f'\n", 123.4567);
	printf("          '1234.5678'  --> '%.4f'\n", 1234.5678);
	printf("         '12345.6789'  --> '%.4f'\n", 12345.6789);
	printf("        '123456.7891'  --> '%.4f'\n", 123456.7891);
	printf("       '1234567.8'     --> '%.1f'\n", 1234567.8);
	printf("       '1234567.8'     --> '%.2f'\n", 1234567.8);
	printf("       '1234567.8'     --> '%.8f'\n", 1234567.8);
	printf("      '12345678.0'     --> '%.1f'\n", 12345678.0);
	printf("       '5345678.0'     --> '%.1f'\n", 5345678.0);
	printf("       '3345678.0'     --> '%.1f'\n", 3345678.0);
	printf("      '22345678.0'     --> '%.1f'\n", 22345678.0);
	printf("     '123456780.0'     --> '%.0f'\n", 123456780.0);
	printf("  '123456770000.0'     --> '%.0f'\n", 123456770000.0);
		/* Max Integer with exact representation */
	printf("       '16777215'       --> '%.0f'\n", (double)16777215);
	printf("       '8388607'       --> '%.0f'\n", (double)8388607);
	printf("       '8388607.5'     --> '%.0f'\n", 8388607.5);
	printf("       '8388608'       --> '%.0f'\n", (double)8388608);
	printf("       '8388609'       --> '%.0f'\n", (double)8388609);
	printf("       '8388610'       --> '%.0f'\n", (double)8388610);
	printf("       '8388611'       --> '%.0f'\n", (double)8388611);
	printf("      '19888610'       --> '%.0f'\n", (double)19888610);

	printf("   '9223372036854775808.0' --> '%.0f'\n", 9223372036854775808.0);
		/* Max number for internal uint64_t without overflow */
	printf("  '18446742974197923840.0' --> '%.0f'\n", 18446742974197923840.0);
	printf("  '18446744073709551616.0' --> '%.0f'\n", 18446744073709551616.0);

	printf("   '1844674297419792384.0' --> '%.1f'\n", 1844674297419792384.0);
	printf("    '184467429741979238.4' --> '%.2f'\n", 184467429741979238.4);
	printf("     '18446743000000000.0' --> '%.3f'\n", 18446743000000000.0);
	printf("      '1844674300000000.0' --> '%.4f'\n", 1844674300000000.0);
	printf("       '184467430000000.0' --> '%.5f'\n", 184467430000000.0);
	printf("        '18446743000000.0' --> '%.6f'\n", 18446743000000.0);
	printf("         '1844674300000.0' --> '%.7f'\n", 1844674300000.0);
	printf("           '18446743000.0' --> '%.8f'\n", 18446743000.0);
	printf("            '1844674300.0' --> '%.9f'\n", 1844674300.0);
	printf("             '184467430.0' --> '%.10f'\n", 184467430.0);
	printf("              '18446743.0' --> '%.11f'\n", 18446743.0);
	printf("               '1844674.3' --> '%.12f'\n", 1844674.3);

	printf("  '1.0' --> '%f'\n", 1.0);
	printf("  '0.1' --> '%f'\n", 0.1);
	printf("  '0.01' --> '%f'\n", 0.01);
	printf("  '0.001' --> '%f'\n", 0.001);
	printf("  '0.0001' --> '%f'\n", 0.0001);
	printf("  '0.00001' --> '%f'\n", 0.00001);
	printf("  '0.000001' --> '%f'\n", 0.000001);
	printf("  '0.0000001' --> '%f'\n", 0.0000001);
	printf("  '0.00000001' --> '%f'\n", 0.00000001);
	printf("  '0.000000001' --> '%f'\n", 0.000000001);
	printf("  '0.0000000001' --> '%f'\n", 0.0000000001);
	printf("  '0.00000000001' --> '%f'\n", 0.00000000001);
	printf("  '0.000000000001' --> '%f'\n", 0.000000000001);
	printf("  '0.5' --> '%f'\n", 0.5);
	printf("  '0.25' --> '%f'\n", 0.25);
	printf("  '0.125' --> '%f'\n", 0.125);
	printf("  '0.0625' --> '%f'\n", 0.0625);
	printf("  '0.03125' --> '%f'\n", 0.03125);
	printf("  '0.015625' --> '%f'\n", 0.015625);
	printf("  '0.0078125' --> '%f'\n", 0.0078125);
	printf("  '0.00390625' --> '%f'\n", 0.00390625);
	printf("  '0.001953125' --> '%f'\n", 0.001953125);
	printf("  '0.0009765625' --> '%f'\n", 0.0009765625);
	printf("  '0.00048828125' --> '%f'\n", 0.00048828125);
	printf("  '0.000244140625' --> '%f'\n", 0.000244140625);

	printf("\n");
}

int osc_init(struct oscillator * osc, unsigned int samplerate,
		float freq, float amp)
{
	float w0;

	assert(osc != NULL);

	w0 = ((float)(2.0 * M_PI) * freq) / samplerate;

	osc->r[0][0] = cos(w0);
	osc->r[0][1] = -sin(w0);
	osc->r[1][0] = -osc->r[0][1];
	osc->r[1][1] = osc->r[0][0];

	osc->p = amp;
	osc->q = 0.0;

	return 0;
}

float osc_sample(struct oscillator * osc)
{
	float x;
	float p0;
	float q0;

	x = osc->q;

	p0 = osc->p;
	q0 = osc->q;
	osc->p = p0 * osc->r[0][0] + q0 * osc->r[0][1];
	osc->q = p0 * osc->r[1][0] + q0 * osc->r[1][1];

	return x;
}

volatile float buffer; /* production buffer */

int sem_empty; /* semaphore to signal an empty buffer */
int sem_full; /* semaphore to signal a full buffer */

int prod_count;
volatile bool prod_done; /* production control flag */

int producer_task(void * arg)
{
	struct oscillator osc1khz;
	unsigned int i;
	float x;

	prod_done = false;

	printf(" %s(): [%d] started...\n", __func__, thinkos_thread_self());
	thinkos_sleep(100);

	osc_init(&osc1khz, 44100, 1000, 1);

	for (i = 0; i < prod_count; i++) {
		/* let's spend some time thinking */
		thinkos_sleep(500);

		x = osc_sample(&osc1khz);

		/* waiting for room to insert a new item */
		thinkos_sem_wait(sem_empty);

		/* insert the produced item in the buffer */
		buffer = x;

		/* signal a full buffer */
		thinkos_sem_post(sem_full);
	}

	prod_done = true;

	return i;
}

int consumer_task(void * arg)
{
	int i = 0;

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
		printf("%12.9f\n", (double)buffer);
		i++;
		/* signal the empty buffer */
		thinkos_sem_post(sem_empty);
	} while (!prod_done);

	/* get the last produced item, if any */
	if (thinkos_sem_timedwait(sem_full, 0) == 0) {
		printf(" %3d ", i);
		printf("%12.9f\n", (double)buffer);
		i++;
		thinkos_sem_post(sem_empty);
	}

	return i;
};

uint32_t producer_stack[128];
uint32_t consumer_stack[128];

void semaphore_test(void)
{
	int producer_th;
	int consumer_th;
	int producer_ret;
	int consumer_ret;

	/* allocate the empty signal semaphores */
	/* initialize the empty as 1 so we can insert an item immediately. */
	sem_empty = thinkos_sem_alloc(1);

	/* allocate the full signal semaphores */
	/* initialize the full as 0 as we don't have produced anything yet. */
	sem_full = thinkos_sem_alloc(0);

	/* create the producer thread */
	producer_th = thinkos_thread_create(producer_task, NULL,
			producer_stack, sizeof(producer_stack));

	/* create the consuer thread */
	consumer_th = thinkos_thread_create(consumer_task, NULL,
			consumer_stack, sizeof(consumer_stack));

	printf(" * Empty semaphore: %d\n", sem_empty);
	printf(" * Full semaphore: %d\n", sem_full);
	printf(" * Producer thread: %d\n", producer_th);
	printf(" * Consumer thread: %d\n", consumer_th);
	printf("\n");
	thinkos_sleep(100);

	/* number of items to be produced */
	prod_count = 100;

	/* wait for the production thread to finish */
	producer_ret = thinkos_join(producer_th);

	/* wait for the consumer thread to finish */
	consumer_ret = thinkos_join(consumer_th);

	printf(" * Production return = %d\n", producer_ret);
	printf(" * Consumer return = %d\n", consumer_ret);

	/* release the semaphores */
	thinkos_sem_free(sem_empty);
	thinkos_sem_free(sem_full);

	printf("\n");
};

/* ----------------------------------------------------------------------
 * Console
 * ----------------------------------------------------------------------
 */

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

int main(int argc, char ** argv)
{
	/* Initialize the stdin, stdout and stderr */
	stdio_init();

	/* Print a useful information message */
	printf("\n");
	printf("---------------------------------------------------------\n");
	printf(" ThinkOS - float point example\n");
	printf("---------------------------------------------------------\n");
	printf("\n");

	fpu_test();

	printf_test();

	/* Run the test */
	semaphore_test();

	thinkos_sleep(10000);

	return 0;
}
