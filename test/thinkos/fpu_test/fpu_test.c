/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
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
 * @file fpu_test.c
 * @brief application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <arch/cortex-m3.h>
#include <sys/dcclog.h>
#include <stdlib.h>
#include <thinkos.h>
#include <sys/stm32f.h>

#define IO_LED1A           STM32_GPIOE, 9
#define IO_LED1B           STM32_GPIOE, 10
#define IO_LED1C           STM32_GPIOE, 11
#define IO_LED1D           STM32_GPIOE, 12

static inline void __led_on(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_set(__gpio, __pin);
}

static inline void __led_off(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_clr(__gpio, __pin);
}

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

	/* - LEDs ---------------------------------------------------------*/
	stm32_gpio_clr(IO_LED1A);
	stm32_gpio_mode(IO_LED1A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1B);
	stm32_gpio_mode(IO_LED1B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1C);
	stm32_gpio_mode(IO_LED1C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1D);
	stm32_gpio_mode(IO_LED1D, OUTPUT, PUSH_PULL | SPEED_LOW);
}

struct iir4 {
    float a0;
    float a1;
    float a2;
    float a3;
    float a4;
    float a5;
    float a6;
    float a7;
    float a8;
    float b0;
    float b1;
    float b2;
    float b3;
    float b4;
    float b5;
    float b6;
    float b7;
    float b8;
    float y[5]; /* output samples */
    float x[5]; /* input samples */
};

struct iir4_coef {
    float a[9];
    float b[9];
};

void iir4_init(struct iir4 * filt, const struct iir4_coef * coef) 
{
	filt->a0 = coef->a[0];
	filt->a1 = coef->a[1];
	filt->a2 = coef->a[2];
	filt->a3 = coef->a[3];
	filt->a4 = coef->a[4];
	filt->a5 = coef->a[5];
	filt->a6 = coef->a[6];
	filt->a7 = coef->a[7];
	filt->a8 = coef->a[8];
	filt->b0 = coef->b[0];
	filt->b1 = coef->b[1];
	filt->b2 = coef->b[2];
	filt->b3 = coef->b[3];
	filt->b4 = coef->b[4];
	filt->b5 = coef->b[5];
	filt->b6 = coef->b[6];
	filt->b7 = coef->b[7];
	filt->b8 = coef->b[8];
}

static inline float iir4_filt(struct iir4 * filt, float x) 
{
    int i;
	float y;

    /* shift the old samples */
    for (i = 4; i > 0; --i) {
       filt->x[i] = filt->x[i - 1];
       filt->y[i] = filt->y[i - 1];
    }

    /* Calculate the new output */
    filt->x[0] = x;
    y = filt->a0 * filt->x[0] + 
		filt->a1 * filt->x[1] - filt->b1 * filt->y[1] +
		filt->a2 * filt->x[2] - filt->b2 * filt->y[2] +
		filt->a3 * filt->x[3] - filt->b3 * filt->y[3] +
		filt->a4 * filt->x[4] - filt->b4 * filt->y[4] +
		filt->a5 * filt->x[5] - filt->b5 * filt->y[5] +
		filt->a6 * filt->x[6] - filt->b6 * filt->y[6] +
		filt->a7 * filt->x[7] - filt->b7 * filt->y[7] +
		filt->a8 * filt->x[8] - filt->b8 * filt->y[8];
    
    filt->y[0] = y;

    return y;
}

