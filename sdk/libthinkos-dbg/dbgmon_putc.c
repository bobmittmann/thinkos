#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

int dbgmon_putc(int c, const struct dbgmon_comm * comm)
{
	char buf[1] = { c };
	return dbgmon_comm_send(comm, buf, sizeof(char));
}

