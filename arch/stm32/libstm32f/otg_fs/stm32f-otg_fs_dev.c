/* 
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
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
 * @file stm32f-otg_fs_dev.c
 * @brief STM32F OTG USB driver
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arch/cortex-m3.h>
#include <sys/delay.h>
#include <sys/usb-dev.h>
#include <sys/param.h>
#include <sys/dcclog.h>
#include <vt100.h>

#ifndef STM32_ENABLE_OTG_FS 
#define STM32_ENABLE_OTG_FS 0
#endif 

#ifndef STM32_OTG_FS_INEP_MAX
#define STM32_OTG_FS_INEP_MAX 3
#endif

#ifndef STM32_OTG_FS_OUTEP_MAX
#define STM32_OTG_FS_OUTEP_MAX 3
#endif

#ifndef STM32_OTG_FS_IO_INIT
#define STM32_OTG_FS_IO_INIT 0
#endif

#ifndef STM32_OTG_FS_VBUS_CONNECT
#define STM32_OTG_FS_VBUS_CONNECT 0
#endif

#ifndef STM32_OTG_FS_IRQ_ENABLE
#define STM32_OTG_FS_IRQ_ENABLE 0
#endif

#ifndef STM32_OTG_FS_BULK_PKTS
#define STM32_OTG_FS_BULK_PKTS 4
#endif

#ifndef STM32_OTG_FS_RX_FIFO_SIZE
#define STM32_OTG_FS_RX_FIFO_SIZE 192
#endif

#if defined(STM32F_OTG_FS) && (STM32_ENABLE_OTG_FS)

#define OTG_INEP_MAX     (STM32_OTG_FS_INEP_MAX)
#define OTG_OUTEP_MAX    (STM32_OTG_FS_OUTEP_MAX)
#define OTG_EP_MAX       ((STM32_OTG_FS_INEP_MAX) + \
						  (STM32_OTG_FS_OUTEP_MAX) + 1)
#define OTG_INEP_OFF     (OTG_OUTEP_MAX)

#define OTG_FS_MEM_SIZE    1280

#ifndef DEBUG
#define DEBUG 0
#endif

/* Endpoint state */
enum ep_state {
	EP_UNCONFIGURED = 0,
	EP_IDLE,
	EP_STALLED,
	EP_IN_DATA,
	EP_IN_DATA_ZLP,
	EP_IN_ZLP,
	EP_WAIT_STATUS_IN,
	EP_OUT_DATA,
	EP_OUT_DATA_LAST
};

/* Endpoint control */
struct stm32f_otg_ep {
	uint8_t state;
	uint8_t idx;
	uint16_t xfr_max;
	volatile uint16_t xfr_len;
	volatile uint16_t xfr_cnt;
	union {
		uint32_t xfr_dat;
		uint8_t * xfr_ptr;
	};
	uint8_t * xfr_buf;
	/* endpoint callback */
	union {
		usb_class_on_ep_ev_t on_ev;
		usb_class_on_ep_in_t on_in;
		usb_class_on_ep_out_t on_out;
		usb_class_on_ep_setup_t on_setup;
	};
};

/* USB Device runtime driver data */
struct stm32f_otg_drv {
	struct stm32f_otg_ep ep[OTG_EP_MAX];
	struct usb_class_if * cl;
	const struct usb_class_events * ev;
	struct usb_request req;
	uint16_t fifo_addr;
};

/* EP TX fifo memory allocation */
static void __ep_pktbuf_alloc(struct stm32f_otg_drv * drv, 
							  unsigned int idx, unsigned int siz)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;

	if ((drv->fifo_addr + siz) > OTG_FS_MEM_SIZE) {
		DCC_LOG2(LOG_ERROR, "can't alloc memory, addr=%d siz=%d", 
				 drv->fifo_addr, siz);
		return;
	}

	switch (idx) {
	case 0:
		otg_fs->dieptxf0 = OTG_FS_TX0FD_SET(siz / 4) | 
			OTG_FS_TX0FSA_SET(drv->fifo_addr / 4);
		break;
#if (OTG_INEP_MAX > 1)
	case 1:
		otg_fs->dieptxf1 = OTG_FS_INEPTXFD_SET(siz / 4) | 
			OTG_FS_INEPTXSA_SET(drv->fifo_addr / 4);
		break;
#endif
#if (OTG_INEP_MAX > 2)
	case 2:
		otg_fs->dieptxf2 = OTG_FS_INEPTXFD_SET(siz / 4) | 
			OTG_FS_INEPTXSA_SET(drv->fifo_addr / 4);
		break;
#endif
#if (OTG_INEP_MAX > 3)
	case 3:
		otg_fs->dieptxf3 = OTG_FS_INEPTXFD_SET(siz / 4) | 
			OTG_FS_INEPTXSA_SET(drv->fifo_addr / 4);
		break;
#endif
	}

	DCC_LOG3(LOG_INFO, "addr=%d siz=%d free=%d", drv->fifo_addr, siz,
			 OTG_FS_MEM_SIZE - (drv->fifo_addr + siz));

	drv->fifo_addr += siz;
}

static inline void __copy_from_pktbuf(void * ptr,
							   volatile uint32_t * pop,
							   unsigned int cnt)
{
	uint8_t * dst = (uint8_t *)ptr;
	uint32_t data;
	unsigned int i;

	/* pop data from the fifo and copy to destination buffer */
	for (i = 0; i < (cnt + 3) / 4; i++) {
		data = *pop;
		*dst++ = data;
		*dst++ = data >> 8;
		*dst++ = data >> 16;
		*dst++ = data >> 24;
	}
}

static void __ep_zlp_send(struct stm32f_otg_fs * otg_fs, int idx)
{
	DCC_LOG(LOG_MSG, "Send: ZLP");

	otg_fs->inep[idx].dieptsiz = OTG_FS_PKTCNT_SET(1) | OTG_FS_XFRSIZ_SET(0);
	otg_fs->inep[idx].diepctl |= OTG_FS_EPENA | OTG_FS_CNAK;
}
/*
   • Setting the global OUT NAK

   1. To stop receiving any kind of data in the receive FIFO, the 
   application must set the Global OUT NAK bit by programming the 
   following field:
   – SGONAK = 1 in OTG_DCTL

   2. Wait for the assertion of the GONAKEFF interrupt in OTG_GINTSTS. 
   When asserted, this interrupt indicates that the core has stopped 
   receiving any type of data except SETUP packets.

   3. The application can receive valid OUT packets after it has set 
   SGONAK in OTG_DCTL and before the core asserts the GONAKEFF interrupt 
   (OTG_GINTSTS).

   4. The application can temporarily mask this interrupt by writing to 
   the GONAKEFFM bit in the OTG_GINTMSK register.
   – GONAKEFFM = 0 in the OTG_GINTMSK register

   5. Whenever the application is ready to exit the Global OUT NAK mode, it 
   must clear the SGONAK bit in OTG_DCTL. This also clears the GONAKEFF 
   interrupt (OTG_GINTSTS).
   – CGONAK = 1 in OTG_DCTL

   6. If the application has masked this interrupt earlier, it must be 
   unmasked as follows:
   – GONAKEFFM = 1 in OTG_GINTMSK
 */

static void __ep_out_stall_set(struct stm32f_otg_fs * otg_fs, int idx)
{
	uint32_t doepctl;

	doepctl = otg_fs->outep[idx].doepctl;
	doepctl |= OTG_FS_STALL;
	otg_fs->outep[idx].doepctl = doepctl;

/* Stalling a non-isochronous OUT endpoint
   1. Put the core in the Global OUT NAK mode. 
	otg_fs->dctl |= OTG_FS_SGONAK;

 2. Disable the required endpoint
   – When disabling the endpoint, instead of setting the SNAK bit in 
   OTG_DOEPCTL, set STALL = 1 (in OTG_DOEPCTL).
   The STALL bit always takes precedence over the NAK bit. 

 3. When the application is ready to end the STALL handshake for the 
   endpoint, the STALL bit (in OTG_DOEPCTLx) must be cleared.

   4. If the application is setting or clearing a STALL for an endpoint 
   due to a SetFeature.Endpoint Halt or ClearFeature.Endpoint Halt command, 
   the STALL bit must be set or cleared before the application sets up 
   the Status stage transfer on the control endpoint. 

	otg_fs->dctl |= OTG_FS_CGONAK; */
}

static void __ep_out_stall_clr(struct stm32f_otg_fs * otg_fs, int idx)
{
	uint32_t depctl;
	uint32_t ep_type;

	depctl = otg_fs->outep[idx].doepctl; 
	ep_type = OTG_FS_EPTYP_GET(depctl);
	if (ep_type == OTG_FS_EPTYP_INT || ep_type == OTG_FS_EPTYP_BULK)
		depctl |= OTG_FS_DPID0; /* DATA0 */
	depctl &= ~OTG_FS_STALL;
	otg_fs->outep[idx].doepctl = depctl; 
}

