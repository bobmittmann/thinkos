# U2S-485 Board

This is a small USB to RS485 converter based on a STM32F103C6T6 MCU.

## Packaging

 - LQFP48

## CPU

 - Cortex-M3 @ 72MHz

## Memory

 - Flash - 32KiB
 - SRAM - 10 KiB

## IO

  PA0 - RS485 RX-EN
  PA1 - RS485 TX-EN
  PA2 - USART2 - TXD
  PA3 - USART2 - TXD
  PA6 - LED1
  PA7 - LED2
  
  
  
/* USB */
#define USB_FS_DP STM32_GPIOA, 12
#define USB_FS_DM STM32_GPIOA, 11
#define USB_FS_VBUS STM32_GPIOB, 6 /* PB6 */

/* UART */
#define USART2_RX STM32_GPIOA, 3
#define USART2_TX STM32_GPIOA, 2

/* RS485 */
#define RS485_RXEN STM32_GPIOA, 0
#define RS485_TXEN STM32_GPIOA, 1

/* LEDs */
#define LED1_IO STM32_GPIOA, 6
#define LED2_IO STM32_GPIOA, 7
