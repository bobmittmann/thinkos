/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the libstm32f.
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
 * @file stm32f-pwr.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/* ------------------------------------------------------------------------- */
/* PWR - Power controller */
/* ------------------------------------------------------------------------- */

#ifndef __STM32_PWR_H__
#define __STM32_PWR_H__

/* ------------------------------------------------------------------------- */
/* PWR power control register - CR */
#define STM32_PWR_CR 0x0000

/* Bits [22..31] - Reserved, must be kept at reset value. */


#if defined(STM32F446)

/* Bit 21 - Flash Interface Stop while System Run */
#define PWR_FISSR (1 << 21)
/* 0: Flash interface clock run (Default value)
   1: Flash Interface clock off.
   Note: This bit could not be set while executing with the Flash itself. It
   should be done with a specific routine executed from RAM. */

/* Bit 20 - Flash Memory Stop while System Run */
#define PWR_FMSSR (1 << 20)
/* 0: Flash standard mode (Default value)
   1: Flash forced to be in STOP or Deep Power Down mode (depending of FPDS
   value bit) by hardware.
   Note: This bit could not be set while executing with the Flash itself. It
   should be done with a specific routine executed from RAM */

/* Bits [18..19] - Under-drive enable in stop mode */
#define PWR_UDEN_MSK (0x3 << 18)
#define PWR_UDEN_SET(VAL) (((VAL) << 18) & PWR_UDEN_MSK)
#define PWR_UDEN_GET(REG) (((REG) & PWR_UDEN_MSK) >> 18)
/* These bits are set by software. They allow to achieve a lower power
   consumption in Stop mode but with a longer wakeup time.
   When set, the digital area has less leakage consumption when the device
   enters Stop mode.
   00: Under-drive disable
   01: Reserved
   10: Reserved
   11:Under-drive enable */

/* Bit 17 - Over-drive switching enabled. */
#define PWR_ODSWEN (1 << 17)
/* This bit is set by software. It is cleared automatically by hardware after
   exiting from Stop mode or when the ODEN bit is reset. When set, It is used
   to switch to Over-drive mode.
   To set or reset the ODSWEN bit, the HSI or HSE must be selected as system
   clock.
   The ODSWEN bit must only be set when the ODRDY flag is set to switch to
   Over-drive mode.
   0: Over-drive switching disabled
   1: Over-drive switching enabled
   Note: On any over-drive switch (enabled or disabled), the system clock will
   be stalled during the internal voltage set up. */

/* Bit 16 - Over-drive enable */
#define PWR_ODEN (1 << 16)
/* This bit is set by software. It is cleared automatically by hardware after
   exiting from Stop mode. It is used to enabled the Over-drive mode in order
   to reach a higher frequency.
   To set or reset the ODEN bit, the HSI or HSE must be selected as system
   clock. When the ODEN bit is set, the application must first wait for the
   Over-drive ready flag (ODRDY) to be set before setting the ODSWEN bit.
   0: Over-drive disabled
   1: Over-drive enabled */

/* Bits [14..15] - Regulator voltage scaling output selection */
#define PWR_VOS_MSK (0x3 << 14)
#define PWR_VOS_SET(VAL) (((VAL) << 14) & PWR_VOS_MSK)
#define PWR_VOS_GET(REG) (((REG) & PWR_VOS_MSK) >> 14)
/* These bits control the main internal voltage regulator output voltage to
   achieve a trade-off between performance and power consumption when the
   device does not operate at the maximum frequency (refer to the STM32F446xx
   datasheet for more details).
   These bits can be modified only when the PLL is OFF. The new value
   programmed is active only when the PLL is ON. When the PLL is OFF, the
   voltage scale 3 is automatically selected.
   00: Reserved (Scale 3 mode selected)
   01: Scale 3 mode
   10: Scale 2 mode
   11: Scale 1 mode (reset value) */
   
#define PWR_VOS3 (1 << 14)
#define PWR_VOS2 (2 << 14)
#define PWR_VOS1 (3 << 14)

/* Bit 13 - ADCDC1 */
#define PWR_ADCDC1 (1 << 13)
/* 0: No effect.
   1: Refer to AN4073 for details on how to use this bit.
   Note: This bit can only be set when operating at supply voltage range 2.7
   to 3.6V and when the Prefetch is OFF. */

/* Bit 12 - Reserved, must be kept at reset value. */

