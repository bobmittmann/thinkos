
/*-------------------------------------------------------------------------
* Copyright (c) 2003-2006 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
* Marcus Geelnard
* marcus.geelnard at home.se
*************************************************************************/

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
//#include <io.h>

#include "huffman.h"

#define LZ77_MAGIC 0x4C5A3737
#define LZ78_MAGIC 0x4C5A3738

uint32_t crc32sum(uint32_t __crc, const void * __buf, unsigned int __len);

bool is_pwr2(uint32_t v)
{
	return v && !(v & (v - 1));
}

static uint32_t lz_stringcompare(uint8_t * str1, uint8_t * str2, 
								  uint32_t minlen, uint32_t maxlen)
{
	uint32_t len;

	for (len = minlen; (len < maxlen) && (str1[len] == str2[len]); ++ len);

	return len;
}

static int lz_writevarsize(uint32_t x, uint8_t * buf)
{
    uint32_t y;
    int num_bytes, i, b;

    /* Determine number of bytes needed to store the number x */
    y = x >> 3;
    for( num_bytes = 5; num_bytes >= 2; -- num_bytes ) {
        if( y & 0xfe000000 ) break;
        y <<= 7;
    }

    /* Write all bytes, seven bits in each, with 8:th bit set for all */
    /* but the last byte. */
    for( i = num_bytes-1; i >= 0; -- i ) {
        b = (x >> (i*7)) & 0x0000007f;
        if( i > 0 ) {
            b |= 0x00000080;
        }
        *buf ++ = (uint8_t) b;
    }

    /* Return number of bytes written */
    return num_bytes;
}

static int lz_readvarsize(uint32_t * x, const uint8_t * buf)
{
    uint32_t y, b;
	uint32_t num_bytes;

    /* Read complete value (stop when byte contains zero in 8:th bit) */
    y = 0;
    num_bytes = 0;
    do {
        b = (uint32_t) (*buf ++);
        y = (y << 7) | (b & 0x0000007f);
        ++ num_bytes;
    } while( b & 0x00000080 );

    /* Store value in x */
    *x = y;

    /* Return number of bytes read */
    return num_bytes;
}

uint32_t * lz_hist_make(uint32_t hist[], const uint8_t in[], uint32_t size)
{
    int i;

    /* Create histogram */
    for ( i = 0; i < 256; ++ i) {
        hist[i] = 0;
    }

    for (i = 0; i < size; ++ i) {
        ++hist[in[i]];
    }

	return hist;
}

int lz_hist_fmin(uint32_t hist[])
{
    int min = 0;
	int i;

	for (i = 1; i < 256; ++ i) {
		if (hist[i] < hist[min] ) {
			min = i;
		}
	}

	return min;
}

int lz_hist_f0cnt(uint32_t hist[])
{
    int cnt = 0;
	int i;

	for (i = 0; i < 256; ++ i) {
		if (hist[i] == 0) {
			cnt++;
		}
	}

	return cnt;
}

int lz_compress(const uint8_t in[], uint8_t out[], 
				uint32_t insize, uint32_t max_offset)
{
    uint8_t marker;
    uint8_t symbol;
    uint32_t inpos, outpos, bytesleft;
    uint32_t maxoffset, offset, bestoffset;
    uint32_t maxlength, length, bestlength;
    uint32_t hist[256];
    uint8_t *ptr1, *ptr2;

    /* Do we have anything to compress? */
    if (insize < 1) {
        return 0;
    }

    lz_hist_make(hist, in, insize);
	marker = lz_hist_fmin(hist);

    /* Remember the marker symbol for the decoder */
    out[0] = marker;

    /* Start of compression */
    inpos = 0;
    outpos = 1;

    /* Main compression loop */
    bytesleft = insize;
    do {
        /* Determine most distant position */
        if (inpos > max_offset) 
			maxoffset = max_offset;
        else                        
			maxoffset = inpos;
        /* Get pointer to current position */
        ptr1 = (uint8_t *)&in[inpos];
        /* Search history window for maximum length string match */
        bestlength = 3;
        bestoffset = 0;
        for (offset = 3; offset <= maxoffset; ++offset) {
            /* Get pointer to candidate string */
            ptr2 = &ptr1[-(int)offset];

            /* Quickly determine if this is a candidate (for speed) */
            if( (ptr1[ 0 ] == ptr2[ 0 ]) &&
                (ptr1[ bestlength ] == ptr2[ bestlength ]) ) {
                /* Determine maximum length for this offset */
                maxlength = (bytesleft < offset ? bytesleft : offset);
                /* Count maximum length match at this offset */
                length = lz_stringcompare(ptr1, ptr2, 0, maxlength);
                /* Better match than any previous match? */
                if( length > bestlength ) {
                    bestlength = length;
                    bestoffset = offset;
                }
            }
        }

        /* Was there a good enough match? */
        if( (bestlength >= 8) ||
            ((bestlength == 4) && (bestoffset <= 0x0000007f)) ||
            ((bestlength == 5) && (bestoffset <= 0x00003fff)) ||
            ((bestlength == 6) && (bestoffset <= 0x001fffff)) ||
            ((bestlength == 7) && (bestoffset <= 0x0fffffff)) ) {
            out[ outpos ++ ] = (uint8_t) marker;
            outpos += lz_writevarsize(bestlength, &out[outpos]);
            outpos += lz_writevarsize(bestoffset, &out[outpos]);
            inpos += bestlength;
            bytesleft -= bestlength;
        } else {
            /* Output single byte (or two bytes if marker byte) */
            symbol = in[inpos++];
            out[outpos++] = symbol;
            if (symbol == marker) {
                out[ outpos ++ ] = 0;
            }
            -- bytesleft;
        }
    } while(bytesleft > 3);

    /* Dump remaining bytes, if any */
    while ( inpos < insize) {
        if (in[inpos] == marker) {
            out[outpos++] = marker;
            out[outpos++] = 0;
        } else {
            out[outpos++] = in[inpos];
        }
        ++inpos;
    }

    return outpos;
}

