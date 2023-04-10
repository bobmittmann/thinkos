/* 
 * Copyright(C) 2015 Robinson Mittmann. All Rights Reserved.
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
 * @file mstp_lnk.h
 * @brief MST/TP link layer public header
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __MSTP_LNK_H__
#define __MSTP_LNK_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/serial.h>

/** @mainpage MS/TP Data Link Layer Library
 * 
 *
 * This is an implementation of the @b MS/TP @b Data @b Link @b Layer as 
 * defined in the @b ANSI/ASHRAE @b Standard @b 135-2004 clause 9.
 * 
 * "9 DATA LINK/PHYSICAL LAYERS: MASTER-SLAVE/TOKEN PASSING (MS/TP) LAN
 *
 *
 * This clause describes a Master-Slave/Token-Passing (MS/TP) data link 
 * protocol, which provides the same services to the network layer as ISO 
 * 8802-2 Logical Link Control. It uses services provided by the EIA-485 
 * physical layer. Relevant clauses of EIA-485 are deemed to be included 
 * in this standard by reference. The following hardware is assumed:
 *
 * (a) A UART (Universal Asynchronous Receiver/Transmitter) capable of 
 * transmitting and receiving eight data bits with one stop bit and no parity.
 *
 * (b) An EIA-485 transceiver whose driver may be disabled.
 *
 * (c) A timer with a resolution of five milliseconds or less."
 * 
 */

/** @struct mstp_frame_inf
 *  @brief MS/TP frame info.
 *
 * This strucutre is used by the @c mstp_lnk_send() and @c mstp_lnk_recv()
 * in addition to the payload.
 */
struct mstp_frame_inf {
	uint8_t ftype;  /**< frame type */
	uint8_t daddr; /**< destination address */
	uint8_t saddr; /**< source address */
	uint8_t qos; /**< priority */
};

/** @enum mstp_frame_type 
 * @brief MSTP Frame Types 
 *
 */
enum mstp_frame_type {
	FRM_TOKEN                  = 0x00,
	FRM_POLL_FOR_MASTER        = 0x01,
	FRM_REPLY_POLL_FOR_MASTER  = 0x02, 
	FRM_TEST_REQUEST           = 0x03,
	FRM_TEST_RESPONSE          = 0x04,
	FRM_BACNET_DATA_XPCT_REPLY = 0x05,
	FRM_BACNET_DATA_NO_REPLY   = 0x06,
	FRM_REPLY_POSTPONED        = 0x07,
	FRM_DATA_XPCT_REPLY        = 0x80,
	FRM_DATA_NO_REPLY          = 0xc0
};

struct mstp_lnk_stats {
	uint32_t rx_frm_err;
	uint32_t rx_err;
	uint32_t rx_token;
	uint32_t rx_mgmt;
	uint32_t rx_unicast;
	uint32_t rx_bcast;
	uint32_t rx_drop;
	uint32_t tx_token;
	uint32_t tx_mgmt;
	uint32_t tx_unicast;
	uint32_t tx_bcast;
	uint32_t tx_pfm;
	uint32_t token_lost;
};

/** @enum mstp_lnk_role 
 * @brief MSTP Link Roles 
 *
 */
enum mstp_lnk_role {
	MSTP_LNK_NONE    = 0,
	MSTP_LNK_SLAVE   = 1,
	MSTP_LNK_MASTER  = 2,
	MSTP_LNK_SNIFFER = 3
};

/** @enum mstp_mgmt_event
 * @brief MSTP Management Events
 */
enum mstp_mgmt_event {
	MSTP_EV_LINK_DOWN            = 0,
	MSTP_EV_LINK_UP              = 1,
	MSTP_EV_SOLE_MASTER          = 2,
	MSTP_EV_MULTIMASTER          = 3,
	MSTP_EV_PROBE_BEGIN          = 4,
	MSTP_EV_PROBE_END            = 5,
	MSTP_EV_LINK_LINE_BREAK      = 6,
	MSTP_EV_LINK_TOO_MANY_ERRORS = 7,

	MSTP_EV_LINK_REQ_ECHO        = 0x08,
	MSTP_EV_LINK_REQ_XFER        = 0x09,
	MSTP_EV_LINK_REQ_FTXB        = 0x0a,
	MSTP_EV_LINK_REQ_NOFW        = 0x0b,
	MSTP_EV_LINK_REQ_ISOA        = 0x0c,
	MSTP_EV_LINK_REQ_ROLE_SLAVE  = 0x0d,
	MSTP_EV_LINK_REQ_ROLE_MASTER = 0x0e,