/* Bit 11 - Main regulator in deepsleep under-drive mode */
#define PWR_MRUDS (1 << 11)
/* This bit is set and cleared by software.
   0: Main regulator ON when the device is in Stop mode
   1: Main Regulator in under-drive mode and Flash memory in power-down when
   the device is in Stop under-drive mode. */

/* Bit 10 - Low-power regulator in deepsleep under-drive mode */
#define PWR_LPUDS (1 << 10)
/* This bit is set and cleared by software.
   0: Low-power regulator ON if LPDS bit is set when the device is in Stop
   mode
   1: Low-power regulator in under-drive mode if LPDS bit is set and Flash
   memory in power- down when the device is in Stop under-drive mode. */

#endif /* defined(STM32F446X) */



/* Bit 9 - Flash power-down in Stop mode */
#define PWR_FPDS (1 << 9)
/* When set, the Flash memory enters power-down mode when the device enters
   Stop mode.
   This allows to achieve a lower consumption in stop mode but a longer
   restart time.
   0: Flash memory not in power-down when the device is in Stop mode
   1: Flash memory in power-down when the device is in Stop mode */

/* Bit 8 - Disable backup domain write protection */
#define PWR_DBP (1 << 8)
/* In reset state, the RCC_BDCR register, the RTC registers (including the
   backup registers), and the BRE bit of the PWR_CSR register, are protected
   against parasitic write access. This bit must be set to enable write access
   to these registers.
   0: Access to RTC and RTC Backup registers and backup SRAM disabled
   1: Access to RTC and RTC Backup registers and backup SRAM enabled */

/* Bits [5..7] - PVD level selection */
#define PWR_PLS_MSK (0x7 << 5)
#define PWR_PLS_SET(VAL) (((VAL) << 5) & PWR_PLS_MSK)
#define PWR_PLS_GET(REG) (((REG) & PWR_PLS_MSK) >> 5)
/* These bits are written by software to select the voltage threshold detected
   by the Power Voltage Detector
   000: 2.0 V
   001: 2.1 V
   010: 2.3 V
   011: 2.5 V
   100: 2.6 V
   101: 2.7 V
   110: 2.8 V
   111: 2.9 V
   Note: Refer to the electrical characteristics of the datasheet for more
   details. */

#define PWR_PLS (0x7 << 5)
#define PWR_PLS_2_0V (0x0 << 5)
#define PWR_PLS_2_1V (0x1 << 5)
#define PWR_PLS_2_3V (0x2 << 5)
#define PWR_PLS_2_5V (0x3 << 5)
#define PWR_PLS_2_6V (0x4 << 5)
#define PWR_PLS_2_7V (0x5 << 5)
#define PWR_PLS_2_8V (0x6 << 5)
#define PWR_PLS_2_9V (0x7 << 5)

/* Bit 4 - Power voltage detector enable */
#define PWR_PVDE (1 << 4)
/* This bit is set and cleared by software.
   0: PVD disabled
   1: PVD enabled */

/* Bit 3 - Clear standby flag */
#define PWR_CSBF (1 << 3)
/* This bit is always read as 0.
   0: No effect
   1: Clear the SBF Standby Flag (write). */

/* Bit 2 - Clear wakeup flag */
#define PWR_CWUF (1 << 2)
/* This bit is always read as 0.
   0: No effect
   1: Clear the WUF Wakeup Flag after 2 System clock cycles */

/* Bit 1 - Power-down deepsleep */
#define PWR_PDDS (1 << 1)
/* This bit is set and cleared by software. It works together with the LPDS
   bit.
   0: Enter Stop mode when the CPU enters deepsleep. The regulator status
   depends on the LPDS bit.
   1: Enter Standby mode when the CPU enters deepsleep. */

/* Bit 0 - Low-power deepsleep */
#define PWR_LPDS (1 << 0)
/* This bit is set and cleared by software. It works together with the PDDS
   bit.
   0:Main voltage regulator ON during Stop mode
   1: Low-power voltage regulator ON during Stop mode */

/* ------------------------------------------------------------------------- */
/* PWR power control/status register - CSR */
#define STM32_PWR_CSR 0x0004

/* Bits [20..31] - Reserved, must be kept at reset value. */

/* Bits [18..19] - Under-drive ready flag */
#define PWR_UDRDY_MSK (0x3 << 18)
#define PWR_UDRDY_SET(VAL) (((VAL) << 18) & PWR_UDRDY_MSK)
#define PWR_UDRDY_GET(REG) (((REG) & PWR_UDRDY_MSK) >> 18)
/* These bits are set by hardware when the Under-drive mode is enabled in Stop
   mode and cleared by programming them to 1.
   00: Under-drive is disabled
   01: Reserved
   10: Reserved
   11:Under-drive mode is activated in Stop mode. */

