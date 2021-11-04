/* 
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

/** 
 * @file shell.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include "thinkos_btl-i.h"

static inline int __isspace(int c) {
	return ((c == ' ') || (c == '\t'));
}

int __parseline(char * line, char ** argv, int argmax)
{
	char * cp = line;
	char * tok;
	int n;
	int c;

	for (n = 0; (c = *cp) && (n < argmax); ) {

		/* Remove lead blanks */
		while (__isspace(c)) {
			c = *(++cp);
		}
#if 0
		/* Quotes: copy verbatim */
		if ((c == '\'') || (c == '\"')) {
			int qt = c;
			tok = ++cp;
			for (; ((c = *cp) != qt); cp++) {
				if (c == '\0') {
					/* parse error, unclosed quotes */
					return -1;
				}
			}
			*cp++ = '\0';
			argv[n++] = tok;
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

			if (__isspace(c)) {
				*cp++ = '\0';
				argv[n++] = tok;
				break;
			}

			cp++;
			c = *cp;
		}
	}

	return n;
}

int btl_console_shell(struct btl_shell_env * env)
{
	const char * prompt;
	const char * msg;
	char line[80];
	char * argv[8];
	int argc;
	int cmd;

	msg = env->motd;
	prompt = env->prompt;

	while (thinkos_console_is_connected() <= 0) {
		thinkos_sleep(100);
	}

	krn_console_puts("\r\n");
	if (msg != NULL) {
		krn_console_puts(msg);
	}

	for (;;) {
		int ret;

		krn_console_puts(prompt);
		if ((ret = krn_console_gets(line, sizeof(line))) < 0) {
			return ret;
		}

		if (ret == 0) {
			continue;
		}

		if ((argc = __parseline(line, argv, 8)) <= 0) {
			continue;
		};

		if ((cmd = btl_cmd_lookup(env, argv[0])) <= 0) {
			continue;
		};

		ret = btl_cmd_call(env, argc, argv, cmd); 
		if (ret < 0) {
# if 0
			krn_console_wr("Error");
#endif
		}
	} 

	return 0;
}