void lz_uncompress(uint8_t buf[], uint8_t out[], uint32_t insize, 
				   uint32_t wndsz)
{
    uint8_t marker;
	uint8_t symbol;
	uint8_t wnd[wndsz];
	uint32_t wndmsk;
    uint32_t i;
	uint32_t inpos;
	uint32_t outpos;
	uint32_t cfgpos;
	uint32_t length;
	uint32_t offset;

	wndmsk = wndsz - 1;

    /* Get marker symbol from input stream */
    marker = buf[0];
    inpos = 1;

    /* Main decompression loop */
    outpos = 0;
	cfgpos = 0;
    do {
        symbol = buf[inpos++];
        if (symbol == marker) {
            /* We had a marker byte */
            if (buf[inpos] == 0) {
                /* It was a single occurrence of the marker byte */
                wnd[outpos++ & wndmsk] = marker;
                ++inpos;
            } else {
                /* Extract true length and offset */
                inpos += lz_readvarsize(&length, &buf[inpos]);
                inpos += lz_readvarsize(&offset, &buf[inpos]);

                /* Copy corresponding data from history window */
                for ( i = 0; i < length; ++ i ) {
					uint8_t c;
					c = wnd[(outpos - offset) & wndmsk];
                    wnd[outpos & wndmsk] = c;
					++outpos;
                }
            }
        } else {
            /* No marker, plain copy */
            wnd[(outpos++ & wndmsk)] = symbol;
        }

    	while ((outpos - cfgpos) >= 1) {
			uint32_t c;

			c = wnd[cfgpos & wndmsk];
			out[cfgpos] = c;
			cfgpos++;
		}

    } while (inpos < insize);
}

uint32_t lz_crc_uncompress(uint8_t buf[], uint8_t out[], uint32_t insize, 
					   uint32_t wndsz)
{
    uint8_t marker;
	uint8_t symbol;
	uint8_t wnd[wndsz];
	uint32_t wndmsk;
    uint32_t i;
	uint32_t inpos;
	uint32_t outpos;
	uint32_t cfgpos;
	uint32_t length;
	uint32_t offset;
	uint32_t crc;

	wndmsk = wndsz - 1;

    /* Get marker symbol from input stream */
    marker = buf[0];
    inpos = 1;
	crc = 0;

    /* Main decompression loop */
    outpos = 0;
	cfgpos = 0;
    do {
        symbol = buf[inpos++];
        if (symbol == marker) {
            /* We had a marker byte */
            if (buf[inpos] == 0) {
                /* It was a single occurrence of the marker byte */
                wnd[outpos++ & wndmsk] = marker;
                ++inpos;
            } else {
                /* Extract true length and offset */
                inpos += lz_readvarsize(&length, &buf[inpos]);
                inpos += lz_readvarsize(&offset, &buf[inpos]);

                /* Copy corresponding data from history window */
                for ( i = 0; i < length; ++ i ) {
					uint8_t c;
					c = wnd[(outpos - offset) & wndmsk];
                    wnd[outpos & wndmsk] = c;
					++outpos;
                }
            }
        } else {
            /* No marker, plain copy */
            wnd[(outpos++ & wndmsk)] = symbol;
        }

    	while ((outpos - cfgpos) >= 1) {
			uint32_t c;

			c = wnd[cfgpos & wndmsk];
			out[cfgpos] = c;
			crc = crc32sum(crc, &out[cfgpos], 1);
			cfgpos++;
		}

    } while (inpos < insize);

	return crc;
}
	
