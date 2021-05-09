#ifndef __THINKOS_APP_H__
#define __THINKOS_APP_H__

#ifndef __THINKOS_APP__
#error "Never use <thinkos/app.h> directly; include <thinkos/kernel.h> instead."
#endif 


#ifndef __ASSEMBLER__

#include <thinkos/crc.h>

struct flat_section {
	uint32_t start;
	uint32_t end;
};

struct flat_app {
	uint32_t entry;
	uint32_t stack;
	uint32_t stksz;
	uint32_t size; /* File size */
	union {
		char os_tag[8];
		uint32_t os_key[2];
	};
	union {
		char sys_tag[8];
		uint32_t sys_key[2];
	};
	struct flat_section text;
	struct flat_section data;
	struct flat_section bss;
	struct flat_section ctor;
};

/* File identification magic block 

   This block is used to guess the type of a memory block or file
   based on a pattarn located somewhere inside the file.
 
 */
struct magic_blk {
	struct {
		uint16_t pos; /* Position of the pattern in bytes */
		uint16_t cnt; /* Number of record entries */
	} hdr;
	/* Pattern records */
	struct {
	    uint32_t mask; /* Bitmask */
		uint32_t comp; /* Compare value */
	} rec[];
};

#ifdef __cplusplus
extern "C" {
#endif

bool thinkos_krn_app_check(uintptr_t addr, unsigned int max);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_APP_H__ */

