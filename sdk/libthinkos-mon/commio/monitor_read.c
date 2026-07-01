#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#include <sys/dcclog.h>
#include <vt100.h>

int monitor_comm_read(const struct monitor_comm * comm, 
				   void * buf, unsigned int len)
{
	uint8_t * ptr = (uint8_t *)buf;
	int n;

	do {
		int ret;

		if ((ret = monitor_expect(MONITOR_COMM_RCV)) < 0) {
			DCC_LOG(LOG_WARNING, "monitor_expect()!");
			return ret;
		}

		n = monitor_comm_recv(comm, ptr, len);

	} while (n == 0); 

	return n;
}

