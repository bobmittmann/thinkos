/* 
 * File:	 comm/uart-comm.c
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

#include <sys/dcclog.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

#if (THINKOS_ENABLE_MONITOR)

#ifdef THINKOS_DBGMON_ENABLE_FLOWCTL
#warning "deprecated THINKOS_DBGMON_ENABLE_FLOWCTL"
#endif

#ifndef THINKOS_DBGMON_ENABLE_COMM_STATS
#define THINKOS_DBGMON_ENABLE_COMM_STATS 0
#endif

struct dbgmon_uart_dev {
	/* underling serial device */
	struct serial_dev * usb;
	/* endpoints handlers */
	uint8_t ctl_ep;
	uint8_t in_ep;
	uint8_t out_ep;
	uint8_t int_ep;

	uint8_t rx_paused;
	uint8_t configured;
	volatile uint8_t acm_ctrl; /* modem control lines */

	uint32_t ctl_buf[CDC_CTL_BUF_LEN / 4];

	volatile uint32_t rx_seq; 
	volatile uint32_t rx_ack; 

	uint8_t rx_buf[CDC_EP_IN_MAX_PKT_SIZE];

#if THINKOS_DBGMON_ENABLE_COMM_STATS
	struct {
		uint32_t tx_octet;
		uint32_t rx_octet;
		uint32_t tx_pkt;
		uint32_t rx_pkt;
	} stats;
#endif

};

struct serial_if {
	struct usb_cdc_acm_dev dev;
};

static int dbgmon_usb_cdc_acm_recv(struct usb_cdc_acm_dev * dev)
{
	uint32_t seq;
	uint32_t ack;
	int free;
	int cnt;

	seq = dev->rx_seq;
	ack = dev->rx_ack;
	if ((free = CDC_EP_IN_MAX_PKT_SIZE - (int32_t)(seq - ack)) > 0) {
		int pos;

		pos = seq % CDC_EP_IN_MAX_PKT_SIZE;
		if (pos == 0) {
			int n;

			n = usb_dev_ep_pkt_recv(dev->usb, dev->out_ep, 
									dev->rx_buf, free);
			DCC_LOG4(LOG_MSG, "seq=%d ack=%d free=%d n=%d", 
					 seq, ack, free, n);
			cnt = n;
		} else {
			int m;
			int n;

			n = MIN((CDC_EP_IN_MAX_PKT_SIZE - pos), free);
			m = usb_dev_ep_pkt_recv(dev->usb, dev->out_ep, 
									&dev->rx_buf[pos], n);
			if (m == n)	{
				n = usb_dev_ep_pkt_recv(dev->usb, dev->out_ep, 
										dev->rx_buf, free - m);
				DCC_LOG5(LOG_MSG, "seq=%d ack=%d free=%d n=%d m=%d", 
						 seq, ack, free, n, m);
				cnt = n + m;
			} else {
				DCC_LOG4(LOG_MSG, "seq=%d ack=%d free=%d m=%d", 
						 seq, ack, free, m);
				cnt = m;
			}

		}

		seq += cnt;

		if ((int32_t)(seq - ack) == CDC_EP_IN_MAX_PKT_SIZE) {
			DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_REV " PAUSE " VT_POP);
			dev->rx_paused = true;
		}

		dev->rx_seq = seq;
	} else {
		cnt = 0;
	}

	return cnt;
}

static void dbgmon_usb_on_rcv(usb_class_t * cl, 
						   unsigned int ep_id, unsigned int len)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)cl;

	DCC_LOG1(LOG_MSG, VT_PSH VT_FRD VT_BRI "IRQ len=%d..." VT_POP, len);

	dbgmon_usb_cdc_acm_recv(dev);

	dbgmon_signal(DBGMON_COMM_RCV);
}

static void dbgmon_usb_on_eot(usb_class_t * cl, unsigned int ep_id)
{
	dbgmon_signal(DBGMON_COMM_EOT);
	DCC_LOG(LOG_MSG, "COMM_EOT");
}

static void dbgmon_usb_on_eot_int(usb_class_t * cl, unsigned int ep_id)
{
	DCC_LOG1(LOG_MSG, "ep_id=%d", ep_id);
}

static const usb_dev_ep_info_t dbgmon_usb_in_info = {
	.addr = USB_ENDPOINT_IN + EP_IN0_ADDR,
	.attr = ENDPOINT_TYPE_BULK,
	.mxpktsz = CDC_EP_IN_MAX_PKT_SIZE,
	.on_in = dbgmon_usb_on_eot
};

