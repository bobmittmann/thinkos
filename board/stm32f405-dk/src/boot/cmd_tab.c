/*---------------------------------------------------------------------------
  This file was automatically generated by mkcmd.py. DO NOT edit!
  ---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "cmd_tab.h"

const char * const btl_cmd_sym_tab[] = {
	[BTL_CMD_APP] = "app",
	[BTL_CMD_DIAG] = "diag",
	[BTL_CMD_ERASE] = "erase",
	[BTL_CMD_HELP] = "help",
	[BTL_CMD_INFO] = "info",
	[BTL_CMD_RCVY] = "rcvy",
	[BTL_CMD_REBOOT] = "reboot",
};

const char * const btl_cmd_brief_tab[] = {
	[BTL_CMD_APP] = "Run the application",
	[BTL_CMD_DIAG] = "Run the diagnostics app",
	[BTL_CMD_ERASE] = "erase a flash partition",
	[BTL_CMD_HELP] = "Display a summary list of commands",
	[BTL_CMD_INFO] = "Display system information",
	[BTL_CMD_RCVY] = "YMODEM file receive to flash partition",
	[BTL_CMD_REBOOT] = "Reboot the system",
};

const char btl_cmd_alias_tab[][4] = {
	[BTL_CMD_APP] = "",
	[BTL_CMD_DIAG] = "",
	[BTL_CMD_ERASE] = "e",
	[BTL_CMD_HELP] = "h",
	[BTL_CMD_INFO] = "i",
	[BTL_CMD_RCVY] = "y",
	[BTL_CMD_REBOOT] = "",
};

const char * const btl_cmd_param_tab[] = {
	[BTL_CMD_APP] = NULL,
	[BTL_CMD_DIAG] = NULL,
	[BTL_CMD_ERASE] = "app | diag",
	[BTL_CMD_HELP] = "CMD",
	[BTL_CMD_INFO] = NULL,
	[BTL_CMD_RCVY] = "app | diag",
	[BTL_CMD_REBOOT] = NULL,
};

const btl_cmd_callback_t btl_cmd_call_tab[] = {
	[BTL_CMD_APP] = btl_cmd_app,
	[BTL_CMD_DIAG] = btl_cmd_diag,
	[BTL_CMD_ERASE] = btl_cmd_erase,
	[BTL_CMD_HELP] = btl_cmd_help,
	[BTL_CMD_INFO] = btl_cmd_info,
	[BTL_CMD_RCVY] = btl_cmd_rcvy,
	[BTL_CMD_REBOOT] = btl_cmd_reboot,
};

/*
   List search...
*/

int btl_cmd_lookup(struct btl_shell_env * env, const char * str)
{
	int i;

	for (i = BTL_CMD_FIRST; i <= BTL_CMD_LAST; ++i) {
		if (strcmp(str, btl_cmd_sym_tab[i]) == 0)
			return i;
		if (strcmp(str, btl_cmd_alias_tab[i]) == 0)
			return i;
	}

	return -2;
}

int btl_cmd_first(struct btl_shell_env * env)
{
	return BTL_CMD_FIRST;
}

int btl_cmd_last(struct btl_shell_env * env)
{
	return BTL_CMD_LAST;
}

const char * btl_cmd_name(struct btl_shell_env * env, unsigned int code)
{
	return btl_cmd_sym_tab[code];
}

const char * btl_cmd_alias(struct btl_shell_env * env, unsigned int code)
{
	return btl_cmd_alias_tab[code];
}

const char * btl_cmd_brief(struct btl_shell_env * env, unsigned int code)
{
	return btl_cmd_brief_tab[code];
}

int btl_cmd_call(struct btl_shell_env * env, int argc, char * argv[], unsigned int code)
{
	if (code > BTL_CMD_LAST)
		return -2;
	return btl_cmd_call_tab[code](env, argc, argv);
}
