#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#include <sys/dcclog.h>
#include <vt100.h>

int monitor_comm_write(const struct monitor_comm * comm, 
				   const void * buf, unsigned int len)
{
	uint8_t * ptr = (uint8_t *)buf;
	unsigned int rem;
	int n;

	rem = len;
	while (rem) {
		if (!monitor_comm_isconnected(comm)) {
			/* not connected, discard!! */
			rem = 0;
			break;
		}

		if ((n = monitor_comm_send(comm, ptr, rem)) < 0) {
			return n;
		} 

		if (n > 0) {
			rem -= n;
			ptr += n;
		} else {
			int ret;
			if ((ret = monitor_expect(MONITOR_COMM_EOT)) < 0) {
				return ret;
			}
		}
	}

	return len - rem;
}