static const usb_dev_ep_info_t dbgmon_usb_out_info = {
	.addr = USB_ENDPOINT_OUT + EP_OUT0_ADDR,
	.attr = ENDPOINT_TYPE_BULK,
	.mxpktsz = CDC_EP_OUT_MAX_PKT_SIZE,
	.on_out = dbgmon_usb_on_rcv
};

static const usb_dev_ep_info_t dbgmon_usb_int_info = {
	.addr = USB_ENDPOINT_IN + EP_INT0_ADDR,
	.attr = ENDPOINT_TYPE_INTERRUPT,
	.mxpktsz = CDC_EP_INT_MAX_PKT_SIZE,
	.on_in = dbgmon_usb_on_eot_int
};


/* Setup requests callback handler */
static int dbgmon_usb_on_setup(usb_class_t * cl, 
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
			DCC_LOG1(LOG_TRACE, "GetDesc: Device: len=%d", len);
			break;
		}
#if (THINKOS_DBGMON_ENABLE_USB2_00)
		if (desc == USB_DESCRIPTOR_DEVICE_QUALIFIER) {
			/* Return Device Descriptor */
			*ptr = (void *)&cdc_acm_desc_qual;
			len = sizeof(struct usb_descriptor_device_qualifier);
			DCC_LOG1(LOG_TRACE, "GetDesc: Device: len=%d", len);
			break;
		}
#endif
		if (desc == USB_DESCRIPTOR_CONFIGURATION) {
			/* Return Configuration Descriptor */
			*ptr = (void *)&dbgmon_usb_cdc_acm_desc_cfg;
			len = sizeof(struct cdc_acm_descriptor_set);
			DCC_LOG1(LOG_TRACE, "GetDesc: Config: len=%d", len);
			break;
		}

		if (desc == USB_DESCRIPTOR_STRING) {
			unsigned int n = value & 0xff;
			DCC_LOG1(LOG_TRACE, "GetDesc: String[%d]", n);
			if (n < USB_STRCNT()) {
				*ptr = (void *)dbgmon_usb_cdc_acm_str[n];
				len = dbgmon_usb_cdc_acm_str[n]->length;
			}
			break;
		}
		len = 0;
		DCC_LOG1(LOG_TRACE, "GetDesc: %d ?", desc);
		break;

	case STD_SET_ADDRESS:
		DCC_LOG1(LOG_TRACE, "SetAddr: %d [ADDRESS]", value);
		break;

	case STD_SET_CONFIGURATION: {
		DCC_LOG1(LOG_TRACE, "SetCfg: %d", value);
		if (value) {
			DCC_LOG(LOG_TRACE, "[CONFIGURED]");
			dev[0].in_ep = usb_dev_ep_init(usb, &dbgmon_usb_in_info, NULL, 0);
			dev[0].out_ep = usb_dev_ep_init(usb, &dbgmon_usb_out_info, NULL, 0);
			dev[0].int_ep = usb_dev_ep_init(usb, &dbgmon_usb_int_info, NULL, 0);
			dev->configured = 1;
		} else {
			DCC_LOG(LOG_TRACE, "[UNCONFIGURED]");
			usb_dev_ep_ctl(dev->usb, dev[0].in_ep, USB_EP_DISABLE);
			usb_dev_ep_ctl(dev->usb, dev[0].out_ep, USB_EP_DISABLE);
			usb_dev_ep_ctl(dev->usb, dev[0].int_ep, USB_EP_DISABLE);
			dev->configured = 0;
		}
		break;
	}


	case STD_GET_STATUS_DEVICE:
		DCC_LOG(LOG_TRACE, "GetStatusDev");
		*ptr = (void *)&dbgmon_usb_device_status;
		len = 2;
		break;

	case STD_GET_CONFIGURATION:
		DCC_LOG(LOG_TRACE, "GetCfg");
		*ptr = (void *)&dev->configured;
		len = 1;
		break;

#if DEBUG
	case STD_CLEAR_FEATURE_DEVICE:
		if (value == USB_DEVICE_REMOTE_WAKEUP) {
			DCC_LOG(LOG_TRACE, "SetFeatureDev(REMOTE_WAKEUP)");
		} else if (value == USB_TEST_MODE ) {
			DCC_LOG(LOG_TRACE, "SetFeatureDev(TEST_MODE )");
		} else
			DCC_LOG1(LOG_WARNING, "ClrFeatureDev(%d)", value);
		break;

	case STD_SET_FEATURE_DEVICE:
		if (value == USB_DEVICE_REMOTE_WAKEUP) {
			DCC_LOG(LOG_TRACE, "SetFeatureDev(REMOTE_WAKEUP)");
		} else if (value == USB_TEST_MODE ) {
			DCC_LOG(LOG_TRACE, "SetFeatureDev(TEST_MODE )");
		} else
			DCC_LOG1(LOG_WARNING, "SetFeatureDev(%d)", value);
		break;
