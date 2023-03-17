/* 
 * File:	 usb-cdc.c
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

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <sys/serial.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <vt100.h>

#include <sys/usb-dev.h>
#include <sys/usb-cdc.h>

#include <sys/dcclog.h>

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#if (THINKOS_ENABLE_MONITOR)

#ifndef THINKOS_MONITOR_ENABLE_COMM_STATS
#define THINKOS_MONITOR_ENABLE_COMM_STATS 0
#endif

#ifndef THINKOS_MONITOR_ENABLE_COMM_BRK
#define THINKOS_MONITOR_ENABLE_COMM_BRK    1
#endif

/* Enable USB2.0 */
#ifdef THINKOS_MONITOR_ENABLE_USB2_00 
#define THINKOS_MONITOR_ENABLE_USB2_00 0
#endif

/* Enable USB2.0 high speed device */
#ifdef THINKOS_MONITOR_ENABLE_USB_HS
#define THINKOS_MONITOR_ENABLE_USB_HS 0
#endif


#define EP0_ADDR 0

#if (THINKOS_MONITOR_ENABLE_USB_HS)
#define EP0_MAX_PKT_SIZE 64
#else
#define EP0_MAX_PKT_SIZE 64
#endif

#define EP_OUT0_ADDR 1
#define EP_IN0_ADDR  2
#define EP_INT0_ADDR 3


#ifndef CDC_EP_OUT_MAX_PKT_SIZE
#define CDC_EP_OUT_MAX_PKT_SIZE 64
#endif

#ifndef CDC_EP_IN_MAX_PKT_SIZE 
#define CDC_EP_IN_MAX_PKT_SIZE 64
#endif

#ifndef CDC_EP_INT_MAX_PKT_SIZE 
#define CDC_EP_INT_MAX_PKT_SIZE 32
#endif

#ifndef USB_FEATURE_DEVICE_ENABLED
#define USB_FEATURE_DEVICE_ENABLED 0
#endif

#ifndef USB_STATUS_DEVICE_ENABLED
#define USB_STATUS_DEVICE_ENABLED 0
#endif

#ifndef USB_FEATURE_INTERFACE_ENABLED
#define USB_FEATURE_INTERFACE_ENABLED 0
#endif

#ifndef USB_STATUS_INTERFACE_ENABLED
#define USB_STATUS_INTERFACE_ENABLED 0
#endif

#ifndef USB_FEATURE_ENDPOINT_ENABLED
#define USB_FEATURE_ENDPOINT_ENABLED 0
#endif

#ifndef USB_STATUS_ENDPOINT_ENABLED
#define USB_STATUS_ENDPOINT_ENABLED 0
#endif


#ifndef USB_SYNCH_FRAME_ENABLED
#define USB_SYNCH_FRAME_ENABLED 0
#endif

struct cdc_acm_descriptor_config {
	struct usb_descriptor_configuration cfg;
	struct usb_descriptor_interface comm_if;
	struct cdc_descriptor_header hdr;
	struct cdc_descriptor_call_management cm;
	struct cdc_descriptor_abstract_control_management acm;
	struct cdc_descriptor_union_1slave un;
	struct usb_descriptor_endpoint ep_int;
	struct usb_descriptor_interface if_data;
	struct usb_descriptor_endpoint ep_out;
	struct usb_descriptor_endpoint ep_in;
} __attribute__((__packed__));

struct cdc_acm_descriptor_set {
	struct usb_descriptor_configuration cfg;
	struct usb_descriptor_interface if0;
	struct cdc_descriptor_header hdr0;
	struct cdc_descriptor_call_management cm0;
	struct cdc_descriptor_abstract_control_management acm0;
	struct cdc_descriptor_union_1slave un0;
	struct usb_descriptor_endpoint ep_int0;
	struct usb_descriptor_interface if_data0;
	struct usb_descriptor_endpoint ep_out0;
	struct usb_descriptor_endpoint ep_in0;
} __attribute__((__packed__));

#define ATMEL_VCOM_PRODUCT_ID 0x6119
#define ST_VCOM_PRODUCT_ID    0x5740

#ifndef CDC_ACM_PRODUCT_ID
#define CDC_ACM_PRODUCT_ID ST_VCOM_PRODUCT_ID 
#endif

