#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#define IN_BS      '\x8'
#define IN_DEL      0x7F
#define IN_EOL      '\r'
#define IN_SKIP     '\3'
#define IN_EOF      '\x1A'
#define IN_ESC      '\033'

#define OUT_DEL     "\x8 \x8"
#define OUT_EOL     "\r\n"
#define OUT_SKIP    "^C\r\n"
#define OUT_EOF     "^Z"
#define OUT_BEL     '\7'

int monitor_gets(char * s, int size, const struct monitor_comm * comm)
{
	char buf[1];
	int c;
	int pos;

	/* left room to '\0' */
	size--;
	pos = 0;

	for (;;) {
		if (monitor_comm_recv(comm, buf, sizeof(char)) <= 0)
			return -1;

		c = buf[0];

		if (c == IN_EOL) {
			monitor_puts(OUT_EOL, comm);
			break;
		} else if (c == IN_SKIP) {
			monitor_puts(OUT_SKIP, comm);
			return -1;
		} else if (c == IN_BS || c == IN_DEL) {
			if (pos == 0) {
				monitor_putc(OUT_BEL, comm);
			} else {
				pos--;
				monitor_puts(OUT_DEL, comm);
			}
		} else if (c == IN_ESC) {
		} else if (pos == size) {
			monitor_putc(OUT_BEL, comm);
			continue;
		}

		s[pos++] = c;
		monitor_putc(c, comm);
	}

	s[pos] = '\0';

	return pos;
}

