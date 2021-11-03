

#include <stdarg.h>
#include <thinkos.h>
#include <sys/param.h>

#ifndef KRN_PRINTF_ENABLE_LEFT_ALIGN
#define KRN_PRINTF_ENABLE_LEFT_ALIGN    0
#endif

#ifndef KRN_PRINTF_ENABLE_LARGE_PADDING
#define KRN_PRINTF_ENABLE_LARGE_PADDING 0
#endif

#ifndef KRN_PRINTF_ENABLE_FAST
#define KRN_PRINTF_ENABLE_FAST          0
#endif

#ifndef KRN_PRINTF_ENABLE_UNSIGNED
#define KRN_PRINTF_ENABLE_UNSIGNED      1
#endif

#ifndef KRN_PRINTF_ENABLE_POINTER
#define KRN_PRINTF_ENABLE_POINTER       0
#endif

#ifndef KRN_PRINTF_ENABLE_LONG
#define KRN_PRINTF_ENABLE_LONG          0
#endif

#ifndef KRN_PRINTF_ENABLE_ARG_WIDTH
#define KRN_PRINTF_ENABLE_ARG_WIDTH    0
#endif

int uint2dec(char * s, unsigned int val);
int uint2hex(char * s, unsigned int val);

int ull2dec(char * s, unsigned long long val);
int ull2hex(char * s, unsigned long long val);

#if KRN_PRINTF_ENABLE_LONG
#define BUF_LEN 22
#else
#define BUF_LEN 12
#endif

#define PERCENT 0x01
#define WIDTH 0x02
#define ZERO 0x04
#define LEFT 0x08
#define SIGN 0x10
#define LONG 0x20
#define LONG2 0x40


#if (KRN_PRINTF_ENABLE_LONG)
#undef KRN_PRINTF_ENABLE_LARGE_PADDING
#define KRN_PRINTF_ENABLE_LARGE_PADDING 1
#endif

static const char krn_zeros[]  = {  
#if (KRN_PRINTF_ENABLE_LARGE_PADDING)
	'0', '0', '0', '0', '0', '0', '0', '0', 
	'0', '0', '0', '0', '0', '0', '0', '0', 
	'0', '0', '0', '0', 
#endif
	'0', '0', '0', '0', '0', '0', '0', '0', 
	'0', '0', '0', '0', };
	
static const char krn_blanks[] = {
#if (KRN_PRINTF_ENABLE_LARGE_PADDING)
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
#endif
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };


struct krn_str {
	char * ptr;
	char * end;
};

static int krn_str_write(struct krn_str * str, const void * buf, size_t len)
{
	char * src = (char *)buf;
	char * dst = (char *)str->ptr;
	char * end;
	int cnt;

	end = MIN(str->end, str->ptr + len);

	while (dst != end)
		*dst++ = *src++;

	cnt = dst - str->ptr;
	str->ptr = dst;

	return cnt;
}

