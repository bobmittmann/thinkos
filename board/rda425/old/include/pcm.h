#ifndef __PCM_H__
#define __PCM_H__

#include <stdint.h>

struct pcm16 {
	uint32_t len;
	uint32_t samplerate;
	int16_t sample[];
};

#ifdef __cplusplus
extern "C" {
#endif

struct pcm16 * pcm16_create(unsigned int len, unsigned int samplerate);

int pcm16_destroy(struct pcm16 * pcm);

/* Add a sinusoidal waveform to the existing one */
int pcm16_sin(struct pcm16 * pcm, float freq, float amp);

/* Add a sinusoidal sweep */
int pcm16_sweep(struct pcm16 * pcm, float f0, float f1, float amp);

/* Add white noise to the the waveform */
int pcm16_white_noise(struct pcm16 * pcm, float level);

/* write the waveform into a file suitable for gnuplot */
int pcm16_dump(const char * fname, struct pcm16 * pcm);


int pcm16_c_dump(const char * fname, struct pcm16 * pcm);

#ifdef __cplusplus
}
#endif


#endif /* __PCM_H__ */

