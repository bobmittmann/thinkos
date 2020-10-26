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
 * @file usb/cdc.h
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __USB_CDC_H__
#define __USB_CDC_H__

#include <stdint.h>

/*  Communication device class specification version 1.10 */
#define CDC1_10                                 0x0110


/* CDC Device Descriptor. */
/* Device class code when using the CDC class. */
#define CDC_DEVICE_CLASS               0x02
/* Device subclass code when using the CDC class. */
#define CDC_DEVICE_SUBCLASS            0x00
/* Device protocol code when using the CDC class. */
#define CDC_DEVICE_PROTOCOL            0x00


/* Interface class codes */
#define CDC_INTERFACE_CLASS_COMMUNICATION             0x02
/* Interface class code for a data class interface */
#define CDC_INTERFACE_CLASS_DATA                      0x0A
/* Interface subclass code for a data class interface */
#define CDC_INTERFACE_SUBCLASS_DATA     0x00
/* Protocol code for a data class interface which does not implement any
    particular protocol */
#define CDC_INTERFACE_DATA_PROTOCOL_NONE   0x00

/* Communication interface class subclass codes */
/* usbcdc11.pdf - Section 4.3 - Table 16 */
#define CDC_DIRECT_LINE_CONTROL_MODEL           0x01
#define CDC_ABSTRACT_CONTROL_MODEL              0x02
#define CDC_TELEPHONE_CONTROL_MODEL             0x03
#define CDC_MULTI_CHANNEL_CONTROL_MODEL         0x04
#define CDC_CAPI_CONTROL_MODEL                  0x05
#define CDC_ETHERNET_NETWORKING_CONTROL_MODEL   0x06
#define CDC_ATM_NETWORKING_CONTROL_MODEL        0x07

/* Communication interface class control protocol codes */
/* usbcdc11.pdf - Section 4.4 - Table 17 */
#define CDC_PROTOCOL_COMMON_AT_COMMANDS         0x01

/* Data interface class protocol codes */
/* usbcdc11.pdf - Section 4.7 - Table 19 */
#define CDC_PROTOCOL_ISDN_BRI                   0x30
#define CDC_PROTOCOL_HDLC                       0x31
#define CDC_PROTOCOL_TRANSPARENT                0x32
#define CDC_PROTOCOL_Q921_MANAGEMENT            0x50
#define CDC_PROTOCOL_Q921_DATA_LINK             0x51
#define CDC_PROTOCOL_Q921_MULTIPLEXOR           0x52
#define CDC_PROTOCOL_V42                        0x90
#define CDC_PROTOCOL_EURO_ISDN                  0x91
#define CDC_PROTOCOL_V24_RATE_ADAPTATION        0x92
#define CDC_PROTOCOL_CAPI                       0x93
#define CDC_PROTOCOL_HOST_BASED_DRIVER          0xFD
#define CDC_PROTOCOL_DESCRIBED_IN_PUFD          0xFE

#define CDC_PROTOCOL_NONE                       0x00

/* CDC class-specific request codes */
/* Values of the bRequest field for the various class-specific requests 
   defined in the CDC specification. 
   usbcdc11.pdf - Section 6.2 - Table 45 */
#define CDC_SEND_ENCAPSULATED_COMMAND       0x00
#define CDC_GET_ENCAPSULATED_RESPONSE       0x01
#define CDC_SET_COMM_FEATURE                0x02
#define CDC_GET_COMM_FEATURE                0x03
#define CDC_CLEAR_COMM_FEATURE              0x04
#define CDC_SET_AUX_LINE_STATE              0x10
#define CDC_SET_HOOK_STATE                  0x11
#define CDC_PULSE_SETUP                     0x12
#define CDC_SEND_PULSE                      0x13
#define CDC_SET_PULSE_TIME                  0x14
#define CDC_RING_AUX_JACK                   0x15
#define CDC_SET_LINE_CODING                 0x20
#define CDC_GET_LINE_CODING                 0x21
#define CDC_SET_CONTROL_LINE_STATE          0x22
#define CDC_SEND_BREAK                      0x23
#define CDC_SET_RINGER_PARMS                0x30
#define CDC_GET_RINGER_PARMS                0x31
#define CDC_SET_OPERATION_PARMS             0x32
#define CDC_GET_OPERATION_PARMS             0x33
#define CDC_SET_LINE_PARMS                  0x34
#define CDC_GET_LINE_PARMS                  0x35
#define CDC_DIAL_DIGITS                     0x36
#define CDC_SET_UNIT_PARAMETER              0x37
#define CDC_GET_UNIT_PARAMETER              0x38
#define CDC_CLEAR_UNIT_PARAMETER            0x39
#define CDC_GET_PROFILE                     0x3A
#define CDC_SET_ETHERNET_MULTICAST_FILTERS  0x40
#define CDC_SET_ETHERNET_PMP_FILTER         0x41
#define CDC_GET_ETHERNET_PMP_FILTER         0x42
#define CDC_SET_ETHERNET_PACKET_FILTER      0x43
#define CDC_GET_ETHERNET_STATISTIC          0x44
#define CDC_SET_ATM_DATA_FORMAT             0x50
#define CDC_GET_ATM_DEVICE_STATISTICS       0x51
#define CDC_SET_ATM_DEFAULT_VC              0x52
#define CDC_GET_ATM_VC_STATISTICS           0x53

