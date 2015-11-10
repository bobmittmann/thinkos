/* ---------------------------------------------------------------------------
 * File: net.c
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#include <string.h>
#include <stdlib.h>
#include <thinkos.h>
#include <assert.h>
#include <sys/serial.h>
#include <sys/stm32f.h>

#include "net.h"
#include "gpio.h"

#define NET_ADDR_BCAST 0xff

/* -------------------------------------------------------------------------
   RS485 network - Link Layer
   ------------------------------------------------------------------------- */

/* Driver private data */
static struct {
	uint8_t addr; /* this panel address */
    struct serial_dev * serial; /* serial device driver */
} rs485;

#define PKT_SYNC 0x55
#define PKT_TMO_MS 100

/* Link layer header */
struct lnkhdr {
	uint8_t sync;
	uint8_t daddr;
	uint8_t saddr;
	uint8_t datalen;
};

int netlnk_recv(void * data, unsigned int max)
{
	struct serial_dev * serial = rs485.serial;
	unsigned int my_addr = rs485.addr;
	struct lnkhdr hdr;

	for (;;) {
		uint8_t * cp;
		int rem;
		int cnt;

		/* Receive header */
		cp = (uint8_t *)&hdr;
		rem = sizeof(struct lnkhdr);
		do {
			if ((cnt = serial_recv(serial, cp, rem, PKT_TMO_MS)) <= 0)
				break;
			rem -= cnt;
			cp += cnt;
		} while (rem);

		if (rem != 0)
			continue;

		if (hdr.sync != PKT_SYNC)
			continue;

		/* destination address */
		if ((hdr.daddr != NET_ADDR_BCAST) && (hdr.daddr != my_addr))
			continue;

		/* data length */
		if (hdr.datalen > max)
			continue;

		/* Receive payload */
		cp = (uint8_t *)data;
		rem = hdr.datalen;
		while (rem  > 0) {
			if ((cnt = serial_recv(serial, cp, rem, PKT_TMO_MS)) <= 0)
				break;
			rem -= cnt;
			cp += cnt;
		}

		if (rem == 0)
			break;
	}

	return hdr.datalen;
}

int netlnk_send(unsigned int daddr, void * data, unsigned int len)
{
	struct serial_dev * serial = rs485.serial;
	struct lnkhdr hdr;

	/* prepare header */
	hdr.sync = PKT_SYNC;
	hdr.daddr = daddr;
	hdr.saddr = rs485.addr;
	hdr.datalen = len;
	/* send header */
	serial_send(serial, &hdr, sizeof(struct lnkhdr));
	/* send payload */
	return serial_send(serial, data, len);
}

unsigned int netlnk_addr(void)
{
	return rs485.addr;
}

#define UART_TX STM32_GPIOA, 9
#define UART_RX STM32_GPIOA, 10

void netlnk_init(unsigned int addr)
{
    /* Configure UART IO pins */
    stm32_gpio_mode(UART_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
    stm32_gpio_af(UART_TX, GPIO_AF7);
    stm32_gpio_mode(UART_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART_RX, GPIO_AF7);
    /* Open the serial port */
    rs485.serial = stm32f_uart1_serial_init(9600, SERIAL_8N1);
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

	assert((len == 0) || (len > 0 && data != NULL));
	assert(len <= NET_DGRAM_DATA_LEN_MAX);

	dgram[0] = msg_type;
	memcpy(&dgram[1], data, len);

	return netlnk_send(NET_ADDR_BCAST, dgram, len + NET_DGRAM_HEADER_LEN);
}

int net_recv(uint8_t * msg_type, void * data, unsigned int max)
{
	uint8_t dgram[NET_DGRAM_HEADER_LEN + NET_DGRAM_DATA_LEN_MAX];
	int len;

	assert(msg_type != NULL);
	assert(data != NULL);

	len = netlnk_recv(dgram, NET_DGRAM_DATA_LEN_MAX);
	assert(len >= NET_DGRAM_HEADER_LEN);

	len -= NET_DGRAM_HEADER_LEN;
	assert(len <= max);

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
