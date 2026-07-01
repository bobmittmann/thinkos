#include <thinkos.h>

/* default */
void __nvic_irq_isr(void)
{
	thinkos_sleep(100);
}

void nvic_irq0_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq1_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq2_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq3_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq4_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq5_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq6_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));
void nvic_irq7_isr(void) __attribute__((weak, alias("__nvic_irq_isr")));










void nvic_call(void)
{
	__nvic_irq_isr();
}


