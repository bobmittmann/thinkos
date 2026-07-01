/* 
 * thinkos.h
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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

/* 
 * @file thinkos.h
 * @brief ThinkOS API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifndef __THINKOS_DRV_H__
#define __THINKOS_DRV_H__

#ifndef __ASSEMBLER__

#include <stdint.h>


/**
 * thinkos_sem_post_i() - signal a semaphore inside an interrupt handler.
 * @sem: semaphore handler
 *
 * Warning: no argument validation is performed. 
 */
void thinkos_sem_post_i(int sem);

/**
 * thinkos_ev_raise_i() - signal an event from inside an interrupt handler.
 * @set: event set handler
 * @ev: event identifier
 *
 * Warning: no argument validation is performed. 
 */
void thinkos_ev_raise_i(int set, int ev);

/**
 * thinkos_flag_give_i() - signal a flag form inside an interrupt handler.
 * @flag: flag handler
 *
 * Warning: no argument validation is performed. 
 */
void thinkos_flag_give_i(int flag);

/**
 * Open or signal the gate from inside an interrupt handler.
 * 
 * This call is similar to the @c thinkos_gate_open() except that it
 * is safe to ba called from inside an interrupt handler.
 *
 * @gate: The gate descriptor.
 *
 * Warning: no argument validation is performed. 
 */
void thinkos_gate_open_i(int gate);



#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_DRV_H__ */

