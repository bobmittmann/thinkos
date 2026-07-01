#include <thinkos.h>

void thinkos_abort(void) 
{
	thinkos_core_reset(THINKOS_CORE_RESET_KEY);
}