	MSTP_EV_LINK_DO_ECHO         = 0x88,
	MSTP_EV_LINK_DO_XFER         = 0x89,
	MSTP_EV_LINK_DO_FTXB         = 0x8a,
	MSTP_EV_LINK_DO_NOFW         = 0x8b,
	MSTP_EV_LINK_DO_ISOA         = 0x8c,
	MSTP_EV_LINK_DO_ROLE_SLAVE   = 0x8d,
	MSTP_EV_LINK_DO_ROLE_MASTER  = 0x8e
};


/** @enum mstp_link_mgmt
 * @brief MSTP Management Events
 */
enum mstp_link_mgmt_req {
	LINK_REQ_ECHO           = 0x08,
	LINK_REQ_XFER           = 0x09,
	LINK_REQ_FTXB           = 0x0a,
	LINK_REQ_NOFW           = 0x0b,
	LINK_REQ_ISOA           = 0x0c,
	LINK_REQ_ROLE_SLAVE     = 0x0d,
	LINK_REQ_ROLE_MASTER    = 0x0e,

	LINK_REPLY_ECHO         = 0x88,
	LINK_REPLY_XFER         = 0x89,
	LINK_REPLY_FTXB         = 0x8a,
	LINK_REPLY_NOFW         = 0x8b,
	LINK_REPLY_ISOA         = 0x8c,
	LINK_REPLY_ROLE_SLAVE   = 0x8d,
	LINK_REPLY_ROLE_MASTER  = 0x8e
};


/** @def MSTP_ADDR_BCAST
 * @brief MSTP broadcast address
 *
 */
#define MSTP_ADDR_BCAST 255
 
/** @struct mstp_lnk
 *  @brief MS/TP link control strcuture.
 *
 * An opaque structure representing a MS/TP link control device.
 */
