/* 
 * Copyright(c) 2009-2012 BORESTE (www.boreste.com). All Rights Reserved.
 *
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file sys/usb.h
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYS_USB_H__
#define __SYS_USB_H__

#include <stdint.h>

enum usb_state {
	USB_STATE_ATTACHED,
	USB_STATE_POWERED,
	USB_STATE_DEFAULT,
	USB_STATE_ADDRESS,
	USB_STATE_CONFIGURED,
	USB_STATE_SUSPENDED
};


#define STD_CLEAR_FEATURE_DEVICE      0x0100
#define STD_CLEAR_FEATURE_INTERFACE   0x0101
#define STD_CLEAR_FEATURE_ENDPOINT    0x0102

#define STD_SET_FEATURE_DEVICE        0x0300
#define STD_SET_FEATURE_INTERFACE     0x0301
#define STD_SET_FEATURE_ENDPOINT      0x0302

#define STD_SET_ADDRESS               0x0500
#define STD_GET_DESCRIPTOR            0x0680
#define STD_SET_DESCRIPTOR            0x0700
#define STD_GET_CONFIGURATION         0x0880
#define STD_SET_CONFIGURATION         0x0900
#define STD_GET_INTERFACE             0x0A81
#define STD_SET_INTERFACE             0x0B01
#define STD_SYNCH_FRAME               0x0C82



struct usb_device_satus {
	uint16_t bRemoteWakeuo: 1; 
	uint16_t bSelfPowered: 1;
	uint16_t res: 14;
} __attribute__((__packed__));

#define USB_DEVICE_STATUS_REMOTE_WAKEUP (1 << 1)
#define USB_DEVICE_STATUS_SELF_POWERED (1 << 0)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SYS_USB_H__ */

