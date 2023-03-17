#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

int uint2dec(char * s, unsigned int val);
int uint2hex(char * s, unsigned int val);

static const char monitor_blanks[] = { 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

static const char monitor_zeros[]  = { 
	'0', '0', '0', '0', '0', '0', '0', '0',
	'0', '0', '0', '0' };

void monitor_comm_send_int(int32_t val, unsigned int width, 
                          const struct monitor_comm * comm) 
{
	char s[16];
	int n;

	if (val < 0) {
		val = -val;
		n = uint2dec(&s[1], val);
		n++;
		s[0] = '-';
	} else {
		n = uint2dec(s, val);
	}
	monitor_comm_write(comm, monitor_blanks, width - n);
	monitor_comm_write(comm, s, n);
}

void monitor_comm_send_uint(uint32_t val, unsigned int width, 
                           const struct monitor_comm * comm) 
{
	char s[16];
	int n;

	n = uint2dec(s, val);
	monitor_comm_write(comm, monitor_blanks, width - n);
	monitor_comm_write(comm, s, n);
}

void monitor_comm_send_str(const char * s, unsigned int width, 
                          const struct monitor_comm * comm) 
{
	int n = 0;

	while (s[n] != '\0')
		n++;

	monitor_comm_write(comm, monitor_blanks, width - n);
	monitor_comm_write(comm, s, n);
}

void monitor_comm_send_blanks(unsigned int width, 
                             const struct monitor_comm * comm) 
{
	monitor_comm_write(comm, monitor_blanks, width);
}

void monitor_comm_send_hex(uint32_t val, unsigned int width, 
                          const struct monitor_comm * comm) 
{
	char s[12];
	int n;

	n = uint2hex(s, val);
	monitor_comm_write(comm, monitor_zeros, width - n);
	monitor_comm_write(comm, s, n);
}

