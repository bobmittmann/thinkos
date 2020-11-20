#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "fixpt.h"
#include "pcm.h"

struct pcm16 * pcm16_create(unsigned int len, unsigned int samplerate)
{
	struct pcm16 * pcm;

	if ((pcm = calloc(1, sizeof(struct pcm16) + 
					  len * sizeof(int16_t))) == NULL) {
		fprintf(stderr, "%s: calloc() failed: %s", __func__, strerror(errno));
		return NULL;
	};

	pcm->len = len;
	pcm->samplerate = samplerate;

	return pcm;
}

int pcm16_destroy(struct pcm16 * pcm)
{
	if (pcm == NULL) {
		fprintf(stderr, "%s: NULL pointer.", __func__);
		return -1;
	};

	free(pcm);

	return 0;
}

/*  Create a sinusoidal waveform  */
/* XXX: this function actually produces: -cos(t) .. */
int pcm16_cos(struct pcm16 * pcm, float freq, float amp) 
{
	int32_t x, x0;
	int32_t y, y0;
	int32_t c;
	int32_t z;
	int32_t a;
	float w0;
	int i;

	if (pcm == NULL) {
		fprintf(stderr, "%s: NULL pointer.", __func__);
		return -1;
	};

	if (amp >= 1)
		a = Q15(1.0);
	else if (amp <= -1)
		a = Q15(-1.0);
	else
		a = Q15(amp);

	w0 = (2.0 * M_PI * freq) / pcm->samplerate;
	c = Q15(2.0 * sin(w0 / 2.0));

	x = Q15(1.0);
	y = c / 2;

	z = pcm->sample[0];
	pcm->sample[0] = Q15_SAT(z + Q15_MUL(x, a));

	for (i = 1; i < pcm->len; ++i) {
		x0 = x;
		y0 = y;
		x = x0 - Q15_MUL(c, y0);
		y = Q15_MUL(c, x) + y0;
		/* saturate  */
		x = Q15_SAT(x);

		/* add to the existing data in the buffer */
		z = pcm->sample[i];
		pcm->sample[i] = Q15_SAT(z + Q15_MUL(x,a));
	}

	return 0;
}

int pcm16_sin(struct pcm16 * pcm, float freq, float amp) 
{
	int32_t x;
	int32_t z;
	int32_t a;
	float w0;
	float p, p0;
	float q, q0;
	int i;
	float r[2][2];

	if (pcm == NULL) {
		fprintf(stderr, "%s: NULL pointer.", __func__);
		return -1;
	};

	if (amp >= 1)
		a = Q15(1.0);
	else if (amp <= -1)
		a = Q15(-1.0);
	else
		a = Q15(amp);

	w0 = (2.0 * M_PI * freq) / pcm->samplerate;

	r[0][0] = cos(w0);
	r[0][1] = -sin(w0);
	r[1][0] = sin(w0);
	r[1][1] = cos(w0);

	p = 1.0;
	q = 0.0;

	for (i = 0; i < pcm->len; ++i) {
		x = Q15(q);
		/* saturate  */
		x = Q15_SAT(x);
		/* add to the existing data in the buffer */
		z = pcm->sample[i];
		pcm->sample[i] = Q15_SAT(z + Q15_MUL(x,a));

		p0 = p;
		q0 = q;
		p = p0 * r[0][0] + q0 * r[0][1];
		q = p0 * r[1][0] + q0 * r[1][1];
	}

	return 0;
}