#endif

	/* Standard Interface Requests */
	case STD_GET_STATUS_INTERFACE:
		DCC_LOG1(LOG_TRACE, "GetStatusIf(%d)", index);
		*ptr = (void *)&dbgmon_usb_interface_status;
		len = 2;
		break;

#if DEBUG
	case STD_CLEAR_FEATURE_INTERFACE:
		DCC_LOG2(LOG_TRACE, "ClrFeatureIf(%d,%d)", index, value);
		break;

	case STD_SET_FEATURE_INTERFACE:
		DCC_LOG2(LOG_TRACE, "SetFeatureIf(%d,%d)", index, value);
		break;

	case STD_GET_INTERFACE:
		DCC_LOG1(LOG_TRACE, "GetInterface(%d)", index);
		break;

	case STD_SET_INTERFACE:
		DCC_LOG1(LOG_TRACE, "SetInterface(%d)", index);
		break;
#endif


#if DEBUG
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

	case STD_CLEAR_FEATURE_ENDPOINT:
		{
			int ep_addr = index;

			if (value == USB_ENDPOINT_HALT) {
				DCC_LOG2(LOG_TRACE, VT_PSH VT_FMG VT_BRI
						 "ClrFeatureEP Halt (%s %d)" VT_POP, 
						  (ep_addr & USB_ENDPOINT_IN) ? "IN" : "OUT",
						  ep_addr & 0x7f);

				usb_dev_ep_ctl(dev->usb, ep_addr, USB_EP_STALL_CLR);
			} else {
				DCC_LOG2(LOG_TRACE, "ClrFeatureEP(%d,%d)", ep_addr, value);
			}
		}
		break;

	case STD_SET_FEATURE_ENDPOINT:
		{
			int ep_addr = index;

			if (value == USB_ENDPOINT_HALT) {
				DCC_LOG2(LOG_TRACE, VT_PSH VT_FMG VT_BRI
						 "SetFeatureEP Halt (%s %d)" VT_POP, 
						  (ep_addr & USB_ENDPOINT_IN) ? "IN" : "OUT",
						  ep_addr & 0x7f);
				usb_dev_ep_ctl(dev->usb, ep_addr, USB_EP_STALL_SET);
			} else {
				DCC_LOG2(LOG_TRACE, "SetFeatureEP(%d,%d)", ep_addr, value);
			}
		}
		break;

#if DEBUG
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
			DCC_LOG(LOG_TRACE, "CDC SetLn");
			DCC_LOG1(LOG_TRACE, "dsDTERate=%d", lc->dwDTERate);
			DCC_LOG1(LOG_TRACE, "bCharFormat=%d", lc->bCharFormat);
			DCC_LOG1(LOG_TRACE, "bParityType=%d", lc->bParityType);
			DCC_LOG1(LOG_TRACE, "bDataBits=%d", lc->bDataBits);
			break;
		}

	case GET_LINE_CODING:
		DCC_LOG(LOG_TRACE, "CDC GetLn");
		/* Return Line Coding */
		*ptr = (void *)&dbgmon_usb_usb_cdc_lc;
		len = sizeof(struct cdc_line_coding);
		break;

	case SET_CONTROL_LINE_STATE:
		dev->acm_ctrl = value;
		DCC_LOG2(LOG_TRACE, "CDC_DTE_PRESENT=%d ACTIVATE_CARRIER=%d",
				(value & CDC_DTE_PRESENT) ? 1 : 0,
				(value & CDC_ACTIVATE_CARRIER) ? 1 : 0);
		/* signal monitor */
		dbgmon_signal(DBGMON_COMM_CTL);
		break;

	default:
		DCC_LOG5(LOG_WARNING, "CDC t=%x r=%x v=%x i=%d l=%d",
				req->type, req->request, value, req->index, len);
		break;
	}

	return len;
}

static const usb_dev_ep_info_t dbgmon_usb_ep0_info = {
	.addr = 0,
	.attr = ENDPOINT_TYPE_CONTROL,
	.mxpktsz = EP0_MAX_PKT_SIZE,
	.on_setup = dbgmon_usb_on_setup
};

