/* fault_test.c
 * ------------
 */

#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>

#include "board.h"

uint32_t stack[4][1024] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf thread5_init = {
	.stack_ptr = stack[3],
	.stack_size = 4096,
	.thread_id = 5,
	.paused = false,
	.tag = "TASK5"
};

const struct thinkos_thread_inf thread4_init = {
	.stack_ptr = stack[2],
	.stack_size = 4096,
	.thread_id = 4,
	.paused = false,
	.tag = "TASK4"
};

const struct thinkos_thread_inf thread3_init = {
	.stack_ptr = stack[1],
	.stack_size = 4096,
	.thread_id = 3,
	.paused = false,
	.tag = "TASK3"
};

const struct thinkos_thread_inf thread2_init = {
	.stack_ptr = stack[0],
	.stack_size = 4096,
	.thread_id = 2,
	.paused = false,
	.tag = "TASK2"
};


void stdio_init(void)
{
	FILE *f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

#define QUEUE_SIZE 8

/* insert at the head, collect at the tail */
struct queue {
	int cond;
	int mutex;
	volatile uint32_t head;
	volatile uint32_t tail;
	uint32_t ring[QUEUE_SIZE];
};

void enqueue(struct queue * q, uint32_t val) 
{
	thinkos_mutex_lock(q->mutex);

	while (q->head == (q->tail + QUEUE_SIZE)) {
		printf("<%d> fifo full\r\n", thinkos_thread_self());
		thinkos_cond_wait(q->cond, q->mutex);
	}

	q->ring[q->head++ % QUEUE_SIZE] = val;

	thinkos_mutex_unlock(q->mutex);

	thinkos_cond_signal(q->cond);
}

int dequeue(struct queue * q) 
{
	uint32_t val;

	thinkos_mutex_lock(q->mutex);

	while (q->head == q->tail) {
		printf("<%d> fifo empty\r\n", thinkos_thread_self());
		while (thinkos_cond_timedwait(q->cond, q->mutex, 1000) < 0) {
			printf("<%d> timeout...\r\n", thinkos_thread_self());
		};
	}

	val = q->ring[q->tail++ % QUEUE_SIZE];

	thinkos_mutex_unlock(q->mutex);

	thinkos_cond_signal(q->cond);
	return val;
}

int producer_task(struct queue * q)
{
	int self = thinkos_thread_self();
	int count = self << 24;
	for (;;) {
		count++;
		enqueue(q, count); 
		if ((count % 5) == 0) {
			thinkos_sleep(4000);
		} else {
			thinkos_sleep(250);
		}
	}

	return 0;
}

int consumer_task(struct queue * q)
{
	int self = thinkos_thread_self();
	uint32_t val;
	int i;

	for (i =0;;++i) {
		int from;
		val = dequeue(q); 
		from = val >> 24;
		val = val & 0x00ffffff;
		printf("<%d> Recived %2d from %d\r\n", self, val, from);
		if ((i % 8) == 0)
			thinkos_sleep(100);
		else
			thinkos_sleep(250);
	}

	return 0;
}


/* ---------------------------------------------------------------------------
 * Application main function
 * ---------------------------------------------------------------------------
 */

int main(int argc, char **argv)
{
	struct queue q;

	thinkos_sleep(1000);

	/* Initializes stdio so we can use printf and such. */
	stdio_init();

	q.tail = q.head = 0;
	q.mutex = thinkos_mutex_alloc();
	q.cond = thinkos_cond_alloc();

	thinkos_thread_create_inf(C_TASK(consumer_task), 
							  C_ARG(&q),
							  &thread2_init);
#if 0
	thinkos_thread_create_inf(C_TASK(consumer_task), 
							  C_ARG(&q),
							  &thread3_init);

	thinkos_thread_create_inf(C_TASK(producer_task), 
							  C_ARG(&q),
							  &thread4_init);
#endif
	thinkos_thread_create_inf(C_TASK(producer_task), 
							  C_ARG(&q),
							  &thread5_init);

	for(;;) {
		thinkos_sleep(600000);
	}


	return 0;
}




