#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

int uint2dec(char * s, unsigned int val);
int uint2hex(char * s, unsigned int val);

static const char dbgmon_blanks[] = { 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

static const char dbgmon_zeros[]  = { 
	'0', '0', '0', '0', '0', '0', '0', '0',
	'0', '0', '0', '0' };

void dbgmon_comm_send_int(int32_t val, unsigned int width, 
                          const struct dbgmon_comm * comm) 
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
	dbgmon_comm_send(comm, dbgmon_blanks, width - n);
	dbgmon_comm_send(comm, s, n);
}

void dbgmon_comm_send_uint(uint32_t val, unsigned int width, 
                           const struct dbgmon_comm * comm) 
{
	char s[16];
	int n;

	n = uint2dec(s, val);
	dbgmon_comm_send(comm, dbgmon_blanks, width - n);
	dbgmon_comm_send(comm, s, n);
}

void dbgmon_comm_send_str(const char * s, unsigned int width, 
                          const struct dbgmon_comm * comm) 
{
	int n = 0;

	while (s[n] != '\0')
		n++;

	dbgmon_comm_send(comm, dbgmon_blanks, width - n);
	dbgmon_comm_send(comm, s, n);
}

void dbgmon_comm_send_blanks(unsigned int width, 
                             const struct dbgmon_comm * comm) 
{
	dbgmon_comm_send(comm, dbgmon_blanks, width);
}

void dbgmon_comm_send_hex(uint32_t val, unsigned int width, 
                          const struct dbgmon_comm * comm) 
{
	char s[12];
	int n;

	n = uint2hex(s, val);
	dbgmon_comm_send(comm, dbgmon_zeros, width - n);
	dbgmon_comm_send(comm, s, n);
}

