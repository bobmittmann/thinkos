/* fault_test.c
 * ------------
 */

#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>

#include "board.h"

uint32_t stack[3][1024] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf main_thread_init = {
	.stack_ptr = stack[0],
	.stack_size = 4096,
	.priority = 2,
	.thread_id = 2,
	.paused = false,
	.tag = "MAIN"
};

const struct thinkos_thread_inf test1_thread_init = {
	.stack_ptr = stack[1],
	.stack_size = 4096,
	.priority = 8,
	.thread_id = 8,
	.paused = false,
	.tag = "TASK1"
};

const struct thinkos_thread_inf test2_thread_init = {
	.stack_ptr = stack[2],
	.stack_size = 4096,
	.priority = 4,
	.thread_id = 4,
	.paused = false,
	.tag = "TASK2"
};


void stdio_init(void)
{
	FILE *f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

void read_fault(void);
void write_fault(void);

void undefined_instruction_fault(void) 
{
	asm volatile ("udf 0\n"::);
}

/*--------------------------------------------------------------------------
  Interrupt handlers  
  --------------------------------------------------------------------------*/

volatile uint32_t irq_count = 0 ;

void stm32f_tim2_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	/* Clear timer interrupt flags */
	tim->sr = 0;

	irq_count++;
}

void stm32f_tim3_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM3;
	int i;

	/* Clear timer interrupt flags */
	tim->sr = 0;

	for (;;) {
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
		for (i = 0; i < 4000000; ++i)
			__NOP();
	}
}

/*--------------------------------------------------------------------------
  Timers initialization
  --------------------------------------------------------------------------*/
void stm32_tim_init(struct stm32f_tim * tim, uint32_t period_us)
{
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = (((uint64_t)stm32f_tim1_hz  * period_us) / 1000000);
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;
	/* Timer configuration */
	tim->cr1 = 0; /* Disable counter */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = n - 1;
	tim->egr = TIM_UG;
	while (tim->sr != 0)
		tim->sr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
}

void stm32_tim_en(struct stm32f_tim * tim, bool en)
{
	uint32_t cr1 = tim->cr1 & ~TIM_CEN;
	tim->cr1 = cr1 | (en ? TIM_CEN : 0); /* Enable counter */
}

/*--------------------------------------------------------------------------
  Fault generators
  --------------------------------------------------------------------------*/

void read_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0ffffff0);
	uint32_t x;

	x = *ptr;
	(void)x;
}

void write_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x20000000);
	uint32_t x = 0xaaaaaaaa;

	*ptr = x;
}

volatile uint32_t heap[2048];

int32_t stack_fault_on_service_entry(unsigned int size)
{
	int32_t buf[size/4];
	int32_t x = 0x00bafafa;

	buf[size/4 -1] = x;
	thinkos_yield();

	return buf[0];
}

void unaligned_stack_fault(void)
{
	int8_t *cp = (int8_t *)heap;
	int32_t *ptr = (int32_t *)++cp;
	int32_t x = 0x00bafafa;

	asm volatile ("adds %0, #1\n"
				  "str %1, [%0]\n": : "l"(ptr), "l"(x));

	thinkos_yield();
}

void instruction_fetch_fault(void)
{
	intptr_t ptr = (intptr_t)(&stack[0] - 1);

	asm volatile ("add %0, #1\n"
				  "mov pc, %0\n": : "l"(ptr));

	thinkos_yield();
}

void instruction_stack_fault(void)
{
	int32_t ret;

	asm volatile ("mov %0, lr\n"
				  "add %0, #1\n"
				  "mov lr, %0\n": "=l"(ret) :);

	thinkos_yield();
}


void divide_by_zero_fault(void)
{
	int32_t x = 0x00bafafa;
	int32_t y = 0;

	heap[0] = x / y;

	thinkos_yield();
}


void dummy_process(int32_t * buf, int len)
{
	heap[0] = buf[len - 1];	
}

void stack_fault_on_systick(unsigned int size)
{
	int32_t buf[size/4];

	dummy_process(buf, size/4);
	for (;;) {
	}	
}

int test1_task(int mutex)
{
	uint32_t cnt = 0;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* configure interrupts */
	if (thinkos_irq_register(STM32F_IRQ_TIM2, IRQ_PRIORITY_VERY_HIGH, 
						 stm32f_tim2_isr) == THINKOS_OK) {
		printf("thinkos_irq_register() ok\r\n");
	};
	/* enable interrupts */
	thinkos_irq_enable(STM32F_IRQ_TIM2);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM2, 200);
	
	stm32_tim_en(STM32F_TIM2, true);

	stack_fault_on_systick(8192);

	for (;;) {
		if (irq_count != cnt) {
			cnt = irq_count; 
		}
	}

	return 0;
}


