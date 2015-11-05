/* Copyright(C) 2015 Mircom Group */

#include <sys/stm32f.h>
#include "gpio.h"

/* GPIO pin description */ 
struct gpio {
	struct stm32_gpio * gpio;
	uint8_t pin;
	uint8_t mode;
	uint8_t opt;
};

/* ----------------------------------------------------------------------
 * Fire Alarm GPIO Map
 * ----------------------------------------------------------------------
 */

const struct gpio gpio_map[] = {
	[GPIO_LED1] =    { STM32_GPIOE, 12,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED9 */
	[GPIO_LED2] =    { STM32_GPIOE, 11,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED7 */
	[GPIO_LED3] =    { STM32_GPIOE, 10,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED5 */
	[GPIO_LED4] =    { STM32_GPIOE,  9,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED3 */
	[GPIO_LED5] =    { STM32_GPIOE,  8,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED4 */
	[GPIO_LED6] =    { STM32_GPIOE, 15,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED6 */
	[GPIO_LED7] =    { STM32_GPIOE, 14,
			OUTPUT, PUSH_PULL | SPEED_LOW },  /* LED8 */
	[GPIO_LED8] =    { STM32_GPIOE, 13,
			OUTPUT, PUSH_PULL | SPEED_LOW }, /* LED10 */
	[GPIO_SWITCH] =  { STM32_GPIOA,  0,
			 INPUT, SPEED_LOW },   /* User Switch */
	[GPIO_NAC_OUT] = { STM32_GPIOA,  7,
			OUTPUT, PUSH_PULL | SPEED_LOW },
	[GPIO_ZONE1] =   { STM32_GPIOA,  1,
			 INPUT, PULL_UP | SPEED_LOW },
	[GPIO_ZONE2] =   { STM32_GPIOA,  3,
			 INPUT, PULL_UP | SPEED_LOW },
	[GPIO_ZONE3] =   { STM32_GPIOA,  5,
			 INPUT, PULL_UP | SPEED_LOW },
	[GPIO_ZONE4] =   { STM32_GPIOA,  6,
			 INPUT, PULL_UP | SPEED_LOW },
	[GPIO_ADDRESS] = { STM32_GPIOC,  1,
			 INPUT, PULL_UP | SPEED_LOW }
};

#define GPIO_MAP_LEN (sizeof(gpio_map) / sizeof(struct gpio))

/* ----------------------------------------------------------------------
 * GPIO API
 * ----------------------------------------------------------------------
 */

void gpio_set(unsigned int __io)
{
	stm32_gpio_set(gpio_map[__io].gpio, gpio_map[__io].pin);
}

void gpio_clr(unsigned int __io)
{
	stm32_gpio_clr(gpio_map[__io].gpio, gpio_map[__io].pin);
}

unsigned int gpio_status(unsigned int __io)
{
	return stm32_gpio_stat(gpio_map[__io].gpio, gpio_map[__io].pin) ? 1 : 0;
}

void gpio_init(void)
{
	int i;

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

	for (i = 0; i < GPIO_MAP_LEN; ++i) {
		stm32_gpio_mode(gpio_map[i].gpio, gpio_map[i].pin,
				gpio_map[i].mode, gpio_map[i].opt);
	}
}
