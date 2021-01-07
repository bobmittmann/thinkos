/* 
 * thinkos/kernel.h
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

#ifndef __THINKOS_CONSOLE_H__
#define __THINKOS_CONSOLE_H__

#ifndef __THINKOS_CONSOLE__
#error "<thinkos/console.h> is restricted to privileged kernel code only!"
#endif 

#ifndef __ASSEMBLER__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- 
 * Console Kernel API
 * --------------------------------------------------------------------------*/

/* intialize kernel console */
void thinkos_krn_console_init(void);

/* set/clear the kernel console's connected flag */
void thinkos_krn_console_connect_set(bool val); 

/* set/clear the kernel console's raw mode flag */
void thinkos_krn_console_raw_mode_set(bool val);

/* return the kernel console's raw mode flag state */
bool thinkos_krn_console_is_raw_mode(void); 

/* get a pointer to the console's recieving pipe */
int thinkos_console_rx_pipe_ptr(uint8_t ** ptr);
/* commit 'cnt' octets on console's recieving pipe */
void thinkos_console_rx_pipe_commit(int cnt); 

int thinkos_console_tx_pipe_ptr(uint8_t ** ptr);

void thinkos_console_tx_pipe_commit(int cnt);


int __console_getc(unsigned int tmo);

int __console_puts(const char * s);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_CONSOLE_H__ */

