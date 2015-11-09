/* ---------------------------------------------------------------------------
 * File: zone.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <thinkos.h>
#include <assert.h>

#include "gpio.h"
#include "zone.h"
#include "net.h"

/* Number of local zones */
#define ZONE_COUNT ZONE_LOCAL_COUNT

/* GPIO mapping  */
const unsigned int zone_gpio_lut[ZONE_COUNT] = {
	GPIO_ZONE1,
	GPIO_ZONE2,
	GPIO_ZONE3,
	GPIO_ZONE4
};

#define ZONE_EVENT_MAX 16

/* Driver private data */
static struct {
	int sem;
	struct {
		volatile unsigned int head;
		unsigned int tail;
		struct zone_status buf[ZONE_EVENT_MAX];
	} queue;
} zonedrv;

static int zone_task(void * arg)
{
	/* Last two state of the zone */
	bool state[ZONE_COUNT][2];
	unsigned int panel;
	int i;

	/* Get the initial status of the input zones */
	for (i = 0; i < ZONE_COUNT; ++i) {
		state[i][0] = gpio_status(zone_gpio_lut[i]);
		state[i][1] = state[i][0];
	}

	/* this panel id */
	panel = net_local_addr();

	for (;;) {
		bool val[ZONE_COUNT];

		/* wait for 20 milliseconds */
		thinkos_sleep(20);

		/* Get the initial status of the input zones */
		for (i = 0; i < ZONE_COUNT; ++i)
			val[i] = gpio_status(zone_gpio_lut[i]) ? false : true;

		for (i = 0; i < ZONE_COUNT; ++i) {
			/* debouncing */
			if (val[i] == state[i][0]) {
				/* transition */
				if (val[i] != state[i][1]) {
					struct zone_status * entry;
					unsigned int head;

					state[i][1] = val[i];

					head = zonedrv.queue.head;
					assert ((head - zonedrv.queue.tail) < ZONE_EVENT_MAX);

					entry = &zonedrv.queue.buf[head % ZONE_EVENT_MAX];
					/* set the zone address */
					entry->zone.panel = panel;
					entry->zone.input = i + 1;
					/* set the zone active status */
					entry->active = state[i];
					zonedrv.queue.head = head + 1;
					thinkos_sem_post(zonedrv.sem);
				}
			} else
				state[i][0] = val[i];
		}
	}

	return 0;
}

/* Stack for the zone input polling thread */
static uint32_t zone_stack[64];

/* ----------------------------------------------------
   UI API
   ---------------------------------------------------- */

void zonedrv_init(void)
{
	/* Allocate a new semaphore for zone events */
	zonedrv.sem = thinkos_sem_alloc(0);
	zonedrv.queue.head = 0;
	zonedrv.queue.tail = 0;

	/* Create a thread for zone polling */
	thinkos_thread_create(zone_task, NULL, zone_stack, sizeof(zone_stack));
}

void zone_status_get(struct zone_status * pzs)
{
	struct zone_status * entry;
	unsigned int tail;

	/* Wait for a status semaphore to be signaled... */
	thinkos_sem_wait(zonedrv.sem);

	tail = zonedrv.queue.tail;

	assert(tail != zonedrv.queue.head);

	entry = &zonedrv.queue.buf[tail % ZONE_EVENT_MAX];
	pzs->zone = entry->zone;
	pzs->active = entry->active;
	zonedrv.queue.tail = tail + 1;
}