struct mstp_lnk;

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup linkmgmt MS/TP link management
 *
 * This group of functions are used to allocate, initialize,
 * and manage MS/TP links.
 *
 * @{
 */

/** @brief Alloc a MS/TP link control structure
 *
 * The link control structure is allocated from a pool of resources.
 * The link control structure should be initialized by calling the
 * @c mstp_lnk_init() function;
 * Subsequent MS/TP link operations will be performed by referencing 
 * the returning pointer.
 *
 * @return On success, a pointer to a MS/TP link control structure is
 * returned. On error @b NULL is returned.
 */
struct mstp_lnk * mstp_lnk_alloc(void);

struct mstp_lnk * mstp_lnk_getinstance(unsigned int id);

struct mstp_lnk_comm;

/** @brief Initialize a MS/TP link control structure
 *
 * This function binds a MS/TP link control structure with a tag,
 * an address and a serial device.
 *
 * @param lnk The MS/TP link control structure.
 * @param tag String naming this link.
 * @param addr The MAC address for this link.
 * @param role The role of the device on the network.
 * @param dev A serial device driver.
 * @return On success, 0 is returned. On error #-1 is returned.
 */
int mstp_lnk_init(struct mstp_lnk * lnk, const char * tag, 
				  unsigned int addr, enum mstp_lnk_role role, 
				  struct mstp_lnk_comm * comm);


/** @brief Uninitializes an MS/TP link.
 *
 * @param lnk The MS/TP link control structure.
 * @return On success, 0 is returned. On error #-1 is returned.
 */
int mstp_lnk_done(struct mstp_lnk * mstp);

/** @brief Start sending and receiving data on MS/TP link.
 *
 * @param lnk The MS/TP link control structure.
 * @return On success, 0 is returned. On error #-1 is returned.
 */
int mstp_lnk_resume(struct mstp_lnk * lnk);

/** @brief Stop sending/receiving data on MS/TP link.
 *
 * @param lnk The MS/TP link control structure.
 * @return On success, 0 is returned. On error #-1 is returned.
 */
int mstp_lnk_stop(struct mstp_lnk * lnk);

/** @brief  MS/TP link master processing loop.
 *
 * This function receives frames from the serial interface, processes 
 * them as appropriate and sends back responses, and tokens.
 * It maintains the link.
 * A thread should be reserved exclusively to call this function.
 *
 * @param lnk The MS/TP link control structure.
 */
void mstp_lnk_master_loop(struct mstp_lnk * lnk);

/** @brief  MS/TP link slave processing loop.
 *
 * This function receives frames from the serial interface, processes 
 * them as appropriate and sends back responses.
 * A thread should be reserved exclusively to call this function.
 *
 * @param lnk The MS/TP link control structure.
 */
void mstp_lnk_slave_loop(struct mstp_lnk * lnk);

/** @brief  MS/TP link sniffer processing loop.
 *
 * This function receives frames from the serial interface, processes 
 *
 * @param lnk The MS/TP link control structure.
 */
void mstp_lnk_slave_loop(struct mstp_lnk * lnk);

/** @brief  Get the MS/TP station address.
 *
 * This function returns the current station adddress.
 *
 * @param lnk The MS/TP link control structure.
 * @param cbk Callback function.
 *
 * @return On success, returns the configured address. On error #-1 is returned.
 */

int mstp_lnk_addr_get(struct mstp_lnk * lnk);

/** @brief  MS/TP link management callback registering.
 *
 * This function assigns a function to be called whenever an management event
 *
 * The callback function should be fast to avoid interference with the link
 * control.
 *
 * @param lnk The MS/TP link control structure.
 * @param cbk Callback function.
 */
int mstp_lnk_mgmt(struct mstp_lnk * lnk,
		int (*cbk)(struct mstp_lnk *, unsigned int));

int mstp_lnk_mgmt_set(struct mstp_lnk *lnk,
					  int (*cbk) (struct mstp_lnk *, unsigned int));

int mstp_lnk_stats_get(struct mstp_lnk * lnk,
					   struct mstp_lnk_stats * stats, bool reset);

unsigned int mstp_lnk_getnetmap(struct mstp_lnk * lnk, uint8_t map[],
		unsigned int max);

unsigned int mstp_lnk_active_get(struct mstp_lnk *lnk, uint8_t map[],
								 unsigned int max);

void mstp_lnk_clrnetmap (struct mstp_lnk * lnk);

bool mstp_lnk_firstaddr (struct mstp_lnk * mstp);

void mstp_lnk_activemasters (struct mstp_lnk * lnk);		

int mstp_lnk_addr_set(struct mstp_lnk *lnk, unsigned int addr);

/**@}*/




/** @defgroup linkio MS/TP link data sending/receiving 
 * This group of functions are used to send and receive
 * data trough the MS/TP links.
 *
 * The link should have been previously initialized by calling the
 * appropriated link management calls. Also the link state machine
 * must be running by calling @c mstp_lnk_loop().
 * @{
 */

/** @brief MS/TP link frame send.
 *
 * This function sends a frames on a MS/TP link.
 *
 * @param lnk The MS/TP link control structure.
 * @param buf Pointer to the frame payload.
 * @param count Number of bytes to send.
 * @param inf Pointer to a frame info structure containing the destination
 * address and the frame type.
 * @return On success, the number of bytes sent is returned.
 * On error @b -1 is returned.
 */

int mstp_lnk_send(struct mstp_lnk * lnk, const void * buf,
				  unsigned int count, const struct mstp_frame_inf * inf);

/** @brief MS/TP link frame receive.
 *
 * This function receives a frames on a MS/TP link.
 *
 * @param lnk The MS/TP link control structure.
 * @param buf Pointer to the frame payload receiving buffer.
 * @param count Maximum number of bytes to be received.
 * @param inf Pointer to a frame info structure containing the source 
 * address and the frame type.
 * @return On success, the number of bytes received is returned.
 * On error @b -1 is returned.
 */

int mstp_lnk_recv(struct mstp_lnk * lnk, void * buf, 
				  unsigned int count, struct mstp_frame_inf * inf);


/** @brief MS/TP link half_duplex operation set.
 *
 * This function configures the MS/TP link to operate in half_duplex mode.
 * or full-duplex mode.
 *
 * @param lnk The MS/TP link control structure.
 * @param on The value of the half_duplex flag:
 * 			- false, full-duplex 
 * 			- true, half duplex
 * @return On success, the previous value of the flag is returned. 
 * On error #-1 is returned. 
 *
 */
 int mstp_lnk_half_duplex_set(struct mstp_lnk * lnk, bool on);

/** @brief MS/TP link role .
 *
 * This function sets or changes the role of the MS/TP link.
 *
 * @param lnk The MS/TP link control structure.
 * @param role can be set to one of the folloing values: 
 *  - MSTP_LNK_MASTER : Master station
 *  - MSTP_LNK_SLAVE: Slave station
 *  - MSTP_LNK_SNIFFER: Monitoring station
 *
 * @return On success, the previous roler of the flag is returned. 
 * On error #-1 is returned. 
 *
 */
int mstp_lnk_role_set(struct mstp_lnk * lnk, enum mstp_lnk_role role);

int mstp_lnk_role_get(struct mstp_lnk *lnk);

int mstp_lnk_role_req_get(struct mstp_lnk *lnk);

struct mstp_lnk_mgmt_msg {
	uint8_t saddr;
	uint8_t daddr;
	uint8_t req;
	uint8_t opt;
};

ssize_t mstp_lnk_mgmt_send(struct mstp_lnk *lnk, 
						   const struct mstp_lnk_mgmt_msg * msg, 
						   const void *buf, size_t cnt);

ssize_t mstp_lnk_mgmt_recv(struct mstp_lnk *lnk, struct mstp_lnk_mgmt_msg * msg, 
						   void *buf, size_t max);


int mstp_lnk_up_timedwait(struct mstp_lnk *lnk, unsigned int tmo_ms);

/**@}*/
#ifdef __cplusplus
}
#endif

#endif /* __MSTP_LNK_H__ */

