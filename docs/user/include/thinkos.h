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

#ifndef __THINKOS_H__
#define __THINKOS_H__

#include <stdint.h>

/** 
 * thinkos_krn_init() - Initializes the ThinkOS kernel.
 * @opt: Optinal arguments
 * 
 * This is the longer description of thinkos_krn_init().
 *
 * Return: On return the current program execution thread turns into 
 *         the first thread of the system. 
 */
int thinkos_krn_init(unsigned int opt);


/**
 * foobar() - Brief description of foobar.
 * @arg: Description of argument of foobar.
 *
 * Longer description of foobar.
 *
 * Return: Description of return value of foobar.
 */
int foobar(int arg);

#endif /* __THINKOS_H__ */
