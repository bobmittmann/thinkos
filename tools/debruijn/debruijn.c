/* $Id: dcclog.c,v 1.10 2006/09/28 19:31:45 bob Exp $ 
 *
 * File:	debruijn.c
 * Module:
 * Project:	
 * Author:	Bob Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2019 Robinson Mittmann. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void usage(char * prog)
{
	fprintf(stderr, "Usage: %s [OPTION...] [SCRIPT...]\n", prog);
	fprintf(stderr, "Find the 'de Bruijn' sequences ..\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -?     \tShow this help message\n");
	fprintf(stderr, "  -n     N\tnumber of bits\n");
	fprintf(stderr, "  -v[v]  \tVerbosity level\n");
	fprintf(stderr, "  -V     \tPrint version\n");
	fprintf(stderr, "\n");
	exit(0);
}

void version(char * prog)
{
	fprintf(stderr, "%s\n", prog);
	fprintf(stderr, "(C)Copyright Bob Mittmann (bobmittmann@gmail.com)\n");
	exit(1);
}

void parse_err(char * prog, char * opt)
{
	fprintf(stderr, "%s: invalid option %s\n", prog, opt);
	exit(1);
}

static inline uint64_t ror(uint64_t x, int b, int n)
{
	uint64_t msk = UINT64_MAX >> (64 - b);

	return ((x << (b - n)) & msk) | (x >> n);
}

static inline uint64_t rol(uint64_t x, int b, int n)
{
	uint64_t msk = UINT64_MAX >> (64 - b);

	return ((x << n) & msk) | (x >> (b - n));
}

int _bin_debruijn_seq(unsigned int n)
{
	int64_t i;
	int64_t k;

	if (n > 8)
		return 1;

	k = 0;
	for (i = (1LL << 27) - 1; i < (1LL << 32); ++i) {
		int64_t	x = i;
		int64_t msk = (31LL << 27);
		int64_t cmp = x & msk;
		unsigned int j;

		j = 27;
		for (;;) {
			if (--j == 0) {
				k++;
				break;
			}
			x <<= 1;
			if ((x & msk) == cmp)
				break;
		}

	}

	printf("%I64d %.6f\n", k, k / 1000000.0);

	return 0;
}

void seq_print(uint64_t seq, int b, int n)
{
	int len = (1 << b);
	uint64_t msk;
	int j;
	int q;

	q = len - 1;
	msk = ((uint64_t)q);

	for (j = 0; j < n; ++j) {
		int x;

		x = (int)((seq & msk));
		printf(" %2d", x);

		seq = ror(seq, len, 1);
//		printf(",%04I64x ", seq);
	}
}

void seq_lookup_print(uint64_t seq, int b, int n, int m)
{
	uint64_t msk;
	int8_t lut[64];
	int len = (1 << b);
	int j;
	int q;

	q = len - 1;
	msk = ((uint64_t)q);

	for (j = 0; j < sizeof(lut); ++j)
		lut[j] = -1;


	for (j = 0; j < n; ++j) {
		int x;

		x = (int)((seq & msk));
		lut[x] = j;

		seq = ror(seq, len, 1);
	}

	for (j = 0; j < len; ++j) {
		int x;

		x = lut[j];
		printf(" %2d", x);
	}
}


void _seq_lookup_print(uint64_t seq, int b, int n, int m)
{
	uint64_t msk;
	int8_t lut[256];
//	int len = (1 << b);
	int j;
	int mb = ceil(log2(m));

	msk = (uint64_t)(mb - 1);
	//msk = (uint64_t)(len - 1);
	//printf(" mb=%d", mb);

	for (j = 0; j < sizeof(lut); ++j)
		lut[j] = -1;

	for (j = 0; j < (1 << mb); ++j) {
		int x;

		x = (int)((seq & msk));
		lut[x] = j;

	//	seq = ror(seq, len, 1);
		seq >>= 1;
	}

	for (j = 0; j < m; ++j) {
		int x;

		x = lut[j];
		printf(" %2d", x);
	}

}

int seq_k = 0;

struct seq_meta {
	uint8_t b;
	uint8_t n;
	uint8_t m;
	uint32_t cnt;
	void (* on_find)(uint64_t seq, int b, int n, int m);
};

void subseq_find(uint64_t seq, int lvl, struct seq_meta * meta)
{
	int b = meta->b;
	int len = (1 << b);


	if (lvl == 0) {
		meta->on_find(seq, b, meta->n, meta->m); 
	} else {
		uint32_t msk;
		uint32_t y;
		uint32_t x;
		int j;

		lvl--;

		msk = (uint32_t)(len - 1);
		/* New value of the sequence (0 shift)  */
		x = (uint64_t)(seq >> lvl) & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			y = (uint64_t)(seq >> j) & msk;
			if (y == x) {
				break;
			}
		}

		if (j == len)
			subseq_find(seq, lvl, meta);

		/* New value of the sequence (1 shift) */
		seq |= (1LL << lvl);
		x = (uint64_t)(seq >> lvl) & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			y = (uint64_t)(seq >> j) & msk;
			if (y == x) {
				break;
			}
		}

		if (j == len)
			subseq_find(seq, lvl, meta);
	}
}

