#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

int monitor_getc(const struct monitor_comm * comm)
{
	char buf[1];

	if (monitor_comm_read(comm, buf, sizeof(char)) <= 0)
		return -1;

	monitor_comm_write(comm, buf, sizeof(char));

	return	buf[0];
}

