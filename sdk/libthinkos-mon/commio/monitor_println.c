#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <sys/param.h>

void monitor_println(const char * ln, const struct monitor_comm * comm)
{
	monitor_comm_write(comm, ln, n);
	monitor_comm_write(comm, "\r\n", 2);
}

