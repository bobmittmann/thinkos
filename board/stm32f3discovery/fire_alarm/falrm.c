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

struct zone {
	bool active;
	bool alarm;
	bool acked;
	uint8_t led;
	unsigned int seq;
};

struct {
	struct zone zone[PANEL_COUNT + 1][ZONE_LOCAL_COUNT + 1];
	volatile unsigned int alarm_cnt;
	volatile unsigned int ack_seq;
	int mutex;
} fa;


void zone_status_change(struct zone_status * pzs)
{
	struct zone * zone = &fa.zone[pzs->zone.panel][pzs->zone.input];

	thinkos_mutex_lock(fa.mutex);

	if (pzs->active) {
		assert(!zone->active);
		/* Zone zone activation */
		zone->active = true;
		zone->alarm = true;
		zone->acked = false;
		/* update the zone count */
		if (fa.alarm_cnt++ == 0) {
			/* first zone, activate NAC */
			nac_on();
		}
		/* set the zone activation sequence */
		zone->seq = fa.alarm_cnt;
		ui_led_blink(zone->led);
	} else {
		/* Zone deactivation */
		zone->active = false; /* clear the zone active flag */
		if (zone->acked) {
			/* Clear the alarm condition */
			zone->alarm = false;
			/* turn off led */
			ui_led_off(zone->led);
			if (--fa.alarm_cnt == 0) {
				/* if no more active zones shut off the NAC */
				nac_off();
			}
		}
	}

	thinkos_mutex_unlock(fa.mutex);
}

bool alarm_ack(void)
{
	unsigned int seq = fa.ack_seq + 1;
	int ack_cnt = 0;
	int panel;
	int input;

	thinkos_mutex_lock(fa.mutex);

	for (panel = 1; panel <= PANEL_COUNT; ++panel) {
		for (input = 1; input <= ZONE_LOCAL_COUNT; ++input) {
			struct zone * zone = &fa.zone[panel][input];
			if (zone->alarm && !zone->acked && zone->seq == seq) {
				/* Acknowledge the alarm */
				zone->acked = true;
				/* Check whether the zone is still active */
				if (zone->active) {
					/* turn led steady on*/
					ui_led_on(zone->led);
				} else {
					/* Clear the alarm condition */
					zone->alarm = false;
					/* turn off led */
					ui_led_off(zone->led);
					if (--fa.alarm_cnt == 0) {
						/* if no more active zones shut off the NAC */
						nac_off();
					}
				}
				ack_cnt++;
				/* update ack sequence */
				fa.ack_seq = seq;
			}
		}
	}

	thinkos_mutex_unlock(fa.mutex);

	assert(ack_cnt <= 1);

	return ack_cnt ? true : false;
}

static int ui_input_task(void * arg)
{
	int key;

	for (;;) {
		/* Wait for the Ack Button press ... */
		key = ui_key_get();
		if (key != UI_KEY_ACK)
			continue;

		if (alarm_ack()) {
			/* Send an alarm ack message */
			net_send(MSG_ALARM_ACK, NULL, 0);
		}
	}

	return 0;
}

static int zone_input_task(void * arg)
{
	struct zone_status zs;

	for (;;) {
		/* Wait for a zone status change ... */
		zone_status_get(&zs);
		zone_status_change(&zs);
		/* Send the zone status change message */
		net_send(MSG_ZONE_STATUS, &zs, sizeof(struct zone_status));
	}

	return 0;
}

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
			zone_status_change(pzs);
			break;
		case MSG_ALARM_ACK:
			alarm_ack();
			break;
		}
	}

	return 0;
}

uint32_t ui_input_stack[64];
uint32_t net_input_stack[64];
uint32_t zone_input_stack[64];

int falrm_init(void)
{
	int panel;
	int input;

	thinkos_thread_create(ui_input_task, NULL, 
						  ui_input_stack, sizeof(ui_input_stack));

	thinkos_thread_create(net_input_task, NULL, 
						  net_input_stack, sizeof(net_input_stack));

	thinkos_thread_create(zone_input_task, NULL, 
						  zone_input_stack, sizeof(zone_input_stack));

	for (panel = 1; panel <= PANEL_COUNT; ++panel) {
		for (input = 1; input <= ZONE_LOCAL_COUNT; ++input) {
			struct zone * zone = &fa.zone[panel][input];

			zone->active = false;
			zone->alarm = false;
			zone->acked = false;
			zone->seq = 0;
			zone->led = panel * ZONE_LOCAL_COUNT + input;
		}
	}

	return 0;
}

