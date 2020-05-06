/* fault_test.c
 * ------------
 */

#include <fixpt.h>
#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>

#include "board.h"

void stdio_init(void)
{
	FILE *f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

/*--------------------------------------------------------------------------
  Fault generators
  --------------------------------------------------------------------------*/

void read_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		x = *ptr;
		(void)x;
		ptr += 0x10000000 / (2 << 4);
	}
}

void write_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x = 0;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		*ptr = x;
		ptr += 0x10000000 / (2 << 4);
	}
}

void stack_fault(uint32_t * ptr)
{
	uint32_t x = 0x20000800;
	int i;

	for (i = 0; i < 8192; ++i) {
		ptr[i] = x;
	}
}

/*--------------------------------------------------------------------------
  Interrupt handlers  
  --------------------------------------------------------------------------*/

unsigned int irq_count = 0 ;

void stm32f_tim2_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	/* Clear timer interrupt flags */
	tim->sr = 0;

	irq_count++;

	if (irq_count == 30)
		read_fault();
}

void stm32f_tim3_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM3;
	int i;

	/* Clear timer interrupt flags */
	tim->sr = 0;

	for (;;) {
		printf("3");
		for (i = 0; i < 10000000; ++i)
			__NOP();
	}
}

void stm32f_tim4_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM4;
	int i;

	/* Clear timer interrupt flags */
	tim->sr = 0;

	for (;;) {
		printf("4");
		for (i = 0; i < 4000000; ++i)
			__NOP();
	}
}

void stm32f_tim5_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM5;
	int i;

	/* Clear timer interrupt flags */
	tim->sr = 0;

	for (;;) {
		printf("5");
		for (i = 0; i < 4000000; ++i)
			__NOP();
	}
}

/*--------------------------------------------------------------------------
  Timers initialization
  --------------------------------------------------------------------------*/
void stm32_tim_init(struct stm32f_tim * tim, uint32_t period_ms)
{
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = (stm32f_tim1_hz / 1000) * period_ms;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = n - 1;
	tim->cr1 = TIM_CEN; /* Enable counter */
	tim->egr = TIM_UG;
	while (tim->sr == 0);
	tim->sr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
}

void timer2_init(uint32_t period_ms)
{
	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM2, period_ms);
	/* configure interrupts */
	cm3_irq_pri_set(STM32F_IRQ_TIM2, IRQ_PRIORITY_VERY_HIGH);
	/* enable interrupts */
	cm3_irq_enable(STM32F_IRQ_TIM2);
}

void timer3_init(uint32_t period_ms)
{
	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM3);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM3, period_ms);
	/* configure interrupts */
//	cm3_irq_pri_set(STM32F_IRQ_TIM3, IRQ_PRIORITY_HIGH);
	/* enable interrupts */
//	cm3_irq_enable(STM32F_IRQ_TIM3);
}

void timer4_init(uint32_t period_ms)
{
	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM4);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM4, period_ms);
	/* configure interrupts */
//	cm3_irq_pri_set(STM32F_IRQ_TIM4, IRQ_PRIORITY_REGULAR);
	/* enable interrupts */
//	cm3_irq_enable(STM32F_IRQ_TIM4);
}

void timer5_init(uint32_t period_ms)
{
	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM5);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM5, period_ms);
	/* configure interrupts */
//	cm3_irq_pri_set(STM32F_IRQ_TIM5, IRQ_PRIORITY_LOW);
	/* enable interrupts */
//	cm3_irq_enable(STM32F_IRQ_TIM5);
}

volatile uint32_t heap[128];


int test1_task(void * arg)
{
	uint32_t * x = (uint32_t *)heap;
	int i;

	for (i = 0; i < 32; ++i) {
		x[i] = 4 * i;
	}
	thinkos_sleep(200);

	for (i = 0; i < 32; ++i) {
		x[i] = 2 * i;
	}
	thinkos_sleep(200);

	return 0;
}

uint32_t test1_stack[1024] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf test1_thread_init = {
	.stack_ptr = test1_stack,
	.stack_size = sizeof(test1_stack),
	.priority = 16,
	.thread_id = 16,
	.paused = false,
	.tag = "TASK1"
};

/* ---------------------------------------------------------------------------
 * Application main function
 * ---------------------------------------------------------------------------
 */

int main(int argc, char **argv)
{
//	unsigned int i;

	/* load udelay calibration factor from OS */
	thinkos_udelay_factor(&udelay_factor);

	/* Initializes stdio so we can use printf and such. */
	stdio_init();

	printf("1. wait..\n");
	thinkos_sleep(1000);
	printf("2. new task..\n");

//	timer5_init(200);
//	timer4_init(400);
//	timer3_init(600);
//	timer2_init(100);

//	for (i = 10; i > 0; --i) {
//		printf(".");
//		thinkos_sleep(500);
//	}

//	printf(" ^^^\r\n");
/*
	asm("movs r0, #1\n"
		"1:\n"
		"cmp r0, #0\n"
		"bne 1b\n"); 
*/
	/* Start the main application thread */
	thinkos_thread_create_inf((int (*)(void *))test1_task, (void *)stdout,
							&test1_thread_init);

	printf("3. switch..\n");
	thinkos_sleep(100);

	printf("4. destroy..\n");
	stack_fault(test1_stack);

	printf("5. wait..\n");
	thinkos_sleep(500);

	printf("6. read..\n");
	read_fault();

	return 0;
}




