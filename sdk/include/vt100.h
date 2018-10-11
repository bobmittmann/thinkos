/* 
 * Copyright(c) 2004-2015 BORESTE (www.boreste.com). All Rights Reserved.
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
 * @file vt100.h
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __VT100_H__
#define __VT100_H__


/******************************************************************************
 VT100 definitions
******************************************************************************/

/* octal 033 = hexadecimal 01B = <ESC> */
#define ESC		"\033"

/******************************************************************************
 VT100 Set Display Attributes definitions
******************************************************************************/
/* Font mode */
#define VT100_NORMAL        0
#define VT100_BRIGHT		1
#define VT100_DIM           2
#define VT100_UNDERLINE     3
#define VT100_BLINK         4
#define VT100_REVERSE		5
#define VT100_HIDDEN		6

/* Foreground colors */
#define VT100_FG_BLACK		30
#define VT100_FG_RED		31
#define VT100_FG_GREEN		32
#define VT100_FG_YELLOW		33
#define VT100_FG_BLUE		34
#define VT100_FG_MAGENTA	35
#define VT100_FG_CYAN		36
#define VT100_FG_WHITE		37

/* Background colors */
#define VT100_BG_BLACK		40
#define VT100_BG_RED		41
#define VT100_BG_GREEN		42
#define VT100_BG_YELLOW		43
#define VT100_BG_BLUE		44
#define VT100_BG_MAGENTA	45
#define VT100_BG_CYAN		46
#define VT100_BG_WHITE		47

/* Set disp attr macro */
#define VT100_DISP_ATTR(_ATTR_) { printf(ESC"[%dm",_ATTR_); } 
#define VT100_ATTR(_ATTR_)	{ printf(ESC"[%dm",_ATTR_); }
#define VT100_GOTOXY(X, Y)	{ printf(ESC"[%d;%df", Y, X); }

/******************************************************************************
 VT100 Device Status definitions
******************************************************************************/

#define _GOTO_(Y, X)        ESC"["#Y";"#X"f"

#define _GOTOXY_            ESC"[%d;%df"

#define _TERM_RESET_		ESC "c"
#define _CURSOR_HOME_		ESC "[H"
#define _CURSOR_UP_         ESC "[A"
#define _CURSOR_DOWN_		ESC "[B"
#define _CURSOR_FORWARD_	ESC "[C"
#define _CURSOR_BACKWARD_   ESC "[D"

#define _CURSOR_SHOW_       ESC "[?25h"
#define _CURSOR_HIDE_       ESC "[?25l"

#define _CURSOR_PUSH_		ESC "[s"
#define _CURSOR_POP_		ESC "[u"
#define _ATTR_PUSH_		ESC "7"
#define _ATTR_POP_		ESC "8"

/* Clear */
#define _CLREOL_		ESC "[K"
#define _CLRSOL_		ESC "[1K"
#define _CLRLN_			ESC "[2K"
#define _CLRDOWN_		ESC "[J"
#define _CLRUP_			ESC "[1J"
#define _CLRSCR_		ESC "[2J"

/* Attributes */
#define _NORMAL_		ESC "[0m"
#define _BRIGHT_		ESC "[1m"
#define _DIM_			ESC "[2m"
#define _UNDERLINE_		ESC "[4m"
#define _BLINK_			ESC "[5m"
#define _REVERSE_		ESC "[7m"
#define _HIDDEN_		ESC "[8m"

/* Foreground colors */
#define _FG_BLACK_		ESC "[30m"
#define _FG_RED_		ESC "[31m"
#define _FG_GREEN_		ESC "[32m"
#define _FG_YELLOW_		ESC "[33m"
#define _FG_BLUE_       ESC "[34m"
#define _FG_MAGENTA_    ESC "[35m"
#define _FG_CYAN_		ESC "[36m"
#define _FG_WHITE_		ESC "[37m"