static const struct usb_descriptor_device cdc_acm_desc_dev = {
	/* Size of this descriptor in bytes */
	.length = sizeof(struct usb_descriptor_device),
	/* DEVICE descriptor type */
	.type = USB_DESCRIPTOR_DEVICE,
	/* USB specification release number */
#if (THINKOS_MONITOR_ENABLE_USB2_00)
/*	The DEVICE descriptor of a high-speed capable device has a version 
	number of 2.0 (0200H). If the device is full-speed only or low-speed 
	only, this version number indicates that it will respond correctly to 
	a request for the device_qualifier desciptor (i.e., it will respond 
	with a request error). */
	.usb_release = USB2_00,
#else
	.usb_release = USB1_10,
#endif
	/* Class code */
	.dev_class = USB_CLASS_COMMUNICATION,
	/* Subclass code */
	.dev_subclass = 0x00,
	/* Protocol code */
	.dev_proto = 0x00,
	/* Control endpoint 0 max. packet size */
	/* If the device is operating at high-speed, the bMaxPacketSize0 field 
	   must be 64 indicating a 64 byte maximum packet. High-speed operation 
	   does not allow other maximum packet sizes for the control 
	   endpoint (endpoint 0). */
	.max_pkt_sz0 = EP0_MAX_PKT_SIZE,
	/* Vendor ID */
	.vendor_id = USB_VENDOR_ST,
	/* Product ID */
	.product_id = CDC_ACM_PRODUCT_ID,
	/* Device release number */
	.dev_release = 2,
	/* Index of manufacturer string descriptor */
	.manufacturer = 1,
	/* Index of product string descriptor */
	.product = 2,
	/* Index of S.N.  string descriptor */
	.serial_num = 3,
	/* Number of possible configurations */
	.num_of_conf = 1
};

#if (THINKOS_MONITOR_ENABLE_USB2_00)
/* device qualifier structure provide information on a high-speed
   capable device if the device was operating at the other speed.
   see usb_20.pdf - Section 9.6.2 */
static const struct usb_descriptor_device_qualifier cdc_acm_desc_qual = {
	/* Size of this descriptor in bytes */
	.length = sizeof(struct usb_descriptor_device_qualifier),
	/* DEVICE_QUALIFIER descriptor type */
	.descriptortype = USB_DESCRIPTOR_DEVICE_QUALIFIER,
	/* USB specification release number */
	.usb = USB2_00,
	/* Class code */
	.deviceclass = CDC_DEVICE_CLASS,
	/* Sub-class code */
	.devicesubclass = CDC_DEVICE_SUBCLASS,
	/* Protocol code */
	.deviceprotocol = CDC_DEVICE_PROTOCOL,
	/* Control endpoint 0 max. packet size */
	.maxpacketsize0 = 0,
	/* Number of possible configurations */
	.numconfigurations = 0,
	/* Reserved for future use, must be 0 */
	.reserved = 0 
};
#endif

#define CDC_IF0 0

/* Configuration 1 descriptor */
static const struct cdc_acm_descriptor_set monitor_usb_cdc_acm_desc_cfg = {
	.cfg = {
		.length = sizeof(struct usb_descriptor_configuration),
		.type = USB_DESCRIPTOR_CONFIGURATION,
		.total_length = sizeof(struct cdc_acm_descriptor_set),
		.num_interfaces = 2,
		.configuration_value = 1,
		.configuration = 0,
		.attributes = USB_CONFIG_SELF_WAKEUP,
		.max_power = USB_POWER_MA(20)
	},
	/* CDC 0 */
	/* Communication Class Interface Descriptor */
	.if0 = {
		.length = sizeof(struct usb_descriptor_interface),
		.type = USB_DESCRIPTOR_INTERFACE,
		.number = CDC_IF0,
		.alt_setting = 0,
		.num_endpoints = 1,
		.ceclass = CDC_INTERFACE_CLASS_COMMUNICATION,
		.esubclass = CDC_ABSTRACT_CONTROL_MODEL,
		.protocol = CDC_PROTOCOL_NONE,
		.interface = 4
	},
	/* Header Functional Descriptor */
	.hdr0 = {
		.bFunctionLength = sizeof(struct cdc_descriptor_header),
		.bDescriptorType = CDC_CS_INTERFACE,
		.bDescriptorSubtype = CDC_HEADER,
		.bcdCDC = CDC1_10
	},
	/* Call Management Functional Descriptor */
	.cm0 = {
		.bFunctionLength = sizeof(struct cdc_descriptor_call_management),
		.bDescriptorType = CDC_CS_INTERFACE,
		.bDescriptorSubtype = CDC_CALL_MANAGEMENT,
		.bmCapabilities = CDC_CALL_MANAGEMENT_SELF,
		/* interface used for call management */
		.bDataInterface = CDC_IF0 + 1
	},
	/* Abstract Control Management Functional Descriptor */
	.acm0 = {
		.bFunctionLength = 
			sizeof(struct cdc_descriptor_abstract_control_management),
		.bDescriptorType = CDC_CS_INTERFACE,
		.bDescriptorSubtype = CDC_ABSTRACT_CONTROL_MANAGEMENT,
		.bmCapabilities = CDC_ACM_COMMFEATURE 
#if 0
			+ CDC_ACM_LINE
#endif
#if (THINKOS_MONITOR_ENABLE_COMM_BRK)
			+ CDC_ACM_SENDBREAK
#endif
	},
	/* Union Functional Descriptor */
	.un0 = {
		.bFunctionLength = sizeof(struct cdc_descriptor_union_1slave),
		.bDescriptorType = CDC_CS_INTERFACE,
		.bDescriptorSubtype = CDC_UNION,
		.bMasterInterface = CDC_IF0,
		.bSlaveInterface = CDC_IF0 + 1
	},
	/* Endpoint 3 descriptor */
	.ep_int0 = {
		.length = sizeof(struct usb_descriptor_endpoint),
		.type = USB_DESCRIPTOR_ENDPOINT,
		.endpointaddress= USB_ENDPOINT_IN + EP_INT0_ADDR,
		.attributes = ENDPOINT_TYPE_INTERRUPT,
		.maxpacketsize = CDC_EP_INT_MAX_PKT_SIZE,
		.interval = 100
	},
	/* Data Class Interface Descriptor Requirement */
	.if_data0 = {
		.length = sizeof(struct usb_descriptor_interface),
		.type = USB_DESCRIPTOR_INTERFACE,
		.number = CDC_IF0 + 1,
		.alt_setting = 0,
		.num_endpoints = 2,
		.ceclass =  CDC_INTERFACE_CLASS_DATA,
		.esubclass = CDC_INTERFACE_SUBCLASS_DATA,
		.protocol = CDC_PROTOCOL_NONE,
		.interface = CDC_IF0
	},
	/* First alternate setting */
	/* Endpoint 1 descriptor */
	.ep_out0 = {
		.length = sizeof(struct usb_descriptor_endpoint),
		.type = USB_DESCRIPTOR_ENDPOINT,
		.endpointaddress = USB_ENDPOINT_OUT + EP_OUT0_ADDR,
		.attributes = ENDPOINT_TYPE_BULK,
		.maxpacketsize = CDC_EP_OUT_MAX_PKT_SIZE,
		.interval = 0
	},
	/* Endpoint 2 descriptor */
	.ep_in0 = {
		.length = sizeof(struct usb_descriptor_endpoint),
		.type = USB_DESCRIPTOR_ENDPOINT,
		.endpointaddress= USB_ENDPOINT_IN + EP_IN0_ADDR,
		.attributes = ENDPOINT_TYPE_BULK,
		.maxpacketsize = CDC_EP_IN_MAX_PKT_SIZE,
		.interval = 0
	}

};

