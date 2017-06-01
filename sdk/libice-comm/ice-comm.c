/* 
 * File:	ice-comm.c
 * Author:  Robinson Mittmann (bobmittmann@gmail.com)
 * Target: 
 * Comment: 
 * Copyright(C) 2012 Bob Mittmann. All Rights Reserved.
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

#include <sys/ice-comm.h>
#include <arch/cortex-m3.h>

#ifdef NEED_ICE_COMM_BLK
#define NEED_ICE_COMM_BLK 0
#endif

#if NEED_ICE_COMM_BLK
struct ice_comm_blk ice_comm_blk;
#endif

void ice_comm_sync(void) 
{
	struct ice_comm_blk * comm = ICE_COMM_BLK;
	uint32_t fm = cm3_faultmask_get(); /* save fault mask */
	uint32_t ro;
	uint32_t wo;

	cm3_cpsid_f(); /* disable interrupts and faults */

	ro = COMM_RO(0, 0, 0);
	wo = COMM_WO(0, 0, DEV_SYNC);

	comm->ro = ro;
	comm->wo = wo;

	/* Update the Debug Core Register Data Register */
	CM3_DCB->dcrdr = wo; 

	cm3_faultmask_set(fm);  /* restore fault mask */
}

void ice_comm_connect(void) 
{
	struct ice_comm_blk * comm = ICE_COMM_BLK;
	uint32_t fm = cm3_faultmask_get(); /* save fault mask */
	uint32_t wo;

	cm3_cpsid_f(); /* disable interrupts and faults */
	comm->dev = DEV_SYNC;
	
	wo = comm->wo;
	wo = COMM_DEV_SET(wo, DEV_SYNC);
	comm->wo = wo;
	CM3_DCB->dcrdr = wo; 
	while (comm->dbg != DBG_CONNECTED) {
		if (comm->dbg == DBG_SYNC) {
			wo = COMM_DEV_SET(wo, DEV_CONNECTED);
			comm->wo = wo;
			CM3_DCB->dcrdr = wo; 
		}
	}
	cm3_faultmask_set(fm);  /* restore fault mask */
}