static void __ep_in_stall_set(struct stm32f_otg_fs * otg_fs, int idx)
{
	uint32_t diepctl;

	diepctl = otg_fs->inep[idx].diepctl;
	diepctl |= OTG_FS_STALL | ((diepctl & OTG_FS_EPENA) ? OTG_FS_EPDISD : 0);
	otg_fs->inep[idx].diepctl = diepctl; 
}

static void __ep_in_stall_clr(struct stm32f_otg_fs * otg_fs, int idx)
{
	uint32_t depctl;
	uint32_t ep_type;

	depctl = otg_fs->inep[idx].diepctl;
	ep_type = OTG_FS_EPTYP_GET(depctl);
	if (ep_type == OTG_FS_EPTYP_INT || ep_type == OTG_FS_EPTYP_BULK)
		depctl |= OTG_FS_DPID0; /* DATA0 */
	depctl &= ~OTG_FS_STALL;
	otg_fs->inep[idx].diepctl = depctl; 
}

static void __ep0_tx_setup(struct stm32f_otg_fs * otg_fs, int len)
{
	uint32_t depctl;
	uint32_t mpsiz;
	uint32_t xfrsiz;
	uint32_t pktcnt;

	depctl = otg_fs->inep[0].diepctl;
	mpsiz = OTG_FS_EP0_MPSIZ_GET(depctl);

	if (len > 0) {
		pktcnt = (len + (mpsiz - 1)) / mpsiz;
		if (pktcnt > 1) {
			pktcnt = 1;
			xfrsiz =  mpsiz * 1;
		} else {
			xfrsiz = len;
		}
	} else {
		/* zero lenght packet */
		pktcnt = 1;
		xfrsiz = 0;
	}

	DCC_LOG3(LOG_MSG, "[0] pktcnt=%d xfrsiz=%d rem=%d", 
			 pktcnt, xfrsiz, len - xfrsiz);

	otg_fs->inep[0].dieptsiz = OTG_FS_PKTCNT_SET(pktcnt) | 
		OTG_FS_XFRSIZ_SET(xfrsiz); 

	/* umask FIFO empty interrupt */
	otg_fs->diepempmsk |= (1 << 0);

}

static void __ep0_tx_push(struct stm32f_otg_drv * drv)
{
	struct stm32f_otg_ep * ep = &drv->ep[0];
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	uint32_t pktcnt;
	uint32_t diepctl;
	uint32_t dieptsiz;
	uint32_t mpsiz;
	uint32_t xfrsiz;
	uint32_t free;
	uint8_t * cp;
	int pos;
	int cnt;
	int i;

	dieptsiz = otg_fs->inep[0].dieptsiz;
	xfrsiz = OTG_FS_XFRSIZ_GET(dieptsiz);
	pktcnt = OTG_FS_PKTCNT_GET(dieptsiz);
	diepctl = otg_fs->inep[0].diepctl;

	if (xfrsiz == 0) {
		otg_fs->diepempmsk &= ~(1 << 0);
		if (pktcnt != 0) {
			DCC_LOG1(LOG_PANIC, VT_PSH VT_FGR VT_REV
					 "[0] pktcnt(%d) != 0 !!!" VT_POP, pktcnt);
			otg_fs->inep[0].diepctl = diepctl | OTG_FS_EPENA | OTG_FS_CNAK; 
		} else {
			DCC_LOG(LOG_MSG, VT_PSH VT_FGR 
					"[0] no pending data..." VT_POP);
		}
		return;
	}

	free = otg_fs->inep[0].dtxfsts * 4;
	mpsiz = OTG_FS_EP0_MPSIZ_GET(diepctl);

#if DEBUG
	(void)pktcnt;

	DCC_LOG4(LOG_MSG, VT_PSH VT_FGR 
			 "[0] mpsiz=%d pktcnt=%d xfrsiz=%d free=%d" VT_POP, 
			 mpsiz, pktcnt, xfrsiz, free);
#endif

	if (xfrsiz < mpsiz) {
		if (free < xfrsiz) {
			DCC_LOG2(LOG_ERROR, VT_PSH VT_FGR VT_REV
					 "free(%d) < xfrsiz(%d) !!!" VT_POP, free, xfrsiz);
			otg_fs->diepempmsk &= ~(1 << 0);
			return;
		}
		/* Transfer the last partial packet */
		cnt = xfrsiz;
	} else {
		if (free < mpsiz) {
			DCC_LOG2(LOG_ERROR, VT_PSH VT_FGR VT_REV
					 "free(%d) < mpsiz(%d) !!!" VT_POP, 
					 free, mpsiz);
			otg_fs->diepempmsk &= ~(1 << 0);
			return;
		}
		if (free < xfrsiz) {
			/* Transfer only full packets */
			cnt = (free / mpsiz) * mpsiz;
		} else {
			/* Transfer all */
			cnt = xfrsiz;
		}
	}

	otg_fs->inep[0].diepctl = diepctl | OTG_FS_EPENA | OTG_FS_CNAK; 

	/* push into fifo */
	pos = ep->xfr_cnt;
	cp = (uint8_t *)&(ep->xfr_ptr[pos]);
	for (i = 0; i < cnt; i += 4) {
		uint32_t data;
		data = cp[0] + (cp[1] << 8) + (cp[2] << 16) + (cp[3] << 24);
		otg_fs->dfifo[0].push = data;
		cp += 4;
	}	
	ep->xfr_cnt = pos + cnt;
}

static void __ep0_tx_done(struct stm32f_otg_drv * drv)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	struct stm32f_otg_ep * ep = &drv->ep[0];
	unsigned int rem;

	if (ep->state == EP_WAIT_STATUS_IN) {
		struct usb_request * req = &drv->req;
		void * dummy = NULL;
		/* End of SETUP transaction (OUT Data Phase) */
		if (ep->on_setup(drv->cl, req, dummy) < 0) {
			DCC_LOG(LOG_WARNING, "EP0 [SETUP] IN Dev->Host stall!");
		}
		ep->state = EP_IDLE;
		DCC_LOG(LOG_MSG, "EP0 [IDLE]");
		return;
	}

	rem = ep->xfr_len - ep->xfr_cnt;

	if (rem == 0) {
		otg_fs->diepempmsk &= ~(1 << 0);
		/* Prepare to receive SETUP packets */
		otg_fs->outep[0].doeptsiz = OTG_FS_STUPCNT_SET(3) | 
			OTG_FS_PKTCNT_SET(1) | OTG_FS_XFRSIZ_SET(40);
		/* EP enable */
		otg_fs->outep[0].doepctl |= OTG_FS_EPENA | OTG_FS_CNAK;
		ep->state = EP_IN_DATA;
		DCC_LOG(LOG_MSG, "EP0 [IN_DATA]");
		return;
	} 
	
	DCC_LOG1(LOG_MSG, "EP0 [SETUP] IN Dev->Host (%d)", rem);
	__ep0_tx_setup(otg_fs, rem);
}


static void __ep_tx_done(struct stm32f_otg_drv * drv, int idx)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	int ep_id = idx + OTG_INEP_OFF;
	struct stm32f_otg_ep * ep;
	uint32_t diepctl;

	ep = &drv->ep[ep_id];
	diepctl = otg_fs->inep[idx].diepctl;
	if (ep->state == EP_IN_DATA_ZLP) {
		DCC_LOG4(LOG_INFO, VT_PSH VT_FGR
				 "[%d] [IN_DATA_ZLP] -> [IN_ZLP] EPENA=%d NAKSTS=%d DPID=%d." 
				 VT_POP, idx, 
				 diepctl & OTG_FS_EPENA ? 1 : 0, 
				 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
				 diepctl & OTG_FS_DPID ? 1 : 0);
		otg_fs->inep[idx].dieptsiz = OTG_FS_PKTCNT_SET(1) | 
			OTG_FS_XFRSIZ_SET(0);
		otg_fs->inep[idx].diepctl = diepctl | OTG_FS_EPENA;
		ep->state = EP_IN_ZLP;
		return;
	}

#if DEBUG
	if (ep->state == EP_IN_DATA) {
		DCC_LOG4(LOG_MSG, VT_PSH VT_FGR
				 "[%d] [IN_DATA]->[IDLE] EPENA=%d NAKSTS=%d DPID=%d." 
				 VT_POP, idx, 
				 diepctl & OTG_FS_EPENA ? 1 : 0, 
				 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
				 diepctl & OTG_FS_DPID ? 1 : 0);
	} else if (ep->state == EP_IN_ZLP) {
		DCC_LOG4(LOG_MSG, VT_PSH VT_FGR
				 "[%d] [IN_ZLP]->[IDLE] EPENA=%d NAKSTS=%d DPID=%d." 
				 VT_POP, idx, 
				 diepctl & OTG_FS_EPENA ? 1 : 0, 
				 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
				 diepctl & OTG_FS_DPID ? 1 : 0);
	} else {
		DCC_LOG5(LOG_MSG, VT_PSH VT_FRD
				 "[%d] [state=%d]->[IDLE]  EPENA=%d NAKSTS=%d DPID=%d." 
				 VT_POP, idx, ep->state,
				 diepctl & OTG_FS_EPENA ? 1 : 0, 
				 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
				 diepctl & OTG_FS_DPID ? 1 : 0);
	}
