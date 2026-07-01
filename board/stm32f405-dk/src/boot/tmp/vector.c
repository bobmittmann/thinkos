#include <thinkos.h>

typedef void (* nvic_isr_t)(void);

void nvic_irq0_isr(void);
void nvic_irq1_isr(void);
void nvic_irq2_isr(void);
void nvic_irq3_isr(void);
void nvic_irq4_isr(void);
void nvic_irq5_isr(void);
void nvic_irq6_isr(void);
void nvic_irq7_isr(void);

static void __irq0_isr(void) __attribute__((weakref, alias("nvic_irq0_isr")));
static void __irq1_isr(void) __attribute__((weakref, alias("nvic_irq1_isr")));
static void __irq2_isr(void) __attribute__((weakref, alias("nvic_irq2_isr")));
static void __irq3_isr(void) __attribute__((weakref, alias("nvic_irq3_isr")));
static void __irq4_isr(void) __attribute__((weakref, alias("nvic_irq4_isr")));
static void __irq5_isr(void) __attribute__((weakref, alias("nvic_irq5_isr")));
static void __irq6_isr(void) __attribute__((weakref, alias("nvic_irq6_isr")));
static void __irq7_isr(void) __attribute__((weakref, alias("nvic_irq7_isr")));

nvic_isr_t const nvic_vector[] = {
	__irq0_isr,
	__irq1_isr,
	__irq2_isr,
	__irq3_isr,
	__irq4_isr,
	__irq5_isr,
	__irq6_isr,
	__irq7_isr
};

void nvic_isr(int irqno)
{
	nvic_vector[irqno]();
}


