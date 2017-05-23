/* 
 * File:	 usbaudio.h
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


#include "usbaudio-i.h"
#include <sys/param.h>
#include <string.h>

#define ATMEL_VCOM_PRODUCT_ID 0x6119
#define ST_VCOM_PRODUCT_ID 0x5740

#ifndef CDC_ACM_PRODUCT_ID
#define CDC_ACM_PRODUCT_ID ST_VCOM_PRODUCT_ID 
#endif

#define AUDIO_OUT_EP                                  0x01
#define USB_AUDIO_CONFIG_DESC_SIZ                     109
#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

#define AUDIO_OUT_STREAMING_CTRL                      0x02


#if 0
USBD_ClassTypeDef  USBD_AUDIO =
{
	USBD_AUDIO_Init,
	USBD_AUDIO_DeInit,
	USBD_AUDIO_Setup,
	USBD_AUDIO_EP0_TxReady,
	USBD_AUDIO_EP0_RxReady,
	USBD_AUDIO_DataIn,
	USBD_AUDIO_DataOut,
	USBD_AUDIO_SOF,
	USBD_AUDIO_IsoINIncomplete,
	USBD_AUDIO_IsoOutIncomplete,
	USBD_AUDIO_GetCfgDesc,
	USBD_AUDIO_GetCfgDesc,
	USBD_AUDIO_GetCfgDesc,
	USBD_AUDIO_GetDeviceQualifierDesc,
};
#endif

#if 0
const struct usb_descriptor_device usbaudio_desc_dev = {
	/* Size of this descriptor in bytes */
	sizeof(struct usb_descriptor_device),
	/* DEVICE descriptor type */
	USB_DESCRIPTOR_DEVICE,
	/* USB specification release number */
	USB1_10,
	/* Class code */
	USB_CLASS_COMMUNICATION,
	/* Subclass code */
	0x00,
	/* Protocol code */
	0x00,
	/* Control endpoint 0 max. packet size */
	EP0_MAX_PKT_SIZE,
	/* Vendor ID */
	USB_VENDOR_ST,
	/* Product ID */
	CDC_ACM_PRODUCT_ID,
	/* Device release number */
	0x0002,
	/* Index of manufacturer string descriptor */
	0x01,
	/* Index of product string descriptor */
	0x02,
	/* Index of S.N.  string descriptor */
	0x03,
	/* Number of possible configurations */
	0x01
};

