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

#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#include <thinkos.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <xmodem.h>
#include <vt100.h>
#include <ascii.h>

#include "board.h"
#include "version.h"

#include <sys/dcclog.h>

static const struct comm_dev console_comm_dev = {
	.arg = NULL,
	.op = {
		.send = krn_console_dev_send,
		.recv = krn_console_dev_recv
	}
};

int stm32f1x_flash_erase(struct stm32_flash * flash, off_t offs, size_t len);

int stm32f1x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len);

/* Receive a file and write it into the flash using the YMODEM protocol */
int __flash_ymodem_recv(void)
{
	struct stm32_flash * flash = STM32_FLASH;
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t offs = 16 * 1024;
	uint8_t buf[1024];
	char * fname;
	int ret;

	stm32f1x_flash_erase(flash, offs, 16 * 1024);

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);

	fname = (char *)buf;
	while ((ret = ymodem_rcv_start(&ry, fname, &fsize)) > 0) {
		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			int cnt = ret;
			ret = stm32f1x_flash_write(flash, offs, buf, cnt);
			DCC_LOG1(LOG_ERROR, "thinkos_flash_mem_write()=>%d", ret);
			offs += cnt;
		}
		if (ret < 0) {
			DCC_LOG1(LOG_WARNING, "ret=%d", ret);
			break;
		}
	} 

	DCC_LOG(LOG_TRACE, "YMODEM stop.");

	ymodem_rcv_flush(&ry);

	return ret;
}

const char help[] = 
	"Options:\r\n" 
	"\tY - YMODEM receive\r\n" 
	"\tQ - Quit\r\n" 
;

int console_shell_task(void)
{
	int c = '\r';
	int i = 0;

//	while (thinkos_console_is_connected() <= 0) {
//		thinkos_sleep(100);
//	}
	krn_console_puts("\r\n...\r\n");

	do {
//		if (thinkos_console_is_connected() <= 0)
//			continue;

		switch (c) {
		case 'Y':
			__flash_ymodem_recv();
			break;

		case '\r':
		case 'h':
			krn_console_puts(help);
			krn_console_puts("> ");
			break;

		}

		if (i++ & 1) {
			stm32_gpio_set(LED1_IO);
			stm32_gpio_clr(LED2_IO);
		} else {
			stm32_gpio_set(LED2_IO);
			stm32_gpio_clr(LED1_IO);
		}

		c = krn_console_getc(500);
	} while (c != 'Q');

	krn_console_puts("\r\nBye\r\n");

	return 0;
}

const struct thinkos_thread_initializer yrecv_thread_init = {
	.stack_base = APP_STACK_BASE,
	.stack_size = APP_STACK_SIZE,
	.task_entry = (uintptr_t)console_shell_task,
	.task_exit = (uintptr_t)thinkos_krn_abort_at_exit,
	.task_arg[0] = (uintptr_t)"yrc",
	.task_arg[1] = 1,
	.task_arg[2] = 0,
	.task_arg[3] = 0,
	.priority = 1,
	.paused = false,
	.privileged = false,
};

const struct thinkos_thread_initializer app_thread_init = {
	.stack_base = APP_STACK_BASE,
	.stack_size = APP_STACK_SIZE,
	.task_entry = (uintptr_t)0x00004000,
	.task_exit = (uintptr_t)NULL,
	.task_arg[0] = (uintptr_t)"app",
	.task_arg[1] = 1,
	.task_arg[2] = 0,
	.task_arg[3] = 0,
	.priority = 1,
	.paused = false,
	.privileged = false,
};

void board_on_break(struct thinkos_rt * krn, const struct monitor_comm * comm)
{
	thinkos_krn_thread_init(krn, 1, &yrecv_thread_init);
}

