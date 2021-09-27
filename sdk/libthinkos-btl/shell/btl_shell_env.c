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
 * @file btl_shell_env.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include "thinkos_btl-i.h"

static struct btl_shell_env btl_shell_env_singleton;

struct btl_shell_env * btl_shell_env_getinstance(void)
{
	return &btl_shell_env_singleton;
}

void btl_shell_env_prompt_set(struct btl_shell_env * env, const char * str)
{
	env->prompt = str;
}

void btl_shell_env_motd_set(struct btl_shell_env * env, const char * str)
{
	env->motd = str;
}

void btl_shell_env_init(struct btl_shell_env * env, 
					   const char * motd, const char * prompt)
{
	env->prompt = prompt;
	env->motd = motd;
}

