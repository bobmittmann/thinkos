/* 
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

/** 
 * @file erd.h
 * @brief ThinkOS liberd
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __ERD_H__
#define __ERD_H__

#include <stdint.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>

#ifdef __cplusplus
extern "C" {
#endif

void erd_stub_task(struct dmon_comm * comm);

#ifdef __cplusplus
}
#endif	

#endif /* __ERD_H__ */

