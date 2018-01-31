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
 * @file sys/stm32f.h
 * @brief YARD-ICE libstm32f
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYS_STM32F_H__
#define __SYS_STM32F_H__

#ifdef STM32F429
#include <arch/stm32f429.h>
#endif

#ifdef STM32F427
#include <arch/stm32f427.h>
#endif

#ifdef STM32F407
#include <arch/stm32f407.h>
#endif

#ifdef STM32F405
#include <arch/stm32f405.h>
#endif

#ifdef STM32F207
#include <arch/stm32f207.h>
#endif

#if defined(STM32F303) || defined(STM32F303XB) || defined(STM32F303XC)
#include <arch/stm32f303.h>
#endif

#if defined(STM32F103) || defined(STM32F103X6)
#include <arch/stm32f103.h>
#endif

#ifdef STM32F100
#include <arch/stm32f100.h>
#endif

#if defined(STM32L151X6) || defined(STM32L151X8) || defined(STM32L151XB) || \
	defined(STM32L151XC)
#include <arch/stm32l151.h>
#endif

#if defined(STM32F446XC) || defined(STM32F446XE)
#include <arch/stm32f446.h>
#endif

#if defined(STM32L433XC) || defined(STM32L433XB)
#include <arch/stm32l433.h>
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>
#include <arch/cortex-m3.h>

/*---------------------------------------------------------------------
 * USB Device
 *---------------------------------------------------------------------*/
#include <sys/usb-dev.h>
extern const usb_dev_t stm32f_usb_fs_dev;
extern const usb_dev_t stm32f_otg_fs_dev;
extern const usb_dev_t stm32f_otg_hs_dev;


/*---------------------------------------------------------------------
 * DMA 
 *---------------------------------------------------------------------*/

extern const uint8_t stm32f_dma_isr_base_lut[];
extern const uint8_t stm32f_dma1_irqnum_lut[];
extern const uint8_t stm32f_dma2_irqnum_lut[];

#if defined(STM32F2X) || defined(STM32F4X)

struct stm32_dmactl {
	/* DMA Stream ID */
	uint8_t id;
	/* DMA IRQ number */
	uint8_t irqno;
	/* DMA stream */
	struct stm32f_dma_stream * strm;
	/* Bitband pointer to DMA interrupt status flags */
	uint32_t * isr;
	/* Bitband pointer to DMA interrupt clear flags */
	uint32_t * ifcr;
};

static inline uint32_t * dma_isr_bitband(struct stm32f_dma * dma,
										 int stream) {
	return CM3_BITBAND_DEV(&dma->lisr, stm32f_dma_isr_base_lut[stream]);
}

static inline uint32_t * dma_ifcr_bitband(struct stm32f_dma * dma,
										 int stream) {
	return CM3_BITBAND_DEV(&dma->lifcr, stm32f_dma_isr_base_lut[stream]);
}

#define FEIF_BIT 0
#define DMEIF_BIT 2
#define TEIF_BIT 3
#define HTIF_BIT 4
#define TCIF_BIT 5

#endif

#if defined(STM32F1X) || defined(STM32F3X) || defined(STM32L1X) \
	|| defined(STM32L4X)

struct stm32_dmactl {
	/* DMA Stream ID */
	uint8_t id;
	/* DMA IRQ number */
	uint8_t irqno;
	/* DMA stream */
	struct stm32f_dma_stream * strm;
	/* Bitband pointer to DMA interrupt status flags */
	uint32_t * isr;
	/* Bitband pointer to DMA interrupt clear flags */
	uint32_t * ifcr;
};

static inline uint32_t * dma_isr_bitband(struct stm32f_dma * dma,
										 int stream) {
	return CM3_BITBAND_DEV(&dma->isr, (stream << 2));
}

static inline uint32_t * dma_ifcr_bitband(struct stm32f_dma * dma,
										 int stream) {
	return CM3_BITBAND_DEV(&dma->ifcr, (stream << 2));
}

#define TEIF_BIT 3
#define HTIF_BIT 2
#define TCIF_BIT 1
#define GIF_BIT  0

#endif

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------
 * DMA
 *---------------------------------------------------------------------*/

void stm32_dmactl_init(struct stm32_dmactl * ctl,
		struct stm32f_dma * dma, int id);

/*---------------------------------------------------------------------
 * FSMC
 *---------------------------------------------------------------------*/

void stm32f_fsmc_init(void);

void stm32f_fsmc_speed(int div);

/*---------------------------------------------------------------------
 * EXTI 
 *---------------------------------------------------------------------*/

#define EXTI_EDGE_RISING 1

void stm32f_exti_init(struct stm32_gpio * gpio, unsigned int pin, 
					  unsigned int opt);

/*---------------------------------------------------------------------
 * MCO
 *---------------------------------------------------------------------*/

void stm32f_mco2_init(void);
void stm32f_mco2_disable(void);
void stm32f_mco2_enable(void);

