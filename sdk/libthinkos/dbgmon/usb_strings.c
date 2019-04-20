/* 
 * File:	 usb_comm-quueue.c
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

#include <string.h>
#include <stdbool.h>
#include <sys/usb-dev.h>

/* LangID = 0x0409: U.S. English */
const struct usb_descriptor_string language_english_us = {
	4, USB_DESCRIPTOR_STRING, { 0x0409 }
};

const struct usb_descriptor_string stmicroelectronics_str = {
	19 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'S', 'T', 'M', 'i', 'c', 'r', 'o', 'e', 'l', 'e', 
		'c', 't', 'r', 'o', 'n', 'i', 'c', 's', 0 
	}
};

const struct usb_descriptor_string stmicro_str = {
	8 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'S', 'T', 'M', 'i', 'c', 'r', 'o', 0
	}
};

const struct usb_descriptor_string composite_demo_str = {
	15 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'C', 'o', 'm', 'p', 'o', 's', 'i', 't', 'e', ' ', 
		'D', 'e', 'm', 'o', '0'
	}
};

const struct usb_descriptor_string atmel_str = {
	6 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'A', 't', 'm', 'e', 'l', '0'
	}
};

/* Interface 0: "ST VCOM" */
const struct usb_descriptor_string st_vcom_str = {
	8 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'S', 'T', ' ', 'V', 'C', 'O', 'M', 0
	}
};

/* Product name: "ThinkOS Debug Monitor" */
const struct usb_descriptor_string thinkos_debug_monitor_str = {
	22 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'T', 'h', 'i', 'n', 'k', 'O', 'S', ' ', 'D', 'e', 
		'b', 'u', 'g', ' ', 'M', 'o', 'n', 'i', 't', 'o', 
		'r', 0
	}
};

const struct usb_descriptor_string thinkos_com_str = {
	17 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'T', 'h', 'i', 'n', 'k', 'O', 'S', ' ', 'C', 'O', 
		'M', ' ', 'D', 'u', 'a', 'l', 0
	}
};
const struct usb_descriptor_string usb_serial_cdc_device_str = {
	23 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'U', 'S', 'B', ' ', 'S', 'e', 'r', 'i', 'a', 'l', 
		' ', '(', 'C', 'D', 'C', ')', 'D', 'e', 'v', 'i',
		'c', 'e', 0
	}
};

const struct usb_descriptor_string thinkos_str = {
	8 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'T', 'h', 'i', 'n', 'k', 'O', 'S', 0
	}
};

const struct usb_descriptor_string debug_monitor_str = {
	22 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'T', 'h', 'i', 'n', 'k', 'O', 'S', ' ', 'D', 'e', 
		'b', 'u', 'g', '/', 'M', 'o', 'n', 'i', 't', 'o', 
		'r', 0
	}
};

const struct usb_descriptor_string console_str = {
	16 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'T', 'h', 'i', 'n', 'k', 'O', 'S', ' ', 'C', 'o', 
		'n', 's', 'o', 'l', 'e', 0
	}
};

const struct usb_descriptor_string serial_num_str = {
	15 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'5', '4', '6', '8', '6', '9', '6', 'e', '6', 'b',
		'4', 'f', '5', '3', 0
	}
};