#define CDC_STOP_BITS_1   (0 << 0)
#define CDC_STOP_BITS_1_5 (1 << 0)
#define CDC_STOP_BITS_2   (2 << 0)
#define CDC_STOP_BITS_ERR (3 << 0)
#define CDC_CHAR_FORMAT_SET(VAL)  (((VAL) & 0x3) << 0)
#define CDC_CHAR_FORMAT_GET(VAL)  (((VAL) >> 0) & 0x3)
#define CDC_STOP_BITS_MAX 2

#define CDC_PARITY_NONE   (0 << 2)
#define CDC_PARITY_ODD    (1 << 2)
#define CDC_PARITY_EVEN   (2 << 2)
#define CDC_PARITY_MARK   (3 << 2)
#define CDC_PARITY_SPACE  (4 << 2)
#define CDC_PARITY_ERR    (7 << 2)
#define CDC_PARITY_SET(VAL)  (((VAL) & 0x7) << 2)
#define CDC_PARITY_GET(VAL)  (((VAL) >> 2) & 0x7)
#define CDC_PARITY_MAX    4

#define CDC_DATA_BITS_5   (0 << 5)
#define CDC_DATA_BITS_6   (1 << 5)
#define CDC_DATA_BITS_7   (2 << 5)
#define CDC_DATA_BITS_8   (3 << 5)
#define CDC_DATA_BITS_16  (4 << 5)
#define CDC_DATA_BITS_ERR (7 << 5)
#define CDC_DATA_BITS_SET(VAL)  data_bit_lut[(VAL) & 0x1f]
#define CDC_DATA_BITS_GET(VAL)  data_bit_rev_lut[((VAL) >> 5) & 0x7]
#define CDC_DATA_BITS_MAX 16


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

/* LangID = 0x0409: U.S. English */
const struct usb_descriptor_string monitor_usb_language_english_us = {
	4, USB_DESCRIPTOR_STRING, { 0x0409 }
};

const struct usb_descriptor_string monitor_usb_stmicro_str = {
	8 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'S', 'T', 'M', 'i', 'c', 'r', 'o', 0
	}
};

/* Interface 0: "ST VCOM" */
const struct usb_descriptor_string monitor_usb_st_vcom_str = {
	8 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'S', 'T', ' ', 'V', 'C', 'O', 'M', 0
	}
};

const struct usb_descriptor_string monitor_usb_thinkos_str = {
	8 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'T', 'h', 'i', 'n', 'k', 'O', 'S', 0
	}
};