/* Type values for the bDescriptorType field of functional descriptors */
/* usbcdc11.pdf - Section 5.2.3 - Table 24 */
#define CDC_CS_INTERFACE                    0x24
#define CDC_CS_ENDPOINT                     0x25

/* Type values for the bDescriptorSubtype field of functional */
/*         descriptors */
/* usbcdc11.pdf - Section 5.2.3 - Table 25 */
#define CDC_HEADER                          0x00
#define CDC_CALL_MANAGEMENT                 0x01
#define CDC_ABSTRACT_CONTROL_MANAGEMENT     0x02
#define CDC_DIRECT_LINE_MANAGEMENT          0x03
#define CDC_TELEPHONE_RINGER                0x04
#define CDC_REPORTING_CAPABILITIES          0x05
#define CDC_UNION                           0x06
#define CDC_COUNTRY_SELECTION               0x07
#define CDC_TELEPHONE_OPERATIONAL_MODES     0x08
#define CDC_USB_TERMINAL                    0x09
#define CDC_NETWORK_CHANNEL                 0x0A
#define CDC_PROTOCOL_UNIT                   0x0B
#define CDC_EXTENSION_UNIT                  0x0C
#define CDC_MULTI_CHANNEL_MANAGEMENT        0x0D
#define CDC_CAPI_CONTROL_MANAGEMENT         0x0E
#define CDC_ETHERNET_NETWORKING             0x0F
#define CDC_ATM_NETWORKING                  0x10

/* Control signal bitmap values for the SetControlLineState request */
/* usbcdc11.pdf - Section 6.2.14 - Table 51 */
#define CDC_DTE_PRESENT                     (1 << 0)
#define CDC_ACTIVATE_CARRIER                (1 << 1)

/* Serial state notification bitmap values. */
/* usbcdc11.pdf - Section 6.3.5 - Table 69 */
#define CDC_SERIAL_STATE_OVERRUN            (1 << 6)
#define CDC_SERIAL_STATE_PARITY             (1 << 5)
#define CDC_SERIAL_STATE_FRAMING            (1 << 4)
#define CDC_SERIAL_STATE_RING               (1 << 3)
#define CDC_SERIAL_STATE_BREAK              (1 << 2)
#define CDC_SERIAL_STATE_TX_CARRIER         (1 << 1)
#define CDC_SERIAL_STATE_RX_CARRIER         (1 << 0)

/* Notification requests */
/* usbcdc11.pdf - Section 6.3 - Table 68 */
#define CDC_NOTIFICATION_NETWORK_CONNECTION 0x00
#define CDC_NOTIFICATION_SERIAL_STATE       0x20



/* Device handles call management itself. */
#define CDC_CALL_MANAGEMENT_SELF (1 << 0)
/* Device can exchange call management information over a 
   Data class interface. */
#define CDC_CALL_MANAGEMENT_DATA (1 << 1)


/* usb_cdc_acm USB CDC ACM Capabilities */
/* Device supports the request combination of SetCommFeature, ClearCommFeature
   and GetCommFeature. */
#define CDC_ACM_COMMFEATURE          (1 << 0)
/* Device supports the request combination of SetLineCoding, GetLineCoding and
   SetControlLineState. */
#define CDC_ACM_LINE                 (1 << 1)
/* Device supports the SendBreak request. */
#define CDC_ACM_SENDBREAK            (1 << 2)
/* Device supports the NetworkConnection notification. */
#define CDC_ACM_NETWORKCONNECTION    (1 << 3)


#define USB_CDC_NOTIFICATION 0xa1

/* Header functional descriptor 
   This header must precede any list of class-specific descriptors.
   usbcdc11.pdf - Section 5.2.3.1 */
struct cdc_descriptor_header {
	/* Size of this descriptor in bytes */
	uint8_t bFunctionLength;
	/* CS_INTERFACE descriptor type */
	uint8_t bDescriptorType;
	/* Header functional descriptor subtype */
	uint8_t bDescriptorSubtype;
	/* USB CDC specification release version */
	uint16_t bcdCDC;
} __attribute__((__packed__));

