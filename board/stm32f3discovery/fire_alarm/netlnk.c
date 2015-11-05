/* ---------------------------------------------------------------------------
 * File: ui.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <thinkos.h>
#include <sys/serial.h>
#include <sys/stm32f.h>

#include "gpio.h"

#define UI_LED_COUNT 8

/* Driver private data */
static struct {
	uint8_t addr;
    struct serial_dev * serial;
} rs485;

/* ----------------------------------------------------
   UI API
   ---------------------------------------------------- */
#define UART_TX STM32_GPIOC, 6
#define UART_RX STM32_GPIOC, 7

#define SYNC 0x55
#define NET_BCAST 0xff
#define PKT_TMO_MS 100

int netlnk_recv(void * data, unsigned int max)
{
	struct serial_dev * serial = rs485.serial;
	unsigned int my_addr = rs485.addr;
	uint8_t hdr[4];
	uint8_t * cp;
	int len;
	int rem;
	int c;

	for (;;) {
		cp = hdr;

		/* Wait for a SYNC */
		if ((serial_recv(serial, cp, 1, 1000) <= 0) || (*cp++ != SYNC))
			continue;

		/* Get the destination address */
		if (serial_recv(serial, cp, 1, PKT_TMO_MS) <= 0)
			continue;

		if ((*cp != NET_BCAST) || (*cp != my_addr))
			continue;

		/* Get the packet length */
		if (serial_recv(serial, ++cp, 1, PKT_TMO_MS) <= 0)
			continue;

		len = *cp;
		if (len > max)
			continue;

		/* Get the packet's payload */
		rem = len;
		cp = (uint8_t *)data;
		while (rem  > 0) {
			int cnt;

			if ((cnt = serial_recv(serial, cp, rem, PKT_TMO_MS)) <= 0)
				break;

			rem -= cnt;
			cp += cnt;
		}

		if (rem == 0)
			break;
	}

	return len;
}

int netlnk_send(unsigned int daddr, void * data, unsigned int len)
{
	struct serial_dev * serial = rs485.serial;
	unsigned int my_addr = rs485.addr;
	uint8_t hdr[4];

	hdr[0] = SYNC;
	hdr[1] = daddr;
	hdr[2] = my_addr;
	hdr[3] = len;

	serial_send(serial, hdr, 4);

	return serial_send(serial, data, len);
}

unsigned int netlnk_addr(void)
{
	return rs485.addr;
}

void netlnk_init(void)
{
    stm32_gpio_clock_en(STM32_GPIOC);
    /* Configure UART IO pins */
    stm32_gpio_mode(UART_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
    stm32_gpio_af(UART_TX, GPIO_AF8);
    stm32_gpio_mode(UART_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART_RX, GPIO_AF8);
    /* Open the serial port */
    rs485.serial =  stm32f_uart2_serial_init(115200, SERIAL_8N1);
    /* Get the local address from the GPIO */
    rs485.addr = gpio_status(GPIO_ADDRESS) + 1;
  }