#endif

	ep->state = EP_IDLE;
	/* call class endpoint callback */
	ep->on_in(drv->cl, ep_id);
}

int stm32f_otg_dev_ep_pkt_xmit(struct stm32f_otg_drv * drv, int ep_id,
							   void * buf, unsigned int len)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	struct stm32f_otg_ep * ep;
	uint32_t diepctl;
	uint32_t mpsiz;
	uint32_t xfrsiz;
	uint32_t pktcnt;
	uint32_t free;
	uint8_t * cp;
	unsigned int idx;
	unsigned int i;

	ep = &drv->ep[ep_id];
	idx = ep->idx;

	diepctl = otg_fs->inep[idx].diepctl;
	if (ep->state != EP_IDLE) {
#if DEBUG
		if (ep->state == EP_IN_DATA) {
			DCC_LOG4(LOG_MSG, VT_PSH VT_FMG
					 "[%d] IN_DATA EPENA=%d NAKSTS=%d DPID=%d." 
					 VT_POP, idx, 
					 diepctl & OTG_FS_EPENA ? 1 : 0, 
					 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
					 diepctl & OTG_FS_DPID ? 1 : 0);
		} else if (ep->state == EP_IN_DATA_ZLP) {
			DCC_LOG4(LOG_MSG, VT_PSH VT_FMG
					 "[%d] IN_DATA_ZLP EPENA=%d NAKSTS=%d DPID=%d." 
					 VT_POP, idx, 
					 diepctl & OTG_FS_EPENA ? 1 : 0, 
					 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
					 diepctl & OTG_FS_DPID ? 1 : 0);
		} else if (ep->state == EP_IN_ZLP) {
			DCC_LOG4(LOG_MSG, VT_PSH VT_FMG
					 "[%d] IN_ZLP EPENA=%d NAKSTS=%d DPID=%d." 
					 VT_POP, idx, 
					 diepctl & OTG_FS_EPENA ? 1 : 0, 
					 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
					 diepctl & OTG_FS_DPID ? 1 : 0);
		} else {
			DCC_LOG5(LOG_WARNING, VT_PSH VT_FMG
					 "[%d] state=%d EPENA=%d NAKSTS=%d DPID=%d." 
					 VT_POP, idx, ep->state,
					 diepctl & OTG_FS_EPENA ? 1 : 0, 
					 diepctl & OTG_FS_NAKSTS ? 1 : 0, 
					 diepctl & OTG_FS_DPID ? 1 : 0);
		}
#endif
		return 0;
	}

	mpsiz = OTG_FS_MPSIZ_GET(diepctl);
	free = otg_fs->inep[idx].dtxfsts * 4;
	xfrsiz = MIN(len, free);

	if (xfrsiz > 0) {
		pktcnt = (xfrsiz + (mpsiz - 1)) / mpsiz;
		if (pktcnt > STM32_OTG_FS_BULK_PKTS) {
			pktcnt = STM32_OTG_FS_BULK_PKTS;
			xfrsiz = STM32_OTG_FS_BULK_PKTS * mpsiz;
		} 
	} else {
		/* zero lenght packet */
		pktcnt = 1;
		xfrsiz = 0;
	}

	if ((xfrsiz % mpsiz) == 0) {
		ep->state = EP_IN_DATA_ZLP; /* data + ZLP */
		DCC_LOG3(LOG_INFO, VT_PSH VT_FGR 
				 "[%d] [IDLE]->[IN_DATA_ZLP]! pktcnt=%d xfrsiz=%d" VT_POP,
				 idx, pktcnt, xfrsiz);
	} else {
		ep->state = EP_IN_DATA;
		DCC_LOG3(LOG_INFO, VT_PSH VT_FGR 
				 "[%d] [IDLE]->[IN_DATA] pktcnt=%d xfrsiz=%d" VT_POP, 
				 idx, pktcnt, xfrsiz);
	}

	otg_fs->inep[idx].dieptsiz = OTG_FS_PKTCNT_SET(pktcnt) | 
		OTG_FS_XFRSIZ_SET(xfrsiz); 
	otg_fs->inep[idx].diepctl = diepctl | OTG_FS_EPENA; 

	cp = (uint8_t *)buf;
	for (i = 0; i < xfrsiz; i += 4) {
		uint32_t data;
		data = cp[0] + (cp[1] << 8) + (cp[2] << 16) + (cp[3] << 24);
		otg_fs->dfifo[idx].push = data;
		cp += 4;
	}	

	return xfrsiz;
}

static void __ep0_rx_pop(struct stm32f_otg_drv * drv, int len)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	volatile uint32_t * pop = &otg_fs->dfifo[0].pop;
	struct stm32f_otg_ep * ep = &drv->ep[0];
	uint8_t * dst;
	uint32_t data;
	int wcnt;
	int rem;
	int cnt;
	int i;

	/* Number of words in the receive fifo */
	wcnt = (len + 3) / 4;
	DCC_LOG1(LOG_MSG, "poping %d words from FIFO.", wcnt);

	cnt = ep->xfr_cnt;
	rem = ep->xfr_len - cnt;
	if (rem >= len) {
		/* If we have enough room in the destination buffer
		 * pop data from the fifo and copy to destination buffer */
		dst = &ep->xfr_ptr[cnt];
		for (i = 0; i < wcnt; ++i) {
			data = *pop;
			*dst++ = data;
			*dst++ = data >> 8;
			*dst++ = data >> 16;
			*dst++ = data >> 24;
		}
		ep->xfr_cnt = cnt + len;
	} else {
		DCC_LOG(LOG_WARNING, "not room to copy the whole packet, discarding!");
		for (i = 0; i < wcnt; ++i) {
			data = *pop;
			(void)data;
		}
	}
}

void __ep_out_xfer_comp(struct stm32f_otg_drv * drv, int idx)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	int ep_id = idx;
	struct stm32f_otg_ep * ep = &drv->ep[ep_id];
	uint32_t pktcnt;
	uint32_t xfrsiz;
	uint32_t doepctl;
	uint32_t mpsiz;

	/* Disable SOF interrupts */
	//				otg_fs->gintmsk &= ~OTG_FS_SOFM;
	doepctl = otg_fs->outep[idx].doepctl;
	mpsiz = OTG_FS_MPSIZ_GET(doepctl);
	xfrsiz = ep->xfr_max;
	pktcnt = xfrsiz / mpsiz;

	DCC_LOG3(LOG_MSG, VT_PSH VT_FYW 
			 "OTG_EPENA=%d pktcnt=%d xfrsiz=%d" VT_POP,
			 (doepctl & OTG_FS_EPENA) ? 1 : 0, pktcnt, xfrsiz);

	/* Prepare EP_OUT to receive */
	otg_fs->outep[idx].doeptsiz = OTG_FS_PKTCNT_SET(pktcnt) |
		OTG_FS_XFRSIZ_SET(xfrsiz);

	/* 5. After the data payload is popped from the receive FIFO, the 
	   RXFLVL interrupt (OTG_FS_GINTSTS) must be unmasked. */
	doepctl |= OTG_FS_EPENA | OTG_FS_CNAK; 
	/* Clear NAK, prepare to receive  ... */
	otg_fs->outep[idx].doepctl = doepctl;
}

int stm32f_otg_dev_ep_pkt_recv(struct stm32f_otg_drv * drv, int ep_id,
							   void * buf, int len)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	uint8_t * cp = (uint8_t *)buf;
	struct stm32f_otg_ep * ep;
	uint32_t data;
	uint32_t doeptsiz;
	uint32_t pktcnt;
	uint32_t xfrsiz;
	uint32_t doepctl;
	int idx;
	int cnt;
	int rem;
	int n;

#if DEBUG
	if ((unsigned int)ep_id >= OTG_EP_MAX) {
		DCC_LOG1(LOG_WARNING, "invalid ep_id=%d!", ep_id);
		return -1;
	}
