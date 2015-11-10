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

static void zone_event_enqueu(int panel, int input, bool active)
{
	struct zone_status * entry;
	unsigned int head;

	head = zonedrv.queue.head;
	assert ((head - zonedrv.queue.tail) < ZONE_EVENT_MAX);

	entry = &zonedrv.queue.buf[head % ZONE_EVENT_MAX];
	/* set the zone address */
	entry->zone.panel = panel;
	entry->zone.input = input;
	/* set the zone active status */
	entry->active = active;
	zonedrv.queue.head = head + 1;
	thinkos_sem_post(zonedrv.sem);
}

#define AVG_FILT_N 4

#define ZONE_DET_LVL 32
#define ZONE_DET_LO (2 * AVG_FILT_N)
#define ZONE_DET_HI ((ZONE_DET_LVL - 2) * AVG_FILT_N)

static int zone_task(void * arg)
{
	unsigned int avg[ZONE_COUNT]; /* Zone input filter */
	bool active[ZONE_COUNT];      /* Zone state */
	unsigned int panel;           /* Panel address */
	int i;

	/* Set the initial status of the input zones */
	for (i = 0; i < ZONE_COUNT; ++i) {
		avg[i] = 0;
		active[i] = false;
	}

	/* This panel id */
	panel = net_local_addr();

	for (;;) {
		int val[ZONE_COUNT];

		/* Wait for 25 milliseconds */
		thinkos_sleep(25);

		/* Get the status of the input zones */
		for (i = 0; i < ZONE_COUNT; ++i)
			val[i] = gpio_status(zone_gpio_lut[i]) ? 0 : ZONE_DET_LVL;

		/* Moving average Filter */
		for (i = 0; i < ZONE_COUNT; ++i)
			avg[i] = ((avg[i] * (AVG_FILT_N - 1)) / AVG_FILT_N) + val[i];

		/* Transition detection */
		for (i = 0; i < ZONE_COUNT; ++i) {
			if (!active[i] && (avg[i] > ZONE_DET_HI)) {
				active[i] = true;
				zone_event_enqueu(panel, i + 1, true);
			} else	if (active[i] && (avg[i] < ZONE_DET_LO)) {
				active[i] = false;
				zone_event_enqueu(panel, i + 1, false);
			}
		}
	}

	return 0;
}

/* Stack for the zone input polling thread */
static uint32_t zone_stack[64];

/* ----------------------------------------------------
   Zone Detection API
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

