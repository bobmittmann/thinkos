/* ---------------------------------------------------------------------------
 * File: falrm.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __FALRM_H__
#define __FALRM_H__

#include <stdint.h>
#include <stdbool.h>

/* Maximum number of panels allowed in the system */
#define PANEL_COUNT 2
/* Number of local detection zones for a single panel */
#define ZONE_LOCAL_COUNT  4
/* Number of remote zones tracked by a panel */
#define ZONE_REMOTE_COUNT 4
/* Total number of zones supported by a single panel */
#define ZONE_TOTAL_COUNT (ZONE_LOCAL_COUNT + ZONE_REMOTE_COUNT)

/* Fire Alarm Zone Address */
struct zone_id {
	uint8_t panel; /* The panel number: 1,2 */
	uint8_t input; /* The input number: 1..4 */
};

/* Fire Alarm Detection Zone Status */
struct zone_status {
	struct zone_id zone; /* The zone address: 1.1 .. 2.4 */
	bool active;         /* The status of the zone */
};

#ifdef __cplusplus
extern "C" {
#endif

int falrm_init(void);

#ifdef __cplusplus
}
#endif	

#endif /* __FALRM_H__ */

