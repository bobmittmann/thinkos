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
 * @file btl_board.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include "thinkos_btl-i.h"

void btl_mem_info_print(const struct thinkos_mem_desc * mem)
{
	const char * tag;
	uint32_t base;
	uint32_t size;
	char s[64];
	int align;
	bool ro;
	int i;

	if (mem== NULL)
		return;

	krn_snprintf(s, sizeof(s), "  %s:\r\n", mem->tag);
	krn_console_puts(s);
	for (i = 0; i < mem->cnt; ++i) {
		tag = mem->blk[i].tag;
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->base + mem->blk[i].off;
		ro = (mem->blk[i].opt == M_RO) ? 1 : 0;
		align = ((mem->blk[i].opt & 3) + 1) * 8;
		krn_snprintf(s, sizeof(s), "    %8s %08x-%08x %8d %6s %2d\r\n",
					  tag, base, base + size - 1, size, 
					  ro ? "RO" : "RW", align);
		krn_console_puts(s);
	}

}

void btl_board_info(const struct thinkos_board * board)
{
	char s[64];
	unsigned int i;

	krn_snprintf(s, sizeof(s), "Board: %s <%s>", board->name, board->desc);
	krn_console_wrln(s);
	krn_snprintf(s, sizeof(s), "Hardware: %s rev %d.%d", 
				 board->hw.tag, board->hw.ver.major, board->hw.ver.minor);
	krn_console_wrln(s);

	krn_snprintf(s, sizeof(s), "Firmware: %s-%d.%d.%d (%s) " __DATE__ 
				  ", " __TIME__ "",
				  board->sw.tag, 
				  board->sw.ver.major, 
				  board->sw.ver.minor, 
				  board->sw.ver.build,
#if DEBUG
				  "debug"
#else
				  "release"
#endif
				  ); 
	krn_console_wrln(s);

	/* compiler version string */
	krn_console_wrln("Compiler: GCC-" __VERSION__);

	/* memory blocks */
	krn_console_wrln("Memory:");
	krn_console_wrln("         Tag    Begin      End"
				  "     Size  Flags  Align");

	for (i = 0; i < board->memory->cnt; ++i) {
		btl_mem_info_print(board->memory->desc[i]);
	}
	krn_console_puts("\r\n");
}

