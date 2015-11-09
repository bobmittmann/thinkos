/* ---------------------------------------------------------------------------
 * File: falrm.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __FALRM_H__
#define __FALRM_H__

#include <stdint.h>
#include <stdbool.h>

#define PANEL_COUNT 2

#define ZONE_LOCAL_COUNT 4
#define ZONE_REMOTE_COUNT 4
#define ZONE_TOTAL_COUNT (ZONE_LOCAL_COUNT + ZONE_REMOTE_COUNT)

struct zone_id {
	uint8_t panel;
	uint8_t input;
};

/* Firel Alarm dwtection Zone Status */ 
struct zone_status {
	struct zone_id zone;
	bool active;
};

#ifdef __cplusplus
extern "C" {
#endif

int falrm_init(void);

#ifdef __cplusplus
}
#endif	

#endif /* __FALRM_H__ */

