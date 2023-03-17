
/** 
 * @file console.c
 * @brief 
 * @author Robinson Mittmann <bmittmann@mircomgroup.com>
 */

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <thinkos.h>

#include <sys/shell.h>
#include <sys/tty.h>

#include "dac.h"

extern const struct shell_cmd shell_cmd_tab[];
void select_xilophone(void);
void select_piano(void);

/*****************************************************************************
 * Help
 *****************************************************************************/

int cmd_help(FILE * f, int argc, char **argv)
{
	const struct shell_cmd *cmd;

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
		fprintf(f, "  %-10s %-4s   %s\n", cmd->name, cmd->alias,
			cmd->desc);
	}

	return 0;
}

int cmd_vol(FILE * f, int argc, char **argv)
{
	int vol;

	if (argc > 2)
		return -1;

	if (argc > 1) {
		vol = atoi(argv[1]);
		dac_gain_set(vol / 100.0);
	} else {
		vol = dac_gain_get() * 100;
	}

	fprintf(f, "Volume = %d%%\r\n", vol);

	return 0;
}

int cmd_piano(FILE * f, int argc, char **argv)
{
	select_piano();
	fprintf(f, "Piano selected\n");
	return 0;
}

int cmd_xilophone(FILE * f, int argc, char **argv)
{
	select_xilophone();
	fprintf(f, "Xilophone selected\n");
	return 0;
}

int cmd_test(FILE * f, int argc, char **argv)
{
	fprintf(f, "\ntest cmd\n");
	return 0;
}

const struct shell_cmd shell_cmd_tab[] = {
	{cmd_help, "help", "?",
	 "[COMMAND]", "show command usage (help [CMD])"},

	{cmd_vol, "vol", "v",
	 "[0..100]", "adjust volume"},

	{cmd_piano, "piano", "p",
	 "", "select piano"},

	{cmd_xilophone, "xiloph", "x",
	 "", "select xilophone"},

	{NULL, "", "", NULL, NULL}
};

static char prompt_buf[16];

const char * shell_prompt(void)
{
	char * s = (char *)prompt_buf;

	sprintf(s, "[SYNTH]$ ");
	return s;
}

void shell_greeting(FILE * f)
{
	fprintf(f, "\r\nSynthesizer!\r\n");
}

void __attribute__ ((noreturn)) shell_task(void *arg)
{
	FILE *f_raw = (FILE *)arg;
	struct tty_dev *tty;
	FILE *f_tty;

	thinkos_sleep(1000);

	tty = tty_attach(f_raw);
	f_tty = tty_fopen(tty);

	if (NULL != f_tty) {
		for (;;) {
			shell(f_tty, shell_prompt, shell_greeting, shell_cmd_tab);
			thinkos_sleep(1000);
		}
	}
	for (;;);
}

uint32_t shell_stack[2048] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf shell_thread_inf = {
	.stack_ptr = shell_stack,
	.stack_size = sizeof(shell_stack),
	.priority = 4,
	.thread_id = 4,
	.paused = false,
	.tag = "SHELL"
};

void shell_init(void)
{
	thinkos_thread_create_inf((void *)shell_task, (void *)stdout,
				  &shell_thread_inf);
}

