#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

int monitor_putc(int c, const struct monitor_comm * comm)
{
	char buf[1] = { c };
	return monitor_comm_write(comm, buf, sizeof(char));
}

