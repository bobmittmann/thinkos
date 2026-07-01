#ifndef __THINKOS_CTRL_H__
#define __THINKOS_CTRL_H__

#ifndef __THINKOS_CTRL__
#error "Never use <thinkos/ctrl.h> directly; include <thinkos/kernel.h> instead."
#endif 

#ifndef __ASSEMBLER__

struct thinkos_release {
	uint8_t tag[10];
	struct {
		uint8_t major;
		uint8_t minor;
		uint16_t build;
	} ver;
	uint32_t date;
};

struct thinkos_version {
	uint8_t major;
	uint8_t minor;
	char mach[14];
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_CTRL_H__ */

