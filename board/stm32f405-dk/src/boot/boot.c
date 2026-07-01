<<<<<<< HEAD
/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file boot.c
 * @brief YARD-ICE bootloader main
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>

#include <sys/dcclog.h>
#include <sys/console.h>
=======
/** 
 * @file boot.c
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Bootloader descriptor and hardware initialization
 * 
 */

#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <thinkos.h>

#include <trace.h>
#include <vt100.h>
#include <sys/delay.h>
#include <sys/dcclog.h>
>>>>>>> krnsvc

#include "board.h"
#include "version.h"

<<<<<<< HEAD
#define MONITOR_AUTOBOOT 1
#define MONITOR_SHELL 2

void monitor_task(const struct dbgmon_comm * comm, void * param);

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);

	/* - USB OTG FS -------------------------------------------------------- */
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	/* XXX: something wrong with this configuration!!! Set as input instead ...
	 */
	//stm32_gpio_mode(OTG_FS_VBUS, INPUT, SPEED_LOW);
	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);

	/* - LEDs -------------------------------------------------------------- */
	stm32_gpio_clr(IO_LED1);
	stm32_gpio_mode(IO_LED1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2);
	stm32_gpio_mode(IO_LED2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3);
	stm32_gpio_mode(IO_LED3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4);
	stm32_gpio_mode(IO_LED4, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* - Switch ----------------------------------------------------------- */
	stm32_gpio_mode(IO_SW1, OUTPUT, PULL_UP | SPEED_LOW);

}

void board_on_softreset(void)
{
	struct stm32_rcc *rcc = STM32_RCC;

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

	rcc->ahb1enr = (1 << RCC_GPIOA);
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* disable all peripherals clock sources except USB_OTG and GPIOA */
	rcc->ahb1enr = (1 << RCC_GPIOA);
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* reinitialize IO's */
	io_init();

	__led_on(IO_LED1);

	DCC_LOG(LOG_WARNING, "/!\\ NVIC interrupts enabled");
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
}


int board_init(void)
{
	io_init();

	DCC_LOG(LOG_WARNING, "/!\\ NVIC interrupts enabled");
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

	return 1;
}


void __attribute__((noreturn)) main(int argc, char ** argv)
{
	const struct dbgmon_comm * comm;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	DCC_LOG(LOG_TRACE, "1. cm3_udelay_calibrate().");
	cm3_udelay_calibrate();

	DCC_LOG(LOG_TRACE, "2. thinkos_init().");
	thinkos_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0));

	DCC_LOG(LOG_TRACE, "3. board_init().");
	board_init();

	__led_on(IO_LED1);
	udelay(32768);

	DCC_LOG(LOG_TRACE, "4. usb_comm_init()");
	comm = usb_comm_init(&stm32f_otg_fs_dev);
	//comm = custom_comm_init();

	DCC_LOG(LOG_TRACE, "5. thinkos_console_init()");
	thinkos_console_init();

	__led_on(IO_LED2);
	udelay(32768);


	DCC_LOG(LOG_TRACE, "6. thinkos_mpu_init()");
	thinkos_mpu_init(0x1000);

	DCC_LOG(LOG_TRACE, "7. thinkos_userland()");
	thinkos_userland();

	__led_on(IO_LED3);
	udelay(32768);


	DCC_LOG(LOG_TRACE, "8. thinkos_dbgmon()");
	/* starts monitor with shell enabled */
	thinkos_dbgmon(monitor_task, comm, 
				   (void *)(MONITOR_SHELL | MONITOR_AUTOBOOT));

	DCC_LOG(LOG_TRACE, "10. thinkos_thread_abort()");
	thinkos_thread_abort(0);

	DCC_LOG(LOG_ERROR, "11. unreachable code reched!!!");
	for(;;);
=======
void board_init(void);
void board_reset(void);
void boot_monitor_task(const struct monitor_comm * comm, void * arg,
					   struct thinkos_krn *);
int board_integrity_check(void);

extern const struct thinkos_mem_map board_mem_map;
extern const struct thinkos_board this_board;
extern const struct thinkos_flash_desc board_flash_desc;

extern const struct thinkos_comm stm32_uart1_comm_instance;
extern const struct thinkos_comm usb_cdc_comm_instance;


void usb_vbus_connect(bool connect)
{
	if (connect)
		stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);
	else
		stm32_gpio_mode(OTG_FS_VBUS, INPUT, 0);
}

void main(int argc, char ** argv)
{
	struct btl_shell_env * env = btl_shell_env_getinstance();
	struct thinkos_krn * krn = &thinkos_krn;
	const struct monitor_comm * comm;
//	int h;

#if DEBUG
	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
	mdelay(125);

	DCC_LOG(LOG_TRACE, "\n\n" VT_PSH VT_BRI VT_FBL);
	DCC_LOG(LOG_TRACE, "*************************************************");
	DCC_LOG(LOG_TRACE, "*    STM32F405-DK ThinkOS Custom Bootloader     *");
	DCC_LOG(LOG_TRACE, "*************************************************"
			VT_POP "\n\n");

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR "* 1. thinkos_krn_init()." VT_POP);
#endif

	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED |
					 THINKOS_OPT_STACK_SIZE(32768), &board_mem_map);

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR "* 2. board_init()." VT_POP);
#endif
	board_init();

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR "* 3. thinkos_krn_flash_drv_init()." 
			VT_POP);
#endif
	thinkos_krn_flash_drv_init(krn, 0, &board_flash_desc);

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR "* 4. thinkos_krn_comm_init()." 
			VT_POP);
#endif
//	thinkos_krn_comm_init(krn, 0, &usb_cdc_comm_instance, 
//						  (void *)&stm32f_otg_fs_dev);
//	thinkos_krn_comm_init(krn, 1, &stm32_uart1_comm_instance, NULL);
  
#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR "* 5. usb_comm_init()." VT_POP);
#endif
	comm = usb_comm_init(&stm32f_otg_fs_dev);

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 6. thinkos_krn_monitor_init()." VT_POP);
#endif
	thinkos_krn_monitor_init(krn, comm, boot_monitor_task, (void *)&this_board);
	board_reset();

	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 7. thinkos_krn_sched_on()." VT_POP);
#endif
	thinkos_krn_sched_on(krn);

//	h = thinkos_comm_open(0);

//	DCC_LOG(LOG_TRACE, "thinkos_sleep()...");
//	thinkos_sleep(2000);

//	thinkos_comm_timedsend(h, "Hello world!\r\n", 14, 48);
//	thinkos_comm_timedsend(h, "Many, but not all people.\r\n", 27, 0);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR
			"* 8. btl_flash_app_exec()..." VT_POP);
	btl_flash_app_exec("APP", 0, 0);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FYW
			"* 9. btl_flash_app_exec() failed." VT_POP);

	btl_console_shell(env);
>>>>>>> krnsvc
}

