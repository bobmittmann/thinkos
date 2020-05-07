/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 *PCM
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file dac.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stm32f.h>
#include <thinkos.h>
#include <fixpt.h>
#include <string.h>
#include <sys/param.h>

#include "board.h"
#include "dac.h"
#include "mad.h"

#define SHRT_MAX 32767

#ifndef DAC_FRAME_SIZE
#define DAC_FRAME_SIZE 64
#endif

#ifndef DAC_SAMPLERATE
#define DAC_SAMPLERATE 22050
#endif

#define DAC2_GPIO STM32_GPIOA, 5
#define DAC2_DMA_CHAN STM32_DMA_CHANNEL4

const int16_t dac_silence[DAC_FRAME_SIZE] = { 0, }; 

struct {
	uint16_t dma_buf[2][DAC_FRAME_SIZE];
	volatile int32_t offs;
	volatile int32_t gain;
	volatile uint32_t samp_cnt;
	volatile uint32_t clk;
	int16_t * volatile src_ptr;
	volatile uint8_t enabled;
	uint8_t flag;
} dac_rt;

int dac_task(void *arg)
{
	struct stm32f_dma *dma = STM32F_DMA1;

	dac_rt.samp_cnt = 0;
	dac_rt.clk = 0;

	for (;;) {
		uint16_t *dst;
		int16_t *src;
		uint32_t isr;
		int32_t gain = dac_rt.gain;
		int32_t offs = dac_rt.offs;
		int i;

		/* Wait for ADC convertion to finish */
		thinkos_irq_wait(STM32_IRQ_DMA1_CH4);
		isr = dma->isr;
		if (isr & DMA_TEIF4) {
			/* clear the DMA transfer error flag */
			dma->ifcr = DMA_CTEIF4;
		}
		if ((isr & (DMA_TCIF4 | DMA_CHTIF4)) == 0)
			continue;

		/* clear the DMA transfer complete flags */
		dma->ifcr = DMA_TCIF4 | DMA_CHTIF4;

		if (isr & DMA_HTIF4) {
			dst = dac_rt.dma_buf[0];
		} else {
			dst = dac_rt.dma_buf[1];
		}

		src = dac_rt.src_ptr;

		for (i = 0; i < DAC_FRAME_SIZE; ++i) {
			int32_t xi;
			int32_t yi;

			xi = src[i];

			yi = Q15_MUL(xi, gain);

			dst[i] = __SSAT((int32_t) yi, 16) + offs;
		}
		
		dac_rt.clk += DAC_FRAME_SIZE;
		thinkos_flag_give(dac_rt.flag);
	}

	return 0;
}

void dac_pcm8_play(const uint8_t pcm[], unsigned int len)
{
	int16_t buf[DAC_FRAME_SIZE];
	uint32_t clk;
	unsigned int j;
	

	thinkos_flag_take(dac_rt.flag);
	clk = dac_rt.clk;
	thinkos_flag_take(dac_rt.flag);
	clk = dac_rt.clk;

	j = 0;
	while (j < len) {
		int rem;
		int n;
		int i;

		rem = len - j;
		n = MIN(rem, DAC_FRAME_SIZE);

		for (i = 0; i < n; ++i)
			buf[i] = (pcm[j++] - 128) * 256;

		for (; i < DAC_FRAME_SIZE; ++i)
			buf[i] = 0;


		clk += DAC_FRAME_SIZE;
		thinkos_flag_take(dac_rt.flag);
		dac_rt.src_ptr = buf;
	}

	thinkos_flag_take(dac_rt.flag);
	dac_rt.src_ptr = (int16_t *)dac_silence;
}

