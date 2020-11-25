

#include <time.h>
#include <tthinkos.h>

int clock_getres(clockid_t clk_id, struct timespec *res)
{
	return 0;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	return 0;
}

int clock_settime(clockid_t clk_id, const struct timespec *tp)
{
	return 0;
}
