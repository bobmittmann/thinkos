/* app.c - COMM card main application
 * -----
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file app.c
 * @brief
 * @author Vijaykumar Trambadia <vtrambadia@mircomgroup.com>
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <fixpt.h>
#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>

#include "board.h"
#include "io.h"

void stdio_init(void)
{
	FILE *f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

/* ---------------------------------------------------------------------------
 * Application main function
 * ---------------------------------------------------------------------------
 */

int main(int argc, char **argv)
{
	unsigned int i;

	/* load udelay calibration factor from OS */
	thinkos_udelay_factor(&udelay_factor);

	/* Initializes stdio so we can use printf and such.
	   Attention: It's not recommended to use stdio calls inside of 
	   any code meant for release, except for shell utilities.  */
	stdio_init();

	printf("Test..\n");
	thinkos_sleep(100);

	io_init();

	led_flash(1, 500);
	led_blink(2, 200, 200);

	for (i = 0; i < 200; ++i) {
		int ev;

		ev = btn_event_wait();
		switch (ev) {
		case EVENT_NONE:
			printf("NONE\n");
			break;

		case EVENT_CLICK:
			printf("CLICK\n");
			break;

		case EVENT_DBL_CLICK:
			printf("DBL_CLICK\n");
			break;

		case EVENT_HOLD1:
			printf("HOLD1\n");
			break;

		case EVENT_CLICK_N_HOLD:
			printf("CLICK_N_HOLD\n");
			break;

		case EVENT_HOLD2:
			printf("HOLD2\n");
			break;

		default:
			printf("?\n");

		}
	}


	return 42;
}