const struct usb_descriptor_string monitor_usb_serial_num_str = {
	5 * 2 + 4, USB_DESCRIPTOR_STRING, {
		'1', '1', '1', '1', 0
	}
};

#pragma GCC diagnostic pop

//"USB Serial (CDC) Generic Device"

static const struct usb_descriptor_string * const monitor_usb_cdc_acm_str[] = {
	&monitor_usb_language_english_us,
	&monitor_usb_stmicro_str,
	&monitor_usb_thinkos_str,
	&monitor_usb_serial_num_str,
	&monitor_usb_st_vcom_str 
};

#define USB_STRCNT() (sizeof(monitor_usb_cdc_acm_str) / sizeof(uint8_t *))

static const struct cdc_line_coding monitor_usb_usb_cdc_lc = {
    .dwDTERate = 38400,
    .bCharFormat = 0,
    .bParityType = 0,
    .bDataBits = 8
};

#if (USB_STATUS_DEVICE_ENABLED)
static const uint16_t monitor_usb_device_status = 
	USB_DEVICE_STATUS_SELF_POWERED;
static const uint16_t monitor_usb_interface_status = 0x0000;
#endif

#define ACM_USB_SUSPENDED (1 << 1)
#define ACM_CONNECTED     (1 << 2)

struct usb_cdc_acm_dev {
	/* underling USB device */
	struct usb_dev * usb;
	/* endpoints handlers */
	uint8_t ctl_ep;
	uint8_t in_ep;
	uint8_t out_ep;
	uint8_t int_ep;

#define CDC_CTL_BUF_LEN 16
	/* control endpoint buffer */
	uint32_t ctl_buf[CDC_CTL_BUF_LEN / 4];

	uint8_t configured;
	volatile uint8_t shadow; /* shadow (toggle) status bits */
	volatile uint8_t status; /* device status */
	volatile uint8_t rx_paused;

	volatile uint32_t rx_seq; 
	volatile uint32_t rx_ack; 

#define CDC_RX_BUF_SIZE CDC_EP_OUT_MAX_PKT_SIZE
	uint8_t rx_buf[CDC_RX_BUF_SIZE];

#if (THINKOS_MONITOR_ENABLE_COMM_STATS)
	struct {
		uint32_t tx_octet;
		uint32_t rx_octet;
		uint32_t tx_pkt;
		uint32_t rx_pkt;
	} stats;
#endif

};

struct usb_class_if {
	struct usb_cdc_acm_dev dev;
};

static int monitor_usb_cdc_acm_recv(struct usb_cdc_acm_dev * dev)
{
	uint32_t seq;
	uint32_t ack;
	int free;
	int cnt;

	seq = dev->rx_seq;
	ack = dev->rx_ack;
	if ((free = CDC_RX_BUF_SIZE - (int32_t)(seq - ack)) > 0) {
		int pos;

		pos = seq % CDC_RX_BUF_SIZE;
		if (pos == 0) {
			int n;

			n = usb_dev_ep_pkt_recv(dev->usb, dev->out_ep, 
									dev->rx_buf, free);
			DCC_LOG4(LOG_INFO, "seq=%d ack=%d free=%d n=%d", 
					 seq, ack, free, n);
			cnt = n;
		} else {
			int m;
			int n;

			n = MIN((CDC_RX_BUF_SIZE - pos), free);
			m = usb_dev_ep_pkt_recv(dev->usb, dev->out_ep, 
									&dev->rx_buf[pos], n);
			if ((m == n) && (n != free)) {
				n = usb_dev_ep_pkt_recv(dev->usb, dev->out_ep, 
										dev->rx_buf, free - m);
				DCC_LOG5(LOG_INFO, "seq=%d ack=%d free=%d n=%d m=%d", 
						 seq, ack, free, n, m);
				cnt = n + m;
			} else {
				DCC_LOG4(LOG_INFO, "seq=%d ack=%d free=%d m=%d", 
						 seq, ack, free, m);
				cnt = m;
			}

		}

		seq += cnt;

		if ((int32_t)(seq - ack) == CDC_RX_BUF_SIZE) {
			DCC_LOG(LOG_INFO, VT_PSH VT_FCY VT_REV " RX PAUSE " VT_POP);
			dev->rx_paused = true;
		}

		dev->rx_seq = seq;
	} else {
		cnt = 0;
	}

	return cnt;
}

static void monitor_usb_on_rcv(usb_class_t * cl, 
						   unsigned int ep_id, unsigned int len)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)cl;
	unsigned int cnt;

	cnt = monitor_usb_cdc_acm_recv(dev);
	(void)cnt;

	DCC_LOG2(LOG_INFO, VT_PSH VT_FCY VT_REV 
			 "IRQ pkt_len=%d rcv_cnt=%d COMM_RCV..." 
			 VT_POP, 
			 len, cnt);
	
	monitor_signal(MONITOR_COMM_RCV);
}