#endif

	DCC_LOG2(LOG_MSG, "ep_id=%d len=%d", ep_id, len);
	ep = &drv->ep[ep_id];
	idx = ep->idx;
	
	/* transfer data from fifo */
	rem = ep->xfr_len - ep->xfr_cnt;
	cnt = MIN(len, rem);
	if (cnt == 0)
		return 0;

	doepctl = otg_fs->outep[idx].doepctl;
	doeptsiz = otg_fs->outep[idx].doeptsiz;
	pktcnt = OTG_FS_PKTCNT_GET(doeptsiz);
	(void)pktcnt;
	xfrsiz = OTG_FS_XFRSIZ_GET(doeptsiz);
	(void)xfrsiz;

	if ((n = ep->xfr_cnt & 0x003) > 0) {
		n = 4 - n;
		n = MIN(n, cnt);

		data = ep->xfr_dat;
		switch (n) {
			case 3:
				*cp++ = data;
				data >>= 8;
				/* FALLTHROUGH */
			case 2:
				*cp++ = data;
				data >>= 8;
				/* FALLTHROUGH */
			case 1:
				*cp++ = data;
				data >>= 8;
		}
		ep->xfr_dat = data;
	}

	if (doepctl & OTG_FS_EPENA) {
		n = cnt - n;
		while (n >= 4) {
			/* word by word trasfer */
			data = otg_fs->dfifo[idx].pop;
			cp[0] = data;
			cp[1] = data >> 8;
			cp[2] = data >> 16;
			cp[3] = data >> 24;
			cp += 4;
			n -= 4;
		}

		if (n > 0) {
			/* remaining data */
			data = otg_fs->dfifo[idx].pop;
			switch (n) {
			case 3:
				*cp++ = data;
				data >>= 8;
				/* FALLTHROUGH */
			case 2:
				*cp++ = data;
				data >>= 8;
				/* FALLTHROUGH */
			case 1:
				*cp++ = data;
				data >>= 8;
			}
			ep->xfr_dat = data;
		}

	
	} else {
		DCC_LOG3(LOG_ERROR, VT_PSH VT_FRD VT_UND VT_BRI
				 "OTG_EPENA=%d pktcnt=%d xfrsiz=%d" VT_POP,
				 (doepctl & OTG_FS_EPENA) ? 1 : 0, pktcnt, xfrsiz);
	}


	ep->xfr_cnt += cnt;

	rem -= cnt;

	if (((ep->xfr_len + 3) & ~0x3) == ((ep->xfr_cnt + 3) & ~0x3)) {
		/* 5. After the data payload is popped from the receive FIFO, the 
		   RXFLVL interrupt (OTG_FS_GINTSTS) must be unmasked. */
		DCC_LOG(LOG_INFO, VT_PSH VT_FYW VT_REV " FIFO IRQ enabled " VT_POP);
		/* Reenable RX fifo interrupts */
		otg_fs->gintmsk |= OTG_FS_RXFLVLM;
		/* FIXME: implement timeout... */
	}

	return cnt;
}

int stm32f_otg_dev_ep_ctl(struct stm32f_otg_drv * drv, 
						  unsigned int ep_addr, unsigned int opt)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	struct stm32f_otg_ep * ep;
	int ep_id;
	int idx;

	if ((idx = ep_addr & 0x7f) > 3) {
		DCC_LOG1(LOG_ERROR, "invalid address addr=%d", ep_addr);
		return -1;
	}

	if (ep_addr & USB_ENDPOINT_IN) {
		ep_id = idx + OTG_INEP_OFF;
		DCC_LOG(LOG_INFO, "IN ENDPOINT");
	} else {
		ep_id = idx;
		DCC_LOG(LOG_INFO, "OUT ENDPOINT");
	}

#if DEBUG
	if ((unsigned int)ep_id >= OTG_EP_MAX) {
		DCC_LOG(LOG_WARNING, "invalid EP");
		return -1;
	}
#endif

	DCC_LOG2(LOG_MSG, "ep=%d opt=%d", ep_id, opt);
	ep = &drv->ep[ep_id];

#if DEBUG
	if (idx != ep->idx) {
		DCC_LOG(LOG_WARNING, "unconfigured EP!!!!");
		return -1;
	}
#endif

	switch (opt) {
	case USB_EP_RECV_OK:
		break;

	case USB_EP_NAK_CLR:
		/* Clear NAK, prepare to receive  ... */
		otg_fs->outep[idx].doepctl |= OTG_FS_CNAK;
		break;

	case USB_EP_NAK_SET:
		otg_fs->outep[idx].doepctl |= OTG_FS_SNAK;
		break;

	case USB_EP_ZLP_SEND:
		__ep_zlp_send(otg_fs, idx);
		break;

	case USB_EP_STALL_SET:
		if (ep_addr & USB_ENDPOINT_IN)
			__ep_in_stall_set(otg_fs, idx);
		else
			__ep_out_stall_set(otg_fs, idx);
		DCC_LOG1(LOG_MSG, "[%d] [STALLED]", idx);
		ep->state = EP_STALLED;
		break;

	case USB_EP_STALL_CLR:
		if (ep_addr & USB_ENDPOINT_IN)
			__ep_in_stall_clr(otg_fs, idx);
		else
			__ep_out_stall_clr(otg_fs, idx);
		DCC_LOG1(LOG_MSG, "[%d] [IDLE]", idx);
		ep->state = EP_IDLE;
		break;

	case USB_EP_DISABLE:
		ep->state = EP_UNCONFIGURED;
		break;

	default:
		return -1;
	}

	return 0;
}

int stm32f_otg_dev_ep_init(struct stm32f_otg_drv * drv, 
						   const usb_dev_ep_info_t * info, 
						   void * xfr_buf, int buf_len)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	struct stm32f_otg_ep * ep;
	int mxpktsz = info->mxpktsz;
	int ep_id;
	int idx;

	if ((idx = info->addr & 0x7f) > 3) {
		DCC_LOG1(LOG_ERROR, "invalid address addr=%d", info->addr);
		return -1;
	}

	if (info->addr & USB_ENDPOINT_IN) {
		ep_id = idx + OTG_INEP_OFF;
		DCC_LOG(LOG_INFO, "IN ENDPOINT");
	} else {
		ep_id = idx;
		DCC_LOG(LOG_INFO, "OUT ENDPOINT");
	}

	ep = &drv->ep[ep_id];
	ep->xfr_buf = (uint8_t *)xfr_buf;
	ep->xfr_len = 0;
	ep->xfr_cnt = 0;
	ep->state = EP_IDLE;
	ep->on_ev = info->on_ev;
	ep->idx = idx;

	DCC_LOG5(LOG_INFO, "%s ep_id=%d addr=%d idx=%d mxpktsz=%d", 
			 (info->addr & USB_ENDPOINT_IN) ? "IN" : "OUT",
			 ep_id, info->addr & 0x7f, idx, mxpktsz);

	/* mask FIFO empty interrupt */
	otg_fs->diepempmsk &= ~(1 << idx);

	if (ep_id == 0) {
		ep->xfr_max = mxpktsz;

		/*
		if (ep->xfr_max > buf_len) {
			DCC_LOG(LOG_ERROR, "transfer to large to fit in the buffer!");
		} */

		/* Initialize EP0 */
		otg_fs->inep[0].diepctl = OTG_FS_TXFNUM_SET(0) |
			OTG_FS_EP0_MPSIZ_SET(mxpktsz);

		/* 3. Set up the Data FIFO RAM for each of the FIFOs
		   - Program the OTG_FS_GRXFSIZ register, to be able to receive
		   control OUT data and setup data. If thresholding is not enabled,
		   at a minimum, this must be equal to 1 max packet size of
		   control endpoint 0 + 2 Words (for the status of the control OUT
		   data packet) + 10 Words (for setup packets).
		   - Program the OTG_FS_TX0FSIZ register (depending on the FIFO number
		   chosen) to be able to transmit control IN data. At a minimum, this
		   must be equal to 1 max packet size of control endpoint 0. */

		__ep_pktbuf_alloc(drv, 0, ep->xfr_max);

		/*  4. Program the following fields in the endpoint-specific registers
			for control OUT endpoint 0 to receive a SETUP packet
			- STUPCNT = 3 in OTG_FS_DOEPTSIZ0 (to receive up to 3 back-to-back
			SETUP packets)
			At this point, all initialization required to receive SETUP packets
			is done. */

		/* Prepare to receive */
		otg_fs->outep[0].doeptsiz = OTG_FS_STUPCNT_SET(3) | 
			OTG_FS_PKTCNT_SET(1) | OTG_FS_XFRSIZ_SET(40);
		/* EP enable */
		otg_fs->outep[0].doepctl |= OTG_FS_EPENA | OTG_FS_CNAK;

		/* Unmask EP0 interrupts */
		otg_fs->daintmsk = OTG_FS_IEPM0 | OTG_FS_OEPM0;

		DCC_LOG2(LOG_MSG, "TX[0]: addr=%04x size=%d",
				 OTG_FS_TX0FSA_GET(otg_fs->dieptxf0) * 4,
				 OTG_FS_TX0FD_GET(otg_fs->dieptxf0) * 4);
	} else {
		uint32_t depctl;

		if (info->addr & USB_ENDPOINT_IN) {
			if ((info->attr & 0x03) == ENDPOINT_TYPE_BULK) {
				ep->xfr_max = STM32_OTG_FS_BULK_PKTS * mxpktsz;
			} else {
				ep->xfr_max = mxpktsz;
			}
			__ep_pktbuf_alloc(drv, idx, ep->xfr_max);
		} else {
			ep->xfr_max = mxpktsz;
		}

		depctl = OTG_FS_MPSIZ_SET(mxpktsz);
		depctl |= OTG_FS_SD0PID | OTG_FS_USBAEP;

		switch (info->attr & 0x03) {
		case ENDPOINT_TYPE_CONTROL:
			DCC_LOG(LOG_MSG, "ENDPOINT_TYPE_CONTROL");
			depctl |= OTG_FS_EPTYP_SET(OTG_FS_EPTYP_CTRL);
			break;

		case ENDPOINT_TYPE_ISOCHRONOUS:
			DCC_LOG(LOG_MSG, "ENDPOINT_TYPE_ISOCHRONOUS");
			depctl |= OTG_FS_EPTYP_SET(OTG_FS_EPTYP_ISOC);
			break;

		case ENDPOINT_TYPE_BULK:
			DCC_LOG(LOG_MSG, "ENDPOINT_TYPE_BULK");
			depctl |= OTG_FS_EPTYP_SET(OTG_FS_EPTYP_BULK);
			break;

		case ENDPOINT_TYPE_INTERRUPT:
			DCC_LOG(LOG_MSG, "ENDPOINT_TYPE_INTERRUPT");
			depctl |= OTG_FS_EPTYP_SET(OTG_FS_EPTYP_INT);
			break;
		}

		if (info->addr & USB_ENDPOINT_IN) {
			/* Set TX fifo number */
			depctl |= OTG_FS_TXFNUM_SET(idx);
			otg_fs->inep[idx].dieptsiz = 0;
			/* Activate IN endpoint */
			otg_fs->inep[idx].diepctl = depctl;
			/* Enable endpoint interrupt */
			otg_fs->daintmsk |= OTG_FS_IEPM(idx);

		} else {
			uint32_t xfrsiz;
			uint32_t pktcnt;

			/* get the size of the RX fifio */
			xfrsiz = ep->xfr_max;
			pktcnt = xfrsiz / mxpktsz;

			DCC_LOG3(LOG_MSG, VT_PSH VT_FYW 
					 "OTG_EPENA=%d pktcnt=%d xfrsiz=%d" VT_POP,
					 (depctl & OTG_FS_EPENA) ? 1 : 0, pktcnt, xfrsiz);

			otg_fs->outep[idx].doepctl = depctl | OTG_FS_SNAK;

			/* Prepare EP_OUT to receive */
			otg_fs->outep[idx].doeptsiz = OTG_FS_PKTCNT_SET(pktcnt) |
				OTG_FS_XFRSIZ_SET(xfrsiz);

			/* EP enable */
			/* FIXME: the single input fifo creates a problem as 
			   packets pending on the fifo for one endpoint blocks packets for 
			   other  endpoints. */
			/* Clear NAK, prepare to receive  ... */
			otg_fs->outep[idx].doepctl = depctl | OTG_FS_EPENA | OTG_FS_CNAK;

			/* Enable endpoint interrupt */
			otg_fs->daintmsk |= OTG_FS_OEPM(idx);
		}

	}

	/* 2. Once the endpoint is activated, the core starts decoding the
	   tokens addressed to that endpoint and sends out a valid
	   handshake for each valid token received for the
	   endpoint. */

	return ep_id;
}

