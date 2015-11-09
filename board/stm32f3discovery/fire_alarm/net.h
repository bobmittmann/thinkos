/* ---------------------------------------------------------------------------
 * File: net.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __NET_H__
#define __NET_H__

#include "falrm.h"

enum msg_type {
	MSG_SYS_STARTUP = 0,
	MSG_ZONE_STATUS = 1,
	MSG_ALARM_ACK   = 2
};

#ifdef __cplusplus
extern "C" {
#endif

void net_init(void);

unsigned int net_local_addr(void);

int net_recv(uint8_t * msg_type, void * data, unsigned int max);

int net_send(uint8_t msg_type, const void * data, unsigned int max);

#ifdef __cplusplus
}
#endif	

#endif /* __NET_H__ */

