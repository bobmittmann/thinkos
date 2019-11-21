#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

int dbgmon_puts(const char * s, const struct dbgmon_comm * comm)
{
	int n = 0;

	while (s[n] != '\0')
		n++;
	return dbgmon_comm_send(comm, s, n);
}

