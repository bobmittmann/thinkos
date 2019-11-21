#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

int dbgmon_getc(const struct dbgmon_comm * comm)
{
	char buf[1];

	if (dbgmon_comm_recv(comm, buf, sizeof(char)) <= 0)
		return -1;

	dbgmon_comm_send(comm, buf, sizeof(char));

	return	buf[0];
}

