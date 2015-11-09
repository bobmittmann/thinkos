/* ---------------------------------------------------------------------------
 * File: zone.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __ZONE_H__
#define __ZONE_H__

#include "falrm.h"

#ifdef __cplusplus
extern "C" {
#endif

void zonedrv_init(void);

void zone_status_get(struct zone_status * pzs);

#ifdef __cplusplus
}
#endif	

#endif /* __ZONE_H__ */

