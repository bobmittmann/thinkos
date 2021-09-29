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
 * @file classic.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __SHELL_I__
#include "shell-i.h"

/* -----------------------------------------------------------------------------
   Classic shell api
 */

struct classic_shell_env {
	FILE * fout;
	FILE * fin;
	const struct shell_ops * op;
	const struct shell_cmd * cmdtab;
	const char * (* prompt)(void); 
};

#define CLASSIC_SHELL_ENV(__T) ((struct classic_shell_env *)(__T))

const char * classic_prompt_get(struct shell_env * env, char * buf, size_t len)
{
	const char * (* prompt)(void) = CLASSIC_SHELL_ENV(env)->prompt; 
	if (prompt != NULL)
		strlcpy(buf, prompt(), len);
	else
		strlcpy(buf, "[App]> ", len);
	return buf;
}

const char * classic_cmd_name(struct shell_env * env, unsigned int code)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;
	return cmdtab[code].name;
}

const char * classic_cmd_alias(struct shell_env * env, unsigned int code)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;
	return cmdtab[code].alias;
}

const char * classic_cmd_brief(struct shell_env * env, unsigned int code)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;
	return cmdtab[code].usage;
}

const char * classic_cmd_detail(struct shell_env * env, unsigned int code)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;
	return cmdtab[code].desc;
}

int classic_cmd_lookup(struct shell_env * env, const char * line)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;
	char * s;
	char * cp;
	int n;
	int i;

	if ((cp = (char *)line) == NULL) {
		return -1;
	}

	/* remove leading spaces */
	for (; isspace(*cp); cp++);
	s = cp;

	/* get the command name lenght */
	for (; (*cp != '\0') && (*cp != ';') && !isspace(*cp); cp++);
	n = cp - s;

	if (n == 0) {
		return -1;
	}

	for (i = 0; cmdtab[i].callback != NULL; ++i) {
		const char * name = cmdtab[i].name;
		const char * alias = cmdtab[i].alias;

		if ((name[n] == '\0' && strncmp(s, name, n) == 0) ||
			(alias[n] == '\0' && strncmp(s, alias, n) == 0)) {
			return i;
		}
	}

	return -1;
}

int classic_cmd_exec(struct shell_env * env, int argc, 
						 char * argv[], unsigned int code)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;
	FILE * fout = env->fout;

	if (cmdtab[code].callback == NULL)
		return -1;

	return cmdtab[code].callback(fout, argc, argv);
}

int classic_cmd_first(struct shell_env * env)
{
	return 0;
}

int classic_cmd_next(struct shell_env * env, unsigned int code)
{
	const struct shell_cmd * cmdtab = CLASSIC_SHELL_ENV(env)->cmdtab;

	return (cmdtab[++code].callback != NULL) ? code : -1;
}

const struct shell_ops shell_classic_ops = {
	.prompt_get = classic_prompt_get,
	.cmd_name = classic_cmd_name,
	.cmd_alias = classic_cmd_alias,
	.cmd_brief = classic_cmd_brief,
	.cmd_detail = classic_cmd_detail,
	.cmd_lookup = classic_cmd_lookup,
	.cmd_first = classic_cmd_first,
	.cmd_next = classic_cmd_next,
	.cmd_exec = classic_cmd_exec
};


int shell(FILE * f, const char * (* prompt)(void), 
		  void (* greeting)(FILE *),
		  const struct shell_cmd cmdtab[])
{
	char hist_buf[SIZEOF_CMD_HISTORY + SHELL_HISTORY_MAX * SHELL_LINE_MAX];
	struct cmd_history * history;
	struct classic_shell_env env;

	if (f == NULL)
		return -1;

	env.fin = f;
	env.fout = f;
	env.cmdtab = cmdtab;
	env.prompt = prompt;
	env.op = &shell_classic_ops;

	memset(hist_buf, 0, sizeof(hist_buf));
	history = history_init(hist_buf, sizeof(hist_buf), SHELL_LINE_MAX);

	return shell_history(SHELL_ENV(&env), history, greeting);
}

