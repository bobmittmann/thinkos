/* board.h - hardware configuration
 * -------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file board.h
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Hardware configuration options.
 * 
 * This file is common to the application as well as the bootloader.
 * It holds configuration options related to the microconroler and 
 * hardware in general.
 * 
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <sys/stm32f.h>

/* -------------------------------------------------------------------------
 * USB 
 * ------------------------------------------------------------------------- */
#define USB_FS_DP         STM32_GPIOA, 12
#define USB_FS_DM         STM32_GPIOA, 11

/* -------------------------------------------------------------------------
 * JTAG 
 * ------------------------------------------------------------------------- */
#define IO_JTMS           STM32_GPIOA, 13
#define IO_JTDI           STM32_GPIOA, 15
#define IO_JTDO           STM32_GPIOB, 3
#define IO_JTCK           STM32_GPIOA, 14
#define IO_JNTRST         STM32_GPIOB, 4

/**************************************************************************
 * Lattice FPGA (iCE40LP384) connections
 **************************************************************************/

#define IO_ICE40_SPI_SCK  STM32_GPIOA, 1
#define IO_ICE40_SPI_SDO  IO_JNTRST
#define IO_ICE40_SPI_SDI  STM32_GPIOB, 5
#define IO_ICE40_CSEL     STM32_GPIOA, 4
#define IO_ICE40_SPI_SS   STM32_GPIOA, 4

#define ICE40_SPI         STM32F_SPI1      /* SPI peripheral */
#define ICE40_CLK_SPI     STM32_CLK_SPI1   /* SPI peripheral clock id */
#define ICE40_SPI_AF      GPIO_AF5         /* SPI GPIO alternate function */

#define IO_ICE40_CRESET   STM32_GPIOA, 9
#define IO_ICE40_CDONE    STM32_GPIOA, 10

#define IO_ICE40_MODE     IO_ICE40_CDONE

/* -------------------------------------------------------------------------
 * Application SPI (sharing following pins)
 * ------------------------------------------------------------------------- */
#define IO_APP_SPI_SS     STM32_GPIOA, 4
#define IO_APP_SPI_SCK    STM32_GPIOA, 1
#define IO_APP_SPI_MISO   IO_JNTRST        
#define IO_APP_SPI_MOSI   STM32_GPIOB, 5

#define APP_SPI           STM32F_SPI1      /* SPI peripheral */
#define APP_CLK_SPI       STM32_CLK_SPI1   /* SPI peripheral clock id */
#define APP_SPI_AF        GPIO_AF5         /* SPI GPIO alternate function */

/* -------------------------------------------------------------------------
 * IO
 * ------------------------------------------------------------------------- */
#define IO_1              STM32_GPIOB, 8   /* FPGA IO 1 */
#define IO_2              STM32_GPIOB, 12  /* FPGA IO 2 */
#define IO_ANC_JMP        IO_JTDO          /* This pin is shared with JTAG */

/* -------------------------------------------------------------------------
 * FPGA
 * ------------------------------------------------------------------------- */

#define IO_MCO            STM32_GPIOA, 8

/* -------------------------------------------------------------------------
 * DAC 
 * ------------------------------------------------------------------------- */

#define IO_DAC1           STM32_GPIOA, 5

/* -------------------------------------------------------------------------
 * Rotary Switch
 * ------------------------------------------------------------------------- */
#define IO_SW0            STM32_GPIOC, 13
#define IO_SW1            STM32_GPIOC, 14
#define IO_SW2            STM32_GPIOC, 15
#define IO_SW3            STM32_GPIOB, 2

/* -------------------------------------------------------------------------
 * SAI
 * ------------------------------------------------------------------------- */
#define IO_SAI1_SD          STM32_GPIOB, 15
#define IO_SAI1_SCK         STM32_GPIOB, 13
#define IO_SAI1_FS          STM32_GPIOB, 9

#define IO_SAI1_MCLK        STM32_GPIOB, 14 /* This pin is used as a GPIO */

#define SAI1_AF             GPIO_AF13

/* -------------------------------------------------------------------------
 * COMM UART
 * ------------------------------------------------------------------------- */
#define IO_USART3_TX        STM32_GPIOB, 10
#define IO_USART3_RX        STM32_GPIOB, 11