/* Abstract control management functional descriptor 
   Describes the command supported by the communication interface class
   with the Abstract Control Model subclass code. 
   usbcdc11.pdf - Section 5.2.3.3 */
struct cdc_descriptor_abstract_control_management {
	/* Size of this descriptor in bytes */
	uint8_t bFunctionLength;
	/* CS_INTERFACE descriptor type */
	uint8_t bDescriptorType;
	/* Abstract control management functional descriptor subtype */
	uint8_t bDescriptorSubtype;
	/* Capabilities supported by this configuration */
	uint8_t bmCapabilities;
} __attribute__((__packed__));

/* Union functional descriptors 
   Describes the relationship between a group of interfaces that can
   be considered to form a functional unit. */
/* usbcdc11.pdf - Section 5.2.3.8 */
struct cdc_descriptor_union  {
	/* Size of this descriptor in bytes */
	uint8_t bFunctionLength;
	/* CS_INTERFACE descriptor type */
	uint8_t bDescriptorType;
	/* Union functional descriptor subtype */
	uint8_t bDescriptorSubtype;
	/* The interface number designated as master */
	uint8_t bMasterInterface;
} __attribute__((__packed__));

/* Call management functional descriptor 
   Describes the processing of calls for the communication class interface. 
   usbcdc11.pdf - Section 5.2.3.2 */
struct cdc_descriptor_call_management {
	/* Size of this descriptor in bytes */
	uint8_t bFunctionLength;
	/* CS_INTERFACE descriptor type */
	uint8_t bDescriptorType;
	/* Call management functional descriptor subtype */
	uint8_t bDescriptorSubtype;
	/* The capabilities that this configuration supports */
	uint8_t bmCapabilities;
	/* Interface number of the data class interface used for call management */
	/* (optional) */
	uint8_t bDataInterface;
} __attribute__((__packed__));

/* Union functional descriptors with one slave interface */

struct cdc_descriptor_union_1slave  {
	/* Size of this descriptor in bytes */
	uint8_t bFunctionLength;
	/* CS_INTERFACE descriptor type */
	uint8_t bDescriptorType;
	/* Union functional descriptor subtype */
	uint8_t bDescriptorSubtype;
	/* The interface number designated as master */
	uint8_t bMasterInterface;
	/* Slave interface 0 */
	uint8_t bSlaveInterface;
} __attribute__((__packed__));

/* Line coding structure 
   Format of the data returned when a GetLineCoding request is received */
/* usbcdc11.pdf - Section 6.2.13 */
struct cdc_line_coding {
	/* Data terminal rate in bits per second */
	uint32_t dwDTERate;
	/* Number of stop bits */
	uint8_t bCharFormat;
	/* Parity bit type */
	uint8_t bParityType;
	/* Number of data bits */
	uint8_t bDataBits;
} __attribute__((__packed__));

/* Communication Interface Class notifications that the device uses to 
   notify the host of interface, or endpoint events. */

struct cdc_notification {
	/* Characteristics of the request */
	uint8_t bmRequestType;
	/* Particular notification */
	uint8_t bNotification;
	/* Request-specific parameter */
	uint16_t wValue;
	/* Request-specific parameter */
	uint16_t wIndex;
	/* Length of data for the data phase */
	uint16_t wLength;
	/* data */
	uint8_t bData[];
} __attribute__((__packed__));


/* Serial state notification */
struct serial_state {
	/* Characteristics of the request */
	uint8_t bmRequestType;
	/* Particular notification */
	uint8_t bNotification;
	/* Request-specific parameter */
	uint16_t wValue;
	/* Request-specific parameter */
	uint16_t wIndex;
	/* Length of data for the data phase */
	uint16_t wLength;
	/* UART State Bitmap */
	uint16_t bRxCarrier: 1; /* DCD */
	uint16_t bTxCarrier: 1; /* DSR */
	uint16_t bBreak: 1;
	uint16_t bRingSignal: 1;

	uint16_t bFraming: 1;
	uint16_t bParity: 1;
	uint16_t bOverRun: 1;
	uint16_t res: 9;
} __attribute__((__packed__));

/* CDC Class Specific Request Code */
#define SEND_ENCAPSULATED_COMMAND     0x0021
#define GET_ENCAPSULATED_RESPONSE     0x01a1

#define SET_LINE_CODING               0x2021
#define GET_LINE_CODING               0x21a1
#define SET_CONTROL_LINE_STATE        0x2221
#define SEND_BREAK                    0x2321

#endif /* __USB_CDC_H__ */

