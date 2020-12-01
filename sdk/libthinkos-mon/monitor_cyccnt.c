#include "thinkos_mon-i.h"

uint32_t monitor_threads_cyc_sum(uint32_t cyc[],uint32_t cycref[], 
                                 unsigned int from, unsigned int cnt)
{
#if (THINKOS_ENABLE_PROFILING)
	uint32_t sum;
	unsigned int to;
	unsigned int i;

	if (thinkos_dbg_threads_cyc_get(cyc, from, cnt) < 0)
		return 0;

	to = from + cnt;
	sum = 0;
	for (i = from; i < to; ++i) {
		uint32_t dif;
		uint32_t cnt = cyc[i];
		uint32_t ref = cycref[i];

		cycref[i] = cnt;
		dif = cnt - ref; 
		cyc[i] = dif;
		sum += dif;
	}

	return sum;
#else
	return 0;
#endif
}