static void monitor_usb_on_eot(usb_class_t * cl, unsigned int ep_id)
{
	monitor_signal(MONITOR_COMM_EOT);
	DCC_LOG(LOG_MSG, "COMM_EOT");
}

static void monitor_usb_on_eot_int(usb_class_t * cl, unsigned int ep_id)
{
	DCC_LOG1(LOG_MSG, "ep_id=%d", ep_id);
}

static const usb_dev_ep_info_t monitor_usb_in_info = {
	.addr = USB_ENDPOINT_IN + EP_IN0_ADDR,
	.attr = ENDPOINT_TYPE_BULK,
	.mxpktsz = CDC_EP_IN_MAX_PKT_SIZE,
	.on_in = monitor_usb_on_eot
};

static const usb_dev_ep_info_t monitor_usb_out_info = {
	.addr = USB_ENDPOINT_OUT + EP_OUT0_ADDR,
	.attr = ENDPOINT_TYPE_BULK,
	.mxpktsz = CDC_EP_OUT_MAX_PKT_SIZE,
	.on_out = monitor_usb_on_rcv
};

static const usb_dev_ep_info_t monitor_usb_int_info = {
	.addr = USB_ENDPOINT_IN + EP_INT0_ADDR,
	.attr = ENDPOINT_TYPE_INTERRUPT,
	.mxpktsz = CDC_EP_INT_MAX_PKT_SIZE,
	.on_in = monitor_usb_on_eot_int
};


/* Setup requests callback handler */
static int monitor_usb_on_setup(usb_class_t * cl, 
								struct usb_request * req, void ** ptr) 
{
	struct usb_cdc_acm_dev * dev = &cl->dev;
	struct usb_dev * usb = dev->usb;
	int value = req->value;
	int index = req->index;
	int len = 0;
	int desc;

	(void)index;


	/* Handle supported standard device request Cf
	 Table 9-3 in USB specification Rev 1.1 */