/*---------------------------------------------------------------------
 * USB Device
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
 * USART 
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
 * CRC32 
 *---------------------------------------------------------------------*/
uint32_t stm32_crc32(void  * buf, unsigned int len);

/*---------------------------------------------------------------------
 * SPI
 *---------------------------------------------------------------------*/

int stm32f_spi_init(struct stm32f_spi * spi, unsigned int freq,
		             unsigned int opt);
int stm32f_spi_putc(struct stm32f_spi * spi, int c);
int stm32f_spi_getc(struct stm32f_spi * spi);
int stm32f_spi_drain(struct stm32f_spi * spi);

#ifdef STM32F_ETH 
/*---------------------------------------------------------------------
 * Ethernet
 *---------------------------------------------------------------------*/
void stm32f_eth_init(struct stm32f_eth * eth);
void stm32f_eth_mac_get(struct stm32f_eth * eth, int idx, uint8_t * mac);
void stm32f_eth_mac_set(struct stm32f_eth * eth, int idx, const uint8_t * mac);
#endif

#include <sys/file.h>

extern const struct fileop stm32_usart_fops;

extern const struct fileop stm32_usart_fops_raw;

extern const struct file stm32_usart5_file;

#include <sys/sysclk.h>

#ifdef __cplusplus
}
#endif

#ifdef STM32F_USB
/*---------------------------------------------------------------------
 * USB Full Speed
 *---------------------------------------------------------------------*/

void stm32f_usb_io_init(void);

void stm32f_usb_pullup(struct stm32f_usb * usb, bool connect);

void stm32f_usb_power_on(struct stm32f_usb * usb);

void stm32f_usb_power_off(struct stm32f_usb * usb);

void stm32f_usb_ep0_init(struct stm32f_usb * usb, int mxpktsz);

void stm32f_usb_ep_init(struct stm32f_usb * usb, int ep_id,
		struct usb_descriptor_endpoint * desc);
#endif

/*---------------------------------------------------------------------
 * Flash Memory
 *---------------------------------------------------------------------*/

int stm32_flash_erase(unsigned int offs, unsigned int len);

int stm32_flash_write(uint32_t offs, const void * buf, unsigned int len);

void stm32_eeprom_unlock(void);

int stm32_eeprom_wr32(uint32_t offs, uint32_t val);

int stm32_eeprom_rd32(uint32_t offs, uint32_t * val);

int stm32_eeprom_wr16(uint32_t offs, uint16_t val);

/*---------------------------------------------------------------------
 * RTC
 *---------------------------------------------------------------------*/

int stm32f_rtc_init(void);

/*---------------------------------------------------------------------
 * NVRAM (Backupp SRAM
 *---------------------------------------------------------------------*/

void stm32f_nvram_env_init(void);

/*---------------------------------------------------------------------
 * Serial 
 *---------------------------------------------------------------------*/

struct serial_dev * stm32f_uart1_serial_init(unsigned int baudrate, 
											 unsigned int flags);

struct serial_dev * stm32f_uart2_serial_init(unsigned int baudrate, 
											 unsigned int flags);

struct serial_dev * stm32f_uart3_serial_init(unsigned int baudrate, 
											 unsigned int flags);

struct serial_dev * stm32f_uart4_serial_init(unsigned int baudrate, 
											 unsigned int flags);

struct serial_dev * stm32f_uart5_serial_init(unsigned int baudrate, 
											 unsigned int flags);

struct serial_dev * stm32f_uart6_serial_init(unsigned int baudrate, 
											 unsigned int flags);

struct serial_dev * stm32f_uart7_serial_init(unsigned int baudrate,
											 unsigned int flags);

struct serial_dev * stm32f_uart8_serial_init(unsigned int baudrate,
											 unsigned int flags);

struct serial_dev * stm32f_uart1_serial_dma_init(unsigned int baudrate, 
												 unsigned int flags);

struct serial_dev * stm32f_uart2_serial_dma_init(unsigned int baudrate, 
												 unsigned int flags);

struct serial_dev * stm32f_uart3_serial_dma_init(unsigned int baudrate, 
												 unsigned int flags);

struct serial_dev * stm32f_uart4_serial_dma_init(unsigned int baudrate, 
												 unsigned int flags);

struct serial_dev * stm32f_uart5_serial_dma_init(unsigned int baudrate, 
												 unsigned int flags);

struct serial_dev * stm32f_uart6_serial_dma_init(unsigned int baudrate, 
												 unsigned int flags);

struct serial_dev * stm32f_uart7_serial_dma_init(unsigned int baudrate,
												 unsigned int flags);

struct i2s_dev * stm32_spi2_i2s_init(unsigned int samplerate, 
									 unsigned int flags);

struct i2s_dev * stm32_spi3_i2s_init(unsigned int samplerate, 
									 unsigned int flags);
#endif /* __ASSEMBLER__ */

#endif /* __SYS_STM32F_H__ */

