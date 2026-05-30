/* 
 * thinkos_svc.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */


#ifndef __THINKOS_I_CALLS_H__
#define __THINKOS_I_CALLS_H__

#ifndef __THINKOS_I_CALLS__
#error "Never use <thinkos/i_calls.h> directly; include <thinkos.h> instead."
#endif 

struct thinkos_i_call_tab {
	void (* sem_post)(int sem);
	void (* ev_raise)(int set, int ev);
	void (* flag_give)(int flag);
	void (* gate_open)(int gate);
};

/* --------------------------------------------------------------------------
 * IRQ
 * --------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Fixed vector for i_call table pointer */
#define THINKOS_KRN_I_CALL_PTR (7 * 4)

static inline void  __attribute__((always_inline)) 
	thinkos_sem_post_i(int sem) {
	struct thinkos_i_call_tab ** i_call = 
		(struct thinkos_i_call_tab **)(THINKOS_KRN_I_CALL_PTR);
	(*i_call)->sem_post(sem);
}

static inline void __attribute__((always_inline)) 
	thinkos_ev_raise_i(int set, int ev) { 
	struct thinkos_i_call_tab ** i_call = 
		(struct thinkos_i_call_tab **)(THINKOS_KRN_I_CALL_PTR);
	(*i_call)->ev_raise(set, ev);
}

static inline void __attribute__((always_inline)) 
	thinkos_flag_give_i(int flag) {
	struct thinkos_i_call_tab ** i_call = 
		(struct thinkos_i_call_tab **)(THINKOS_KRN_I_CALL_PTR);
	(*i_call)->flag_give(flag);
}

static inline void __attribute__((always_inline)) 
	thinkos_gate_open_i(int gate) {
	struct thinkos_i_call_tab ** i_call = 
		(struct thinkos_i_call_tab **)(THINKOS_KRN_I_CALL_PTR);
	(*i_call)->gate_open(gate);
}

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_I_CALLS_H__ */

