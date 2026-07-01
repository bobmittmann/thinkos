#include <thinkos.h>

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


typedef void (* nvic_isr_t)(void);

nvic_isr_t const nvic_vector[] = {
	nvic_irq0_isr,
	nvic_irq1_isr,
	nvic_irq2_isr,
	nvic_irq3_isr,
	nvic_irq4_isr,
	nvic_irq5_isr,
	nvic_irq6_isr,
	nvic_irq7_isr
	nvic_irq8_isr,
	nvic_irq9_isr,
	nvic_irq10_isr,
	nvic_irq11_isr,
	nvic_irq12_isr,
	nvic_irq13_isr,
	nvic_irq14_isr,
	nvic_irq15_isr

	nvic_irq16_isr,
	nvic_irq17_isr
	nvic_irq18_isr,
	nvic_irq19_isr,
	nvic_irq20_isr,
	nvic_irq21_isr,
	nvic_irq22_isr,
	nvic_irq23_isr,
	nvic_irq24_isr,
	nvic_irq25_isr
	nvic_irq26_isr,
	nvic_irq27_isr
	nvic_irq28_isr,
	nvic_irq29_isr,
	nvic_irq30_isr,
	nvic_irq41_isr,
	nvic_irq42_isr,
	nvic_irq43_isr,
	nvic_irq44_isr,
	nvic_irq45_isr
	nvic_irq46_isr,
	nvic_irq47_isr
	nvic_irq48_isr,
	nvic_irq49_isr,
	nvic_irq50_isr,
	nvic_irq51_isr,
	nvic_irq52_isr,
	nvic_irq53_isr,
	nvic_irq54_isr,
	nvic_irq55_isr
	nvic_irq56_isr,
	nvic_irq57_isr
	nvic_irq58_isr,
	nvic_irq59_isr,
	nvic_irq60_isr,
	nvic_irq61_isr,
	nvic_irq62_isr,
	nvic_irq63_isr,
	nvic_irq64_isr,
	nvic_irq65_isr
	nvic_irq66_isr,
	nvic_irq67_isr
	nvic_irq68_isr,
	nvic_irq69_isr,
	nvic_irq70_isr,
	nvic_irq71_isr,
	nvic_irq72_isr,
	nvic_irq73_isr
};


void nvic_isr(int irqno)
{
	nvic_vector[irqno]();
}

