#ifndef __AMP_H__
#define __AMP_H__

#include <stdint.h>

struct amp;

#ifdef __cplusplus
extern "C" {
#endif

void amp_supv(void);
void amp_init(void);
int amp_power(unsigned int ckt, bool on);

#ifdef __cplusplus
}
#endif


#endif /* __AMP_H__ */

