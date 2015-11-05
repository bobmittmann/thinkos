/* ---------------------------------------------------------------------------
 * File: net.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __NET_H__
#define __NET_H__

#ifdef __cplusplus
extern "C" {
#endif

int netlnk_recv(void * data, unsigned int max);

int netlnk_send(unsigned int daddr, void * data, unsigned int len);

void netlnk_init(void);

unsigned int netlnk_addr(void);

#ifdef __cplusplus
}
#endif	

#endif /* __NET_H__ */