static void dbgmon_usb_on_reset(usb_class_t * cl)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)cl;
	DCC_LOG(LOG_WARNING, "...");
	/* reset control lines */
	dev->acm_ctrl = 0;
	/* initializes EP0 */
	dev->ctl_ep = usb_dev_ep_init(dev->usb, &dbgmon_usb_ep0_info, 
								  dev->ctl_buf, CDC_CTL_BUF_LEN);
	/* wakeup xmit */
	dbgmon_signal(DBGMON_COMM_EOT);
	dbgmon_signal(DBGMON_COMM_CTL);
}

static void dbgmon_usb_on_suspend(usb_class_t * cl)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)cl;
	DCC_LOG(LOG_TRACE, "...");
	dev->acm_ctrl = 0;
}

static void dbgmon_usb_on_wakeup(usb_class_t * cl)
{
	DCC_LOG(LOG_TRACE, "...");
}

static void dbgmon_usb_on_error(usb_class_t * cl, int code)
{
	DCC_LOG(LOG_TRACE, "...");
}

static int dbgmon_usb_comm_send(const void * comm, 
								const void * buf, unsigned int len)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;
	uint8_t * ptr = (uint8_t *)buf;
	unsigned int rem;
	int ret;
	int n;

	DCC_LOG1(LOG_MSG, VT_PSH VT_FMG "entry len=%d" VT_POP, len);

	rem = len;
	while (rem) {

		if (dev->acm_ctrl == 0) {
//			DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_REV "not connected!=%d" VT_POP);
			/* not connected, discard!! */
			rem = 0;
			break;
		}

		n = usb_dev_ep_pkt_xmit(dev->usb, dev->in_ep, ptr, rem);
		DCC_LOG2(LOG_INFO, "usb_dev_ep_pkt_xmit(%d) %d", rem, n);
		if (n < 0) {
#if THINKOS_DBGMON_ENABLE_COMM_STATS
			DCC_LOG1(LOG_WARNING, "usb_dev_ep_pkt_xmit() failed (pkt=%d)!", 
					 dev->stats.tx_pkt);
#else
			DCC_LOG(LOG_WARNING, "usb_dev_ep_pkt_xmit() failed");
#endif
			return n;
		} else if (n > 0) {
#if THINKOS_DBGMON_ENABLE_COMM_STATS
			dev->stats.tx_pkt++;
			dev->stats.tx_octet += n;
#endif
			rem -= n;
			ptr += n;
		}  else {
			DCC_LOG(LOG_MSG, "dbgmon_expect(DBGMON_COMM_EOT)!");
			if ((ret = dbgmon_expect(DBGMON_COMM_EOT)) < 0) {
				DCC_LOG(LOG_WARNING, "dbgmon_expect()!");
				return ret;
			}
		}
	}

	DCC_LOG1(LOG_MSG, VT_PSH VT_FMG VT_REV "return=%d" VT_POP, len - rem);

	return len - rem;
}

static int dbgmon_usb_comm_recv(const void * comm, 
								void * buf, unsigned int len)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;
	uint8_t * dst = (uint8_t *)buf;
	uint32_t ack;
	unsigned int pos;
	unsigned int cnt;
	unsigned int n;
	int ret;

	ack = dev->rx_ack;
	do {
		if ((ret = dbgmon_expect(DBGMON_COMM_RCV)) < 0) {
			DCC_LOG(LOG_WARNING, "dbgmon_expect()!");
			return ret;
		}
	 } while ((n = (int32_t)(dev->rx_seq - ack)) == 0);

	cnt = MIN(n, len);
	pos = ack % CDC_EP_IN_MAX_PKT_SIZE;

	if (pos == 0) {
		DCC_LOG4(LOG_INFO, "1. n=%d ack=%d pos=%d cnt=%d...", 
				 n, ack, pos, cnt);
		__thinkos_memcpy(dst, dev->rx_buf, cnt);
	} else {
		unsigned int m = CDC_EP_IN_MAX_PKT_SIZE - pos;
		unsigned int l;

		m = MIN(m, cnt);
		__thinkos_memcpy(dst, &dev->rx_buf[pos], m);
		dst += m;

		l = cnt - m;
		__thinkos_memcpy(dst, dev->rx_buf, l);

		DCC_LOG6(LOG_INFO, "2. m=%d l=%d n=%d ack=%d pos=%d cnt=%d...", 
				 m, l, n, ack, pos, cnt);
	}

