/* vt_menu.h
 * ---------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file vt_menun.h
 * @brief
 * @author Bob Mittmann <bmittmann@mircomgroup.com>
 */

#ifndef __VT_MENU_H__
#define __VT_MENU_H__

#include <string.h>
#include <stdint.h>
#include <sys/vt.h>

#define VT_MENU_ENTRY_TAG_SZ 15 
#define VT_MENU_ENTRY_DESC_SZ 31 
#define VT_MENU_TITLE_SZ 30

struct vt_menu_entry {
	char key;
	char tag[VT_MENU_ENTRY_TAG_SZ + 1];
	char desc[VT_MENU_ENTRY_DESC_SZ + 1];
};

struct vt_menu_def {
	char title[VT_MENU_TITLE_SZ + 1];
	uint8_t len;
	const struct vt_menu_entry entry[];
};

struct vt_win_menu;

#ifdef __cplusplus
extern "C" {
#endif

struct vt_win_menu * vt_menu_create(struct vt_pos pos, 
								 struct vt_size size,
								 struct vt_attrs attr);

int vt_menu_init(struct vt_win_menu * menu, const struct vt_menu_def * def);

int vt_menu_select(struct vt_win_menu * menu);

#ifdef __cplusplus
}
#endif
#endif /* __VT_MENU_H__ */