	switch ((req->request << 8) | req->type) {

	/* Standard Device Requests */
		
	case STD_GET_DESCRIPTOR:
		desc = value >> 8;

		if (desc == USB_DESCRIPTOR_DEVICE) {
			/* Return Device Descriptor */
			*ptr = (void *)&cdc_acm_desc_dev;
			len = sizeof(struct usb_descriptor_device);
			DCC_LOG1(LOG_INFO, "GetDesc: Device: len=%d", len);
			break;
		}
#if (THINKOS_MONITOR_ENABLE_USB2_00)
		if (desc == USB_DESCRIPTOR_DEVICE_QUALIFIER) {
			/* Return Device Descriptor */
			*ptr = (void *)&cdc_acm_desc_qual;
			len = sizeof(struct usb_descriptor_device_qualifier);
			DCC_LOG1(LOG_INFO, "GetDesc: Device: len=%d", len);
			break;
		}
#endif
		if (desc == USB_DESCRIPTOR_CONFIGURATION) {
			/* Return Configuration Descriptor */
			*ptr = (void *)&monitor_usb_cdc_acm_desc_cfg;
			len = sizeof(struct cdc_acm_descriptor_set);
			DCC_LOG1(LOG_INFO, "GetDesc: Config: len=%d", len);
			break;
		}

		if (desc == USB_DESCRIPTOR_STRING) {
			unsigned int n = value & 0xff;
			DCC_LOG1(LOG_INFO, "GetDesc: String[%d]", n);
			if (n < USB_STRCNT()) {
				*ptr = (void *)monitor_usb_cdc_acm_str[n];
				len = monitor_usb_cdc_acm_str[n]->length;
			}
			break;
		}
		len = 0;
		DCC_LOG1(LOG_INFO, "GetDesc: %d ?", desc);
		break;

	case STD_SET_ADDRESS:
		DCC_LOG1(LOG_INFO, "SetAddr: %d [ADDRESS]", value);
		break;

	case STD_SET_CONFIGURATION: {
		DCC_LOG1(LOG_INFO, "SetCfg: %d", value);
		if (value) {
			DCC_LOG(LOG_INFO, "[CONFIGURED]");
			dev[0].in_ep = usb_dev_ep_init(usb, &monitor_usb_in_info, 
										   NULL, 0);
			dev[0].out_ep = usb_dev_ep_init(usb, &monitor_usb_out_info, 
											NULL, 0);
			dev[0].int_ep = usb_dev_ep_init(usb, &monitor_usb_int_info, 
											NULL, 0);
			dev->configured = 1;
		} else {
			DCC_LOG(LOG_INFO, "[UNCONFIGURED]");
			usb_dev_ep_ctl(dev->usb, dev[0].in_ep, USB_EP_DISABLE);
			usb_dev_ep_ctl(dev->usb, dev[0].out_ep, USB_EP_DISABLE);
			usb_dev_ep_ctl(dev->usb, dev[0].int_ep, USB_EP_DISABLE);
			dev->configured = 0;
		}
		break;
	}

	case STD_GET_CONFIGURATION:
		DCC_LOG(LOG_INFO, "GetCfg");
		*ptr = (void *)&dev->configured;
		len = 1;
		break;


#if (USB_STATUS_DEVICE_ENABLED)
	case STD_GET_STATUS_DEVICE:
		DCC_LOG(LOG_INFO, "GetStatusDev");
		*ptr = (void *)&monitor_usb_device_status;
		len = 2;
		break;
#endif

#if (USB_FEATURE_DEVICE_ENABLED)
	case STD_CLEAR_FEATURE_DEVICE:
		if (value == USB_DEVICE_REMOTE_WAKEUP) {
			DCC_LOG(LOG_INFO, "SetFeatureDev(REMOTE_WAKEUP)");
		} else if (value == USB_TEST_MODE ) {
			DCC_LOG(LOG_INFO, "SetFeatureDev(TEST_MODE)");
		} else {
			DCC_LOG1(LOG_WARNING, "ClrFeatureDev(%d)", value);
		}
		break;

	case STD_SET_FEATURE_DEVICE:
		if (value == USB_DEVICE_REMOTE_WAKEUP) {
			DCC_LOG(LOG_INFO, "SetFeatureDev(REMOTE_WAKEUP)");
		} else if (value == USB_TEST_MODE ) {
			DCC_LOG(LOG_INFO, "SetFeatureDev(TEST_MODE )");
		} else {
			DCC_LOG1(LOG_WARNING, "SetFeatureDev(%d)", value);
		}
		break;
#endif

#if (USB_STATUS_INTERFACE_ENABLED)
	/* Standard Interface Requests */
	case STD_GET_STATUS_INTERFACE:
		DCC_LOG1(LOG_INFO, "GetStatusIf(%d)", index);
		*ptr = (void *)&monitor_usb_interface_status;
		len = 2;
		break;
#endif

#if (USB_FEATURE_INTERFACE_ENABLED)
	case STD_CLEAR_FEATURE_INTERFACE:
		DCC_LOG2(LOG_INFO, "ClrFeatureIf(%d,%d)", index, value);
		break;

	case STD_SET_FEATURE_INTERFACE:
		DCC_LOG2(LOG_INFO, "SetFeatureIf(%d,%d)", index, value);
		break;

	case STD_GET_INTERFACE:
		DCC_LOG1(LOG_INFO, "GetInterface(%d)", index);
		break;

	case STD_SET_INTERFACE:
		DCC_LOG1(LOG_INFO, "SetInterface(%d)", index);
		break;
#endif

#if (USB_STATUS_ENDPOINT_ENABLED)
	/* Standard Endpoint Requests */
	case STD_GET_STATUS_ENDPOINT:
		{
			int ep_addr = index;
			(void)ep_addr;
		
			DCC_LOG1(LOG_TRACE, VT_PSH VT_FRD VT_BRI 
					 "GetStatusEP(%d)" VT_POP, ep_addr);
		}
		break;
#endif

#if (USB_FEATURE_ENDPOINT_ENABLED)
	case STD_CLEAR_FEATURE_ENDPOINT:
		{
			int ep_addr = index;

			if (value == USB_ENDPOINT_HALT) {
				DCC_LOG2(LOG_INFO, VT_PSH VT_FMG VT_BRI
						 "ClrFeatureEP Halt (%s %d)" VT_POP, 
						  (ep_addr & USB_ENDPOINT_IN) ? "IN" : "OUT",
						  ep_addr & 0x7f);

				usb_dev_ep_ctl(dev->usb, ep_addr, USB_EP_STALL_CLR);
			} else {
				DCC_LOG2(LOG_INFO, "ClrFeatureEP(%d,%d)", ep_addr, value);
			}
		}
		break;

	case STD_SET_FEATURE_ENDPOINT:
		{
			int ep_addr = index;

			if (value == USB_ENDPOINT_HALT) {
				DCC_LOG2(LOG_INFO, VT_PSH VT_FMG VT_BRI
						 "SetFeatureEP Halt (%s %d)" VT_POP, 
						  (ep_addr & USB_ENDPOINT_IN) ? "IN" : "OUT",
						  ep_addr & 0x7f);
				usb_dev_ep_ctl(dev->usb, ep_addr, USB_EP_STALL_SET);
			} else {
				DCC_LOG2(LOG_INFO, "SetFeatureEP(%d,%d)", ep_addr, value);
			}
		}
		break;
#endif

#if (USB_SYNCH_FRAME_ENABLED)
	case STD_SYNCH_FRAME:
		DCC_LOG1(LOG_TRACE, "SetSynchFrame:%d", index);
		break;
#endif

	/* -------------------------------------------------------------------- 
	 * Class specific requests 
	 */

	case SET_LINE_CODING: 
		{
			struct cdc_line_coding * lc;
			lc = (struct cdc_line_coding *)dev->ctl_buf;
			(void)lc;
			DCC_LOG(LOG_INFO, "CDC SetLn");
			DCC_LOG1(LOG_INFO, "dsDTERate=%d", lc->dwDTERate);
			DCC_LOG1(LOG_INFO, "bCharFormat=%d", lc->bCharFormat);
			DCC_LOG1(LOG_INFO, "bParityType=%d", lc->bParityType);
			DCC_LOG1(LOG_INFO, "bDataBits=%d", lc->bDataBits);
			break;
		}

	case GET_LINE_CODING:
		DCC_LOG(LOG_INFO, "CDC GetLn");
		/* Return Line Coding */
		*ptr = (void *)&monitor_usb_usb_cdc_lc;
		len = sizeof(struct cdc_line_coding);
		break;

	case SET_CONTROL_LINE_STATE:
		DCC_LOG2(LOG_INFO, "CDC_DTE_PRESENT=%d ACTIVATE_CARRIER=%d",
				(value & CDC_DTE_PRESENT) ? 1 : 0,
				(value & CDC_ACTIVATE_CARRIER) ? 1 : 0);
		if (value & CDC_DTE_PRESENT) 
			dev->status |= COMM_ST_CONNECTED;
		else
			dev->status &= ~COMM_ST_CONNECTED;
		/* signal monitor */
		monitor_signal(MONITOR_COMM_CTL);
		break;

#if (THINKOS_MONITOR_ENABLE_COMM_BRK)
	case SEND_BREAK:
		DCC_LOG1(LOG_TRACE, "CDC Send Break value=%d", value);
		if (value != 0) {
			uint32_t status;
			uint32_t toggle;

			status = dev->status;
			/* the bit state depends on shadow and status */
			/* toggle if the bit is not set */
			toggle = ~(status ^ dev->shadow) & COMM_ST_BREAK_REQ;
			dev->status = status ^ toggle;
			monitor_signal(MONITOR_COMM_BRK);
		}

		break;
#endif

	default:
		DCC_LOG5(LOG_WARNING, "CDC t=%x r=%x v=%x i=%d l=%d",
				req->type, req->request, value, req->index, len);
		break;
	}