#if THINKOS_DBGMON_ENABLE_COMM_STATS
	dev->stats.rx_octet += cnt;
#endif

	ack += cnt;
	dev->rx_ack = ack;

	if (dev->rx_paused && ((dev->rx_seq - ack) < CDC_EP_IN_MAX_PKT_SIZE)) {
		DCC_LOG(LOG_INFO, VT_PSH VT_FCY VT_REV " RESUME " VT_POP);
		dev->rx_paused = false;
		dbgmon_usb_cdc_acm_recv(dev);
	}

	if ((int32_t)(dev->rx_seq - ack) > 0) {
		DCC_LOG(LOG_MSG, "signal DBGMON_COMM_RCV!");
		/* Pending data on fifo, resignal .. */
		dbgmon_signal(DBGMON_COMM_RCV);
	}

	return cnt;
}


static int dbgmon_usb_comm_connect(const void * comm)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;
//	struct cdc_notification * pkt;
//	uint32_t buf[4];

	int ret;

	while ((dev->acm_ctrl & CDC_DTE_PRESENT) == 0) {
		DCC_LOG1(LOG_TRACE, "ctrl=%02x, waiting...", dev->acm_ctrl);
		if ((ret = dbgmon_expect(DBGMON_COMM_CTL)) < 0) {
			DCC_LOG1(LOG_WARNING, "ret=%d!!", ret);
			return ret;
		}
	}

#if 0
	if ((dev->acm.flags & ACM_CONNECTED) == 0) {
		dev->acm.flags |= ACM_CONNECTED;
		pkt = (struct cdc_notification *)buf;
		/* bmRequestType */
		pkt->bmRequestType = USB_CDC_NOTIFICATION;
		/* bNotification */
		pkt->bNotification = CDC_NOTIFICATION_SERIAL_STATE;
		/* wValue */
		pkt->wValue = 0;
		/* wIndex */
		pkt->wIndex = 1;
		/* wLength */
		pkt->wLength = 2;
		/* data */
		pkt->bData[0] = CDC_SERIAL_STATE_TX_CARRIER | 
			CDC_SERIAL_STATE_RX_CARRIER;
		pkt->bData[1] = 0;

		ret = usb_dev_ep_pkt_xmit(dev->usb, dev->int_ep, pkt, 
								  sizeof(struct cdc_notification));
		if (ret < 0) {
			DCC_LOG(LOG_WARNING, "usb_dev_ep_pkt_xmit() failed!");
			return ret;
		}
	}
#endif

	return 0;
}

static bool dbgmon_usb_comm_isconnected(const void * comm)
{
	struct usb_cdc_acm_dev * dev = (struct usb_cdc_acm_dev *)comm;

	/* Pending data on fifo, resignal .. */
	if ((int32_t)(dev->rx_seq - dev->rx_ack) > 0) {
		DCC_LOG(LOG_WARNING, "signal DBGMON_COMM_RCV!");
		dbgmon_signal(DBGMON_COMM_RCV);
	}

	return (dev->acm_ctrl & CDC_DTE_PRESENT) ? true : false;
}

static struct dbgmon_uart_comm_drv dbgmon_uart_comm_drv_instnce;

static const usb_class_events_t dbgmon_usb_ev = {
	.on_reset = dbgmon_usb_on_reset,
	.on_suspend = dbgmon_usb_on_suspend,
	.on_wakeup = dbgmon_usb_on_wakeup,
	.on_error = dbgmon_usb_on_error
};

static const struct dbgmon_comm_op dbgmon_uart_comm_op = {
	.send = dbgmon_uart_comm_send,
	.recv = dbgmon_uart_comm_recv,
	.connect = dbgmon_uart_comm_connect,
	.isconnected = dbgmon_uart_comm_isconnected
};

static const struct dbgmon_comm dbgmon_uart_comm_instance = {
	.dev = (void *)&dbgmon_uart_comm_drv_instnce,
	.op = &dbgmon_uart_comm_op,
};

const struct dbgmon_comm * thinkos_dbgmon_uart_comm_init(const struct 
														 serial_dev * ser)
{
	struct dbgmon_uart_comm_drv * drv = &dbgmon_uart_comm_drv_instnce;

	/* initialize USB device */

	DCC_LOG(LOG_TRACE, "serial_dev_init()");
//	serial_dev_init(dev->usb, cl, &dbgmon_usb_ev);

	return &dbgmon_usb_comm_instance;
}

const struct dbgmon_comm * custom_comm_getinstance(void)
{
	return &dbgmon_usb_comm_instance;
}

#endif


