/* 
 * File:	 usb-test.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/stm32f.h>

#include <sys/shell.h>
#include <sys/null.h>
#include <sys/delay.h>
#include <sys/tty.h>

#include <thinkos.h>
#include <trace.h>

#include "amp.h"

extern const struct shell_cmd shell_cmd_tab[];
extern const char * version_str;
extern const char * copyright_str;

/*****************************************************************************
 * Help
 *****************************************************************************/

int cmd_help(FILE *f, int argc, char ** argv)
{
	const struct shell_cmd * cmd;

	if (argc > 2)
		return -1;

	if (argc > 1) {
		if ((cmd = cmd_lookup(shell_cmd_tab, argv[1])) == NULL) {
			fprintf(f, " Not found: '%s'\n", argv[1]);
			return -1;
		}

		fprintf(f, "  %s, %s - %s\n", cmd->name, cmd->alias, cmd->desc);
		fprintf(f, "  usage: %s %s\n\n", argv[1], cmd->usage);

		return 0;
	}

	fprintf(f, "\n COMMAND:   ALIAS:  DESCIPTION: \n");
	for (cmd = shell_cmd_tab; cmd->callback != NULL; cmd++) {
		fprintf(f, "  %-10s %-4s   %s\n", cmd->name, cmd->alias, cmd->desc);
	}

	return 0;
}

/*****************************************************************************
 * Environment 
 *****************************************************************************/

int cmd_set(FILE * f, int argc, char ** argv)
{
	char * val;
	char * env;
	int i;

	if (argc > 1) {
		if (argc < 3) {
			fprintf(f, "Argument invalid or missing!\n");
			fprintf(f, "usage: %s [VAR VALUE]\n", argv[0]);
			return -1;
		}

		val = argv[2];

		fprintf(f, "%s=%s\n", argv[1], val);
	
		if (setenv(argv[1], val, 1) < 0) {
			fprintf(f, "setenv(): fail!\n");
			return -1;
		};

		return 0;
	}

	for (i = 0; (env =  environ[i]) != NULL; i++) {
		fprintf(f, "%s\n", env);
	};

	return 0;
}

int cmd_get(FILE * f, int argc, char ** argv)
{
	char * env;

	if (argc != 2) {
		fprintf(f, "Argument invalid or missing!\n");
		fprintf(f, "usage: %s VAR\n", argv[0]);
		return -1;
	}

	if ((env = getenv(argv[1])) == NULL) {
		fprintf(f, "getenv() fail!\n");
		return -1;
	};

	fprintf(f, "%s\n", env);

	return 0;
}

/*****************************************************************************
 * RS485 Network
 *****************************************************************************/

int cmd_stat(FILE * f, int argc, char ** argv)
{
#if 0
	struct netstats stat;
	bool clear = false;
	
	if (argc > 2)
		return SHELL_ERR_EXTRA_ARGS;

	if (argc > 1) {
		if ((strcmp(argv[1], "clear") == 0) || 
			(strcmp(argv[1], "clr") == 0) ||
			(strcmp(argv[1], "c") == 0)) {
			clear = true;
		} 
	}

	net_get_stats(&stat, clear);

	fprintf(f, "    |     octets | packets |  errors | seq err |\n");
	fprintf(f, " TX | %10u | %7u | %7u |         |\n", 
			stat.tx.octet_cnt, stat.tx.pkt_cnt, stat.tx.err_cnt);
	fprintf(f, " RX | %10u | %7u | %7u | %7u |\n", 
			stat.rx.octet_cnt, stat.rx.pkt_cnt, 
			stat.rx.err_cnt, stat.rx.seq_err_cnt);
#endif
	return 0;
}

int cmd_reboot(FILE * f, int argc, char ** argv)
{
	if (argc > 1)
		return SHELL_ERR_EXTRA_ARGS;

	fprintf(f, "Restarting in 2 seconds...\n");

	thinkos_sleep(2000);

	cm3_sysrst();

	return 0;
}

void spv_trace_autoflush_set(bool autoflush);
void spv_trace_file_set(FILE* f);

