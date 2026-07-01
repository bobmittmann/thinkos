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

static const char punct_str[][2] = {
	"!", "\"", "#", "$", "%", "&", "'", 
	"(", ")", "*", "+", ",", "-", ".", "/", 
	"", "", "", "", "", "", "", "", "", "",
	":", ";", "<", "=", ">", "?", "@",
	"", "", "", "", "", "", "", "", "", "", "", "", "", 
	"", "", "", "", "", "", "", "", "", "", "", "", "", 
	"[", "\\", "]", "^", "", "`",
	"", "", "", "", "", "", "", "", "", "", "", "", "", 
	"", "", "", "", "", "", "", "", "", "", "", "", "", 
	"{", "|", "}", "~"
};

static const char dbl_punct_str[][4] = {
	"<<",
	"==",
	">>"
};

int shell_parseline(char * line, char ** argv, int argmax)
{
	char * tok = NULL;
	int n;
	int c;
	char * cp = line;

	c = *cp;
	/* Leave an extra space for double tokens in the loop when 
	 * a punctuation is adjacent to another token */
	--argmax;
	for (n = 0; (n < argmax); ) {
		/* Remove lead blanks */
		while (isspace(c)) {
			cp++;
			c = *cp;
		}
#if 1
		int qt;

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
#endif
		tok = cp;

		for (;;) {
			if (c == '\0') {
				 if (tok != cp)
					argv[n++] = tok;
				return n;
			}

			if (isspace(c)) {
				argv[n++] = tok;
				*cp = '\0';
				cp++;
				c = *cp;
				break;
			}

			if ((c >= '<') && (c <= '>') && (cp[1] == c)) {
				char * punct = (char *)dbl_punct_str[c - '<'];
				if (tok != cp) {
					argv[n++] = tok;
				}
				argv[n++] = punct;
				*cp = '\0';
				/* skip two */
				cp += 2;
				c = *cp;
				break;
			}

			if ((c >= '!') && (c <= '~')) {
				char * punct = (char *)punct_str[c - '!'];
				if (*punct != '\0') {
					if (tok != cp) {
						argv[n++] = tok;
					}
					argv[n++] = punct;
					*cp = '\0';
					cp++;
					c = *cp;
					break;
				}
			}

			cp++;
			c = *cp;
		}
	}

	return n;
}

