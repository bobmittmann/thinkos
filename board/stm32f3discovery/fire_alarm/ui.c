/* ---------------------------------------------------------------------------
 * File: ui.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <thinkos.h>

#include "gpio.h"
#include "ui.h"

/* Internal LED state */
enum led_state {
	LED_OFF,
	LED_ON,
	LED_BLINK
};

/* Number of LEDs */
#define UI_LED_COUNT 8

/* Driver private data */
static struct {
	struct {
		uint8_t gpio;
		volatile uint8_t state;
	} led[UI_LED_COUNT];
	struct {
		uint8_t gpio;
		int8_t sem;
	} key;
} ui;

/* User interface thread: runs each 50 milliseconds and
 * controls the LEDs whose state are set to LED_BLINK. Also it reads
 * the Ack Button input, debounces it, and signal a semaphore when
 * it detects that the button was pressed.
 */
static int ui_task(void * arg)
{
	unsigned int cnt; /* free running counter */
	unsigned int sw_prev; /* key switch state (used for debouncing) */
	unsigned int sw_edge; /* key switch state (used for edge detection) */

	/* Get the initial status of the key switch GPIO */
	sw_prev = gpio_status(ui.key.gpio);
	sw_edge = sw_prev;

	for (cnt = 0; ; ++cnt) {
		unsigned int sw = 0;
		int step = cnt % 10; /* count ten steps of 50 milliseconds */
		int i;

		/* Process LEDs */
		if (step == 0) { /* t = 0 */
			/* Turn on all blinking LEDs */
			for (i = 0; i < UI_LED_COUNT; ++i) {
				if (ui.led[i].state == LED_BLINK)
					gpio_clr(ui.led[i].gpio); /* Turn LED on */
			}
		} else if (step == 5) { /* 250 milliseconds */
			/* Turn off all blinking LEDs */
			for (i = 0; i < UI_LED_COUNT; ++i) {
				if (ui.led[i].state == LED_BLINK)
					gpio_set(ui.led[i].gpio); /* Turn LED off */
			}
		}

		/* Get the status of the GPIO */
		sw = gpio_status(ui.key.gpio);
		if (sw == sw_prev) { /* Debouncing */
			/* Only process the signal if the current and the last
			 * pin status matches. */
			if (sw != sw_edge) { /* Edge detection */
				/* If the current status is different then the last
				 * then the switch was either pressed or released. */
				if (sw == 1) { /* Key pressed */
					thinkos_sem_post(ui.key.sem);
				}
			}
			sw_edge = sw;
		} 
		sw_prev = sw;

		/* wait for 50 milliseconds */
		thinkos_sleep(50);
	}

	return 0;
}

/* Stack for the User Interface polling thread */
static uint32_t ui_stack[64];

/* ----------------------------------------------------
   UI API
   ---------------------------------------------------- */

void ui_init(void)
{
	int i;
	int thread_id;

	/* Initialize LEDs GPIO mapping */
	ui.led[0].gpio = GPIO_LED1;
	ui.led[1].gpio = GPIO_LED2;
	ui.led[2].gpio = GPIO_LED3;
	ui.led[3].gpio = GPIO_LED4;
	ui.led[4].gpio = GPIO_LED5;
	ui.led[5].gpio = GPIO_LED6;
	ui.led[6].gpio = GPIO_LED7;
	ui.led[7].gpio = GPIO_LED8;

	/* Turn off all LEDs */
	for (i = 0; i < UI_LED_COUNT; ++i) {
		ui.led[i].state = LED_OFF;
		gpio_set(ui.led[i].gpio); /* Turn LED off */
	}

	/* Initialize Key Switch GPIO mapping */
	ui.key.gpio = GPIO_SWITCH;
	/* Allocate a new semaphore for key press events */
	ui.key.sem = thinkos_sem_alloc(0);
	/* Create a thread for input polling and LED blinking */
    thinkos_thread_create(ui_task, NULL, ui_stack, sizeof(ui_stack));
}

void ui_led_on(unsigned int id)
{
	if (--id < UI_LED_COUNT) {
		ui.led[id].state = LED_ON;
		gpio_clr(ui.led[id].gpio); /* Turn LED on */
	}
}

void ui_led_off(unsigned int id)
{
	if (--id < UI_LED_COUNT) {
		ui.led[id].state = LED_OFF;
		gpio_set(ui.led[id].gpio); /* Turn LED off */
	}
}

void ui_led_blink(unsigned int id)
{
	if (--id < UI_LED_COUNT) {
		ui.led[id].state = LED_BLINK;
		gpio_set(ui.led[id].gpio); /* Turn LED off */
	}
}

int ui_key_get(void)
{
	/* Wait for a key press semaphore to be signaled... */
	thinkos_sem_wait(ui.key.sem);
	/* Return the key Id */
	return UI_KEY_ACK;
}
