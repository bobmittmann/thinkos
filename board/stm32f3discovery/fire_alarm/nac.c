/* ---------------------------------------------------------------------------
 * File: nac.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdbool.h>
#include <thinkos.h>

#include "gpio.h"
#include "nac.h"

/* Driver private data */
static struct {
	volatile bool active;
} nacdrv;

static int nac_task(void * arg)
{
	unsigned int cnt = 0;
	bool active = false;

	for (;;) {
		if (nacdrv.active) {
			if (!active) {
				/* Activate */
				cnt = 0;
				active = true;
			}
		} else {
			if (active) {
				/* Deactivate */
				active = false;
				gpio_clr(GPIO_NAC_OUT);
			}
		}

		if (active) {
			/* Temporal three patern */
			switch (cnt++ % 8) {
			case 0:
			case 2:
			case 4:
				gpio_set(GPIO_NAC_OUT);
				break;
			case 1:
			case 3:
			case 5:
				gpio_clr(GPIO_NAC_OUT);
			}
		}

		thinkos_sleep(500);
	}

	return 0;
}

/* Stack for the User Interface polling thread */
static uint32_t nac_stack[64];

/* ----------------------------------------------------
   NAC API
   ---------------------------------------------------- */

void nacdrv_init(void)
{
	/* Create a thread for nac pattern */
    thinkos_thread_create(nac_task, NULL, nac_stack, sizeof(nac_stack));
}

void nac_on(void)
{
	nacdrv.active = true;
}

void nac_off(void)
{
	nacdrv.active = false;
	gpio_clr(GPIO_NAC_OUT);
}

