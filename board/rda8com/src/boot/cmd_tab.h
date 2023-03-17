/*---------------------------------------------------------------------------
  This file was automatically generated by mkcmd.py. DO NOT edit!
  ---------------------------------------------------------------------------*/

#ifndef __CMD_TAB_H__
#define __CMD_TAB_H__

#include <stdint.h>

#define BOOT_CMD_APP    1
#define BOOT_CMD_DIAG    2
#define BOOT_CMD_ERASE    3
#define BOOT_CMD_HELP    4
#define BOOT_CMD_INFO    5
#define BOOT_CMD_RCVY    6
#define BOOT_CMD_REBOOT    7

#define BOOT_CMD_FIRST 1
#define BOOT_CMD_LAST 7

typedef int(* boot_cmd_callback_t)(int argc, char * argv[]);

extern const char * const boot_cmd_sym_tab[];
extern const char * const boot_cmd_brief_tab[];
extern const char * const boot_cmd_desc_tab[];
extern const char boot_cmd_alias_tab[][4];

extern boot_cmd_callback_t const boot_cmd_call_tab[];

#ifdef __cplusplus
extern "C" {
#endif

int boot_cmd_lookup(const char * str);

/*---------------------------------------------------------------------------
  Callbacks!
  ---------------------------------------------------------------------------*/

int boot_cmd_app(int argc, char * argv[]);
int boot_cmd_diag(int argc, char * argv[]);
int boot_cmd_erase(int argc, char * argv[]);
int boot_cmd_help(int argc, char * argv[]);
int boot_cmd_info(int argc, char * argv[]);
int boot_cmd_rcvy(int argc, char * argv[]);
int boot_cmd_reboot(int argc, char * argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __CMD_TAB_H__ */