#define OTG_FS_DP   STM32_GPIOA, 12
#define OTG_FS_DM   STM32_GPIOA, 11
#define OTG_FS_ID   STM32_GPIOA, 10

#ifdef STM32_OTG_FS_VBUS 
#define OTG_FS_VBUS STM32_OTG_FS_VBUS
#else
#define OTG_FS_VBUS STM32_GPIOA, 9
#endif

static inline struct stm32_gpio * vbus_gpio(struct stm32_gpio *__gpio, 
											int __pin) {
	return __gpio;
}

static void otg_io_init(void)
{
#if STM32_OTG_FS_IO_INIT
	DCC_LOG(LOG_MSG, "Configuring GPIO pins...");

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);

	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

#if STM32_OTG_FS_VBUS_CONNECT
	DCC_LOG(LOG_MSG, "Configuring VBUS GPIO ...");
	if (vbus_gpio(OTG_FS_VBUS) == STM32_GPIOB)
		stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);

	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);
	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);
#endif

#if 0
	stm32_gpio_af(OTG_FS_ID, GPIO_AF10);
	stm32_gpio_mode(OTG_FS_ID, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
#endif

#endif
}

#if STM32_OTG_FS_VBUS_CONNECT
static void otg_vbus_connect(bool connect)
{
	if (connect)
		stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);
	else
		stm32_gpio_mode(OTG_FS_VBUS, INPUT, 0);
}
#endif

static void otg_connect(struct stm32f_otg_fs * otg_fs)
{
#if STM32_OTG_FS_VBUS_CONNECT
	otg_vbus_connect(true);
#endif
	/* Connect device */
	otg_fs->dctl &= ~OTG_FS_SDIS;
	udelay(3000);
}

static void otg_disconnect(struct stm32f_otg_fs * otg_fs)
{
#if STM32_OTG_FS_VBUS_CONNECT
	otg_vbus_connect(false);
#endif
	otg_fs->dctl |= OTG_FS_SDIS;
	udelay(3000);

}

void stm32_otg_power_on(struct stm32f_otg_fs * otg_fs)
{
	DCC_LOG(LOG_MSG, "Enabling USB FS clock...");
	stm32_clk_enable(STM32_RCC, STM32_CLK_OTGFS);

	otg_connect(otg_fs);

	DCC_LOG(LOG_MSG, "[ATTACHED]");

	/* Enable Cortex interrupt */
#if STM32_OTG_FS_IRQ_ENABLE
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
#endif
}

void stm32_otg_power_off(struct stm32f_otg_fs * otg_fs)
{
	otg_disconnect(otg_fs);

	DCC_LOG(LOG_MSG, "Disabling USB device clock...");
	stm32_clk_disable(STM32_RCC, STM32_CLK_OTGFS);
}

int stm32f_otg_fs_dev_init(struct stm32f_otg_drv * drv, 
						   struct usb_class_if * cif,
						   const struct usb_class_events * ev)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;

	drv->cl = cif;
	drv->ev = ev;

	/* Initialize IO pins */
	otg_io_init();

	DCC_LOG(LOG_INFO, "Enabling USB FS clock...");
	stm32_clk_enable(STM32_RCC, STM32_CLK_OTGFS);
	
	/* Initialize as a device */
	stm32f_otg_fs_device_init(otg_fs);

	/* 2. Program the OTG_FS_GINTMSK register to unmask the 
	   following interrupts:
	   â€“ Wakeup 
	   â€“ USB reset
	   â€“ Enumeration done
	   â€“ Early suspend
	   â€“ USB suspend
	   â€“ SOF */
#if DEBUG
	otg_fs->gintmsk = 
		OTG_FS_WUIM |
		OTG_FS_SRQIM |
		OTG_FS_DISCINT |
		OTG_FS_CIDSCHGM |
		OTG_FS_LPMINTM |
		OTG_FS_RSTDETM |
		OTG_FS_IISOOXFRM |
		OTG_FS_IISOIXFRM |
		OTG_FS_OEPINTM |
		OTG_FS_IEPINTM |
		OTG_FS_EPMISM |
		OTG_FS_EOPFM |
		OTG_FS_ISOODRPM |
		OTG_FS_USBRSTM |
		OTG_FS_ENUMDNEM |
		OTG_FS_USBSUSPM |
		OTG_FS_ESUSPM |
		OTG_FS_GONAKEFFM |
		OTG_FS_GINAKEFFM |
		OTG_FS_NPTXFEM |
		OTG_FS_RXFLVLM |
		OTG_FS_SOFM |
		OTG_FS_OTGINT |
		OTG_FS_MMISM;
#else
	otg_fs->gintmsk = OTG_FS_WUIM | OTG_FS_USBRSTM | OTG_FS_ENUMDNEM | 
		OTG_FS_ESUSPM | OTG_FS_USBSUSPM;
#endif

#ifdef STM32F446X
	otg_connect(otg_fs);
#endif

	/* Enable Cortex interrupt */
#if STM32_OTG_FS_IRQ_ENABLE
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
#endif

	DCC_LOG(LOG_MSG, "done ----------------------------------------");

	return 0;
}

/* 
 *  ---------------------------------------------------------------------------
 * Interrupt handling
 * ---------------------------------------------------------------------------
 */

static void stm32f_otg_dev_ep_out(struct stm32f_otg_drv * drv, 
								  int idx, int len)
{
	struct stm32f_otg_ep * ep = &drv->ep[idx];
//	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	int ep_id = idx;

	DCC_LOG1(LOG_MSG, "ep_id=%d", ep_id);

	ep->xfr_len = len;
	ep->xfr_cnt = 0;

	/* FIXME: the single input fifo creates a problem as 
	   packets pending on the fifo for one endpoint blocks packets for 
	   other  endpoints. Ex: an outstanding OUT packet may block a control
	   packet. Either the upper layer garantees the removal or
	   buffering at this layer should be implemented.
	 */
	/* XXX: set the nak on endpoint ???? */
//	otg_fs->outep[idx].doepctl |= OTG_FS_SNAK;

	/* call class endpoint callback */
	ep->on_out(drv->cl, ep_id, len);
}

