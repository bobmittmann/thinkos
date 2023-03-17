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
 * @file parseline.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __SHELL_I__
#include "shell-i.h"

#include <ctype.h>

#ifndef SHELL_ENABLE_OPERATORS 
#define SHELL_ENABLE_OPERATORS 0
#endif

#if SHELL_ENABLE_OPERATORS
static const char punct_str[][2] = {
	"!", "\"", "#", "$", "%", "&", "'", 
	"(", ")", "*", "+", ",", "-", ".", "/", 
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	":", ";", "<", "=", ">", "?", "@",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", 
	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 
	"[", "\\", "]", "^", "_", "`",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", 
	"{", "|", "}", "~"
};
#endif

const char * op_assign = "=";
const char * op_equal = "==";

//static const char * op_plus = "+";
//static const char * op_inc = "++";

int shell_parseline(char * line, char ** argv, int argmax)
{
	char * tok = NULL;
	int qt;
	int n;
	int c;
	char * cp = line;
#if SHELL_ENABLE_OPERATORS
	char * punct;
#endif

	c = *cp;

	for (n = 0; (n < argmax); ) {
		/* Remove lead blanks */
		while (isspace(c)) {
			cp++;
			c = *cp;
		}

		/* Quotes: copy verbatim */
		if ((c == '\'') || (c == '\"')) {
			qt = c;
			tok = ++cp;
			for (; ((c = *cp) != qt); cp++) {
				if (c == '\0') {
					/* parse error, unclosed quotes */
					return -1;
				}
			}
			*cp++ = '\0';
			argv[n++] = tok;
			c = *cp;
			continue;
		}

		tok = cp;

		for (;;) {
			if (c == '\0') {
				 if (tok != cp)
					argv[n++] = tok;
				return n;
			}

#if 0
			if (c == '=') {
				if (tok != cp) {
					*cp = '\0';
				} else	{
					tok = (char *)op_assign;
					cp++;
					c = *cp;
				}
				argv[n++] = tok;
				break;
			}
#endif

			if (isspace(c)) {
				*cp = '\0';
				cp++;
				c = *cp;
				argv[n++] = tok;
				break;
			}

#if SHELL_ENABLE_OPERATORS
			if (ispunct(c) && (c != '.') && (c != '_')) {
#if 0
				if ((c == '<') && (cp[1] == '<')) {
					*cp++ = '\0';
					punct = "<<";
				} else {
					if ((c == '>') && (cp[1] == '>')) {
						*cp++ = '\0';
						punct = ">>";
					} else {
						punct = (char *)punct_str[c - '!'];
					}
				}
#endif


				if (tok != cp) {
					*cp = '\0';
				} else	{
					punct = (char *)punct_str[c - '!'];
					tok = (char *)punct;
					cp++;
					c = *cp;
				}
				argv[n++] = tok;

				break;
			} 
#endif

			cp++;
			c = *cp;
		}
	}

	return n;
}

