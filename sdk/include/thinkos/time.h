#ifndef __THINKOS_TIME_H__
#define __THINKOS_TIME_H__

#ifndef __THINKOS_TIME__
#error "Never use <thinkos/time.h> directly; include <thinkos/kernel.h> instead."
#endif 


#ifndef __ASSEMBLER__

union krn_time {
	uint64_t u64;
	struct {
		uint32_t frac;
		uint32_t sec;
	};
};


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_TIME_H__ */