static void stm32f_otg_dev_ep0_out(struct stm32f_otg_drv * drv)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	struct stm32f_otg_ep * ep = &drv->ep[0];

	/* last and only transfer */
	if (ep->state == EP_OUT_DATA_LAST) {
		ep->state = EP_WAIT_STATUS_IN;
		DCC_LOG(LOG_MSG, "EP0 [WAIT_STATUS_IN]");
		__ep_zlp_send(otg_fs, 0);
		return;
	} 

	if (ep->state == EP_WAIT_STATUS_IN) {
		DCC_LOG(LOG_WARNING, "ep->state != EP_WAIT_STATUS_IN!");
	}
}

static void stm32f_otg_dev_ep0_setup(struct stm32f_otg_drv * drv)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	struct usb_request * req = &drv->req;
	struct stm32f_otg_ep * ep = &drv->ep[0];
	int len;

	DCC_LOG3(LOG_MSG, "type=0x%02x request=0x%02x len=%d ",
			req->type, req->request, req->length);

	/* No-Data control SETUP transaction */
	if (req->length == 0) {
		void * dummy = NULL;

		ep->xfr_ptr = ep->xfr_buf;
		ep->xfr_len = 0;
		ep->xfr_cnt = 0;

		DCC_LOG(LOG_INFO, "EP0 [SETUP] Host->Dev len=0");
		if (((req->request << 8) | req->type) == STD_SET_ADDRESS) {
			DCC_LOG1(LOG_MSG, "address=%d",  req->value);
			stm32f_otg_fs_addr_set(otg_fs, req->value);
		}

		ep->on_setup(drv->cl, req, dummy);
		__ep_zlp_send(otg_fs, 0);
		ep->state = EP_IDLE;
		DCC_LOG(LOG_INFO, "EP0 [IDLE]");

		return;
	}

	if (req->type & 0x80) {
		/* Control Read SETUP transaction (IN Data Phase) */
		DCC_LOG1(LOG_INFO, "EP0 [SETUP] IN Dev->Host (%d)", req->length);
		ep->xfr_ptr = NULL;
		len = ep->on_setup(drv->cl, req, (void *)&ep->xfr_ptr);
		ep->xfr_len = MIN(req->length, len);
		ep->xfr_cnt = 0;
		/* prepare fifo to transmit */
		__ep0_tx_setup(otg_fs, ep->xfr_len);
	} else {
		/* Control Write SETUP transaction (OUT Data Phase) */
		ep->xfr_ptr = ep->xfr_buf;
		ep->xfr_len = req->length;
		ep->xfr_cnt = 0;

		DCC_LOG1(LOG_INFO, "EP0 [SETUP] OUT Host->Dev (%d)", req->length);

		if (ep->xfr_len < ep->xfr_max) {
			/* last and only transfer */
			ep->state = EP_OUT_DATA_LAST;
			DCC_LOG(LOG_INFO, "EP0 [OUT_DATA_LAST] OUT Host->Dev!!!!");
		} else {
			ep->state = EP_OUT_DATA;
			DCC_LOG(LOG_INFO, "EP0 [OUT_DATA] OUT Host->Dev!!!!");
		}

		DCC_LOG(LOG_MSG, "Prepare to receive");

		/* Prepare to receive DATA */
		otg_fs->outep[0].doeptsiz = OTG_FS_STUPCNT_SET(0) |
			OTG_FS_PKTCNT_SET(1) | OTG_FS_XFRSIZ_SET(ep->xfr_max);
		/* EP enable */
		otg_fs->outep[0].doepctl |= OTG_FS_EPENA | OTG_FS_CNAK;
	}

}

static void stm32f_otg_dev_reset(struct stm32f_otg_drv * drv)
{
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	uint32_t siz;
	int i;

	DCC_LOG(LOG_INFO, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

	/* Clear the Remote Wake-up Signaling */
	otg_fs->dctl &= ~OTG_FS_RWUSIG;

	/* Clear all pending interrupts */
	otg_fs->diepmsk = 0;
	otg_fs->doepmsk = 0;
	otg_fs->daint = 0xffffffff;
	otg_fs->daintmsk = 0;
	otg_fs->diepempmsk = 0;

	for (i = 0; i < 4; i++) {
		otg_fs->inep[i].diepint = 0xff;
		otg_fs->outep[i].doepint = 0xff;
		otg_fs->inep[i].dieptsiz = 0;
	}

	for (i = 0; i < OTG_EP_MAX; i++) {
		drv->ep[i].xfr_len = 0;
		drv->ep[i].xfr_cnt = 0;
	}

	/* Flush the Tx FIFO */
	stm32f_otg_fs_txfifo_flush(otg_fs, TXFIFO_ALL);
	/* Flush the Rx FIFO */
	stm32f_otg_fs_rxfifo_flush(otg_fs);

	/* Reset global interrupts mask */
	otg_fs->gintmsk =  OTG_FS_WUIM |
			OTG_FS_USBRSTM | OTG_FS_ENUMDNEM |
			OTG_FS_ESUSPM | OTG_FS_USBSUSPM;
	//	OTG_FS_SRQIM | OTG_FS_OTGINT;

	/* Reset Device Address */
	otg_fs->dcfg &= ~OTG_FS_DAD_MSK;

	/*  Set global IN NAK */
	otg_fs->dctl |= OTG_FS_SGINAK;

	/* Endpoint initialization on USB reset */

	/* 1. Set the NAK bit for all OUT endpoints
	   – SNAK = 1 in OTG_FS_DOEPCTLx (for all OUT endpoints) */
	for (i = 0; i < OTG_OUTEP_MAX; i++) {
		otg_fs->outep[i].doepctl = OTG_FS_SNAK;
	}

	/* 2. Unmask the following interrupt bits
	   - INEP0 = 1 in OTG_FS_DAINTMSK (control 0 IN endpoint)
	   - OUTEP0 = 1 in OTG_FS_DAINTMSK (control 0 OUT endpoint)
	   - STUP = 1 in DOEPMSK
	   - XFRC = 1 in DOEPMSK
	   - XFRC = 1 in DIEPMSK
	   - TOC = 1 in DIEPMSK */
	otg_fs->doepmsk = OTG_FS_STUPM | OTG_FS_XFRCM | OTG_FS_EPDM;
	otg_fs->diepmsk = OTG_FS_TOM | OTG_FS_EPDM | OTG_FS_XFRCM;
//		OTG_FS_ITTXFEMSK;

	/* 3. Set up the Data FIFO RAM for each of the FIFOs
	   - Program the OTG_FS_GRXFSIZ register, to be able to receive
	   control OUT data and setup data. If thresholding is not enabled,
	   at a minimum, this must be equal to 1 max packet size of
	   control endpoint 0 + 2 Words (for the status of the control OUT
	   data packet) + 10 Words (for setup packets).
	   - Program the OTG_FS_TX0FSIZ register (depending on the FIFO number
	   chosen) to be able to transmit control IN data. At a minimum, this
	   must be equal to 1 max packet size of control endpoint 0. */

	/* reset fifo memory (packet buffer) allocation pointer */
	drv->fifo_addr = 0;
	/* initialize RX fifo size */
	siz = STM32_OTG_FS_RX_FIFO_SIZE;
	otg_fs->grxfsiz = siz / 4;
	/* update fifo memory allocation pointer */
	drv->fifo_addr += siz;

	DCC_LOG2(LOG_MSG, "   RX: addr=%04x size=%d",
			 0, otg_fs->grxfsiz * 4);

	drv->ev->on_reset(drv->cl);
}

/* Private USB device driver data */
struct stm32f_otg_drv stm32f_otg_fs_drv0;

void stm32f_otg_fs_isr(void)
{
	struct stm32f_otg_drv * drv = &stm32f_otg_fs_drv0;
	struct stm32f_otg_fs * otg_fs = STM32F_OTG_FS;
	uint32_t gintsts;
	uint32_t ep_intr;


	gintsts = otg_fs->gintsts & otg_fs->gintmsk;

#if DEBUG
	DCC_LOG1(LOG_MSG, "GINTSTS=0x%08x", gintsts);
#endif

	/* IN endpoints interrupts 
	 * ----------------------------------------------------------- 
	 */
	if (gintsts & OTG_FS_IEPINT) {
		uint32_t diepmsk;
		uint32_t diepint;
		uint32_t diepempmsk;
		uint32_t msk;

		DCC_LOG(LOG_JABBER, "<IEPINT>");

		ep_intr = (otg_fs->daint & otg_fs->daintmsk);
		diepmsk = otg_fs->diepmsk;
		diepempmsk = otg_fs->diepempmsk;

		if (ep_intr & OTG_FS_IEPINT0) {
			/* add the Transmit FIFO empty bit to the mask */
			msk = diepmsk | ((diepempmsk >> 0) & 0x1) << 7;
			diepint = otg_fs->inep[0].diepint & msk;
			if (diepint & OTG_FS_TXFE) {
				DCC_LOG(LOG_MSG, "[0] <IEPINT> <TXFE>");
				__ep0_tx_push(drv);
			}
			if (diepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_MSG, "[0] <IEPINT> <XFRC>");
				__ep0_tx_done(drv);
			} 
			if (diepint & OTG_FS_TOC) {
				/* Bit 3 - Timeout condition */
				DCC_LOG(LOG_WARNING, "[0] <IEPINT> <OTG_FS_TOC>");
			} 
			/* clear interrupts */
			otg_fs->inep[0].diepint = diepint;
		}

#if (OTG_INEP_MAX > 0)
		if (ep_intr & OTG_FS_IEPINT1) {
			diepint = otg_fs->inep[1].diepint;
#if 0
			/* add the Transmit FIFO empty bit to the mask */
			msk = diepmsk | ((diepempmsk >> 1) & 0x1) << 7;
			if (diepint & OTG_FS_TXFE) {
				DCC_LOG(LOG_MSG, "[1] <IEPINT> <TXFE>");
			}
#endif
			
			if (diepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_MSG, VT_PSH VT_FGR
						"[1] <IEPINT> <IN XFRC>" VT_POP);
				__ep_tx_done(drv, 1);
			} 
			if (diepint & OTG_FS_TOC) {
				/* Bit 3 - Timeout condition */
				DCC_LOG(LOG_WARNING, VT_PSH VT_FGR
						"[1] <IEPINT> <OTG_FS_TOC>" VT_POP);
			} 
			/* clear interrupts */
			otg_fs->inep[1].diepint = diepint;
		}
#endif

#if (OTG_INEP_MAX > 1)
		if (ep_intr & OTG_FS_IEPINT2) {
			diepint = otg_fs->inep[2].diepint;
#if 0
			/* add the Transmit FIFO empty bit to the mask */
			msk = diepmsk | ((diepempmsk >> 2) & 0x1) << 7;
			diepint = otg_fs->inep[2].diepint & msk;
			if (diepint & OTG_FS_TXFE) {
				DCC_LOG(LOG_MSG, "[2] <IEPINT> <TXFE>");
			}
#endif
			if (diepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_MSG, "[2] <IEPINT> <XFRC>");
				__ep_tx_done(drv, 2);
			}

			if (diepint & OTG_FS_TOC) {
				/* Bit 3 - Timeout condition */
				DCC_LOG(LOG_WARNING, "[2] <IEPINT> <OTG_FS_TOC>");
			} 
			otg_fs->inep[2].diepint = diepint;
		}