int cmd_trace(FILE * f, int argc, char ** argv)
{
	argc--;
	argv++;
/*
	if (argc) {
		if ((strcmp(*argv, "flush") == 0) || (strcmp(*argv, "f") == 0)) {
			fprintf(f, "flush\n");
//			trace_flush();
			return 0;
		} 

		if ((strcmp(*argv, "null") == 0) || (strcmp(*argv, "n") == 0)) {
			fprintf(f, "null\n");
			spv_trace_file_set(null_fopen(NULL));
			return 0;
		}

		if ((strcmp(*argv, "auto") == 0) || (strcmp(*argv, "a") == 0)) {
			fprintf(f, "Auto flush\n");
			spv_trace_autoflush_set(true);
			return 0;
		}

		if ((strcmp(*argv, "keep") == 0) || (strcmp(*argv, "k") == 0)) {
			fprintf(f, "Keep trace (don't flush)\n");
			spv_trace_autoflush_set(false);
			return 0;
		}

		return SHELL_ERR_ARG_INVALID;
	}

	spv_trace_file_set(f);
	spv_trace_autoflush_set(true); */
	fprintf(f, "Trace file set to this console.\n");

	return 0;
}

int cmd_version(FILE *f, int argc, char ** argv)
{
	if (argc > 1)
		return SHELL_ERR_EXTRA_ARGS;

	fprintf(f, "%s\n", version_str);
	fprintf(f, "%s\n", copyright_str);

	return 0;
}

int cmd_echo(FILE *f, int argc, char ** argv)
{
	int i;

	for (i = 1; i < argc; ++i) {
		if (i != 1)
			fprintf(f, " ");
		fprintf(f, argv[i]);
	}

	fprintf(f, "\n");

	return 0;
}

int cmd_on(FILE *f, int argc, char ** argv)
{
	unsigned int ckt;

	if (argc < 1)
		return SHELL_ERR_ARG_MISSING;

	ckt = atoi(argv[1]);

	return amp_power(ckt, true);
}

int cmd_off(FILE *f, int argc, char ** argv)
{
	unsigned int ckt;

	if (argc < 1)
		return SHELL_ERR_ARG_MISSING;

	ckt = atoi(argv[1]);

	return amp_power(ckt, false);
}

const struct shell_cmd shell_cmd_tab[] = {

	{ cmd_echo, "echo", "", 
		"[STRING]...", "Echo the STRING(s) to terminal" },

	{ cmd_get, "get", "", 
		"VAR", "get environement variable" },

	{ cmd_set, "set", "", 
		"VAR EXPR", "set environement variable" },

	{ cmd_help, "help", "?", 
		"[COMMAND]", "show command usage (help [CMD])" },

	{ cmd_reboot, "reboot", "rst", "", 
		"reboot system" },

	{ cmd_trace, "trace", "t", 
		"[flush | auto | monitor | null | keep]", "handle the trace ring" },

	{ cmd_stat, "stat", "s", 
		"[clear]", "show network statistics info" },

	{ cmd_version, "version", "ver", "", 
		"show version information" },

	{ cmd_on, "on", "", 
		"", "Turn ON amplifier." },

	{ cmd_off, "off", "", 
		"", "Turn OFF amplifier." },

		{ NULL, "", "", NULL, NULL }
};


const char * shell_prompt(void)
{
	return "[RDA420]$ ";
}

void shell_greeting(FILE * f) 
{
	fprintf(f, "\n%s", version_str);
	fprintf(f, "\n%s\n\n", copyright_str);
}

void __attribute__((noreturn)) shell_task(void * arg)
{
	FILE * f_raw = (FILE *)arg;
	struct tty_dev * tty;
	FILE * f_tty;

	INF("<%d> started...", thinkos_thread_self());

	tty = tty_attach(f_raw);
	f_tty = tty_fopen(tty);

	for (;;) {
		printf("\n...\n");
		shell(f_tty, shell_prompt, shell_greeting, shell_cmd_tab);
	}
}

