/* ---------------------------------------------------------------------------
 * File: main.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"
#include "falrm.h"
#include "zone.h"
#include "nac.h"
#include "net.h"
#include "ui.h"

static void stdio_init(void)
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
	/* Initialize the stdio streams so we can use printf() */
	stdio_init();

	/* Initialize GPIO pins */
	gpio_init();

	/* Initialize the User Interface */
	ui_init();

	/* Initialize the network */
	net_init();

	/* Initialize the detection zone driver */
	zonedrv_init();

	/* Initialize the NAC driver */
	nacdrv_init();

	/* Initialize the fire alarm module */
	falrm_init();

	/* Wait a little bit */
	thinkos_sleep(1000);

	/* Send a system startup message */
	net_send(MSG_SYS_STARTUP, NULL, 0);

	/* Do nothing in the main thread */
	for (;;) {
		thinkos_sleep(1000);
	}

	return 0;
}

