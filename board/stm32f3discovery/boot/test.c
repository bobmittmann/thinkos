/* 
 * File:	 usb-test.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <sys/dcclog.h>

#include <thinkos.h>
#include "board.h"
#include "version.h"

void led_on(unsigned int id);
void led_off(unsigned int id);

void test_app(void)
{
	DCC_LOG(LOG_TRACE, "TEST TEST TEST TEST TEST TEST TEST");
	led_on(0);
	led_on(1);
	led_on(2);
	led_on(3);
	led_on(4);
	led_on(5);
	led_on(6);
	led_on(7);
	thinkos_mutex_lock(0);
}

