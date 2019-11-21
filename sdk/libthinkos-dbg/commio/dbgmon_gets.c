#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

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

int dbgmon_gets(char * s, int size, const struct dbgmon_comm * comm)
{
	char buf[1];
	int c;
	int pos;

	/* left room to '\0' */
	size--;
	pos = 0;

	for (;;) {
		if (dbgmon_comm_recv(comm, buf, sizeof(char)) <= 0)
			return -1;

		c = buf[0];

		if (c == IN_EOL) {
			dbgmon_puts(OUT_EOL, comm);
			break;
		} else if (c == IN_SKIP) {
			dbgmon_puts(OUT_SKIP, comm);
			return -1;
		} else if (c == IN_BS || c == IN_DEL) {
			if (pos == 0) {
				dbgmon_putc(OUT_BEL, comm);
			} else {
				pos--;
				dbgmon_puts(OUT_DEL, comm);
			}
		} else if (c == IN_ESC) {
		} else if (pos == size) {
			dbgmon_putc(OUT_BEL, comm);
			continue;
		}

		s[pos++] = c;
		dbgmon_putc(c, comm);
	}

	s[pos] = '\0';

	return pos;
}