/* -------------------------------------------------------------------------
 * PMM UART
 * ------------------------------------------------------------------------- */
#define IO_USART1_TX        STM32_GPIOB, 6
#define IO_USART1_RX        STM32_GPIOB, 7

/* -------------------------------------------------------------------------
 * AMP UART
 * ------------------------------------------------------------------------- */
#define IO_USART2_RX        IO_JTDI

#if (HWREV >= 2)

/* -------------------------------------------------------------------------
 * LEDs
 * ------------------------------------------------------------------------- */
#define IO_LED1             STM32_GPIOH, 3

/* FPGA debug */
#define IO_4                STM32_GPIOB, 14

/* -------------------------------------------------------------------------
 * QSPI
 * ------------------------------------------------------------------------- */
#define IO_QSPI_CLK        STM32_GPIOA, 3
#define IO_QSPI_NCS        STM32_GPIOA, 2
#define IO_QSPI_IO0        STM32_GPIOB, 1
#define IO_QSPI_IO1        STM32_GPIOB, 0
#define IO_QSPI_IO2        STM32_GPIOA, 7
#define IO_QSPI_IO3        STM32_GPIOA, 6

/* -------------------------------------------------------------------------
 * ADC 
 * ------------------------------------------------------------------------- */

#define IO_ADC5            STM32_GPIOA, 0

/* -------------------------------------------------------------------------
 * Service mode jumper
 * ------------------------------------------------------------------------- */

#define IO_DBG_JMP         IO_JTCK /* This pin is shared with JTAG */
#define IO_5               IO_JTMS /* This can be used to drive an LED */

#else


#endif /* (HWREV == 2) */


/* -------------------------------------------------------------------------
 * ThinkOS application address
 * ------------------------------------------------------------------------- */

#include <stdbool.h>

static inline void __led_1_on(void) {
#if (HWREV >= 2)
	stm32_gpio_set(IO_LED1);
#endif
}

static inline void __led_1_off(void) {
#if (HWREV >= 2)
	stm32_gpio_clr(IO_LED1);
#endif
}

static inline void __io_1_set(void) {
	stm32_gpio_set(IO_1);
}

static inline void __io_1_clr(void) {
	stm32_gpio_clr(IO_1);
}

static inline void __io_1_toggle(void) {
	if (stm32_gpio_stat(IO_1)) stm32_gpio_clr(IO_1);
	else stm32_gpio_set(IO_1);
}

static inline bool __io_1_get(void) {
	return stm32_gpio_stat(IO_1) ? true : false;
}

static inline void __io_2_set(void) {
	stm32_gpio_set(IO_2);
}

static inline void __io_2_clr(void) {
	stm32_gpio_clr(IO_2);
}

static inline bool __io_2_get(void) {
	return stm32_gpio_stat(IO_2) ? true : false;
}


static inline void __io_3_set(void) {
	stm32_gpio_set(IO_ICE40_MODE);
}

static inline void __io_3_clr(void) {
	stm32_gpio_clr(IO_ICE40_MODE);
}

static inline bool __io_3_get(void) {
	return stm32_gpio_stat(IO_ICE40_MODE) ? true : false;
}

static inline void __io_4_set(void) {
	stm32_gpio_set(IO_4);
}

static inline void __io_4_clr(void) {
	stm32_gpio_clr(IO_4);
}

static inline void __io_4_toggle(void) {
	if (stm32_gpio_stat(IO_4)) stm32_gpio_clr(IO_4);
	else stm32_gpio_set(IO_4);
}

static inline bool __io_4_get(void) {
	return stm32_gpio_stat(IO_4) ? true : false;
}

static inline void __io_5_set(void) {
	stm32_gpio_set(IO_5);
}

static inline void __io_5_clr(void) {
	stm32_gpio_clr(IO_5);
}

static inline bool __io_5_get(void) {
	return stm32_gpio_stat(IO_5) ? true : false;
}

#ifdef __cplusplus
extern "C" {
#endif

int fpga_configure(void);

int spi_io_master_dma_init(unsigned int freq, uint16_t rx_buf[], 
						   uint16_t tx_buf[], unsigned int len);

void spi_io_enable(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

