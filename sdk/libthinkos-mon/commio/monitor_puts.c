#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

int monitor_puts(const char * s, const struct monitor_comm * comm)
{
	int n = 0;

	while (s[n] != '\0')
		n++;
	return monitor_comm_send(comm, s, n);
}

