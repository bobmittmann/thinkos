/* 
 * Copyright(c) 2004-2012 BORESTE (www.boreste.com). All Rights Reserved.
 *
 * This file is part of the libtcpip.
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
 * @file tcpip/dhcp.h
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/*! \addtogroup tcpip
 * @{
 */
/*! \defgroup tcpip_dhcp tcpip/dhcp.h
 *	\brief DHCP client module header.
 *
 *	Usage: \n
 *	1) Create a struct dhcp_min filled with valid data; \n
 *	2) Start DHCP client; \n
 *	3) Keep ticking it until configuration callback is received; \n
 *	4) Start using the network after configuration. \n
 *
 *	Optionally, the programmer may choose to stop the DHCP client and stop 
 *	ticking it to free some resources. Note that this will result in a 
 *	misbehaviouring implementation (which may or may not cause harm, depending 
 *	on the case). \n
 *
 *	The program may save (by whatever means) the last IP acquired by DHCP so 
 *	it may be used across reboots.
 *
 *	\see RFC 2131 sections 1, 1.3, 1.5, 1.6, 2.2, 3.6 and 3.7 for an overview 
 *	of the protocol.
 * @{
 */
/*! \file
 *
 *	Revision tag: $Revision: 2.3 $ $Date: 2006/04/04 13:27:05 $
 *	@author Carlos A. V. e Vieira (carlos.vieira@boreste.com)
 */

#ifndef __TCPIP_DHCP_H__
#define __TCPIP_DHCP_H__

#include <tcpip/ifnet.h>

struct dhcp;


/*! DHCP client states */
enum dhcp_state {
	DHCP_IDLE       = 0,
	DHCP_INIT       = 1,
	DHCP_SELECTING	= 2,
	DHCP_REQUESTING	= 3,
	//DHCP_CHECKING	= 8,
	//DHCP_INFORMING	= 9,
	//DHCP_PERMANENT	= 10,
	DHCP_BOUND		= 4,
	DHCP_RENEWING	= 5,
	DHCP_REBINDING	= 6,
	/* not yet implemented
	DHCP_RELEASING	= 11, */
	DHCP_INIT_REBOOT = 7,
	DHCP_REBOOTING	= 8,
	//DHCP_BACKING_OFF	= 12,
	DHCP_OFF		= 13
};

enum dhcp_event {
	DHCP_EVENT_BIND = 1,
	DHCP_EVENT_TIMEOUT = 2,
	DHCP_EVENT_LEASE_OUT = 3
};

#ifdef __cplusplus
extern "C" {
#endif

/*!	\brief Starts DHCP client daemon.
 *
 *	\return
 *	Returns <b>0</b> on success and less then 0 on failure.
 */
int dhcpc_start(void);

/*!	\brief Stops DHCP client daemon.
 *
 *	Stops listening on the allocated UDP socket (thus stopping all DHCP client 
 *	action).
 */
void dhcpc_stop(void);

/*!	\brief Binds a DHCP client to a network interface.
 *
 *	Begins searching for a DHCP server to negotiate network parameters and 
 *	configure local network subsystem (IP address, routers, etc.).
 *
 *	\param __ifn
 *	Pointer to the descriptor whose interface is to be configured.
 *	\param __callback
 *	Function to be called in response to DHCP events.
 *	\return
 *	Returns <b>0</b> on success and less then 0 on failure.
 */

int dhcp_ifrelease(struct ifnet * __ifn);

int dhcp_ifdone(struct ifnet * __ifn);

/*! Returns current state of the DHCP client state machine. */
enum dhcp_state dhcp_get_state(struct dhcp * dhcp);

struct dhcp * dhcpc_ifattach(struct ifnet * __ifn,
							void (* __callback)(struct dhcp *, enum dhcp_event));

int dhcp_pause(struct dhcp * __dhcp);

int dhcp_resume(struct dhcp * __dhcp);

struct dhcp * dhcpc_ifconfig(struct ifnet * __ifn,
							void (* __callback)(struct dhcp *, enum dhcp_event));


/* get network ipv4 address */
int dhcp_ipv4_get(struct dhcp * __dhcp, in_addr_t * __addr, in_addr_t * __mask);


int dhcp_gateway_get(struct dhcp * __dhcp, in_addr_t * __gw);

struct ifnet * dhcp_ifget(struct dhcp * __dhcp);

struct dhcp * dhcp_lookup(struct ifnet * __ifn);

#ifdef __cplusplus
}
#endif

#endif /* __TCPIP_DHCP_H__ */

/*! @} */
/*! @} */
