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

#include "monitor-i.h"

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <sys/dcclog.h>

#include "board.h"
#include "version.h"
#include <xmodem.h>


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
int btl_flash_ymodem_recv(const char * tag)
{
	struct stm32_flash * flash = STM32_FLASH;
	struct ymodem_rcv ry;
	unsigned int fsize;
	uint32_t offs = 10 * 1024;
	uint8_t buf[1024];
	char * fname;
	int ret;

	stm32f1x_flash_erase(flash, offs, 22 * 1024);

	ymodem_rcv_init(&ry, &console_comm_dev, XMODEM_RCV_CRC);

	fname = (char *)buf;
	if ((ret = ymodem_rcv_start(&ry, fname, &fsize)) >= 0) {
		while ((ret = ymodem_rcv_loop(&ry, buf, sizeof(buf))) > 0) {
			int cnt = ret;
			if ((ret = stm32f1x_flash_write(flash, offs, buf, cnt)) < 0) {
				break;
			}
			offs += cnt;
		}
	}

	return ret;
}

const struct thinkos_thread_initializer yrecv_thread_init = {
	.stack_base = 0x20000800,
	.stack_size = 8192,
	.task_entry = (uintptr_t)btl_flash_ymodem_recv,
	.task_exit = (uintptr_t)NULL,
	.task_arg[0] = (uintptr_t)"app",
	.task_arg[1] = 1,
	.task_arg[2] = 0,
	.task_arg[3] = 0,
	.priority = 0,
	.paused = false,
	.privileged = 0,
};

const struct thinkos_thread_initializer app_thread_init = {
	.stack_base = 0x20000800,
	.stack_size = 8192,
	.task_entry = (uintptr_t)0x00002800,
	.task_exit = (uintptr_t)NULL,
	.task_arg[0] = (uintptr_t)"app",
	.task_arg[1] = 1,
	.task_arg[2] = 0,
	.task_arg[3] = 0,
	.priority = 0,
	.paused = false,
	.privileged = 0,
};

int board_on_break(const struct monitor_comm * comm)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return thinkos_krn_thread_init(krn, 1, &yrecv_thread_init);
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

	stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
	stm32_gpio_set(USB_FS_VBUS);

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_USB_HP, MONITOR_PRIORITY);
	cm3_irq_pri_set(STM32F_IRQ_USB_LP, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32F_IRQ_USB_HP);
	cm3_irq_enable(STM32F_IRQ_USB_LP);
}

/* Default Monitor Task */
void __attribute__((noreturn)) monitor_task(const struct monitor_comm * comm, void * param)
{
	uint32_t sigmask = 0;
	uint32_t sig;
	bool connected;
	int status;

	/* unmask events */
	sigmask |= (1 << MONITOR_SOFTRST);
	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_TX_PIPE);

	for(;;) {
		switch ((sig = monitor_select(sigmask))) {

		case MONITOR_SOFTRST:
			monitor_clear(MONITOR_SOFTRST);
			board_reset();
			/* FALLTHROUGH */

		case MONITOR_COMM_CTL:
			monitor_clear(MONITOR_COMM_CTL);
			status = monitor_comm_status_get(comm);
			connected = (status & COMM_ST_CONNECTED) ? true : false;
			thinkos_krn_console_connect_set(connected);
			sigmask &= ~((1 << MONITOR_COMM_EOT) | 
						 (1 << MONITOR_COMM_RCV) |
						 (1 << MONITOR_RX_PIPE));
			sigmask |= (1 << MONITOR_TX_PIPE);
			if (connected) {
				sigmask |= ((1 << MONITOR_COMM_EOT) |
							(1 << MONITOR_COMM_RCV));
			}
			break;

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
			monitor_comm_break_ack(comm);
			board_on_break(comm);		
			break;

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
	mdelay(100);
	DCC_LOG(LOG_TRACE, "______________________________________________________");
	DCC_LOG(LOG_TRACE, "_________________ ! Board start up ! _________________");
	DCC_LOG(LOG_TRACE, "______________________________________________________");
	mdelay(100);
#endif

	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED, NULL);

	board_reset();

	comm = usb_comm_init(&stm32f_usb_fs_dev);

//	thinkos_krn_comm_init(krn, 0, &usb_cdc_comm_instance, 
//						  (void *)&stm32f_usb_fs_dev);

	/* starts/restarts monitor with autoboot enabled */
	thinkos_krn_monitor_init(comm, monitor_task, NULL);

//	thinkos_thread_init(1, &app_thread_init);
//
	for (;;) {
		stm32_gpio_set(LED1_IO);
		stm32_gpio_clr(LED2_IO);
		thinkos_sleep(1000);
		stm32_gpio_set(LED2_IO);
		stm32_gpio_clr(LED1_IO);
		thinkos_sleep(1000);
	}

	thinkos_abort();
}