	return len;
}

static const usb_dev_ep_info_t monitor_usb_ep0_info = {
	.addr = 0,
	.attr = ENDPOINT_TYPE_CONTROL,
	.mxpktsz = EP0_MAX_PKT_SIZE,
	.on_setup = monitor_usb_on_setup
};

static void monitor_usb_on_reset(usb_class_t * cl)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)cl;
	DCC_LOG(LOG_WARNING, "...");
	/* reset internal state */
	dev->status = 0;
	dev->shadow = 0;
	dev->configured = 0;
#if 0
	dev->rx_paused = false;
#endif
	/* initializes EP0 */
	dev->ctl_ep = usb_dev_ep_init(dev->usb, &monitor_usb_ep0_info, 
								  dev->ctl_buf, CDC_CTL_BUF_LEN);
	/* wakeup xmit */
	monitor_signal(MONITOR_COMM_EOT);
	monitor_signal(MONITOR_COMM_CTL);
}

static void monitor_usb_on_suspend(usb_class_t * cl)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)cl;
	DCC_LOG(LOG_TRACE, "...");
	dev->status = 0;
	dev->shadow = 0;
}

static void monitor_usb_on_wakeup(usb_class_t * cl)
{
	DCC_LOG(LOG_TRACE, "...");
}

static void monitor_usb_on_error(usb_class_t * cl, int code)
{
	DCC_LOG(LOG_TRACE, "...");

}

static int monitor_usb_comm_send(const void * comm, 
								const void * buf, unsigned int len)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;

	return usb_dev_ep_pkt_xmit(dev->usb, dev->in_ep, buf, len);
}

static int monitor_usb_comm_recv(const void * comm, 
								void * buf, unsigned int len)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;
	uint8_t * dst = (uint8_t *)buf;
	uint32_t ack;
	unsigned int pos;
	unsigned int cnt;
	unsigned int n;

	ack = dev->rx_ack;

	if ((n = (int32_t)(dev->rx_seq - ack)) == 0)  {
		monitor_clear(MONITOR_COMM_RCV);
		return 0;
	}

	cnt = MIN(n, len);
	pos = ack % CDC_RX_BUF_SIZE;

	if (pos == 0) {
		DCC_LOG4(LOG_INFO, "1. n=%d ack=%d pos=%d cnt=%d...", 
				 n, ack, pos, cnt);
		__thinkos_memcpy(dst, dev->rx_buf, cnt);
	} else {
		unsigned int m = CDC_RX_BUF_SIZE - pos;
		unsigned int l;

		m = MIN(m, cnt);
		__thinkos_memcpy(dst, &dev->rx_buf[pos], m);
		dst += m;

		l = cnt - m;
		__thinkos_memcpy(dst, dev->rx_buf, l);

		DCC_LOG6(LOG_INFO, "2. m=%d l=%d n=%d ack=%d pos=%d cnt=%d...", 
				 m, l, n, ack, pos, cnt);
	}

