/* ---------------------------------------------------------------------------
 * File: falrm.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <thinkos.h>
#include <stdlib.h>
#include <assert.h>

#include "falrm.h"
#include "zone.h"
#include "nac.h"
#include "net.h"
#include "ui.h"

/* -------------------------------------------------------------------------
   Fire Alarm Module
   ------------------------------------------------------------------------- */

/* TODO: implement fire alarm module ... */

/* Thread to process user interface events. */
static int ui_input_task(void * arg)
{
	int key;

	for (;;) {
		/* Wait for the Ack Button press ... */
		key = ui_key_get();
		if (key != UI_KEY_ACK)
			continue;

		/* TODO: implement alarm acknowledgment */



	}

	return 0;
}

/* Thread to process local zone input events. */
static int zone_input_task(void * arg)
{
	struct zone_status zs;

	for (;;) {
		/* Wait for a zone status change ... */
		zone_status_get(&zs);

		/* TODO: process zone status change ... */



	}

	return 0;
}

/* Thread to process network messages. */
static int net_input_task(void * arg)
{
	struct zone_status * pzs;
	uint32_t data[8];
	uint8_t msg_type;
	int len;

	for (;;) {
		/* Wait for a message on the network */
		len = net_recv(&msg_type, data, sizeof(data));
		(void)len;

		switch (msg_type) {
		case MSG_SYS_STARTUP:
			break;
		case MSG_ZONE_STATUS:
			pzs = (struct zone_status *)data;
			/* TODO: implement zone status change processing */


			break;
		case MSG_ALARM_ACK:
			/* TODO: implement alarm acknowledgment */



			break;
		}
	}

	return 0;
}

uint32_t ui_input_stack[256];
uint32_t net_input_stack[256];
uint32_t zone_input_stack[256];

int falrm_init(void)
{
	/* TODO: Initialize fire alarm module... */

	thinkos_thread_create(ui_input_task, NULL, 
						  ui_input_stack, sizeof(ui_input_stack));

	thinkos_thread_create(net_input_task, NULL, 
						  net_input_stack, sizeof(net_input_stack));

	thinkos_thread_create(zone_input_task, NULL, 
						  zone_input_stack, sizeof(zone_input_stack));

	return 0;
}
