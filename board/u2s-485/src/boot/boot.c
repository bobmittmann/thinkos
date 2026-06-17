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

#define FLASH_ADDR 0x08000000
#define APP_OFFS (16 * 1024)
#define APP_SIZE (16 * 1024)
#define APP_ADDR (FLASH_ADDR + APP_OFFS)

static const struct comm_dev console_comm_dev = {
#if (THINKOS_COMM_MAX) > 0
	.arg = (void *)THINKOS_COMM_TX_DESC(0),
#endif
	.op = {
		.send = krn_console_dev_send,
		.recv = krn_console_dev_recv
	}
};

int stm32f1x_flash_erase(struct stm32_flash * flash, off_t offs, size_t len);

int stm32f1x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len);

void __app_run(uint32_t app_addr)
{
    DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR
            "* thinkos_app_exec()..." VT_POP);
    thinkos_app_exec(app_addr, 0, 0);
    DCC_LOG(LOG_ERROR, VT_PSH VT_BRI VT_FRD
            "**** thinkos_app_exec() failed." VT_POP);
}

void __flash_erase(uint32_t offs, uint32_t size)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t rem = size;

	krn_console_puts("\r\nErasing... ");
	while (rem > 0) {
		int ret;
		if ((ret = stm32f1x_flash_erase(flash, offs, rem)) <= 0) {
			krn_console_puts("failed!\r\n");
			return;
		}
		offs += ret;
		rem -= ret;
	}

	krn_console_puts("Ok.\r\n");
}

/* Receive a file and write it into the flash using the YMODEM protocol */
void __flash_ymodem_recv(uint32_t offs, uint32_t size)
{
	struct stm32_flash * flash = STM32_FLASH;
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t end = offs + size;
	uint8_t buf[1024];
	char * fname;
	int ret;

	krn_console_puts("\r\nReceiving YMODEM...");

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);

	fname = (char *)buf;
	while ((ret = ymodem_rcv_start(&ry, fname, &fsize)) > 0) {
		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			int cnt = ret;
			uint8_t * cp = buf;
			if (offs >= end) {
				ymodem_rcv_cancel(&ry);
				ret = -1;
				break;
			}
			while (cnt > 0) {
				ret = stm32f1x_flash_write(flash, offs, cp, cnt);
				if (ret <= 0) {
					ymodem_rcv_cancel(&ry);
					break;
				}
				offs += ret;
				cp += ret;
				cnt -= ret;
			}
		}
		if (ret < 0) {
			break;
		}
	} 

	DCC_LOG(LOG_TRACE, "YMODEM stop.");

	ymodem_rcv_flush(&ry);
}

const char help[] = 
	"\r\nOptions:\r\n" 
	"\tR - Run App\r\n" 
	"\tE - Erase App\r\n" 
	"\tY - YMODEM receive\r\n" 
	"\tQ - Quit\r\n" 
	"[boot] > "
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
			__flash_ymodem_recv(APP_OFFS, APP_SIZE);
			break;

		case 'E':
			__flash_erase(APP_OFFS, APP_SIZE);
			break;

		case 'R':
			__app_run(APP_ADDR);
			break;

		case '\r':
		case 'h':
			krn_console_puts(help);
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

const struct thinkos_thread_initializer shell_thread_init = {
	.stack_base = APP_STACK_BASE,
	.stack_size = APP_STACK_SIZE,
	.task_entry = (uintptr_t)console_shell_task,
	.task_exit = (uintptr_t)thinkos_krn_abort_at_exit,
	.task_arg[0] = (uintptr_t)NULL,
	.task_arg[1] = 0,
	.priority = 1,
	.paused = false,
	.privileged = false,
};

#if 0
void board_core_rst(struct thinkos_rt * krn)
{
	thinkos_krn_thread_init(krn, 1, &rst_thread_init);
}

void board_on_break(struct thinkos_rt * krn)
{
}
#endif