void tim7_init(uint32_t freq)
{
	struct stm32f_tim *tim = STM32F_TIM7;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = (stm32f_tim1_hz + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;


//	printf("tim1_clk_hz=%dHz\n", stm32f_tim1_hz);
//	printf("TIM7: freq=%dHz div=%d ->  %dHz", freq, div,
//	    stm32f_tim1_hz / pre / div);

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM7);

	tim->cr1 = 0;		/* Disable the counter */
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE;	/* Update interrupt enable */
	tim->ccmr1 = 0;
	tim->ccr1 = 0;
	tim->cr2 = TIM_MMS_UPDATE;
	tim->sr = 0;
	tim->cr1 = TIM_URS | TIM_CEN;	/* Enable counter */
}

void dac_dma_init(void *src, unsigned int ndt)
{
	struct stm32f_dac *dac = STM32F_DAC1;
	struct stm32f_dma *dma = STM32F_DMA1;
	uint32_t cselr;

	/* DMA clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DMA1);

	/* DMA Disable */
	dma->ch[DAC2_DMA_CHAN].ccr = 0;
	/* Wait for the channel to be ready .. */
	while (dma->ch[DAC2_DMA_CHAN].ccr & DMA_EN) ;

	/* Channel select */
	cselr = dma->cselr & ~(DMA1_C4S_MSK);
	dma->cselr = cselr | DMA1_C4S_TIM7_UP_DAC2;

	/* Peripheral address */
	dma->ch[DAC2_DMA_CHAN].cpar = &dac->dhr12l2;
//      dma->ch[DAC2_DMA_CHAN].cpar = &dac->dhr12r2;
	/* Memory pointer */
	dma->ch[DAC2_DMA_CHAN].cmar = (void *)src;
	/* Number of data items to transfer */
	dma->ch[DAC2_DMA_CHAN].cndtr = ndt;
	/* Configuration for double buffer circular, 
	   half-transfer interrupt  */
	dma->ch[DAC2_DMA_CHAN].ccr = DMA_MSIZE_16 | DMA_PSIZE_16 |
	    DMA_MINC | DMA_CIRC | DMA_DIR_MTP;
	/* enable DAC DMA */
	dma->ch[DAC2_DMA_CHAN].ccr |= DMA_HTIE | DMA_TCIE | DMA_TEIE | DMA_EN;
}

uint32_t dac_stack[64] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf dac_thread_inf = {
	.stack_ptr = dac_stack,
	.stack_size = sizeof(dac_stack),
	.priority = 2,
	.thread_id = 2,
	.paused = 0,
	.tag = "DAC"
};

void dac_init(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	/* DAC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DAC1);
	/* DAC disable */
	dac->cr = 0;

	/* DAC channel 2 initial value */
	dac->dhr12l2 = 32768;

	dac_dma_init(dac_rt.dma_buf[0], 2 * DAC_FRAME_SIZE);

	tim7_init(DAC_SAMPLERATE);

	/* DAC configure with Timer 7 as trigger */
	dac->cr = DAC_TSEL2_TIM7 | DAC_TEN2 | DAC_DMAEN2 | DAC_EN2;

	dac_rt.flag = thinkos_flag_alloc();
	dac_rt.enabled = 0;
	dac_rt.src_ptr = (int16_t *)dac_silence;
	dac_rt.offs = 32768;
	dac_rt.gain = Q15(0.08);

	thinkos_thread_create_inf((void *)dac_task, (void *)NULL,
				  &dac_thread_inf);
}

void dac_start(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	dac->cr |= DAC_EN2;
}

void dac_stop(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	dac->cr &= ~DAC_EN2;
}

void dac_gain_set(int32_t gain)
{
	dac_rt.gain = gain;
}

#define MP3_BUF_LEN 8192
#define MP3_HDR_LEN 512

struct mp3_stream {
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	unsigned char mad_buf[MAD_BUFFER_MDLEN];
	uint32_t len; 
	uint32_t pos;
	uint32_t rem;
};