int fread_u32(uint32_t * xp, FILE *f)
{
	uint8_t buf[4];
	uint32_t w;
	int ret;
	
	if ((ret = fread(buf, 4, 1, f)) <= 0)
		return ret;

#if __BYTE_ORDER == __LITTLE_ENDIAN
	w = (((uint32_t)buf[0]) << 24) +
		(((uint32_t)buf[1]) << 16) +
		(((uint32_t)buf[2]) << 8)  +
		(uint32_t)buf[3];
#elif __BYTE_ORDER == __BIG_ENDIAN
	w = (((uint32_t)buf[3]) << 24) +
		(((uint32_t)buf[2]) << 16) +
		(((uint32_t)buf[1]) << 8)  +
		(uint32_t)buf[0];
#else
# error "Please fix <bits/endian.h>"
#endif
	if (xp != NULL)
		*xp = w;

	return ret;
}

void fwrite_u32(uint32_t x, FILE *f)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	fputc((x >> 24) & 0xff, f );
	fputc((x >> 16) & 0xff, f );
	fputc((x >> 8) & 0xff, f );
	fputc(x & 0xff, f );
#elif __BYTE_ORDER == __BIG_ENDIAN
	fputc(x & 0xff, f );
	fputc((x >> 8) & 0xff, f );
	fputc((x >> 16) & 0xff, f );
	fputc((x >> 24) & 0xff, f );
#else
# error "Please fix <bits/endian.h>"
#endif
}

ssize_t getfilesize(FILE *f)
{
    ssize_t pos, size;

    pos = ftell(f);
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, pos, SEEK_SET);

    return size;
}

void usage(FILE * f, char * prog)
{
	fprintf(f, "Usage: %s [OPTION...] file\n", prog);
	fprintf(f, "lz77 - LZ77 compressor/decompressor.\n");
	fprintf(f, "\n");
	fprintf(f, "  -?        \tShow this help message\n");
	fprintf(f, "  -v[v]     \tVerbosity level\n");
	fprintf(f, "  -V        \tPrint version\n");
	fprintf(f, "  -o FILE   \toutput file\n");
	fprintf(f, "  -w WINSIZE\twindow size\n");
	fprintf(f, "  -c        \tcompress\n");
	fprintf(f, "\n");
}

void version(char * prog)
{
	fprintf(stderr, "%s\n", PACKAGE_STRING);
	fprintf(stderr, "(C)Copyright, Bob Mittmann.\n");
	exit(1);
}


