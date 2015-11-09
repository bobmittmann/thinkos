/* ---------------------------------------------------------------------------
 * File: ui.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <thinkos.h>
#include <sys/serial.h>
#include <sys/stm32f.h>

#include "net.h"
#include "gpio.h"

#define UI_LED_COUNT 8

/* Driver private data */
static struct {
	uint8_t addr;
    struct serial_dev * serial;
} rs485;

struct lnkhdr {
	uint8_t sync;
	uint8_t daddr;
	uint8_t saddr;
	uint8_t datalen;
};

#define NET_ADDR_BCAST 0xff

/* -------------------------------------------------------------------------
   RS485 network - Link Layer
   ------------------------------------------------------------------------- */

#define SYNC 0x55
#define PKT_TMO_MS 100

int netlnk_recv(void * data, unsigned int max)
{
	struct serial_dev * serial = rs485.serial;
	unsigned int my_addr = rs485.addr;
	uint8_t hdr[4];
	uint8_t * cp;
	int len;
	int rem;

	for (;;) {
		thinkos_sleep(1000000);

		cp = hdr;

		/* Wait for a SYNC */
		if ((serial_recv(serial, cp, 1, 1000) <= 0) || (*cp++ != SYNC))
			continue;

		/* Get the destination address */
		if (serial_recv(serial, cp, 1, PKT_TMO_MS) <= 0)
			continue;

		if ((*cp != NET_ADDR_BCAST) || (*cp != my_addr))
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

#define UART_TX STM32_GPIOA, 2
#define UART_RX STM32_GPIOA, 3

void netlnk_init(unsigned int addr)
{
    /* Configure UART IO pins */
    stm32_gpio_mode(UART_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
    stm32_gpio_af(UART_TX, GPIO_AF7);
    stm32_gpio_mode(UART_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART_RX, GPIO_AF7);
    /* Open the serial port */
    rs485.serial = stm32f_uart2_serial_init(38400, SERIAL_8N1);
    /* Set the local address */
    rs485.addr = addr;
 }

/* -------------------------------------------------------------------------
   RS485 network - Transport Layer
   ------------------------------------------------------------------------- */

#define NET_DGRAM_DATA_LEN_MAX 128
#define NET_DGRAM_HEADER_LEN 1

int net_send(uint8_t msg_type, const void * data, unsigned int len)
{
	uint8_t dgram[NET_DGRAM_HEADER_LEN + NET_DGRAM_DATA_LEN_MAX];

	if (data == NULL)
		return -1;

	if (len > NET_DGRAM_DATA_LEN_MAX)
		return -2;

	dgram[0] = msg_type;
	memcpy(&dgram[1], data, len);

	return netlnk_send(NET_ADDR_BCAST, dgram, len + NET_DGRAM_HEADER_LEN);
}

int net_recv(uint8_t * msg_type, void * data, unsigned int max)
{
	uint8_t dgram[NET_DGRAM_HEADER_LEN + NET_DGRAM_DATA_LEN_MAX];
	int len;

	if ((msg_type == NULL) || (data == NULL))
		return -1;

	len = netlnk_recv(dgram, NET_DGRAM_DATA_LEN_MAX);

	if (len < NET_DGRAM_HEADER_LEN) 
		return -3;

	len -= NET_DGRAM_HEADER_LEN;
	if (len > max)
		len = max;

	*msg_type = dgram[0];
	memcpy(data, &dgram[1], len);

	return len;
}

void net_init(void)
{
	unsigned int my_addr;

    /* Get the local address from the GPIO */
    my_addr = gpio_status(GPIO_ADDRESS) + 1;

	/* Initialize link layer */
	netlnk_init(my_addr);
}

unsigned int net_local_addr(void) 
{
	return netlnk_addr();
}