/* Configuration 1 descriptor */
const struct usbaudio_descriptor_config usbaudio_desc_cfg = {
	{
		/* Size of this descriptor in bytes */
		sizeof(struct usb_descriptor_configuration),
		/* CONFIGURATION descriptor type */
		USB_DESCRIPTOR_CONFIGURATION,
		/* Total length of data returned for this configuration */
		sizeof(struct usbaudio_descriptor_config),
		/* Number of interfaces for this configuration */
		2,
		/* Value to use as an argument for the
		   Set Configuration request to select this configuration */
		1,
		/* Index of string descriptor describing this configuration */
		0,
		/* Configuration characteristics - attributes */
		USB_CONFIG_SELF_NOWAKEUP,
		/* Maximum power consumption of the device */
		USB_POWER_MA(100)
	},
	/* Communication Class Interface Descriptor Requirement */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct usb_descriptor_interface),
		/* INTERFACE descriptor type */
		USB_DESCRIPTOR_INTERFACE,
		/* Number of this interface */
		0,
		/* Value used to select this alternate setting */
		0,
		/* Number of endpoints used by this interface
		   (excluding endpoint zero) */
		1,
		/* Class code */
		CDC_INTERFACE_COMMUNICATION,
		/* Sub-class */
		CDC_ABSTRACT_CONTROL_MODEL,
		/* Protocol code: (V.25ter, Common AT commands)*/
		CDC_PROTOCOL_COMMON_AT_COMMANDS,
		/* Index of string descriptor describing this interface */
		0x04
	},
	/* Header Functional Descriptor */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct cdc_header_descriptor),
		/* CS_INTERFACE descriptor type */
		CDC_CS_INTERFACE,
		/* Header functional descriptor subtype */
		CDC_HEADER,
		/* USB CDC specification release version */
		CDC1_10
	},
	/* Call Management Functional Descriptor */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct cdc_call_management_descriptor),
		/* CS_INTERFACE descriptor type */
		CDC_CS_INTERFACE,
		/* Call management functional descriptor subtype */
		CDC_CALL_MANAGEMENT,
		/* The capabilities that this configuration supports */
		1, /* D1 + D0 */
		/* Interface number of the data class
		   interface used for call management */
		1
	},
	/* Abstract Control Management Functional Descriptor */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct cdc_abstract_control_management_descriptor),
		/* CS_INTERFACE descriptor type */
		CDC_CS_INTERFACE,
		/* Abstract control management functional descriptor subtype */
		CDC_ABSTRACT_CONTROL_MANAGEMENT,
		/* Capabilities supported by this configuration */
		0x06
	},
	/* Union Functional Descriptor */
	{
		{
			/* Size of this descriptor in bytes */
			sizeof(struct cdc_union_1slave_descriptor),
			/* CS_INTERFACE descriptor type */
			CDC_CS_INTERFACE,
			/* Union functional descriptor subtype */
			CDC_UNION,
			/* The interface number designated as master */
			0
		},
		/* The interface number designated as first slave */
		{ 1 }
	},
	/* Endpoint 3 descriptor */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct usb_descriptor_endpoint),
		/* ENDPOINT descriptor type */
		USB_DESCRIPTOR_ENDPOINT,
		/* Address of the endpoint on the USB device */
		USB_ENDPOINT_IN + EP_INT_ADDR,
		/* Endpoint attributes when configured */
		ENDPOINT_TYPE_INTERRUPT,
		/* Maximum packet size this endpoint is capable
		   of sending or receiving */
		CDC_EP_INT_MAX_PKT_SIZE,
		/* Interval for polling endpoint (ms) */
		100
	},
	/* Data Class Interface Descriptor Requirement */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct usb_descriptor_interface),
		/* INTERFACE descriptor type */
		USB_DESCRIPTOR_INTERFACE,
		/* Number of this interface */
		1,
		/* Value used to select this alternate setting */
		0,
		/* Number of endpoints used by this interface
		   (excluding endpoint zero) */
		2,
		/* Class code */
		CDC_INTERFACE_DATA,
		/* Sub-class */
		0,
		/* Protocol code */
		0,
		/* Index of string descriptor describing this interface */
		0
	},
	/* First alternate setting */
	/* Endpoint 1 descriptor */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct usb_descriptor_endpoint),
		/* ENDPOINT descriptor type */
		USB_DESCRIPTOR_ENDPOINT,
		/* Address of the endpoint on the USB device */
		USB_ENDPOINT_OUT + EP_OUT_ADDR,
		/* Endpoint attributes when configured */
		ENDPOINT_TYPE_BULK,
		/* Maximum packet size this endpoint is capable of
		   sending or receiving */
		CDC_EP_OUT_MAX_PKT_SIZE,
		/* Interval for polling endpoint for data transfers */
		0x0
	},
	/* Endpoint 2 descriptor */
	{
		/* Size of this descriptor in bytes */
		sizeof(struct usb_descriptor_endpoint),
		/* ENDPOINT descriptor type */
		USB_DESCRIPTOR_ENDPOINT,
		/* Address of the endpoint on the USB device */
		USB_ENDPOINT_IN + EP_IN_ADDR,
		/* Endpoint attributes when configured */
		ENDPOINT_TYPE_BULK,
		/* Maximum packet size this endpoint is capable of
		   sending or receiving */
		CDC_EP_IN_MAX_PKT_SIZE,
		/* Interval for polling endpoint for data transfers */
		0x0
	}
};
#endif



#define USB_LEN_DEV_QUALIFIER_DESC                     0x0A
#define USB_MAX_EP0_SIZE                                  64


#define  SWAPBYTE(addr)        (((uint16_t)(*((uint8_t *)(addr)))) + \
								                               (((uint16_t)(*(((uint8_t *)(addr)) + 1))) << 8))

#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))