int krn_vsnprintf(char * str, size_t size, const char *fmt, va_list ap)
{
	struct krn_str ref;
	char buf[BUF_LEN];
	int flags;
	int cnt;
	int c;
	int w;
	int n;
	int r;
	char * cp;
	union {
		void * ptr;
		unsigned int n;
		int i;
#if (KRN_PRINTF_ENABLE_LONG)
		unsigned long long ull;
		long long ll;
#endif
	} val;

	ref.ptr = str;
	ref.end = str + size - 1; /* reserve space for '\0' */

	if (str == NULL)
		return -1;

	if (size == 0)
		return -1;

	n = 0;
	w = 0;
	cnt = 0;
#if (KRN_PRINTF_ENABLE_FAST)
	cp = (char *)fmt;
#endif
	for (flags = 0; (c = *fmt++); ) {
		if (flags == 0) {
			if (c == '%') {
				w = 0;
				flags = PERCENT;
#if (KRN_PRINTF_ENABLE_FAST)
				if (n) {
					n = krn_str_write(&ref, cp, n);
					cp = (char *)fmt;
					cnt += n;;
					n = 0;
				}
#endif
				continue;
			}

#if (KRN_PRINTF_ENABLE_FAST)
			n++;
#else
			buf[0] = c;
			cnt += krn_str_write(&ref, buf, 1);
#endif
			continue;
		}

		if ((c >= '0') && (c <= '9')) {
			if (!(flags & WIDTH)) {
				flags |= WIDTH;
				if (c == '0') {
					flags |= ZERO;
					continue;
				}
			}
			/* w = w * 10 + c - '0' */
			w = (((w << 2) + w) << 1) + (c - '0');
			continue;
		}

#if (KRN_PRINTF_ENABLE_LEFT_ALIGN)
		if (c == '-') {
			flags |= LEFT;
			continue;
		}
#else
		if (c == '-')
			continue;
#endif

#if (KRN_PRINTF_ENABLE_ARG_WIDTH)
		if (c == '*') {
			w = va_arg(ap, int);
			continue;
		}
#endif

#if (KRN_PRINTF_ENABLE_LONG)
		if (c == 'l') {
			flags |= (flags & LONG) ? LONG2 : LONG;
			continue;
		}
#endif

		if (c == 'c') {
			c = va_arg(ap, int);
			goto print_char;
		}

		if (c == 'd') {
			cp = buf;
#if KRN_PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ll = va_arg(ap, long long);
				if (val.ll < 0) {
					buf[0] = '-';
					val.ll = -val.ll;
					flags |= SIGN;
					cp++;
					w--;
				}
				n = ull2dec(cp, val.ull);
			} else
#endif
			{
				val.i = va_arg(ap, int);
				if (val.i < 0) {
					buf[0] = '-';
					val.i = -val.i;
					flags |= SIGN;
					cp++;
					w--;
				}
				n = uint2dec(cp, val.n);
			}
			goto print_buf;
		}

		if (c == 'x') {
			cp = buf;
#if KRN_PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ull = va_arg(ap, unsigned long long);
				n = ull2hex(cp, val.ull);
			} else
#endif
			{
				val.n = va_arg(ap, unsigned int);
#if (KRN_PRINTF_ENABLE_POINTER)
hexadecimal:
#endif
				n = uint2hex(cp, val.n);
			}
			goto print_buf;
		}

		if (c == 's') {
			cp = va_arg(ap, char *);
			n = 0;
			while (cp[n] != '\0')
				n++;
			goto print_buf;
		}

#if (KRN_PRINTF_ENABLE_UNSIGNED)
		if (c == 'u') {
			cp = buf;
#if KRN_PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ull = va_arg(ap, unsigned long long);
				n = ull2dec(cp, val.ull);
			} else
#endif
			{
				val.n = va_arg(ap, unsigned int);
				n = uint2dec(cp, val.n);
			}
			goto print_buf;
		}
#endif

#if (KRN_PRINTF_ENABLE_POINTER)
		if (c == 'p') {
			val.ptr = va_arg(ap, void *);
			w = 8;
			flags |= ZERO;
			goto hexadecimal;
		}
#endif
	
		flags = 0;
		w = 0;

print_char:
		cp = buf;
		buf[0] = c;
		n = 1;

print_buf:
#if (KRN_PRINTF_ENABLE_LEFT_ALIGN)
		if (!(flags & LEFT) && (w > n)) {
#else
		if (w > n) {
#endif
			if (flags & ZERO) {
				if (flags & SIGN) {
					flags &= ~SIGN;
					cnt += krn_str_write(&ref, buf, 1);
				}
				r = krn_str_write(&ref, krn_zeros, w - n);
			} else {
				r = krn_str_write(&ref, krn_blanks, w - n);
			}
			cnt += r;
		}

		if (flags & SIGN) {
			cnt += krn_str_write(&ref, buf, 1);
		}

		cnt += krn_str_write(&ref, cp, n);

#if (KRN_PRINTF_ENABLE_LEFT_ALIGN)
		if ((flags & LEFT) && (w > n)) {
			r = krn_str_write(&ref, krn_blanks, w - n);
			cnt += r;
		}
#endif

		flags = 0;
		w = 0;

#if (KRN_PRINTF_ENABLE_FAST)
		cp = (char *)fmt;
		n = 0;
#endif
	}

#if (KRN_PRINTF_ENABLE_FAST)
	if (n) {
		r = krn_str_write(&ref, cp, n);
		cnt += r;;
	}
#endif

	str[cnt] = '\0';

	return cnt;
}

int krn_snprintf(char * str, size_t size, const char *fmt, ...) 
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = krn_vsnprintf(str, size, fmt, ap);
	va_end(ap);

	return n;
}

int snprintf(char * str, size_t size, const char *fmt, ...) 
	__attribute__ ((weak, alias ("krn_snprintf")));