#if (THINKOS_MONITOR_ENABLE_COMM_STATS)
	dev->stats.rx_octet += cnt;
#endif

	ack += cnt;
	dev->rx_ack = ack;

	if (dev->rx_paused && ((dev->rx_seq - ack) < (CDC_RX_BUF_SIZE / 2))) {
		DCC_LOG(LOG_INFO, VT_PSH VT_FCY VT_REV " RESUME " VT_POP);
		dev->rx_paused = false;
		/* receive more data into buffer */
		monitor_usb_cdc_acm_recv(dev);
	}

	if ((int32_t)(dev->rx_seq - ack) > 0) {
		DCC_LOG(LOG_INFO, "signal MONITOR_COMM_RCV!");

		/* Pending data on fifo, resignal .. */
		monitor_signal(MONITOR_COMM_RCV);
	}

	return cnt;
}

static int monitor_usb_comm_ctrl(const void * comm, unsigned int opc)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;
	int ret = -1;

	switch (opc) {
	case COMM_CTRL_STATUS_GET:
		/* Pending data on fifo, resignal .. */
		if ((int32_t)(dev->rx_seq - dev->rx_ack) > 0) {
			DCC_LOG(LOG_WARNING, "signal MONITOR_COMM_RCV!");
			monitor_signal(MONITOR_COMM_RCV);
		}
		/* Bits in the shadow register are inverted  */
		ret = dev->status ^ dev->shadow;
		DCC_LOG1(LOG_MSG, "status=%02x", ret);
		break;

	case COMM_CTRL_CONNECT:
		while ((dev->status & COMM_ST_CONNECTED) == 0) {
			DCC_LOG1(LOG_TRACE, "status=%02x, waiting...", dev->status);
			if ((ret = monitor_expect(MONITOR_COMM_CTL)) < 0) {
				DCC_LOG1(LOG_WARNING, "ret=%d!!", ret);
				break;
			}
		}
		break;

#if (THINKOS_MONITOR_ENABLE_COMM_BRK)
	case COMM_CTRL_BREAK_ACK:
		{
			uint32_t status;
			uint32_t toggle;

			monitor_clear(MONITOR_COMM_BRK);
			status = dev->shadow;
			/* the bit state depends on shadow and status */
			/* toggle if the bit is not set */
			toggle = ~(status ^ dev->status) & COMM_ST_BREAK_REQ;
			dev->shadow = status ^ toggle;
		}
		ret = 0;
		break;

#endif

	case COMM_CTRL_DISCONNECT:
	default:
		ret = 0;

	}

	return ret;
}

static struct usb_class_if usb_class_if_instance;

static const usb_class_events_t monitor_usb_ev = {
	.on_reset = monitor_usb_on_reset,
	.on_suspend = monitor_usb_on_suspend,
	.on_wakeup = monitor_usb_on_wakeup,
	.on_error = monitor_usb_on_error
};

static const struct monitor_comm_op monitor_usb_usb_cdc_comm_op = {
	.send = monitor_usb_comm_send,
	.recv = monitor_usb_comm_recv,
	.ctrl = monitor_usb_comm_ctrl
};

static const struct monitor_comm monitor_usb_comm_instance = {
	.dev = (void *)&usb_class_if_instance,
	.op = &monitor_usb_usb_cdc_comm_op,
};

const struct monitor_comm * usb_comm_init(const usb_dev_t * usb)
{
	struct usb_cdc_acm_dev * dev = &usb_class_if_instance.dev;
	struct usb_class_if * cl = &usb_class_if_instance;
	int ret;

	/* initialize USB device */
	dev->usb = (usb_dev_t *)usb;
	dev->rx_seq = 0;
	dev->rx_ack = 0;
	dev->status = 0;
	dev->shadow = 0;
	dev->configured = 0;
	dev->rx_paused = false;

	DCC_LOG1(LOG_INFO, "usb_dev_init(%08x)", dev->usb);
	ret = usb_dev_init(dev->usb, cl, &monitor_usb_ev);
	(void)ret;
	DCC_LOG1(LOG_INFO, "usb_dev_init() --> %d", ret);

	return &monitor_usb_comm_instance;
}

const struct monitor_comm * custom_comm_getinstance(void)
{
	return &monitor_usb_comm_instance;
}

#endif