/*
Filter type: High Pass
Filter model: Butterworth
Filter order: 4
Sampling Frequency: 44 KHz
Cut Frequency: 0.200000 KHz
Coefficents Quantization: float

Z domain Zeros
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000

Z domain Poles
z = 0.973953 + j -0.010622
z = 0.973953 + j 0.010622
z = 0.988817 + j -0.026042
z = 0.988817 + j 0.026042
***************************************************************/
const struct iir4_coef iir_hp200 = {
    .a = {
        0.44453492152938180000,
        -3.87413968611752720000,
        5.78120952917629080000,
        -3.85413968611752720000,
        0.44453492152938180000,
        -3.87413968611752720000,
        5.78120952917629080000,
        -3.85413968611752720000,
        0.86353492152938180000
    },

    .b  = {
        1.00000000000000000000,
        -3.92553975070105480000,
        5.77937889715614440000,
        -3.78207900848596790000,
        1.00000000000000000000,
        -3.92553975070105480000,
        5.77937889715614440000,
        -3.78207900848596790000,
        0.92824049740994874000
    }
};

volatile uint32_t u1[32];
volatile uint32_t u2[32];
volatile float in[64];
volatile float out[64];

void init(void)
{
	int i;

	for (i = 0; i < 64; ++ i)
		in[i] = i * i;

	for (i = 0; i <= 9; ++ i)
		u1[i] = i + (i << 16);
	for (i = 10; i <= 19; ++ i)
		u1[i] = (0x10 - 10 + i) + ((0x10 - 10 + i) << 16);
	for (i = 20; i <= 29; ++ i)
		u1[i] = (0x20 - 20 + i) + ((0x20 - 20 + i) << 16);
	for (i = 30; i <= 31; ++ i)
		u1[i] = (0x30 - 30 + i) + ((0x30 - 30 + i) << 16);
	
	for (i = 0; i < 32; ++ i) {
		u2[i] = u1[i] + (2 << 28);
		u1[i] = u1[i] + (1 << 28);
	}
}

int fpu_task(void * arg)
{
	struct iir4 iir;
	int j;

	iir4_init(&iir, &iir_hp200);

	DCC_LOG(LOG_TRACE, "Filtering...");

	for (j = 0; j < 2; ++j) {
		float x[64];
		float y[64];
		int i;

		DCC_LOG1(LOG_TRACE, "Filter start. SP=%08x", cm3_sp_get());

		for (i = 0; i < 64; ++i)
			x[i] = in[i];

		for (i = 0; i < 64; ++i)
			y[i] = iir4_filt(&iir, x[i]); 

		for (i = 0; i < 64; ++i)
			out[i] = y[i];
	
		DCC_LOG(LOG_TRACE, "Done...");

		asm volatile ("vldmia %0, {s0-s31}\n" : : "r" (u1));
		thinkos_yield();

		thinkos_sleep(1);

		for(;;);
	}

	return 0;
}

int fpu_task1(void * arg)
{
	int j;

	DCC_LOG(LOG_TRACE, "...");

	asm volatile ("vldmia %0, {s0-s31}\n" : : "r" (u1));
	for (j = 0; j < 200000000; ++j) {
		thinkos_sleep(200);
		__led_on(IO_LED1A);
		thinkos_sleep(200);
		__led_off(IO_LED1A);
	}

	return 0;
}

int fpu_task2(void * arg)
{
	int j;

	DCC_LOG(LOG_TRACE, "...");

	asm volatile ("vldmia %0, {s0-s31}\n" : : "r" (u2));
	for (j = 0; j < 200000000; ++j) {
		thinkos_sleep(210);
		__led_on(IO_LED1C);
		thinkos_sleep(210);
		__led_off(IO_LED1C);
	}

	return 0;
}
uint32_t fpu_stack1[256];
uint32_t fpu_stack2[256];

int main(int argc, char ** argv)
{
	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	thinkos_init(THINKOS_OPT_ID(3));

	io_init();
	init();

	thinkos_thread_create(fpu_task1, NULL, fpu_stack1, 
						  1024 | THINKOS_OPT_ID(1));
	thinkos_thread_create(fpu_task2, NULL, fpu_stack2,
						  1024 | THINKOS_OPT_ID(2));

	DCC_LOG(LOG_TRACE, "Sleep...");
	thinkos_sleep(100);

	DCC_LOG(LOG_TRACE, "Wakeup...");
	for(;;) {
	};

	return 0;
}


