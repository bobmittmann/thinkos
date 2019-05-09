/* 
 * File:	 thinkos_flash_mem.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
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

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Os\")")
#endif
#include <thinkos.h>

#include <sys/param.h>
#include <stdbool.h>

#if (THINKOS_ENABLE_FLASH_MEM)

int board_flash_mem_close(int mem);
int board_flash_mem_open(int mem);
int board_flash_mem_read(int mem, void * buf, size_t len);
int board_flash_mem_write(int mem, const void * buf, size_t len);
int board_flash_mem_seek(int mem, off_t offset);
int board_flash_mem_erase(int mem, off_t offset, size_t len);
int board_flash_mem_lock(int mem, off_t offset, size_t len);
int board_flash_mem_unlock(int mem, off_t offset, size_t len);

void thinkos_flash_mem_svc(int32_t * arg, int self)
{
	unsigned int req = arg[0];
	int mem = arg[1];
	void * buf = (void *)arg[2];
	off_t off= (off_t)arg[2];
	size_t len = (size_t)arg[3];
	
#if THINKOS_ENABLE_ARG_CHECK
#endif

	switch (req) {
	case THINKOS_FLASH_MEM_OPEN:
		arg[0] = board_flash_mem_open(mem);
		break;

	case THINKOS_FLASH_MEM_CLOSE:
		arg[0] = board_flash_mem_close(mem);
		break;

	case THINKOS_FLASH_MEM_READ:
		arg[0] = board_flash_mem_read(mem, buf, len);
		break;

	case THINKOS_FLASH_MEM_WRITE:
		arg[0] = board_flash_mem_write(mem, buf, len);
		break;

	case THINKOS_FLASH_MEM_SEEK:
		arg[0] = board_flash_mem_seek(mem, off);
		break;

	case THINKOS_FLASH_MEM_ERASE:
		arg[0] = board_flash_mem_erase(mem, off, len);
		break;

	case THINKOS_FLASH_MEM_LOCK:
		arg[0] = board_flash_mem_lock(mem, off, len);
		break;

	case THINKOS_FLASH_MEM_UNLOCK:
		arg[0] = board_flash_mem_unlock(mem, off, len);
		break;

	default:
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

const char thinkos_flash_mem_nm[] = "FMM";

#endif /* THINKOS_ENABLE_FLASH_MEM */