/* Background colors */
#define _BG_BLACK_		ESC "[40m"
#define _BG_RED_		ESC "[41m"
#define _BG_GREEN_		ESC "[42m"
#define _BG_YELLOW_		ESC "[43m"
#define _BG_BLUE_		ESC "[44m"
#define _BG_MAGENTA_    ESC "[45m"
#define _BG_CYAN_		ESC "[46m"
#define _BG_WHITE_		ESC "[47m"

/* Scrolling */
#define VT100_SET_SCROLL        ESC "[%d;%dr"
#define VT100_SET_SCROLL_ALL    ESC "[r"
#define VT100_SCROLL_DOWN       ESC "[D"
#define VT100_SCROLL_UP         ESC "[M"

/* Terminal Setup */
#define VT100_RESET             ESC "c"
#define VT100_LINE_WRAP_EN      ESC "7h"
#define VT100_LINE_WRAP_DIS     ESC "7l"

#define VT100_CLRSCR            ESC "[2J"
#define VT100_GOTO              ESC "[%d;%df"
#define VT100_CLREOL            ESC "[K"
#define VT100_CURSOR_SHOW       ESC "[?25h"
#define VT100_CURSOR_HIDE       ESC "[?25l"
#define VT100_CURSOR_SAVE       ESC "[s"
#define VT100_CURSOR_UNSAVE     ESC "[u"
#define VT100_ATTR_SAVE         ESC "[7"
#define VT100_ATTR_RESTORE      ESC "[8"

#define VT100_CURSOR_HOME       ESC "[H"
#define VT100_CURSOR_UP         ESC "[A"
#define VT100_CURSOR_DOWN       ESC "[B"
#define VT100_CURSOR_FORWARD    ESC "[C"
#define VT100_CURSOR_BACKWARD   ESC "[D"

/* Save Cursor & Attrs */
#define VT100_SAVE              ESC "7"
/* Restore Cursor & Attrs */
#define VT100_RESTORE           ESC "8"

/* Font mode */
#define VT100_ATTR_NORMAL       ESC "[0m"
#define VT100_ATTR_BRIGHT       ESC "[1m"
#define VT100_ATTR_DIM          ESC "[2m"
#define VT100_ATTR_UNDERLINE    ESC "[3m"
#define VT100_ATTR_BLINK        ESC "[4m"
#define VT100_ATTR_REVERSE      ESC "[5m"
#define VT100_ATTR_HIDDEN       ESC "[6m"

/* Foreground colors */
#define VT100_ATTR_FG_BLACK     ESC "[30m"
#define VT100_ATTR_FG_RED       ESC "[31m"
#define VT100_ATTR_FG_GREEN     ESC "[32m"
#define VT100_ATTR_FG_YELLOW    ESC "[33m"
#define VT100_ATTR_FG_BLUE      ESC "[34m"
#define VT100_ATTR_FG_MAGENTA   ESC "[35m"
#define VT100_ATTR_FG_CYAN      ESC "[36m"
#define VT100_ATTR_FG_WHITE     ESC "[37m"

/* Background colors */
#define VT100_ATTR_BG_BLACK     ESC "[40m"
#define VT100_ATTR_BG_RED       ESC "[41m"
#define VT100_ATTR_BG_GREEN     ESC "[42m"
#define VT100_ATTR_BG_YELLOW    ESC "[43m"
#define VT100_ATTR_BG_BLUE      ESC "[44m"
#define VT100_ATTR_BG_MAGENTA   ESC "[45m"
#define VT100_ATTR_BG_CYAN      ESC "[46m"
#define VT100_ATTR_BG_WHITE     ESC "[47m"

#define VT100_CURSOR_HOME       ESC "[H"

#define VT100_QUERY_CURSOR_POS        ESC "[6n"
#define VT100_REPORT_CURSOR_POS       ESC "[%d;%dR"

#endif /* __VT100_H__ */

