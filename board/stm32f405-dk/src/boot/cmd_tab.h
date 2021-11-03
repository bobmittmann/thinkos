/*---------------------------------------------------------------------------
  This file was automatically generated by mkcmd.py. DO NOT edit!
  ---------------------------------------------------------------------------*/

#ifndef __CMD_TAB_H__
#define __CMD_TAB_H__

#include <stdint.h>

#define BTL_CMD_APP     1
#define BTL_CMD_DIAG    2
#define BTL_CMD_ERASE    3
#define BTL_CMD_HELP    4
#define BTL_CMD_INFO    5
#define BTL_CMD_RCVY    6
#define BTL_CMD_REBOOT    7

#define BTL_CMD_FIRST 1
#define BTL_CMD_LAST 7

struct btl_shell_env;

typedef int(* btl_cmd_callback_t)(struct btl_shell_env * env, int argc, char * argv[]);

extern const char * const btl_cmd_sym_tab[];
extern const char * const btl_cmd_brief_tab[];
extern const char * const btl_cmd_desc_tab[];
extern const char btl_cmd_alias_tab[][4];

extern const btl_cmd_callback_t btl_cmd_call_tab[];

#ifdef __cplusplus
extern "C" {
#endif

int btl_cmd_lookup(struct btl_shell_env * env, const char * str);

int btl_cmd_first(struct btl_shell_env * env);

int btl_cmd_last(struct btl_shell_env * env);

const char * btl_cmd_name(struct btl_shell_env * env, unsigned int code);

const char * btl_cmd_alias(struct btl_shell_env * env, unsigned int code);

const char * btl_cmd_brief(struct btl_shell_env * env, unsigned int code);

const char * btl_cmd_detail(struct btl_shell_env * env, unsigned int code);

int btl_cmd_call(struct btl_shell_env * env, int argc, char * argv[], unsigned int code);
/*---------------------------------------------------------------------------
  Callbacks!
  ---------------------------------------------------------------------------*/

int btl_cmd_app(struct btl_shell_env * env, int argc, char * argv[]);
int btl_cmd_diag(struct btl_shell_env * env, int argc, char * argv[]);
int btl_cmd_erase(struct btl_shell_env * env, int argc, char * argv[]);
int btl_cmd_help(struct btl_shell_env * env, int argc, char * argv[]);
int btl_cmd_info(struct btl_shell_env * env, int argc, char * argv[]);
int btl_cmd_rcvy(struct btl_shell_env * env, int argc, char * argv[]);
int btl_cmd_reboot(struct btl_shell_env * env, int argc, char * argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __CMD_TAB_H__ */

