#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

void monitor_newln(const struct monitor_comm * comm)
{
	monitor_comm_send(comm, "\r\n", 2);
}