/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
void print_frame_info(FILE * fp, struct mad_header * hdr)
{
	const char*layer, *mode, *emphasis;

	/* Convert the layer number to it's printed representation. */
	switch (hdr->layer) {
	case MAD_LAYER_I:
		layer = "I";
		break;
	case MAD_LAYER_II:
		layer = "II";
		break;
	case MAD_LAYER_III:
		layer = "III";
		break;
	default:
		layer = "(unexpected layer value)";
		break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(hdr->mode) {
	case MAD_MODE_SINGLE_CHANNEL:
		mode = "single channel";
		break;
	case MAD_MODE_DUAL_CHANNEL:
		mode = "dual channel";
		break;
	case MAD_MODE_JOINT_STEREO:
		mode = "joint (MS/intensity) stereo";
		break;
	case MAD_MODE_STEREO:
		mode = "normal LR stereo";
		break;
	default:
		mode = "(unexpected mode value)";
		break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
	switch (hdr->emphasis) {
	case MAD_EMPHASIS_NONE:
		emphasis = "no";
		break;
	case MAD_EMPHASIS_50_15_US:
		emphasis = "50/15 us";
		break;
	case MAD_EMPHASIS_CCITT_J_17:
		emphasis = "CCITT J.17";
		break;
	case MAD_EMPHASIS_RESERVED:
		emphasis = "reserved(!)";
		break;
	default:
		emphasis = "(unexpected emphasis value)";
		break;
	}

	fprintf(fp,"%lu kb/s audio MPEG layer %s stream %s CRC, "
			"%s with %s emphasis at %d Hz sample rate\n",
			hdr->bitrate, layer,
			hdr->flags & MAD_FLAG_PROTECTION ? "with" : "without",
			mode, emphasis, hdr->samplerate);
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline signed int __scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));


	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}	

static inline signed int scale(mad_fixed_t sample, mad_fixed_t gain)
{
	sample = mad_f_mul(sample, gain);

	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));


	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}	



/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
static inline int16_t MadFixedToSshort(mad_fixed_t val)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFF FFFFFFFF FFFFFFFF FFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/* Clipping */
	if(val >= MAD_F_ONE)
		return SHRT_MAX;
	if(val <=- MAD_F_ONE)
		return -SHRT_MAX;

	/* Conversion. */
	val = val >> (MAD_F_FRACBITS - 15);

	return (int16_t)val;
}




int mp3_open(struct mp3_stream * mp3, const uint8_t * data, int size)
{
	struct mad_stream * stream;
	struct mad_frame * frame;
	struct mad_synth * synth;

	if (mp3 == NULL)
		return -1;

	stream = &mp3->stream;;
	frame = &mp3->frame;
	synth = &mp3->synth;

	/* First the structures used by libmad must be initialized. */
	mad_stream_init(stream);
    stream->main_data = (void *)&mp3->mad_buf[0];
	mad_frame_init(frame);
//    frame->overlap = mp3->overlap;

//	memset(*frame->overlap, 0, 2 * 32 * 18 * sizeof(mad_fixed_t));

	mad_synth_init(synth);
	mad_stream_buffer(stream, data, size);

	for(;;) {
		if (mad_frame_decode(frame, stream)) {
			if (MAD_RECOVERABLE(stream->error)) {
				/* Do not print a message if the error is a loss of
				 * synchronization and this loss is due to the end of
				 * stream guard bytes. (See the comments marked {3}
				 * supra for more informations about guard bytes.)
				 */
				if (stream->error != MAD_ERROR_LOSTSYNC) {
//					fprintf(stderr,"recoverable frame level error (%s)\n", 
//							mad_stream_errorstr(stream));
				}
			} else if (stream->error != MAD_ERROR_BUFLEN) {
//				fprintf(stderr,"unrecoverable frame level error (%s).\n",
//						mad_stream_errorstr(stream));
				mad_synth_finish(synth);
				mad_frame_finish(frame);
				mad_stream_finish(stream);
				return -2;
			}
		} else {
			break;
		}
	}

	/* The characteristics of the stream's first frame is printed
	 * on stderr. The first frame is representative of the entire
	 * stream.
	 */
//	print_frame_info(stdout, &mp3->frame.header);

	switch (mp3->frame.header.mode) {
	case MAD_MODE_DUAL_CHANNEL:
	case MAD_MODE_JOINT_STEREO:
	case MAD_MODE_STEREO:
		break;
	case MAD_MODE_SINGLE_CHANNEL:
	default:
		break;
	}

	return 0;
}