#endif

#if (OTG_INEP_MAX > 2)
		if (ep_intr & OTG_FS_IEPINT3) {
			diepint = otg_fs->inep[3].diepint;
#if 0
			/* add the Transmit FIFO empty bit to the mask */
			msk = diepmsk | ((diepempmsk >> 3) & 0x1) << 7;
			if (diepint & OTG_FS_TXFE) {
				DCC_LOG(LOG_MSG, "[3] <IEPINT> <TXFE>");
			}
#endif
			if (diepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_MSG, "[3] <IEPINT> <XFRC>");
				__ep_tx_done(drv, 3);
			}
			if (diepint & OTG_FS_TOC) {
				/* Bit 3 - Timeout condition */
				DCC_LOG(LOG_WARNING, "[3] <IEPINT> <OTG_FS_TOC>");
			} 
			otg_fs->inep[3].diepint = diepint;
		}
#endif
	}

	/* OUT endpoints interrupts 
	 * ----------------------------------------------------------- 
	 */

	if (gintsts & OTG_FS_OEPINT) {
		uint32_t doepmsk;

		ep_intr = (otg_fs->daint & otg_fs->daintmsk);
		doepmsk = otg_fs->doepmsk;

		DCC_LOG(LOG_JABBER, "<OEPINT>");

		if (ep_intr & OTG_FS_OEPINT0) {
			uint32_t doepint;

			doepint = otg_fs->outep[0].doepint & doepmsk;
			if (doepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_INFO, VT_PSH VT_FYW 
						"[0] <OEPINT> <OUT_XFRC>" VT_POP);
				stm32f_otg_dev_ep0_out(drv);
			}
			if (doepint & OTG_FS_EPDISD) {
				DCC_LOG(LOG_JABBER, "[0] <OEPINT> <EPDISD>");
			}
			if (doepint & OTG_FS_STUP) {
				DCC_LOG(LOG_INFO, VT_PSH VT_FYW "[0] <OEPINT> <STUP>" VT_POP);
				stm32f_otg_dev_ep0_setup(drv);
			}
			/* clear interrupts */
			otg_fs->outep[0].doepint = doepint;

		}
#if 1
#if (OTG_OUTEP_MAX > 0)
		if (ep_intr & OTG_FS_OEPINT1) {
			uint32_t doepint;

			doepint = otg_fs->outep[1].doepint & doepmsk;
			if (doepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_INFO, "[1] <OEPINT> <OUT_XFRC>");
			}
			if (doepint & OTG_FS_EPDISD) {
				DCC_LOG(LOG_INFO, "[1] <OEPINT> <EPDISD>");
			}
			if (doepint & OTG_FS_STUP) {
				DCC_LOG(LOG_INFO, "[1] <OEPINT> <STUP>");
			}
			/* clear interrupts */
			otg_fs->outep[1].doepint = doepint;
		}
#endif 

#if (OTG_OUTEP_MAX > 1)
		if (ep_intr & OTG_FS_OEPINT2) {
			uint32_t doepint;

			doepint = otg_fs->outep[2].doepint & doepmsk;
			if (doepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_MSG, "[2] <OEPINT> <OUT_XFRC>");
			}
			if (doepint & OTG_FS_EPDISD) {
				DCC_LOG(LOG_MSG, "[2] <OEPINT> <EPDISD>");
			}
			if (doepint & OTG_FS_STUP) {
				DCC_LOG(LOG_MSG, "[2] <OEPINT> <STUP>");
			}
			/* clear interrupts */
			otg_fs->outep[2].doepint = doepint;
		}
#endif 

#if (OTG_OUTEP_MAX > 2)
		if (ep_intr & OTG_FS_OEPINT3) {
			uint32_t doepint;

			doepint = otg_fs->outep[3].doepint & doepmsk;
			if (doepint & OTG_FS_XFRC) {
				DCC_LOG(LOG_MSG, "[3] <OEPINT> <OUT_XFRC>");
			}
			if (doepint & OTG_FS_EPDISD) {
				DCC_LOG(LOG_MSG, "[3] <OEPINT> <EPDISD>");
			}
			if (doepint & OTG_FS_STUP) {
				DCC_LOG(LOG_MSG, "[3] <OEPINT> <STUP>");
			}
			/* clear interrupts */
			otg_fs->outep[3].doepint = doepint;
		}
