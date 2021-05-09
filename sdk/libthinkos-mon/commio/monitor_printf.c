#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <sys/param.h>

#define MONITOR_PTINTF_BUF_SIZE 256

int monitor_printf(const struct monitor_comm * comm, const char *fmt, ... )
{
	char s[MONITOR_PTINTF_BUF_SIZE];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = krn_vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);

	return monitor_comm_send(comm, s, n);
}