void irq_read_fault(void)
{
	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* configure interrupts */
	thinkos_irq_register(STM32F_IRQ_TIM2, IRQ_PRIORITY_HIGH, read_fault);
	/* enable interrupts */
	thinkos_irq_enable(STM32F_IRQ_TIM2);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM2, 200);

	stm32_tim_en(STM32F_TIM2, true);
}

int test2_task(int mutex)
{
	/* wait for access */
	thinkos_mutex_lock(mutex);

	stack_fault_on_service_entry(4096);

	return 0;
}


int sem;

void tim2_sem_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	/* Clear timer interrupt flags */
	tim->sr = 0;

	thinkos_sem_post_i(sem); 
}

void sem_test(void)
{
	int i;

	sem = THINKOS_SEM_DESC(0); 
	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* configure interrupts */
	thinkos_irq_register(STM32F_IRQ_TIM2, IRQ_PRIORITY_HIGH, tim2_sem_isr);
	/* enable interrupts */
	thinkos_irq_enable(STM32F_IRQ_TIM2);
	/* Initialize timer */
	stm32_tim_init(STM32F_TIM2, 500000);

	stm32_tim_en(STM32F_TIM2, true);

	for(i = 0; i < 32; ++i) {
		thinkos_sem_wait(sem);
		printf("Tick %d\r\n", i);
	}
	stm32_tim_en(STM32F_TIM2, false);
}


void print_menu(void)
{
	printf("Options:\r\n");
	printf("\tr - memory management read fault\r\n");
	printf("\tw - memory management write fault\r\n");
	printf("\ts - memmgmt fault on service entry\r\n");
	printf("\tt - memmgmt fault on systick entry\r\n");
	printf("\tg - stack overflow detection \r\n");
	printf("\ti - memmgmt fault on IRQ entry\r\n");
	printf("\tu - undefined instruction fault\r\n");
	printf("\tl - unaligned access fault\r\n");
	printf("\tz - divide by zero fault\r\n");
	printf("\tf - instruction fetch fault\r\n");
	printf("\tk - instruction stack fault\r\n");
	printf("\tp - irq read fault\r\n");
}

int main_task(int mutex)
{
	int c;

	thinkos_mutex_lock(mutex);
	print_menu();
	do {
		switch (c = fgetc(stdin)) {
		case '\r':
			print_menu();
			break;
		case 'r':
			read_fault();
			break;
		case 'w':
			write_fault();
			break;
		case 's':
			stack_fault_on_service_entry(4096);
			break;
		case 't':
			stack_fault_on_systick(4096);
			break;
		case 'u':
			undefined_instruction_fault();
			break;
		case 'l':
			unaligned_stack_fault();
			break;
		case 'z':
			divide_by_zero_fault();
			break;
		case 'f':
			instruction_fetch_fault();
			break;
		case 'k':
			instruction_stack_fault();
			break;
		case 'p':
			irq_read_fault();
			break;
		case '1':
			sem_test();
			break;
		case 'g':
			/* Start the test application thread */
			thinkos_thread_create_inf(C_TASK(test2_task), 
									  C_ARG(mutex),
									  &test2_thread_init);
			thinkos_mutex_unlock(mutex);

			break;
		case 'i':
			/* Start the test application thread */
			thinkos_thread_create_inf(C_TASK(test1_task), 
									  C_ARG(mutex),
									  &test1_thread_init);
		break;
		}
	} while (c != 'q');

	return 0;
}

/* ---------------------------------------------------------------------------
 * Application main function
 * ---------------------------------------------------------------------------
 */

int main(int argc, char **argv)
{
	int mutex;

	/* load udelay calibration factor from OS */
	thinkos_udelay_factor(&udelay_factor);

	/* Initializes stdio so we can use printf and such. */
	stdio_init();

	mutex = THINKOS_MUTEX_DESC(0);

	while (!thinkos_console_is_connected()) {
		thinkos_sleep(100);
	}

	/* Start the main application thread */
	thinkos_thread_create_inf(C_TASK(main_task), 
							  C_ARG(mutex),
							  &main_thread_init);

	/* cancel the current thread */
	thinkos_thread_abort(0);

	return 0;
}