/* Bit 17 - Over-drive mode switching ready */
#define PWR_ODSWRDY (1 << 17)
/* 0: Over-drive mode is not active.
   1: Over-drive mode is active on digital area on 1.2 V domain */

/* Bit 16 - Over-drive mode ready */
#define PWR_ODRDY (1 << 16)
/* 0: Over-drive mode not ready.
   1: Over-drive mode ready */

/* Bit 14 - Regulator voltage scaling output selection ready bit */
#define PWR_VOSRDY (1 << 14)
/* 0: Not ready
   1: Ready */

/* Bits [10..13] - Reserved, must be kept at reset value. */

/* Bit 9 - Backup regulator enable */
#define PWR_BRE (1 << 9)
/* When set, the Backup regulator (used to maintain backup SRAM content in
   Standby and VBAT modes) is enabled. If BRE is reset, the backup regulator
   is switched off. The backup SRAM can still be used but its content will be
   lost in the Standby and VBAT modes. Once set, the application must wait
   that the Backup Regulator Ready flag (BRR) is set to indicate that the data
   written into the RAM will be maintained in the Standby and VBAT modes.
   0: Backup regulator disabled
   1: Backup regulator enabled
   Note: This bit is not reset when the device wakes up from Standby mode, by
   a system reset, or by a power reset. */

/* Bit 8 - Enable WKUP1 pin */
#define PWR_EWUP1 (1 << 8)
/* This bit is set and cleared by software.
   0: WKUP1 pin is used for general purpose I/O. An event on the WKUP1 pin
   does not wakeup the device from Standby mode.
   1: WKUP1 pin is used for wakeup from Standby mode and forced in input pull
   down configuration (rising edge on WKUP1 pin wakes-up the system from
   Standby mode).
   Note: This bit is reset by a system reset. */

/* Bit 7 - Enable WKUP2 pin */
#define PWR_EWUP2 (1 << 7)
/* This bit is set and cleared by software
   0: WKUP2 pin is used for general purpose I/O. An event on the WKUP2 pin
   does not wakeup the device from Standby mode.
   1: WKUP2 pin is used for wakeup from Standby mode and forced in input pull
   down configuration (rising edge on WKUP2 pin wakes-up the system from
   Standby mode).
   Note: This bit is reset by a system reset. */

/* Bits [4..7] - Reserved, must be kept at reset value. */

/* Bit 3 - Backup regulator ready */
#define PWR_BRR (1 << 3)
/* Set by hardware to indicate that the Backup Regulator is ready.
   0: Backup Regulator not ready
   1: Backup Regulator ready
   Note: This bit is not reset when the device wakes up from Standby mode or
   by a system reset or power reset. */

/* Bit 2 - PVD output */
#define PWR_PVDO (1 << 2)
/* This bit is set and cleared by hardware. It is valid only if PVD is enabled
   by the PVDE bit.
   0: VDD is higher than the PVD threshold selected with the PLS[2:0] bits.
   1: VDD is lower than the PVD threshold selected with the PLS[2:0] bits.
   Note: The PVD is stopped by Standby mode. For this reason, this bit is
   equal to 0 after Standby or reset until the PVDE bit is set. */

/* Bit 1 - Standby flag */
#define PWR_SBF (1 << 1)
/* This bit is set by hardware and cleared only by a POR/PDR (power-on
   reset/power-down reset) or by setting the CSBF bit in the PWR power control
   register (PWR_CR)
   0: Device has not been in Standby mode
   1: Device has been in Standby mode */

/* Bit 0 - Wakeup flag */
#define PWR_WUF (1 << 0)
/* This bit is set by hardware and cleared either by a system reset or by
   setting the CWUF bit in the PWR_CR register.
   0: No wakeup event occurred
   1: A wakeup event was received from the WKUP pin or from the RTC alarm
   (Alarm A or Alarm B), RTC Tamper event, RTC TimeStamp event or RTC Wakeup).
   Note: An additional wakeup event is detected if the WKUP pin is enabled (by
   setting the EWUP bit) when the WKUP pin level is already high.
   5.5 PWR register map The following table summarizes the PWR registers. */



#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_pwr {
	volatile uint32_t cr; /* 0x00 */
	volatile uint32_t csr; /* 0x04 */
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32_PWR_H__ */

