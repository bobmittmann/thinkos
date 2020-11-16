#include <thinkos.h>

void uart0_isr(void);
static void nvic_irq0_isr(void) __attribute__((weakref, alias("uart0_isr")));

void uart2_isr(void);
void nvic_irq2_isr(void) __attribute__((alias("uart2_isr")));

void uart2_isr(void)
{
	thinkos_sleep(13566600);
}