int pcm16_sweep(struct pcm16 * pcm, float f0, float f1, float amp) 
{
//	int32_t x, x0;
//	int32_t y, y0;
	int32_t x;
	int32_t z;
	int32_t a;
	float w0;
	float w1;
	float dw;
	float p, p0;
	float q, q0;
	int i;
	float r[2][2];
	float d[2][2];

	if (pcm == NULL) {
		fprintf(stderr, "%s: NULL pointer.", __func__);
		return -1;
	};

	if (amp >= 1)
		a = Q15(1.0);
	else if (amp <= -1)
		a = Q15(-1.0);
	else
		a = Q15(amp);

	w0 = (2.0 * M_PI * f0) / pcm->samplerate;
	w1 = (2.0 * M_PI * f1) / pcm->samplerate;
	dw = (w1 - w0) / pcm->len;

//	s0 = Q15(sin(w0));
//	c0 = Q15(cos(w0));
//
//	s1 = Q15(sin(dw));
//	c1 = Q15(cos(dw));

	r[0][0] = cos(w0);
	r[0][1] = -sin(w0);
	r[1][0] = sin(w0);
	r[1][1] = cos(w0);

	d[0][0] = cos(dw);
	d[0][1] = -sin(dw);
	d[1][0] = sin(dw);
	d[1][1] = cos(dw);

	p = 1.0;
	q = 0.0;
	x = Q15(p);

//	x = 1;
//	y = 0;

	z = pcm->sample[0];
	pcm->sample[0] = Q15_SAT(z + Q15_MUL(x, a));

	for (i = 1; i < pcm->len; ++i) {

//		x0 = x;
//		y0 = y;
//		x = Q15_MUL(x0, c0) - Q15_MUL(y0, s0);
//		y = Q15_MUL(x0, s0) + Q15_MUL(y0, s0);
//		x = Q15_MUL(x0, c0) - Q15_MUL(y0, s0);
//		y = Q15_MUL(x0, s0) + Q15_MUL(y0, s0);

		p0 = p;
		q0 = q;

		p = p0 * r[0][0] + q0 * r[0][1];
		q = p0 * r[1][0] + q0 * r[1][1];

		x = Q15(p);

		/* saturate  */
		x = Q15_SAT(x);
		/* add to the existing data in the buffer */
		z = pcm->sample[i];
		pcm->sample[i] = Q15_SAT(z + Q15_MUL(x,a));


		r[0][0] = r[0][0] * d[0][0] + r[0][1] * d[1][0];
		r[0][1] = r[0][0] * d[0][1] + r[0][1] * d[1][1];
		r[1][0] = r[1][0] * d[0][0] + r[1][1] * d[1][0];
		r[1][1] = r[1][0] * d[0][1] + r[1][1] * d[1][1];
	}

	return 0;

}

/* superimpose a white noise on top of the waveform */
int pcm16_white_noise(struct pcm16 * pcm, float level) 
{
	int16_t w[pcm->len];
	int32_t x;
	int16_t v;
	int16_t a;
	int i;
	int j;
	int n = 16;

	if (pcm == NULL) {
		fprintf(stderr, "%s: NULL pointer.", __func__);
		return -1;
	};

	if (level >= 1)
		a = Q15_MAX;
	else if (level <= -1)
		a = Q15_MIN;
	else
		a = Q15(level);

	/* create a white noise (normal distribution) */
	/* using central limit theorem method */

	/* initialize the vector to set the mean to 0  */
	for (i = 0; i < pcm->len; ++i) 
		w[i] = -n * (65536 / (n * 2));

	for (j = 0; j < n; ++j)
		for (i = 0; i < pcm->len; i++) 
			w[i] += rand() % (65536 / n);

	v = Q15(sqrt(12.0 / n)); // adjust variance to 1
	v = Q15_MUL(a, v);

	/* adjust the level and variance */
	for (i = 0; i < pcm->len; ++i) 
		w[i] = Q15_MUL(w[i], v);

	for (i = 0; i < pcm->len; ++i) {
		x = pcm->sample[i] + w[i];
		pcm->sample[i] = Q15_SAT(x);
	}

	return 0;
}

/* write the waveform into a file suitable for gnuplot */
int pcm16_dump(const char * fname, struct pcm16 * pcm)
{
	FILE * f;
	int i;

	if (fname == NULL) {
		f = stdout;
	} else { 
		if ((f = fopen(fname, "w+")) == NULL) {
			return -1;
		}
	}

	for (i = 0; i < pcm->len; ++i) {
		fprintf(f, "%9.6f %8.5f\n", (float)i / pcm->samplerate, 
				Q15F(pcm->sample[i]));
	}

	fclose(f);

	return 0;
}

/* write the waveform into a C file */
int pcm16_c_dump(const char * fname, struct pcm16 * pcm)
{
	FILE * f;
	int i;

	if (fname == NULL) {
		f = stdout;
	} else { 
		if ((f = fopen(fname, "w+")) == NULL) {
			return -1;
		}
	}

	fprintf(f, "uint16_t samplerate = %d\n", pcm->samplerate);
	fprintf(f, "int16_t wave[] = {");

	for (i = 0; i < pcm->len; ++i) {
		if ((i % 8) == 0) {
			if (i != 0)
				fprintf(f, ",");
			fprintf(f, "\n\t");
		} else 
			fprintf(f, ", ");
		fprintf(f, "%6d", pcm->sample[i]);
	}

	fprintf(f, "\n};\n");

	fclose(f);

	return 0;
}

