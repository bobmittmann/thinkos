/* fault_test.c
 * ------------
 */

#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <sys/tty.h>
#include <thinkos.h>
#include <stdio.h>

#include "board.h"
#include "serial_drv.h"

uint32_t stack1[256] __attribute__ ((aligned(8)));

const struct thinkos_thread_inf test1_thread_init = {
	.stack_ptr = stack1,
	.stack_size = sizeof(stack1),
	.priority = 4,
	.thread_id = 4,
	.paused = false,
	.tag = "TASK1"
};

/*--------------------------------------------------------------------------
  Interrupt handlers  
  --------------------------------------------------------------------------*/

volatile uint32_t irq_count = 200 ;

void stm32f_tim2_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	/* Clear timer interrupt flags */
	tim->sr = 0;

	irq_count++;
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

int test1_task(int mutex)
{
	struct stm32f_tim * tim = STM32F_TIM2;
	uint32_t cnt = 0;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* Initialize timer */
	stm32_tim_init(tim, 500000);
	
	stm32_tim_en(tim, true);

	for (;;) {
//		thinkos_sleep(500);

		if (thinkos_irq_wait(STM32F_IRQ_TIM2) == THINKOS_OK) 
		{
			irq_count++;
			printf("tick()\r\n");
			cnt++;
		} else {
			printf("err\r\n");
		}
		/* Clear timer interrupt flags */
		tim->sr = 0;
	}

	return 0;
}

void stdio_init(void)
{
	struct tty_dev *tty;
	FILE *f_raw;
	FILE *f_tty;

	f_raw = console_fopen();
	tty = tty_attach(f_raw);
	f_tty = tty_fopen(tty);
	(void)f_tty;

	/* initialize STDIO */
	stdout = f_tty;
	stdin = f_tty;
	stderr = f_tty;
}

void print_menu(void)
{
	printf("Options:\r\n");
	printf("\t[w] - write\r\n");
	printf("\t[q] - quit\r\n");
	printf("\t[x] - dump memory\r\n");
	printf("[App] # ");
}

int main_task(void)
{
	int c;
	struct serial_drv * ser;

	ser = serial2_init();

	thinkos_sleep(1000);
	printf("\r\n\r\n--------------------------------------------\r\n");
	print_menu();
	do {
		char s[10];
		int n;
		int i;

		switch (c = fgetc(stdin)) {
		case 'w':
			do {

				printf("\r\n ? ");
				fgets(s, sizeof(s), stdin);
			} while ((n = strtol(s, NULL, 10)) == 0);
			for (i = 0; i < n; ++i) {
				printf("\n%5d", i);
				__serial_write(ser, "Hello", 5);
			}
			break;
		case '\r':
			print_menu();
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
	(void)mutex;

	/* Initializes stdio so we can use printf and such. */
	stdio_init();

	mutex = THINKOS_MUTEX_DESC(0);

///	while (!thinkos_console_is_connected()) {
///	}

	/* Start the test application thread */
	thinkos_thread_create_inf(C_TASK(test1_task), 
								  C_ARG(mutex),
								  &test1_thread_init);

	return main_task();
}




