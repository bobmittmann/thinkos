/* 
 * File:	 /thinkos/string.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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

#ifndef __THINKOS_STRING_H__
#define __THINKOS_STRING_H__

#ifndef __THINKOS_DBGMON__
#error "Never use <thinkos/string.h> directly; include <thinkos.h> instead."
#endif 

#define _(__STR__) \
	 { static const char _s[] __attribute__ ((section(".tinkos.str"))) = __STR__; s; }


#define TAG("") \
	STR = __str
	 { static const char _s[] __attribute__ ((section(".tinkos.str"))) = __STR__; s; }



#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_STRING_H__ */
