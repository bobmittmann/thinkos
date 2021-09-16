/* parse-markup: reST */
  
/* 
 * vt/vt.h
 *
 * Copyright(C) 2021 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the Vt library.
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
 * @file vt.h
 * @brief VT Menu API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __VT_MENU_H__
#define __VT_MENU_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <vt.h>

struct vt_menu_page;

enum vt_menu_item_kind {
	VT_MENU_ITEM_RETCODE = 0,
	VT_MENU_ITEM_SUBMENU = 1,
	VT_MENU_ITEM_LIST = 2,
	VT_MENU_ITEM_SWITCH = 3,
	VT_MENU_ITEM_BACK = 4,
};

enum vt_page_action {
	VT_MENU_GET_SELECT = 0,
	VT_MENU_SET_SELECT = 1
};

enum vt_switch_reqest {
	VT_SWITCH_GET = 0,
	VT_SWITCH_RST = 1,
	VT_SWITCH_SET = 2,
	VT_SWITCH_TOGGLE = 3,
};

#define VT_MENU_ITEM_TEXT_SZ    13 

typedef uint32_t (* vt_action_callback_t)(struct vt_win *, enum vt_page_action,
										  uint32_t, void *);

typedef bool (* vt_switch_callback_t)(struct vt_win *, enum vt_switch_reqest, 
									  void *);

/* Describes an menu item */
struct vt_menu_item {
	uint8_t kind;
	char shortcut; /* Character for quick selection */
	const char text[VT_MENU_ITEM_TEXT_SZ + 1]; /* Text to shown in the menu */  
	char const * summary; /* Summary description of item (help) */
	union {
		int retcode; /* Retrurn code, Only odd numbers allowed.
					othrewise will be treated as a submenu. */
		const struct vt_menu_page * submenu; /* poits to a submenu */
		vt_switch_callback_t f_switch;
	};		
};

#define VTM_RET(_K, _T, _D, _R) { .kind = VT_MENU_ITEM_RETCODE, \
	.shortcut = _K, .text = _T, .summary = _D, .retcode = _R } 

#define VTM_BAK(_K, _T, _D, _R) { .kind = VT_MENU_ITEM_BACK, \
	.shortcut = _K, .text = _T, .summary = _D, .retcode = _R } 

#define VTM_SUB(_K, _T, _D, _S) { .kind = VT_MENU_ITEM_SUBMENU, \
	.shortcut = _K, .text = _T, .summary = _D, .submenu = _S } 

#define VTM_LST(_K, _T, _D, _S) {  .kind = VT_MENU_ITEM_LIST, \
	.shortcut = _K, .text = _T, .summary = _D, .submenu = _S } 

#define VTM_SW(_K, _T, _D, _F) {  .kind = VT_MENU_ITEM_SWITCH, \
	.shortcut = _K, .text = _T, .summary = _D, .f_switch = _F } 



#define VT_MENU_PAGE(_PTR) (struct vt_menu_def *)(_PTR)

struct vt_item_flags_bmp {
	uint32_t inactive; /* Do not display, invalid */
	uint32_t disabled; /* Out-of-Service, gray out */
	uint32_t status;   /* Present value */
	uint32_t error;    /* Error, fault or invalid condition */
};

struct vt_menu_page {
	const char * title;
	vt_action_callback_t action;
	struct vt_item_flags_bmp * flags;
	uint8_t nitems;
	struct vt_menu_item items[];
};

#define VT_WIN_MENU(_WIN) (struct vt_win_menu *)(_WIN)

struct vt_win_menu;

#ifdef __cplusplus
extern "C" {
#endif

/* A message VT_QUIT(code) will be broadcasted 
   when an item is activated. */


struct vt_win_menu * vt_win_menu_create(struct vt_pos pos, 
										struct vt_size size,
										struct vt_attrs attr, 
										const struct vt_menu_page * page,
										void * arg);

//int vt_menu_init(struct vt_win_menu * menu, const struct vt_menu_page * page);

int vt_menu_select(struct vt_win_menu * menu);

#ifdef __cplusplus
}
#endif
#endif /* __VT_MENU_H__ */