/* USB AUDIO device Configuration Descriptor */
//const struct usbaudio_descriptor_config usbaudio_desc_cfg = {
const uint8_t usb_audio_cfgdesc[USB_AUDIO_CONFIG_DESC_SIZ] = {
	/* Configuration 1 */
	0x09,                                 /* bLength */
	USB_DESCRIPTOR_CONFIGURATION,          /* bDescriptorType */
	LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength  109 bytes*/
	HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),
	0x02,                                 /* bNumInterfaces */
	0x01,                                 /* bConfigurationValue */
	0x00,                                 /* iConfiguration */
	0xC0,                                 /* bmAttributes  BUS Powred*/
	0x32,                                 /* bMaxPower = 100 mA*/
	/* 09 byte*/

	/* USB Speaker Standard interface descriptor */
	AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
	USB_DESCRIPTOR_INTERFACE,              /* bDescriptorType */
	0x00,                                 /* bInterfaceNumber */
	0x00,                                 /* bAlternateSetting */
	0x00,                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
	0x00,                                 /* iInterface */
	/* 09 byte*/

	/* USB Speaker Class-specific AC Interface Descriptor */
	AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
	AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
	0x00,          /* 1.00 */             /* bcdADC */
	0x01,
	0x27,                                 /* wTotalLength = 39*/
	0x00,
	0x01,                                 /* bInCollection */
	0x01,                                 /* baInterfaceNr */
	/* 09 byte*/

	/* USB Speaker Input Terminal Descriptor */
	AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
	AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
	0x01,                                 /* bTerminalID */
	0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
	0x01,
	0x00,                                 /* bAssocTerminal */
	0x01,                                 /* bNrChannels */
	0x00,                                 /* wChannelConfig 0x0000  Mono */
	0x00,
	0x00,                                 /* iChannelNames */
	0x00,                                 /* iTerminal */
	/* 12 byte*/

	/* USB Speaker Audio Feature Unit Descriptor */
	0x09,                                 /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
	AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
	AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
	0x01,                                 /* bSourceID */
	0x01,                                 /* bControlSize */
	AUDIO_CONTROL_MUTE,// |AUDIO_CONTROL_VOLUME, /* bmaControls(0) */
	0,                                    /* bmaControls(1) */
	0x00,                                 /* iTerminal */
	/* 09 byte*/

	/*USB Speaker Output Terminal Descriptor */
	0x09,      /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
	AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
	0x03,                                 /* bTerminalID */
	0x01,                                 /* wTerminalType  0x0301*/
	0x03,
	0x00,                                 /* bAssocTerminal */
	0x02,                                 /* bSourceID */
	0x00,                                 /* iTerminal */
	/* 09 byte*/

	/* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
	/* Interface 1, Alternate Setting 0                                             */
	AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
	USB_DESCRIPTOR_INTERFACE,        /* bDescriptorType */
	0x01,                                 /* bInterfaceNumber */
	0x00,                                 /* bAlternateSetting */
	0x00,                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
	0x00,                                 /* iInterface */
	/* 09 byte*/

	/* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
	/* Interface 1, Alternate Setting 1                                           */
	AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
	USB_DESCRIPTOR_INTERFACE,        /* bDescriptorType */
	0x01,                                 /* bInterfaceNumber */
	0x01,                                 /* bAlternateSetting */
	0x01,                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
	0x00,                                 /* iInterface */
	/* 09 byte*/

	/* USB Speaker Audio Streaming Interface Descriptor */
	AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
	AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
	0x01,                                 /* bTerminalLink */
	0x01,                                 /* bDelay */
	0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
	0x00,
	/* 07 byte*/

	/* USB Speaker Audio Type III Format Interface Descriptor */
	0x0B,                                 /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
	AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
	AUDIO_FORMAT_TYPE_III,                /* bFormatType */
	0x02,                                 /* bNrChannels */
	0x02,                                 /* bSubFrameSize :  2 Bytes per frame (16bits) */
	16,                                   /* bBitResolution (16-bits per sample) */
	0x01,                                 /* bSamFreqType only one frequency supported */
	AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),         /* Audio sampling frequency coded on 3 bytes */
	/* 11 byte*/

	/* Endpoint 1 - Standard Descriptor */
	AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
	USB_DESCRIPTOR_ENDPOINT,               /* bDescriptorType */
	AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
	ENDPOINT_TYPE_ISOCHRONOUS,            /* bmAttributes */
	AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
	0x01,                                 /* bInterval */
	0x00,                                 /* bRefresh */
	0x00,                                 /* bSynchAddress */
	/* 09 byte*/

	/* Endpoint - Audio Streaming Descriptor*/
	AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
	AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
	AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
	0x00,                                 /* bmAttributes */
	0x00,                                 /* bLockDelayUnits */
	0x00,                                 /* wLockDelay */
	0x00,
	/* 07 byte*/
} ;

/* USB Standard Device Descriptor */
const uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] = {
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESCRIPTOR_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};