void board_reset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);

	/* Enable Alternate Functions IO clock */
	rcc->apb2enr |= RCC_AFIOEN;

	/* UART */
	stm32_gpio_mode(USART2_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(USART2_RX, INPUT, PULL_UP);

	/* RS 485 */
	stm32_gpio_mode(RS485_RXEN, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(RS485_RXEN);

	stm32_gpio_mode(RS485_TXEN, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(RS485_TXEN);

	stm32_gpio_mode(LED1_IO, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(LED1_IO);

	stm32_gpio_mode(LED2_IO, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(LED2_IO);

	/* USB */
	stm32_gpio_mode(USB_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(USB_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

#if (THINKOS_ENABLE_MONITOR)
//	stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
	stm32_gpio_set(USB_FS_VBUS);
#endif

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_USB_HP, MONITOR_PRIORITY);
	cm3_irq_pri_set(STM32F_IRQ_USB_LP, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32F_IRQ_USB_HP);
	cm3_irq_enable(STM32F_IRQ_USB_LP);
}

void usb_vbus(bool on)
{
	if (on)
		stm32_gpio_mode(USB_FS_VBUS, OUTPUT, PUSH_PULL | SPEED_LOW);
	else
		stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
}

#if (THINKOS_ENABLE_MONITOR)
/* Default Monitor Task */
void __attribute__((noreturn)) monitor_task(const struct monitor_comm * comm, 
											void * param, struct thinkos_rt * krn)
{
	uint32_t sigmask = 0;
	uint32_t sig;

	/* unmask events */
	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_TX_PIPE);

	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_EOT);
	sigmask |= (1 << MONITOR_RX_PIPE);
	sigmask |= (1 << MONITOR_COMM_BRK);

	sigmask |= (1 << MONITOR_ON_CORE_RST);
	sigmask |= (1 << MONITOR_SOFTRST);

//	monitor_unmask(MONITOR_SOFTRST);
//	monitor_unmask(MONITOR_COMM_BRK);
//	monitor_unmask(MONITOR_COMM_CTL);

//	monitor_alarm(2000);

	for(;;) {
		switch ((sig = monitor_select(sigmask))) {
#if 0
		case MONITOR_ALARM:
			monitor_clear(MONITOR_ALARM);
			monitor_alarm(10000);
			DCC_LOG(LOG_TRACE, "ALARM");
	//		__kdump(krn);
			break;
#endif

//		case MONITOR_COMM_CTL:
//			monitor_clear(MONITOR_COMM_CTL);
//			status = monitor_comm_status_get(comm);
//			connected = (status & COMM_ST_CONNECTED) ? true : false;
//			thinkos_krn_console_connect_set(connected);
//			sigmask &= ~((1 << MONITOR_COMM_EOT) | 
//						 (1 << MONITOR_COMM_RCV) |
//						 (1 << MONITOR_RX_PIPE));
//			sigmask |= (1 << MONITOR_TX_PIPE);
//			if (connected) {
//				sigmask |= ((1 << MONITOR_COMM_EOT) |
//							(1 << MONITOR_COMM_RCV));
//			}
//			break;

		case MONITOR_COMM_EOT:
			/* FALLTHROUGH */
		case MONITOR_TX_PIPE:
			sigmask = monitor_on_tx_pipe(comm, sigmask);
			break;

		case MONITOR_COMM_RCV:
			/* FALLTHROUGH */
		case MONITOR_RX_PIPE:
			sigmask = monitor_on_rx_pipe(comm, sigmask);
			break;

		case MONITOR_COMM_BRK:
			monitor_clear(MONITOR_COMM_BRK);
			DCC_LOG(LOG_TRACE, "Break received");
	//		monitor_comm_break_ack(comm);
		//	board_on_break(krn, comm);		
			break;

		case MONITOR_SOFTRST:
			monitor_clear(MONITOR_SOFTRST);
			DCC_LOG(LOG_TRACE, "SOFTRST");
			board_reset();
			/* FALLTHROUGH */
			break;

		/* FALLTHROUGH */
		case MONITOR_ON_CORE_RST:
			DCC_LOG(LOG_TRACE, "Core reset received");
			monitor_clear(MONITOR_ON_CORE_RST);
			board_reset();
			board_on_break(krn, comm);
			break;

		default:
			monitor_clear(sig);
		}
	}
}
#endif

void main(int argc, char ** argv)
{
	struct thinkos_rt * krn = &thinkos_rt;
#if (THINKOS_ENABLE_MONITOR)
	const struct monitor_comm * comm;
#endif
	int i;

	DCC_LOG_INIT();

#if DEBUG
	DCC_LOG_CONNECT();
	mdelay(100);
	DCC_LOG(LOG_TRACE, "--------------");
	DCC_LOG(LOG_TRACE, " U2S-485 Boot ");
	DCC_LOG(LOG_TRACE, "--------------");
	mdelay(100);
#endif

	board_reset();
	usb_vbus(false);

	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED, NULL);

#if (THINKOS_ENABLE_MONITOR)
	comm = usb_comm_init(&stm32f_usb_fs_dev);

	/* starts/restarts monitor with autoboot enabled */
	thinkos_krn_monitor_init(krn, comm, monitor_task, NULL);
#endif

	thinkos_sleep(250);

	usb_vbus(true);

#if 1
	for (i = 0; i < 10; ++i) {
		thinkos_sleep(250);
		if (i & 1) {
			stm32_gpio_set(LED1_IO);
			stm32_gpio_clr(LED2_IO);
		} else {
			stm32_gpio_set(LED2_IO);
			stm32_gpio_clr(LED1_IO);
		}
	}
#endif
//	thinkos_thread_init(1, &app_thread_init);

//	__thinkos_dbg_halt();
}

