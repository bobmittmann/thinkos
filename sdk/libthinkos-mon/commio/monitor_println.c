#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <sys/param.h>

#define MONITOR_PRINTLN_BUF_SIZE 128

void monitor_println(const struct monitor_comm * comm, const char *fmt, ... )
{
	char s[MONITOR_PRINTLN_BUF_SIZE];
	va_list ap;
	int n;

	for (n = 0; n < 79; ++n)
		s[n] = ' ';
	s[n++] = '\r';
	monitor_comm_write(comm, s, n);

	va_start(ap, fmt);
	n = krn_vsnprintf(s, sizeof(s), fmt, ap);
	s[n++] = '\r';
	s[n++] = '\n';
	monitor_comm_write(comm, s, n);
	va_end(ap);
}

void monitor_writeln(const char * ln, const struct monitor_comm * comm)
{
	char s[82];
	int n;

	for (n = 0; n < 79; ++n)
		s[n] = ' ';
	s[n++] = '\r';
	monitor_comm_write(comm, s, n);
	n = 0;
	while (ln[n] != '\0')
		n++;
	monitor_comm_write(comm, ln, n);
	monitor_comm_write(comm, "\r\n", 2);
}

