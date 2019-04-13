/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file sys/param.h
 * @brief YARD-ICE 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

#define MIN(_A, _B) __extension__({ \
	__typeof__ (_A) __A = (_A); \
	__typeof__ (_B) __B = (_B); \
	__A < __B ? __A : __B; })

#define MAX(_A, _B) __extension__({ \
	__typeof__ (_A) __A = (_A); \
	__typeof__ (_B) __B = (_B); \
	__A > __B ? __A : __B; })

#endif	/*__SYS_PARAM_H__ */
