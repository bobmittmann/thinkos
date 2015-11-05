/* ---------------------------------------------------------------------------
 * File: gpio.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __GPIO_H__
#define __GPIO_H__

enum gpio_pin {
	GPIO_LED1,
	GPIO_LED2,
	GPIO_LED3,
	GPIO_LED4,
	GPIO_LED5,
	GPIO_LED6,
	GPIO_LED7,
	GPIO_LED8,
	GPIO_SWITCH,
	GPIO_NAC_OUT,
	GPIO_ZONE1,
	GPIO_ZONE2,
	GPIO_ZONE3,
	GPIO_ZONE4,
	GPIO_ADDRESS
};

#ifdef __cplusplus
extern "C" {
#endif

/* Set an output pin (force to high-level_ */
void gpio_set(unsigned int __io);

/* Clear an output pin (force to low-level) */
void gpio_clr(unsigned int __io);

/* Return the status of an input pin (1 = high, 0 = low) */
unsigned int gpio_status(unsigned int __io);

/* Initialize GPIO pins */
void gpio_init(void);

#ifdef __cplusplus
}
#endif	

#endif /* __GPIO_H__ */

