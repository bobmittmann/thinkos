/* 
 * File:	 usb-test.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "board.h"
#include "version.h"

#include <sys/delay.h>
#include <sys/dcclog.h>

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

	/* - Amplifier power --------------------------------------------------*/
	stm32_gpio_clr(IO_TRIG);
	stm32_gpio_mode(IO_TRIG, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR1);
	stm32_gpio_mode(IO_PWR1, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR2);
	stm32_gpio_mode(IO_PWR2, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR3);
	stm32_gpio_mode(IO_PWR3, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR4);
	stm32_gpio_mode(IO_PWR4, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* Pulse trigger to force power down */
	stm32_gpio_set(IO_TRIG);
	udelay(10);
	stm32_gpio_clr(IO_TRIG);

	/* - USB ----------------------------------------------------*/
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);

	/* - LEDs ---------------------------------------------------------*/
	stm32_gpio_clr(IO_LED1A);
	stm32_gpio_mode(IO_LED1A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1B);
	stm32_gpio_mode(IO_LED1B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_set(IO_LED1C);
	stm32_gpio_mode(IO_LED1C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1D);
	stm32_gpio_mode(IO_LED1D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED2A);
	stm32_gpio_mode(IO_LED2A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2B);
	stm32_gpio_mode(IO_LED2B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_set(IO_LED2C);
	stm32_gpio_mode(IO_LED2C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2D);
	stm32_gpio_mode(IO_LED2D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED3A);
	stm32_gpio_mode(IO_LED3A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3B);
	stm32_gpio_mode(IO_LED3B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_set(IO_LED3C);
	stm32_gpio_mode(IO_LED3C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3D);
	stm32_gpio_mode(IO_LED3D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED4A);
	stm32_gpio_mode(IO_LED4A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4B);
	stm32_gpio_mode(IO_LED4B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_set(IO_LED4C);
	stm32_gpio_mode(IO_LED4C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4D);
	stm32_gpio_mode(IO_LED4D, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* - Relays ---------------------------------------------------------*/
	stm32_gpio_mode(IO_BKP1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP1);

	stm32_gpio_mode(IO_BKP2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP2);

	stm32_gpio_mode(IO_BKP3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP3);

	stm32_gpio_mode(IO_BKP4, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP4);

	stm32_gpio_mode(IO_LINB1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB1);

	stm32_gpio_mode(IO_LINB2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB2);

	stm32_gpio_mode(IO_LINB3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB3);

	stm32_gpio_mode(IO_LINB4, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB4);

	/* - Fault ---------------------------------------------------------*/
	stm32_gpio_mode(IO_FAULT1, INPUT, SPEED_LOW);
#ifndef DEBUG
//	stm32_gpio_mode(IO_FAULT2, INPUT, SPEED_LOW);
#endif
	stm32_gpio_mode(IO_FAULT3, INPUT, SPEED_LOW);
	stm32_gpio_mode(IO_FAULT4, INPUT, SPEED_LOW);

	/* - I2S ---------------------------------------------------------*/
	stm32_gpio_af(IO_I2S2_SD, GPIO_AF5);
	stm32_gpio_af(IO_I2S2_SCLK, GPIO_AF5);
	stm32_gpio_af(IO_I2S2_FS, GPIO_AF5);
	stm32_gpio_af(IO_I2S2_MCLK, GPIO_AF5);
	stm32_gpio_mode(IO_I2S2_SD, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S2_SCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S2_FS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S2_MCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_af(IO_I2S3_SD, GPIO_AF6);
	stm32_gpio_af(IO_I2S3_SCLK, GPIO_AF6);
	stm32_gpio_af(IO_I2S3_FS, GPIO_AF6);
	stm32_gpio_af(IO_I2S3_MCLK, GPIO_AF6);
	stm32_gpio_mode(IO_I2S3_SD, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S3_SCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S3_FS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S3_MCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
}

bool board_init(void)
{
	DCC_LOG1(LOG_TRACE, "clk[AHB]=%d", stm32f_ahb_hz);
	DCC_LOG1(LOG_TRACE, "clk[APB1]=%d", stm32f_apb1_hz);
	DCC_LOG1(LOG_TRACE, "clk[TIM1]=%d", stm32f_tim1_hz);
	DCC_LOG1(LOG_TRACE, "clk[APB2]=%d", stm32f_apb2_hz);
	DCC_LOG1(LOG_TRACE, "clk[TIM2]=%d", stm32f_tim2_hz);

	DCC_LOG(LOG_MSG, "io_init()");
	io_init();
#if 0
	DCC_LOG(LOG_TRACE, "leds_on()");
	__led_on(IO_LED1A);
	__led_on(IO_LED2A);
	__led_on(IO_LED3A);
	__led_on(IO_LED4A);

	udelay(250000);

	__led_off(IO_LED1A);
	__led_off(IO_LED2A);
	__led_off(IO_LED3A);
	__led_off(IO_LED4A);

	__led_on(IO_LED1B);
	__led_on(IO_LED2B);
	__led_on(IO_LED3B);
	__led_on(IO_LED4B);

	udelay(250000);

	__led_off(IO_LED1B);
	__led_off(IO_LED2B);
	__led_off(IO_LED3B);
	__led_off(IO_LED4B);

	__led_on(IO_LED1C);
	__led_on(IO_LED2C);
	__led_on(IO_LED3C);
	__led_on(IO_LED4C);

	udelay(250000);

	__led_off(IO_LED1C);
	__led_off(IO_LED2C);
	__led_off(IO_LED3C);
	__led_off(IO_LED4C);

	__led_on(IO_LED1D);
	__led_on(IO_LED2D);
	__led_on(IO_LED3D);
	__led_on(IO_LED4D);

	udelay(250000);

	__led_off(IO_LED1D);
	__led_off(IO_LED2D);
	__led_off(IO_LED3D);
	__led_off(IO_LED4D);
#endif
	/* set the interrupt priority */
//	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

	return true;
}

void board_softreset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;

	DCC_LOG1(LOG_TRACE, "AHB1ENR=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "AHB2ENR=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "AHB3ENR=0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_TRACE, "APB1ENR=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_TRACE, "APB2ENR=0x%08x", rcc->apb2enr);

	/* Reset all peripherals except USB_OTG and GPIOA */
	rcc->ahb1rstr = ~(1 << RCC_GPIOA); 
	rcc->ahb2rstr = ~(1 << RCC_OTGFS);
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr = ~(0);
	rcc->apb2rstr = ~(0);


	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->ahb3rstr = 0;
	rcc->apb1rstr = 0;
	rcc->apb2rstr = 0;

	/* disable all peripherals clock sources except USB_OTG and GPIOA */
	rcc->ahb1enr = (1 << RCC_GPIOA); 
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* reinitialize IO's */
	io_init();

}

bool board_autoboot(uint32_t tick)
{
	switch (tick & 0x3) {
	case 0:
		__led_off(IO_LED4A);
		__led_on(IO_LED1A);
		break;
	case 1:
		__led_off(IO_LED1A);
		__led_on(IO_LED2A);
		break;
	case 2:
		__led_off(IO_LED2A);
		__led_on(IO_LED3A);
		break;
	case 3:
		__led_off(IO_LED3A);
		__led_on(IO_LED4A);
		break;
	}

	/* Time window autoboot */
	return (tick == 40) ? true : false;
}

