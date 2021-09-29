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
 * @file shell.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __SHELL_I__
#include "shell-i.h"

int shell_history(struct shell_env * env, struct cmd_history * history,
			   void (* greeting)(FILE *))
{
	char line[SHELL_LINE_MAX];
	char * argv[SHELL_ARG_MAX];
	const struct shell_ops * op;
	int argc;
	int cmd;
	char * cp;
	char * st;
	FILE * fin;
	FILE * fout;
	int ret = 0;

	if (env == NULL)
		return -1;

	if ((op = env->op) == NULL)
		return -1;

	if ((fout = env->fout) == NULL)
		return -1;

	if ((fin = env->fin) == NULL)
		return -1;

	if ((op->greeting) != NULL)
		op->greeting(env);

	do {
		op->prompt_get(env, line, SHELL_PROMPT_MAX);

		fputs(line, fout);

		if (history_readline(history, env->fin, line, SHELL_LINE_MAX) == NULL)
			return -1;

		if ((cp = shell_stripline(line)) == NULL)
			continue;

		history_add(history, cp);

		/* get the next statement */
		while ((st = cmd_get_next(&cp)) != NULL) {

			if ((cmd = op->cmd_lookup(env, st)) < 0) {
				fprintf(fout, "Command not found!\n");
				break;
			}

			if ((argc = shell_parseline(line, argv, SHELL_ARG_MAX)) == 0) {
				break;
			}

			ret = op->cmd_exec(env, argc, argv, cmd);
			if (ret < 0) {
				if (SHELL_ERR_ARG_MISSING == ret) {
					fprintf(fout, "  %s, %s - %s\n", 
							op->cmd_name(env, cmd), op->cmd_alias(env, cmd), 
							op->cmd_brief(env, cmd));
					fprintf(fout, "  usage: %s %s\n\n", 
							op->cmd_alias(env, cmd), op->cmd_detail(env, cmd));
				} else if (ret !=  SHELL_ABORT) {
					fprintf(fout, "Error: %d\n", -ret);
					break;
				}
			}
			
		}
	} while (ret != SHELL_ABORT); 

	return 0;
}

int shell_simple(FILE * f, struct shell_ops * op)
{
	struct shell_env env_buf;
	struct shell_env * env = &env_buf;
	char line[SHELL_LINE_MAX];
	char * argv[SHELL_ARG_MAX];
	FILE * fout;
	int argc;
	char * cp;
	char * st;
	int cmd;
	int ret = 0;

	if ((fout = f) == NULL)
		return -1;

	if (op == NULL)
		return -1;

	env->fout = f;
	env->fin = f;
	env->op = op;

	if ((op->greeting) != NULL)
		op->greeting(env);

	do {
		if ((op->greeting) != NULL)
			op->prompt_get(env, line, SHELL_PROMPT_MAX);
		else
			strcpy(line, "[app]> ");

		fputs(line, fout);
		
		fgets(line, SHELL_LINE_MAX, env->fin);
		if (line[0] != '\n')
			continue;

		if ((cp = shell_stripline(line)) == NULL)
			continue;

		/* get the next statement */
		while ((st = cmd_get_next(&cp)) != NULL) {

			if ((cmd = op->cmd_lookup(env, st)) < 0) {
				fprintf(fout, "Command not found!\n");
				break;
			}

			if ((argc = shell_parseline(line, argv, SHELL_ARG_MAX)) == 0) {
				break;
			}

			ret = op->cmd_exec(env, argc, argv, cmd);
			if (ret < 0) {
				if (SHELL_ERR_ARG_MISSING == ret) {
					fprintf(fout, "  %s, %s - %s\n", 
							op->cmd_name(env, cmd), op->cmd_alias(env, cmd), 
							op->cmd_brief(env, cmd));
					fprintf(fout, "  usage: %s %s\n\n", 
							op->cmd_alias(env, cmd), op->cmd_detail(env, cmd));
				} else if (ret !=  SHELL_ABORT) {
					fprintf(fout, "Error: %d\n", -ret);
					break;
				}
			}
			
		}
	} while (ret != SHELL_ABORT); 

	return 0;

}

