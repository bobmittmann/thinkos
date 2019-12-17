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

void seq_print(int64_t seq, int b)
{
	int64_t msk;
	int j;
	int n;

//		printf("%08I64x\n", seq);
	n = b - 1;
	/* Bit shift */
	msk = (31LL << n);
	/* New value of the sequence (0 shift)  */
	

	for (j = n; j >= 0; --j) {
		int x;

		x = (int)((seq & msk) >> n);
		printf(" %2d", x);

		seq <<= 1;
	}

	printf("\n");
}

int seq_k = 0;

static void seq_lookup(int64_t seq, int n)
{
	if (n == 0) {
		seq_print(seq, 32);
		seq_k += 1;
	} else {
		int64_t msk;
		int64_t x;
		int j;

		n--;

		/* Bit shift */
		msk = (31LL << n);
		/* New value of the sequence (0 shift)  */
		x = seq & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = n; j < (32 - 1); ++j) {
			msk <<= 1;
			x <<= 1;
			if ((seq & msk) == x) {
				break;
			}
//			printf("msk = %015I64x\n", msk);
		}

		if (j == (32 - 1))
			seq_lookup(seq, n);

		/* New value of the sequence (1 shift) */
		/* Bit shift */
		msk = (31LL << n);
		/* New value of the sequence (0 shift)  */
		seq |= (1LL << n);
		x = seq & msk;

		/* Compare the new value (y) with previous in the sequence */
		for (j = n; j < (32 - 1); ++j) {
			msk <<= 1;
			x <<= 1;
			if ((seq & msk) == x)
				break;
		}

		if (j == (32 - 1))
			seq_lookup(seq, n);
	}
}

int bin_debruijn_seq(unsigned int n)
{
	int x;

	if (n > 8)
		return 1;

	seq_k = 0;

	for (x = 0; x < 32; ++x) {
	    int64_t seq = (int64_t)x << (32 - 1);

		seq_lookup(seq, 32);
	}


	printf("\nk=%d\n", seq_k);

	return 0;
}

int main(int argc, char *argv[])
{
	extern char *optarg;	/* getopt */
	extern int optind;	/* getopt */
	int verbose = 0;
	char * prog;
	int c;
	int n = 5;

	/* the prog name start just after the last lash */
	if ((prog = (char *)strrchr(argv[0], '/')) == NULL)
		prog = argv[0];
	else
		prog++;

	/* parse the command line options */
	while ((c = getopt(argc, argv, "V?vn:")) > 0) {
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

			case 'n':
				n = atoi(optarg);
				break;

			default:
				parse_err(prog, optarg);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "%s: extra arguments.\n\n", prog);
		usage(prog);
	}
	
	return bin_debruijn_seq(n);
}