void _subseq_find(uint64_t seq, int lvl, struct seq_meta * meta)
{
	int b = meta->b;
	int len = (1 << b);


	if (lvl == 0) {
		meta->on_find(seq, b, meta->n, meta->m); 
	} else {
		uint64_t msk;
		uint64_t x;
		int j;
		int q;

		q = len - 1;
		lvl--;

		msk = (((uint64_t)q) << lvl);
		/* New value of the sequence (0 shift)  */
		x = seq & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			msk <<= 1;
			x <<= 1;
			if ((seq & msk) == x) {
				break;
			}
//			printf("msk = %015I64x\n", msk);
		}

		if (j == len)
			_subseq_find(seq, lvl, meta);

		/* New value of the sequence (1 shift) */
		msk = (((uint64_t)q) << lvl);
		/* New value of the sequence (0 shift)  */
		seq |= (1LL << lvl);
		x = seq & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			msk <<= 1;
			x <<= 1;
			if ((seq & msk) == x)
				break;
		}

		if (j == len)
			_subseq_find(seq, lvl, meta);
	}
}

void __subseq_find(uint64_t seq, int lvl, struct seq_meta * meta)
{
	int b = meta->b;
	int len = (1 << b);

	lvl--;

	if (lvl == 0) {
		meta->on_find(seq, b, meta->n, meta->m); 
	} else {
		uint64_t msk;
		uint64_t x;
		uint64_t y;
		int j;

		msk = (uint64_t)(len - 1);
		//msk = (uint64_t)(len - 1) << lvl;

		/* New value of the sequence (0 shift)  */
		seq = seq << 1;
		x = seq & msk;
		y = seq;

		printf("lvl=%d seq=[", lvl);
		seq_print(seq, b, len);

		printf("] x=%2d y=[", (int)x);

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl; j < len; ++j) {
			y = ror(y, len, 1);
			printf(" %2d", (int)(y & msk));
			if ((y & msk) == x) {
				break;
			}
		}

		printf("]\n");
		if (j == len)
			__subseq_find(seq, lvl, meta);

		/* New value of the sequence (1 shift) */
		seq = seq | 1;
		//seq |= (1LL << lvl);
		x = seq & msk;
		y = seq;

//		printf("seq=%04I64x lvl=%d x=%2d", seq, lvl, (int)x);
		printf("lvl=%d ", lvl);

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			y = ror(y, len, 1);
			printf(" %2d", (int)(y & msk));
			if ((y & msk) == x) {
				break;
			}
		}

		printf("\n");
		if (j == len)
			__subseq_find(seq, lvl, meta);
	}
}

void ___subseq_find(uint64_t seq, int lvl, struct seq_meta * meta)
{
	int b = meta->b;
	int len = (1 << b);


	if (lvl == 0) {
		meta->on_find(seq, b, meta->n, meta->m); 
	} else {
		uint64_t msk;
		uint64_t x;
		int j;
		int q;

		q = len - 1;
		lvl--;

//		msk = (((uint64_t)q) << lvl);
		msk = rol((uint64_t)q, len, lvl);
		/* New value of the sequence (0 shift)  */
		x = seq & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			msk = rol(msk, len, 1);
			x = rol(x, len, 1);
			if ((seq & msk) == x) {
				break;
			}
//			printf("msk = %015I64x\n", msk);
		}

		if (j == len)
			___subseq_find(seq, lvl, meta);

		/* New value of the sequence (1 shift) */
//		msk = (((uint64_t)q) << lvl);
		msk = rol((uint64_t)q, len, lvl);
		/* New value of the sequence (0 shift)  */
		seq |= (1LL << lvl);
		x = seq & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = lvl + 1; j < len; ++j) {
			msk = rol(msk, len, 1);
			x = rol(x, len, 1);
			if ((seq & msk) == x)
				break;
		}

		if (j == len)
			___subseq_find(seq, lvl, meta);
	}
}


void on_find(uint64_t seq, int b, int n, int m)
{
//	seq_print(seq, b, n);
//	printf(" :");
	seq_lookup_print(seq, b, n, m);
	printf("\n");
	seq_k += 1;
}

int bin_debruijn_seq(unsigned int b, unsigned int n, unsigned int m)
{
	struct seq_meta meta;
    uint64_t seq;
	int x;
	int q;

	if (b > 6)
		return 1;

	seq_k = 0;
	q = (1 << b) - 1;
	
	meta.b = b;
	meta.n = n;
	meta.m = m;
	meta.on_find = on_find;

	printf("\nb=%d n=%d m=%d q=%d\n", b, n, m, q);

	for (x = 0; x < (1 << b); ++x) {
//	for (x = 0; x < 1; ++x) {
	    seq = (uint64_t)x << (q);

		subseq_find(seq, (1 << b), &meta);
	}

	printf("k=%d\n", seq_k);

	return 0;
}

int main(int argc, char *argv[])
{
	extern char *optarg;	/* getopt */
	extern int optind;	/* getopt */
	int verbose = 0;
	char * prog;
	int n = 0;
	int m = 0;
	int b = 5;
	int c;

	/* the prog name start just after the last lash */
	if ((prog = (char *)strrchr(argv[0], '/')) == NULL)
		prog = argv[0];
	else
		prog++;

	/* parse the command line options */
	while ((c = getopt(argc, argv, "V?vb:n:m:")) > 0) {
		switch (c) {
			case 'V':
				version(prog);
				break;

			case '?':
				usage(prog);
				break;

			case 'v':
				verbose++;
				break;

			case 'm':
				m = atoi(optarg);
				break;

			case 'n':
				n = atoi(optarg);
				break;

			case 'b':
				b = atoi(optarg);
				break;

			default:
				parse_err(prog, optarg);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "%s: extra arguments.\n\n", prog);
		usage(prog);
	}

	if ((n == 0) || (n > (1 << b))) {
		n = (1 << b);
	}

	if (m == 0) {
		m = n;
	}
	
	return bin_debruijn_seq(b, n, m);
}