#endif
#endif
	}


	/* RxFIFO non-empty */
	if (gintsts & OTG_FS_RXFLVL) {
		uint32_t grxsts;
		int epnum;
		int len;

		/* 1. On catching an RXFLVL interrupt (OTG_FS_GINTSTS register),
		   the application must read the Receive status pop
		   register (OTG_FS_GRXSTSP). */
		grxsts = otg_fs->grxstsp;

		epnum = OTG_FS_EPNUM_GET(grxsts);
		len = OTG_FS_BCNT_GET(grxsts);

		if (epnum == 0) {
			/* 3. If the received packet’s byte count is not 0, the byte count
			   amount of data is popped from the receive Data FIFO and stored 
			   in memory. If the received packet byte count is 0, no data is 
			   popped from the receive data FIFO. */
			switch (grxsts & OTG_FS_PKTSTS_MSK) {
			case OTG_FS_PKTSTS_GOUT_NACK:
				/* Global OUT NAK (triggers an interrupt) */
				DCC_LOG(LOG_MSG, VT_PSH VT_FYW
						"[0] <RXFLVL> <GOUT_NACK>" VT_POP);
				break;
			case OTG_FS_PKTSTS_OUT_DATA_UPDT: {
				/* OUT data packet received */
				DCC_LOG1(LOG_INFO, VT_PSH VT_FYW
						"[0] <RXFLVL> <OUT_DATA_UPDT> len=%d" VT_POP, len);
				__ep0_rx_pop(drv, len);
				break;
			}
			case OTG_FS_PKTSTS_OUT_XFER_COMP:
				DCC_LOG(LOG_MSG, VT_PSH VT_FYW
						"[0] <RXFLVL> <OUT_XFER_COMP>" VT_POP);
				break;
			case OTG_FS_PKTSTS_SETUP_COMP:
				/* SETUP transaction completed (triggers an interrupt) */
				DCC_LOG(LOG_MSG, VT_PSH VT_FYW
						"[0] <RXFLVL> <SETUP_COMP>" VT_POP);
				break;
			case OTG_FS_PKTSTS_SETUP_UPDT:
#if DEBUG
				/* SETUP data packet received */
				if (len != 8) {
					DCC_LOG(LOG_ERROR, VT_PSH VT_FYW VT_BRI VT_REV  
							"setup data len != 8!" VT_POP);
				}
#endif
				/* Copy the received setup packet into the setup buffer */
				__copy_from_pktbuf(&drv->req, &otg_fs->dfifo[0].pop, 8);

				DCC_LOG3(LOG_INFO, VT_PSH VT_FYW
						"[0] <RXFLVL> <SETUP_UPDT> typ=0x%02x req=0x%02x len=%d"
						VT_POP, 
						drv->req.type, drv->req.request, drv->req.length);

				break;
			}
		} else {
			switch (grxsts & OTG_FS_PKTSTS_MSK) {
			case OTG_FS_PKTSTS_OUT_DATA_UPDT:
				/* OUT data packet received */
				DCC_LOG1(LOG_INFO, VT_PSH VT_FCY
						 "[%d] <RXFLVL> <OUT_DATA_UPDT>" VT_POP , epnum);
				/* 2. The application can mask the RXFLVL interrupt (in
				   OTG_FS_GINTSTS) by writing to RXFLVL = 0 (in
				   OTG_FS_GINTMSK), until it has read the packet from
				   the receive FIFO. */
				otg_fs->gintmsk &= ~OTG_FS_RXFLVLM;
				DCC_LOG(LOG_INFO, VT_PSH VT_FYW VT_REV 
						" FIFO IRQ disabled /!\\ " VT_POP);
				/* Enable SOF interrupts */
//				otg_fs->gintmsk |=  OTG_FS_SOFM;
				stm32f_otg_dev_ep_out(drv, epnum, len);
				break;
			case OTG_FS_PKTSTS_OUT_XFER_COMP:
				DCC_LOG1(LOG_INFO, VT_PSH VT_FCY
						 "[%d] <RXFLVL> <OUT_XFER_COMP>" VT_POP , epnum);
				/* EP enable */
				/* FIXME: not clearing the NAK here reduces the performance,
				   but we to guarantee the class driver will 
				   periodically remove the packets from 
				   the fifo. Otherwise the EP0 will not receive its
				   packets and we end up with a deadlock situation */
//				otg_fs->outep[epnum].doepctl |= OTG_FS_EPENA | OTG_FS_CNAK;
				/* Disable SOF interrupts */
//				otg_fs->gintmsk &= ~OTG_FS_SOFM;
				__ep_out_xfer_comp(drv, epnum);
				break;
			}
		}

		/* 5. After the data payload is popped from the receive FIFO, the
		   RXFLVL interrupt (OTG_FS_GINTSTS) must be unmasked. */
		//	otg_fs->gintmsk |= OTG_FS_RXFLVLM;
	}

	/* Global interrupts 
	 * ----------------------------------------------------------- 
	 */

	if (gintsts & OTG_FS_ENUMDNE) {
		/* Unmask global interrupts */
		otg_fs->gintmsk |=  OTG_FS_IEPINTM | OTG_FS_OEPINTM |
			OTG_FS_IISOIXFRM | OTG_FS_IISOOXFRM | OTG_FS_RXFLVLM;
		/*  Clear global IN NAK */
		otg_fs->dctl |= OTG_FS_CGINAK;

		DCC_LOG(LOG_INFO, VT_PSH VT_FYW VT_REV " FIFO IRQ enabled " VT_POP);

	   /* Wait for the ENUMDNE interrupt in OTG_HS_GINTSTS.
	   This interrupt indicates the end of reset on the USB. On receiving this 
	   interrupt, the application must read the OTG_HS_DSTS register to 
	   determine the enumeration speed and perform the steps listed in 
	   Endpoint initialization on enumeration completion on page 1035.
	   At this point, the device is ready to accept SOF packets and perform 
	   control transfers on control endpoint 0. */
	}

	if (gintsts & OTG_FS_USBRST ) {
		/* end of bus reset */
		DCC_LOG(LOG_WARNING, VT_PSH VT_FYW VT_REV
				"<USBRST> --------------- [DEFAULT]" VT_POP);
		stm32f_otg_dev_reset(drv);
	}

#if DEBUG
	if (gintsts & OTG_FS_SRQINT) {
		/* Session request/new session detected interrupt */
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY 
				"<SRQINT>  [POWERED]" VT_POP);
		otg_fs->gintmsk |= OTG_FS_WUIM | OTG_FS_USBRSTM | OTG_FS_ENUMDNEM |
			OTG_FS_ESUSPM | OTG_FS_USBSUSPM;
	}

	if (gintsts & OTG_FS_OTGINT) {
		uint32_t gotgint = otg_fs->gotgint;
		DCC_LOG(LOG_MSG, "<OTGINT>");
		if (gotgint & OTG_FS_OTGINT) {
			DCC_LOG(LOG_MSG, VT_PSH VT_FCY 
					"<SEDET>  [ATTACHED]" VT_POP);
			otg_fs->gintmsk = OTG_FS_SRQIM | OTG_FS_OTGINT;
		}
		otg_fs->gotgint = gotgint;
	}

	if (gintsts & OTG_FS_ESUSP) {
		uint32_t dsts = otg_fs->dsts;
		(void)dsts;

		DCC_LOG4(LOG_INFO, VT_PSH VT_FCY
				 "<ESUSP> %s%s ENUMSPD=%d %s" VT_POP, 
				 (dsts & OTG_FS_EERR) ? " EERR" : "",
				 (dsts & OTG_FS_SUSPSTS) ? " SUSPSTS" : "",
				 OTG_FS_ENUMSPD_GET(dsts),
				 otg_fs->pcgcctl & OTG_FS_PHYSUSP ? "PHYSUSP" : "");
	}

	if (gintsts & OTG_FS_USBSUSP) {
		uint32_t dsts = otg_fs->dsts;
		(void)dsts;

		DCC_LOG4(LOG_INFO, VT_PSH VT_FCY
				 "<USBSUSP> %s%s ENUMSPD=%d %s" VT_POP, 
				 (dsts & OTG_FS_EERR) ? " EERR" : "",
				 (dsts & OTG_FS_SUSPSTS) ? " SUSPSTS" : "",
				 OTG_FS_ENUMSPD_GET(dsts),
				 otg_fs->pcgcctl & OTG_FS_PHYSUSP ? "PHYSUSP" : "");
	}

	if (gintsts & OTG_FS_GONAKEFF) {
		DCC_LOG(LOG_MSG,  VT_PSH VT_FCY VT_DIM "<GONAKEFF>" VT_POP);
		otg_fs->gintmsk &= ~OTG_FS_GONAKEFFM;
	}

	if (gintsts & OTG_FS_SOF) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<SOF>" VT_POP);
	}

	if (gintsts & OTG_FS_PTXFE) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<PTXFE>" VT_POP);
		otg_fs->gintmsk &= ~OTG_FS_PTXFEM;
	}

	if (gintsts & OTG_FS_NPTXFE) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<NPTXFE>" VT_POP);
		otg_fs->gintmsk &= ~OTG_FS_NPTXFEM;
	}

	if (gintsts & OTG_FS_CIDSCHG) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<CIDSCHG>" VT_POP);
		otg_fs->gintmsk &= ~OTG_FS_CIDSCHGM ;
	}

	if (gintsts & OTG_FS_WKUPINT) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<WKUPINT>" VT_POP);
		/* disable resume/wakeup interrupts */
	}

	if (gintsts & OTG_FS_IISOIXFR) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<IISOIXFR>" VT_POP);
	}

	if (gintsts & OTG_FS_INCOMPISOOUT) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM 
				"<INCOMPISOOUT>" VT_POP);
	}

	if (gintsts & OTG_FS_MMIS) {
		DCC_LOG(LOG_MSG, VT_PSH VT_FCY VT_DIM "<MMIS>" VT_POP);
	}
#endif
	/* clear pending interrupts */
	otg_fs->gintsts = gintsts;
}

/* USB device operations */
const struct usb_dev_ops stm32f_otg_fs_ops = {
	.dev_init = (usb_dev_init_t)stm32f_otg_fs_dev_init,
	.ep_init = (usb_dev_ep_init_t)stm32f_otg_dev_ep_init,
	.ep_ctl = (usb_dev_ep_ctl_t)stm32f_otg_dev_ep_ctl,
	.ep_pkt_xmit = (usb_dev_ep_pkt_xmit_t)stm32f_otg_dev_ep_pkt_xmit,
	.ep_pkt_recv = (usb_dev_ep_pkt_recv_t)stm32f_otg_dev_ep_pkt_recv
};

/* USB device driver */
const struct usb_dev stm32f_otg_fs_dev = {
	.priv = (void *)&stm32f_otg_fs_drv0,
	.op = &stm32f_otg_fs_ops
};

#endif /* STM32_ENABLE_OTG_FS && STM32F_OTG_FS */


