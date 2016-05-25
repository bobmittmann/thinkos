/* 
 * File:	 thinkos_rdp.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(c) 2003-2006 BORESTE (www.boreste.com). All Rights Reserved.
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

#ifndef __THINKOS_RDP_H__
#define __THINKOS_RDP_H__


#ifdef __cplusplus
extern "C" {
#endif

int trdb_thread_step(unsigned int id);

int trdp_app_exec(void);

int trdp_app_erase(uint32_t addr, unsigned int size);

int trdp_app_suspend(void);

int trdp_app_continue(void);

int trdp_app_reset(void);


int trdp_breakpoint_set(uint32_t addr, uint32_t size);

int trdp_breakpoint_clear(uint32_t addr, uint32_t size);

int trdp_breakpoint_clear_all(void);


int trdp_watchpoint_set(uint32_t addr, uint32_t size, int access);

int trdp_watchpoint_clear(uint32_t addr, uint32_t size);

int trdp_watchpoint_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_RDP_H__ */