void board_reset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;

	DCC_LOG(LOG_TRACE, "Initializing IO ...");
			
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);

	/* Enable Alternate Functions IO clock */
	rcc->apb2enr |= RCC_AFIOEN;

	/* UART */
	stm32_gpio_mode(USART2_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(USART2_RX, INPUT, PULL_UP);

#if (THINKOS_ENABLE_APP_CRC) 
	/* - CRC --------------------------------------------------------------- */
	stm32_clk_enable(STM32_RCC, STM32_CLK_CRC);
#endif

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

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_USB_HP, IRQ_HIGH_PRIORITY);
	cm3_irq_pri_set(STM32F_IRQ_USB_LP, IRQ_HIGH_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32F_IRQ_USB_HP);
	cm3_irq_enable(STM32F_IRQ_USB_LP);
}

void usb_vbus(bool on)
{
	if (on) {
		stm32_gpio_mode(USB_FS_VBUS, OUTPUT, PUSH_PULL | SPEED_LOW);
		stm32_gpio_set(USB_FS_VBUS);
	} else
		stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
}

#define REQ_SHOW_MENU       MONITOR_USER_EVENT4

/* Default Monitor Task */
void __attribute__((noreturn)) monitor_task(const struct monitor_comm * comm, 
											void * param, struct thinkos_rt * krn)
{
	uint32_t sigmask = 0;
	uint32_t sig;

	/* unmask events */
	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_EOT);
	sigmask |= (1 << MONITOR_TX_PIPE);
	sigmask |= (1 << MONITOR_RX_PIPE);
	sigmask |= (1 << MONITOR_COMM_BRK);
//	sigmask |= (1 << MONITOR_COMM_CTL);

	sigmask |= (1 << MONITOR_ON_CORE_RST);
	sigmask |= (1 << MONITOR_THREAD_FAULT);

	for(;;) {
		switch ((sig = monitor_select(sigmask))) {

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

		case MONITOR_THREAD_FAULT:
			{
				int thread_id;
				int32_t errno;

				monitor_clear(MONITOR_THREAD_FAULT);

				/* get the last thread known to be at fault */
				thread_id = monitor_thread_break_get(&errno);
				(void)thread_id;

				DCC_LOG2(LOG_ERROR, "<%d> error %d !!", thread_id, errno);
				if ((errno >= THINKOS_ERR_APP_INVALID) && 
					(errno <= THINKOS_ERR_APP_BSS_INVALID)) {
					DCC_LOG(LOG_ERROR, "Invalid application !");
					monitor_thread_break_clr();
				}
				if (errno == THINKOS_ERR_SYSCALL_INVALID) {
					DCC_LOG(LOG_ERROR, "Invalid System Call!");
					monitor_thread_break_clr();
				}
			}

			break;

		case MONITOR_COMM_BRK:
			monitor_clear(MONITOR_COMM_BRK);
			DCC_LOG(LOG_TRACE, "Line break received");
			thinkos_krn_req_core_rst(krn);					
			break;

		case MONITOR_ON_CORE_RST:
			monitor_clear(MONITOR_ON_CORE_RST);
			DCC_LOG(LOG_TRACE, "Core reset received");
			board_reset();
			thinkos_krn_thread_init(krn, 3, &shell_thread_init);
			break;

		default:
			DCC_LOG1(LOG_WARNING, "Unhandled signal: %d", sig);
			monitor_clear(sig);
		}
	}
}

extern const struct thinkos_comm usb_cdc_comm_instance;

void main(int argc, char ** argv)
{
	struct thinkos_rt * krn = &thinkos_rt;
	const struct monitor_comm * comm;

	DCC_LOG_INIT();

#if DEBUG
	DCC_LOG_CONNECT();
	DCC_LOG(LOG_TRACE, "--------------");
	DCC_LOG(LOG_TRACE, " U2S-485 Boot ");
	DCC_LOG(LOG_TRACE, "--------------");
#endif

	board_reset();

	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED, NULL);

	comm = usb_comm_init(&stm32f_usb_fs_dev);

	board_reset();

	/* starts/restarts monitor with autoboot enabled */
	thinkos_krn_monitor_init(krn, comm, monitor_task, NULL);

	usb_vbus(true);

	DCC_LOG(LOG_TRACE, "thinkos_sleep()...");
	thinkos_sleep(2000);

	__app_run(APP_ADDR);
//	console_shell_task();
//	thinkos_sleep(2000);
//	board_on_break(krn);
}