int main(int argc,  char **argv)
{
	extern char *optarg;	/* getopt */
	extern int optind;	/* getopt */
	char * prog;
	bool output_set = false;
	char outname[256];
    char *inname;
    uint8_t * in;
	uint8_t * out;
    uint32_t insize;
	uint32_t outsize;
	uint32_t crc32;
	uint32_t magic;
	uint32_t hsize;
	int verbose = 0;
	bool compress = false;
	uint32_t wndsz = 8192;
    FILE *f;

	int c;

	/* the prog name start just after the last lash */
	if ((prog = (char *)basename(argv[0])) == NULL)
		prog = argv[0];

	/* parse the command line options */
	while ((c = getopt(argc, argv, "V?vco:w:")) > 0) {
		switch (c) {
		case 'V':
			version(prog);
			break;

		case '?':
			usage(stdout, prog);
			return 0;

		case 'v':
			verbose++;
			break;

		case 'c':
			compress = true;
			break;

		case 'o':
			strcpy(outname, optarg);
			output_set = true;
			break;

		case 'w':
			wndsz = atoi(optarg);
			break;

		default:
			fprintf(stderr, "%s: invalid option %s\n", prog, optarg);
			return 1;
		}
	}

	if (optind != (argc - 1)) {
		fprintf(stderr, "%s: missing input file.\n\n", prog);
		usage(stderr, prog);
		return 2;
	}

	if (!is_pwr2(wndsz)) {
		fprintf(stderr, "%s: window size must be apower of 2.\n\n", prog);
		usage(stderr, prog);
		return 2;
	}

	inname = argv[optind];

	if (verbose > 1) {
		if (compress) {
			fprintf(stderr, "LZ77 compress\n");
		} else {
			fprintf(stderr, "LZ77 decompress\n");
		}
	}

    /* Open input file */
    if ((f = fopen(inname, "rb" )) == NULL) {
        fprintf(stderr, "Unable to open input file \"%s\".\n", inname);
        return 1;
    }

    /* Get input file size */
    insize = getfilesize(f);

    if (!compress) {
        /* Read header */
        fread_u32(&magic, f);  
        fread_u32(&outsize, f);
        fread_u32(&crc32, f);
        insize -= 12;
		if ((magic != LZ77_MAGIC) && (magic != LZ78_MAGIC)) {
        	fprintf(stderr, "Invalid LZ77 file \"%s\"!\n", inname);
			return 2;
    	}
		if (magic == LZ78_MAGIC) {
			fread_u32(&hsize, f);
        	insize -= 4;
    	}
    }

    /* Read input file */
    if ((in = (uint8_t *)malloc(insize)) == NULL) {
        fprintf(stderr, "Not enough memory!\n");
        fclose( f );
        return 3;
    }
    fread(in, insize, 1, f);
    fclose(f);

	if (output_set) {
    	/* Open output file */
	    if ((f = fopen( outname, "wb" )) == NULL) {
	        fprintf(stderr, "Unable to open output file \"%s\".\n", outname);
			free(in);
			return 4;
	    }
	} else {
		f = stdout;
//		setmode(fileno(stdout), O_BINARY);
	}

    if (compress) {
        /* Worst case buffer size */
        outsize = (insize*104+50)/100 + 384;
    }

    /* Allocate memory for output buffer */
    if ((out = malloc(outsize)) == NULL) {
        fprintf(stderr, "Not enough memory\n");
        fclose(f);
        free(in);
        return 5;
    }

    if (compress) {
#if (ENABLE_HUFFMANN)
		uint8_t * huff;

    	huff = malloc(outsize);

		hsize = huffman_compress(in, huff, insize);

		outsize = lz_compress(huff, out, hsize, wndsz);

		if (verbose) {
			fprintf(stderr, " - wnd=%-8d in=%-8d out=%-8d (%.2f%%)\n", wndsz, 
					outsize, insize, 100*(float)outsize/(float)insize );
		}
		magic = LZ77_MAGIC;
    	if ((huff = malloc(outsize)) == NULL) {
			hsize = INT32_MAX;
		} else {
			hsize = huffman_compress(out, huff, outsize);
		}


		if (hsize < (outsize - 4)) {
			fprintf(stderr, " - lz=%8d huffmann=%8d\n", outsize, hsize); 
			magic = LZ78_MAGIC;
		}

		fprintf(stderr, " - lz=%8d huffmann=%8d\n", outsize, hsize); 
		magic = LZ78_MAGIC;
#else
		outsize = lz_compress(in, out, insize, wndsz);
		if (verbose) {
			fprintf(stderr, " - wnd=%-8d in=%-8d out=%-8d (%.2f%%)\n", wndsz, 
					insize, outsize, 100*(float)outsize/(float)insize );
		}
		magic = LZ77_MAGIC;
#endif

		crc32 = crc32sum(0, in, insize);
        /* Write header */
        fwrite_u32(magic , f);
        fwrite_u32(insize, f);
        fwrite_u32(crc32, f);

		/* Write output file */
		if (magic == LZ78_MAGIC) {
	        fwrite_u32(hsize, f);
		}

		fwrite(out, outsize, 1, f);
	} else {
		uint32_t chk;

		if (magic == LZ78_MAGIC) {
			uint8_t * huff;

			fprintf(stderr, " - lz=%8d huffmann=%8d\n", insize, hsize); 

    		huff = malloc(outsize);

			lz_uncompress(in, huff, hsize, wndsz);

			huffman_uncompress(huff, out, hsize, outsize);
		} else {
			chk = lz_crc_uncompress(in, out, insize, wndsz);
			if (chk != crc32) {
				fprintf(stderr, " - CRC check error [%08x != %08x]!n", chk, crc32);
				fclose(f);
				free(in);
				return 7;
			}

			if (verbose) {
				fprintf(stderr, " - wnd=%-8d in=%-8d out=%-8d (%.2f%%)\n", wndsz, 
						outsize, insize, 100*(float)outsize/(float)insize );
			}
		}

		/* Write output file */
		fwrite(out, outsize, 1, f);
	}

	if (output_set)
    	fclose(f);

    /* Free memory */
    free(in);
    free(out);

    return 0;
}