int mp3_play(struct mp3_stream * mp3)
{
	int16_t buf[DAC_FRAME_SIZE];
	unsigned long frm_cnt = 0;
	struct mad_stream * stream;
	struct mad_frame * frame;
	struct mad_synth * synth;
	unsigned int rem;

	int ret;

	if (mp3 == NULL)
		return -1;

	stream = &mp3->stream;;
	frame = &mp3->frame;
	synth = &mp3->synth;

	if (stream->buffer == NULL)
		return -1;

	thinkos_flag_take(dac_rt.flag);
	thinkos_flag_take(dac_rt.flag);

	rem = 0;

	do {
		mad_fixed_t const * left_ch;
		struct mad_pcm * pcm;
		unsigned int len;
		unsigned int pos;
		unsigned int i;
		unsigned int n;

		frm_cnt++;

		/* Once decoded the frame is synthesized to PCM samples. No errors
		 * are reported by mad_synth_frame();
		 */
		mad_synth_frame(synth, frame);

		/* Synthesized samples must be converted from libmad's fixed
		 * point number to the consumer format. Here we use unsigned
		 * 16 bit big endian integers on two channels. Integer samples
		 * are temporarily stored in a buffer that is flushed when
		 * full.
		 */

		pcm = &synth->pcm;
		left_ch = pcm->samples[0];
		len = pcm->length;
		/* fill in the first buffer */
		n = MIN(DAC_FRAME_SIZE - rem, len);
		pos = 0;
		for (i = rem; i < n; ++i)
			buf[i] = __scale(left_ch[pos++]);

		rem += n;
		len -= n;

		while (rem == DAC_FRAME_SIZE) {
			thinkos_flag_take(dac_rt.flag);
			dac_rt.src_ptr = buf;

			/* fill in the next buffer */
			rem = MIN(len, DAC_FRAME_SIZE);
			for (i = 0; i < rem; ++i)
				buf[i] = __scale(left_ch[pos++]);
			len -= rem;
		}

		/* decode next frame */
		ret = mad_frame_decode(frame, stream);

	} while (ret == 0);

//	thinkos_flag_take(dac_rt.flag);
//	dac_rt.src_ptr = buf;
//	for (i = rem; i < DAC_FRAME_SIZE; ++i)
//		buf[i] = 0;

	thinkos_flag_take(dac_rt.flag);
	dac_rt.src_ptr = (int16_t *)dac_silence;

	return ret;
}


int mp3_stop(struct mp3_stream * mp3)
{
	if (mp3 == NULL)
		return -1;

	/* Mad is no longer used, the structures that were initialized must
	 * now be cleared.
	 */
	mad_synth_finish(&mp3->synth);
	mad_frame_finish(&mp3->frame);
	mad_stream_finish(&mp3->stream);

	return 0;	
}

#ifndef MP3_STREAM_POLL_MAX
#define MP3_STREAM_POLL_MAX 1
#endif

struct mp3_stream mp3_instance[MP3_STREAM_POLL_MAX];

int dac_mp3_play(const uint8_t data[], unsigned int len)
{
	struct mp3_stream * mp3 = &mp3_instance[0];

	if (mp3_open(mp3, data, len) == 0) {
		mp3_play(mp3);
		mp3_stop(mp3);
		return -1;
	}

	return 0;
}

int dac_mp3_decode(unsigned int id, const uint8_t data[], unsigned int len)
{
	struct mp3_stream * mp3;

	if (id >= MP3_STREAM_POLL_MAX) 
		return -1;

	mp3 = &mp3_instance[id];

	if (mp3_open(mp3, data, len) == 0) {
		mp3_play(mp3);
		mp3_stop(mp3);
		return -1;
	}

	return 0;
}

