/* * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved. This file is
   part of the libstm32. This library is free software; you can redistribute it 
   and/or modify it under the terms of the GNU Lesser General Public License as 
   published by the Free Software Foundation; either version 3.0 of the
   License, or (at your option) any later version. This library is distributed
   in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details. You can receive a 
   copy of the GNU Lesser General Public License from http://www.gnu.org/ */

/** 
 * @file stm32-adc.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#ifndef __STM32L4_ADC_H__
#define __STM32L4_ADC_H__

/*-------------------------------------------------------------------------
 * Analog-to-digital converter (ADC)
 *-------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------- */
/* ADC interrupt and status register - ISR */
#define STM32_ADC_ISR 0x0000

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - Injected context queue overflow */
#define ADC_JQOVF (1 << 10)
/* This bit is set by hardware when an Overflow of the Injected Queue of
   Context occurs. It is cleared by software writing 1 to it. Refer to Section
   16.4.21: Queue of context for injected conversions for more information.
   0: No injected context queue overflow occurred (or the flag event was
   already acknowledged and cleared by software)
   1: Injected context queue overflow has occurred */

/* Bit 9 - Analog watchdog 3 flag */
#define ADC_AWD3 (1 << 9)
/* This bit is set by hardware when the converted voltage crosses the values
   programmed in the fields LT3[7:0] and HT3[7:0] of ADC_TR3 register. It is
   cleared by software writing 1 to it.
   0: No analog watchdog 3 event occurred (or the flag event was already
   acknowledged and cleared by software)
   1: Analog watchdog 3 event occurred */

/* Bit 8 - Analog watchdog 2 flag */
#define ADC_AWD2 (1 << 8)
/* This bit is set by hardware when the converted voltage crosses the values
   programmed in the fields LT2[7:0] and HT2[7:0] of ADC_TR2 register. It is
   cleared by software writing 1 to it.
   0: No analog watchdog 2 event occurred (or the flag event was already
   acknowledged and cleared by software)
   1: Analog watchdog 2 event occurred */

/* Bit 7 - Analog watchdog 1 flag */
#define ADC_AWD1 (1 << 7)
/* This bit is set by hardware when the converted voltage crosses the values
   programmed in the fields LT1[11:0] and HT1[11:0] of ADC_TR1 register. It is
   cleared by software. writing 1 to it.
   0: No analog watchdog 1 event occurred (or the flag event was already
   acknowledged and cleared by software)
   1: Analog watchdog 1 event occurred */

/* Bit 6 - Injected channel end of sequence flag */
#define ADC_JEOS (1 << 6)
/* This bit is set by hardware at the end of the conversions of all injected
   channels in the group. It is cleared by software writing 1 to it.
   0: Injected conversion sequence not complete (or the flag event was already
   acknowledged and cleared by software)
   1: Injected conversions complete */

/* Bit 5 - Injected channel end of conversion flag */
#define ADC_JEOC (1 << 5)
/* This bit is set by hardware at the end of each injected conversion of a
   channel when a new data is available in the corresponding ADC_JDRy
   register. It is cleared by software writing 1 to it or by reading the
   corresponding ADC_JDRy register
   0: Injected channel conversion not complete (or the flag event was already
   acknowledged and cleared by software)
   1: Injected channel conversion complete */

/* Bit 4 - ADC overrun */
#define ADC_OVR (1 << 4)
/* This bit is set by hardware when an overrun occurs on a regular channel,
   meaning that a new conversion has completed while the EOC flag was already
   set. It is cleared by software writing 1 to it.
   0: No overrun occurred (or the flag event was already acknowledged and
   cleared by software)
   1: Overrun has occurred */

/* Bit 3 - End of regular sequence flag */
#define ADC_EOS (1 << 3)
/* This bit is set by hardware at the end of the conversions of a regular
   sequence of channels. It is cleared by software writing 1 to it.
   0: Regular Conversions sequence not complete (or the flag event was already
   acknowledged and cleared by software)
   1: Regular Conversions sequence complete */

/* Bit 2 - End of conversion flag */
#define ADC_EOC (1 << 2)
/* This bit is set by hardware at the end of each regular conversion of a
   channel when a new data is available in the ADC_DR register. It is cleared
   by software writing 1 to it or by reading the ADC_DR register
   0: Regular channel conversion not complete (or the flag event was already
   acknowledged and cleared by software)
   1: Regular channel conversion complete */

/* Bit 1 - End of sampling flag */
#define ADC_EOSMP (1 << 1)
/* This bit is set by hardware during the conversion of any channel (only for
   regular channels), at the end of the sampling phase.
   0: not at the end of the sampling phase (or the flag event was already
   acknowledged and cleared by software)
   1: End of sampling phase reached */

/* Bit 0 - ADC ready */
#define ADC_ADRDY (1 << 0)
/* This bit is set by hardware after the ADC has been enabled (bit ADEN=1) and
   when the ADC reaches a state where it is ready to accept conversion
   requests.
   It is cleared by software writing 1 to it.
   0: ADC not yet ready to start conversion (or the flag event was already
   acknowledged and cleared by software)
   1: ADC is ready to start conversion */

/* ------------------------------------------------------------------------- */
/* ADC interrupt enable register - IER */
#define STM32_ADC_IER 0x0004

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - Injected context queue overflow interrupt enable */
#define ADC_JQOVFIE (1 << 10)
/* This bit is set and cleared by software to enable/disable the Injected
   Context Queue Overflow interrupt.
   0: Injected Context Queue Overflow interrupt disabled
   1: Injected Context Queue Overflow interrupt enabled. An interrupt is
   generated when the JQOVF bit is set.
   Note: Software is allowed to write this bit only when JADSTART=0 (which
   ensures that no injected conversion is ongoing). */

/* Bit 9 - Analog watchdog 3 interrupt enable */
#define ADC_AWD3IE (1 << 9)
/* This bit is set and cleared by software to enable/disable the analog
   watchdog 2 interrupt.
   0: Analog watchdog 3 interrupt disabled
   1: Analog watchdog 3 interrupt enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bit 8 - Analog watchdog 2 interrupt enable */
#define ADC_AWD2IE (1 << 8)
/* This bit is set and cleared by software to enable/disable the analog
   watchdog 2 interrupt.
   0: Analog watchdog 2 interrupt disabled
   1: Analog watchdog 2 interrupt enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bit 7 - Analog watchdog 1 interrupt enable */
#define ADC_AWD1IE (1 << 7)
/* This bit is set and cleared by software to enable/disable the analog
   watchdog 1 interrupt.
   0: Analog watchdog 1 interrupt disabled
   1: Analog watchdog 1 interrupt enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bit 6 - End of injected sequence of conversions interrupt enable */
#define ADC_JEOSIE (1 << 6)
/* This bit is set and cleared by software to enable/disable the end of
   injected sequence of conversions interrupt.
   0: JEOS interrupt disabled
   1: JEOS interrupt enabled. An interrupt is generated when the JEOS bit is
   set.
   Note: Software is allowed to write this bit only when JADSTART=0 (which
   ensures that no injected conversion is ongoing). */

/* Bit 5 - End of injected conversion interrupt enable */
#define ADC_JEOCIE (1 << 5)
/* This bit is set and cleared by software to enable/disable the end of an
   injected conversion interrupt.
   0: JEOC interrupt disabled.
   1: JEOC interrupt enabled. An interrupt is generated when the JEOC bit is
   set.
   Note: Software is allowed to write this bit only when JADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 4 - Overrun interrupt enable */
#define ADC_OVRIE (1 << 4)
/* This bit is set and cleared by software to enable/disable the Overrun
   interrupt of a regular conversion.
   0: Overrun interrupt disabled
   1: Overrun interrupt enabled. An interrupt is generated when the OVR bit is
   set.
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 3 - End of regular sequence of conversions interrupt enable */
#define ADC_EOSIE (1 << 3)
/* This bit is set and cleared by software to enable/disable the end of
   regular sequence of conversions interrupt.
   0: EOS interrupt disabled
   1: EOS interrupt enabled. An interrupt is generated when the EOS bit is
   set.
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 2 - End of regular conversion interrupt enable */
#define ADC_EOCIE (1 << 2)
/* This bit is set and cleared by software to enable/disable the end of a
   regular conversion interrupt.
   0: EOC interrupt disabled.
   1: EOC interrupt enabled. An interrupt is generated when the EOC bit is
   set.
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 1 - End of sampling flag interrupt enable for regular conversions */
#define ADC_EOSMPIE (1 << 1)
/* This bit is set and cleared by software to enable/disable the end of the
   sampling phase interrupt for regular conversions.
   0: EOSMP interrupt disabled.
   1: EOSMP interrupt enabled. An interrupt is generated when the EOSMP bit is
   set.
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 0 - ADC ready interrupt enable */
#define ADC_ADRDYIE (1 << 0)
/* This bit is set and cleared by software to enable/disable the ADC Ready
   interrupt.
   0: ADRDY interrupt disabled
   1: ADRDY interrupt enabled. An interrupt is generated when the ADRDY bit is
   set.
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC control register - CR */
#define STM32_ADC_CR 0x0008

/* Bit 31 - ADC calibration */
#define ADC_ADCAL (1 << 31)
/* This bit is set by software to start the calibration of the ADC. Program
   first the bit ADCALDIF to determine if this calibration applies for
   single-ended or differential inputs mode.
   It is cleared by hardware after calibration is complete.
   0: Calibration complete
   1: Write 1 to calibrate the ADC. Read at 1 means that a calibration in
   progress.
   Note: Software is allowed to launch a calibration by setting ADCAL only
   when ADEN=0.
   Note: Software is allowed to update the calibration factor by writing
   ADC_CALFACT only when ADEN=1 and ADSTART=0 and JADSTART=0 (ADC enabled and
   no conversion is ongoing) */

/* Bit 30 - Differential mode for calibration */
#define ADC_ADCALDIF (1 << 30)
/* This bit is set and cleared by software to configure the single-ended or
   differential inputs mode for the calibration.
   0: Writing ADCAL will launch a calibration in single-ended inputs mode.
   1: Writing ADCAL will launch a calibration in differential inputs mode.
   Note: Software is allowed to write this bit only when the ADC is disabled
   and is not calibrating (ADCAL=0, JADSTART=0, JADSTP=0, ADSTART=0, ADSTP=0,
   ADDIS=0 and ADEN=0). */

/* Bit 29 - Deep-power-down enable */
#define ADC_DEEPPWD (1 << 29)
/* This bit is set and cleared by software to put the ADC in Deep-power-down
   mode.
   0: ADC not in Deep-power down
   1: ADC in Deep-power-down (default reset state)
   Note: Software is allowed to write this bit only when the ADC is disabled
   (ADCAL=0, JADSTART=0, JADSTP=0, ADSTART=0, ADSTP=0, ADDIS=0 and ADEN=0). */

/* Bit 28 - ADC voltage regulator enable */
#define ADC_ADVREGEN (1 << 28)
/* This bits is set by software to enable the ADC voltage regulator.
   Before performing any operation such as launching a calibration or enabling
   the ADC, the ADC voltage regulator must first be enabled and the software
   must wait for the regulator start-up time.
   0: ADC Voltage regulator disabled
   1: ADC Voltage regulator enabled.
   For more details about the ADC voltage regulator enable and disable
   sequences, refer to Section 16.4.6: ADC Deep-power-down mode (DEEPPWD) &
   ADC Voltage Regulator (ADVREGEN).
   The software can program this bit field only when the ADC is disabled
   (ADCAL=0, JADSTART=0, ADSTART=0, ADSTP=0, ADDIS=0 and ADEN=0). */

/* Bits [6..27] - Reserved, must be kept at reset value. */

/* Bit 5 - ADC stop of injected conversion command */
#define ADC_JADSTP (1 << 5)
/* This bit is set by software to stop and discard an ongoing injected
   conversion (JADSTP Command).
   It is cleared by hardware when the conversion is effectively discarded and
   the ADC injected sequence and triggers can be re-configured. The ADC is
   then ready to accept a new start of injected conversions (JADSTART
   command).
   0: No ADC stop injected conversion command ongoing
   1: Write 1 to stop injected conversions ongoing. Read 1 means that an ADSTP
   command is in progress.
   Note: Software is allowed to set JADSTP only when JADSTART=1 and ADDIS=0
   (ADC is enabled and eventually converting an injected conversion and there
   is no pending request to disable the ADC) In auto-injection mode (JAUTO=1),
   setting ADSTP bit aborts both regular and injected conversions (do not use
   JADSTP) */

/* Bit 4 - ADC stop of regular conversion command */
#define ADC_ADSTP (1 << 4)
/* This bit is set by software to stop and discard an ongoing regular
   conversion (ADSTP Command).
   It is cleared by hardware when the conversion is effectively discarded and
   the ADC regular sequence and triggers can be re-configured. The ADC is then
   ready to accept a new start of regular conversions (ADSTART command).
   0: No ADC stop regular conversion command ongoing
   1: Write 1 to stop regular conversions ongoing. Read 1 means that an ADSTP
   command is in progress.
   Note: Software is allowed to set ADSTP only when ADSTART=1 and ADDIS=0 (ADC
   is enabled and eventually converting a regular conversion and there is no
   pending request to disable the ADC).
   In auto-injection mode (JAUTO=1), setting ADSTP bit aborts both regular and
   injected conversions (do not use JADSTP). */

/* Bit 3 - ADC start of injected conversion */
#define ADC_JADSTART (1 << 3)
/* This bit is set by software to start ADC conversion of injected channels.
   Depending on the configuration bits JEXTEN, a conversion will start
   immediately (software trigger configuration) or once an injected hardware
   trigger event occurs (hardware trigger configuration).
   It is cleared by hardware: – in single conversion mode when software
   trigger is selected (JEXTSEL=0x0): at the assertion of the End of Injected
   Conversion Sequence (JEOS) flag.
   – in all cases: after the execution of the JADSTP command, at the same time
   that JADSTP is cleared by hardware.
   0: No ADC injected conversion is ongoing.
   1: Write 1 to start injected conversions. Read 1 means that the ADC is
   operating and eventually converting an injected channel.
   Note: Software is allowed to set JADSTART only when ADEN=1 and ADDIS=0 (ADC
   is enabled and there is no pending request to disable the ADC).
   In auto-injection mode (JAUTO=1), regular and auto-injected conversions are
   started by setting bit ADSTART (JADSTART must be kept cleared) */

/* Bit 2 - ADC start of regular conversion */
#define ADC_ADSTART (1 << 2)
/* This bit is set by software to start ADC conversion of regular channels.
   Depending on the configuration bits EXTEN, a conversion will start
   immediately (software trigger configuration) or once a regular hardware
   trigger event occurs (hardware trigger configuration).
   It is cleared by hardware: – in single conversion mode when software
   trigger is selected (EXTSEL=0x0): at the assertion of the End of Regular
   Conversion Sequence (EOS) flag.
   – in all cases: after the execution of the ADSTP command, at the same time
   that ADSTP is cleared by hardware.
   0: No ADC regular conversion is ongoing.
   1: Write 1 to start regular conversions. Read 1 means that the ADC is
   operating and eventually converting a regular channel.
   Note: Software is allowed to set ADSTART only when ADEN=1 and ADDIS=0 (ADC
   is enabled and there is no pending request to disable the ADC) In
   auto-injection mode (JAUTO=1), regular and auto-injected conversions are
   started by setting bit ADSTART (JADSTART must be kept cleared) */

/* Bit 1 - ADC disable command */
#define ADC_ADDIS (1 << 1)
/* This bit is set by software to disable the ADC (ADDIS command) and put it
   into power-down state (OFF state).
   It is cleared by hardware once the ADC is effectively disabled (ADEN is
   also cleared by hardware at this time).
   0: no ADDIS command ongoing
   1: Write 1 to disable the ADC. Read 1 means that an ADDIS command is in
   progress.
   Note: Software is allowed to set ADDIS only when ADEN=1 and both ADSTART=0
   and JADSTART=0 (which ensures that no conversion is ongoing) */

/* Bit 0 - ADC enable control */
#define ADC_ADEN (1 << 0)
/* This bit is set by software to enable the ADC. The ADC will be effectively
   ready to operate once the flag ADRDY has been set.
   It is cleared by hardware when the ADC is disabled, after the execution of
   the ADDIS command.
   0: ADC is disabled (OFF state)
   1: Write 1 to enable the ADC.
   Note: Software is allowed to set ADEN only when all bits of ADC_CR
   registers are 0 (ADCAL=0, JADSTART=0, ADSTART=0, ADSTP=0, ADDIS=0 and
   ADEN=0) except for bit ADVREGEN which must be 1 (and the software must have
   wait for the startup time of the voltage regulator) */

/* ------------------------------------------------------------------------- */
/* ADC configuration register - CFGR */
#define STM32_ADC_CFGR 0x000c

/* Bit 31 - Injected Queue disable */
#define ADC_JQDIS (1 << 31)
/* These bits are set and cleared by software to disable the Injected Queue
   mechanism :
   0: Injected Queue enabled
   1: Injected Queue disabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no regular nor injected conversion is
   ongoing).
   A set or reset of JQDIS bit causes the injected queue to be flushed and the
   JSQR register is cleared. */

/* Bits [26..30] - Analog watchdog 1 channel selection */
#define ADC_AWD1CH (0x1f << 26)
#define ADC_AWD1CH_SET(VAL) (((VAL) << 26) & ADC_AWD1CH)
#define ADC_AWD1CH_GET(REG) (((REG) & ADC_AWD1CH) >> 26)
/* These bits are set and cleared by software. They select the input channel
   to be guarded by the analog watchdog.
   00000: ADC analog input channel-0 monitored by AWD1 (available on ADC1
   only)
   00001: ADC analog input channel-1 monitored by AWD1 .....
   10010: ADC analog input channel-18 monitored by AWD1
   others: reserved, must not be used
   Note: The channel selected by AWD1CH must be also selected into the SQRi or
   JSQRi registers.
   Software is allowed to write these bits only when ADSTART=0 and JADSTART=0
   (which ensures that no conversion is ongoing). */

/* Bit 25 - Automatic injected group conversion */
#define ADC_JAUTO (1 << 25)
/* This bit is set and cleared by software to enable/disable automatic
   injected group conversion after regular group conversion.
   0: Automatic injected group conversion disabled
   1: Automatic injected group conversion enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no regular nor injected conversion is
   ongoing). */

/* Bit 24 - Analog watchdog 1 enable on injected channels */
#define ADC_JAWD1EN (1 << 24)
/* This bit is set and cleared by software
   0: Analog watchdog 1 disabled on injected channels
   1: Analog watchdog 1 enabled on injected channels
   Note: Software is allowed to write this bit only when JADSTART=0 (which
   ensures that no injected conversion is ongoing). */

/* Bit 23 - Analog watchdog 1 enable on regular channels */
#define ADC_AWD1EN (1 << 23)
/* This bit is set and cleared by software
   0: Analog watchdog 1 disabled on regular channels
   1: Analog watchdog 1 enabled on regular channels
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 22 - Enable the watchdog 1 on a single channel or on all channels */
#define ADC_AWD1SGL (1 << 22)
/* This bit is set and cleared by software to enable the analog watchdog on
   the channel identified by the AWD1CH[4:0] bits or on all the channels
   0: Analog watchdog 1 enabled on all channels
   1: Analog watchdog 1 enabled on a single channel
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bit 21 - JSQR queue mode */
#define ADC_JQM (1 << 21)
/* This bit is set and cleared by software.
   It defines how an empty Queue is managed.
   0: JSQR mode 0: The Queue is never empty and maintains the last written
   configuration into JSQR.
   1: JSQR mode 1: The Queue can be empty and when this occurs, the software
   and hardware triggers of the injected sequence are both internally disabled
   just after the completion of the last valid injected sequence.
   Refer to Section 16.4.21: Queue of context for injected conversions for
   more information.
   Note: Software is allowed to write this bit only when JADSTART=0 (which
   ensures that no injected conversion is ongoing). */

/* Bit 20 - Discontinuous mode on injected channels */
#define ADC_JDISCEN (1 << 20)
/* This bit is set and cleared by software to enable/disable discontinuous
   mode on the injected channels of a group.
   0: Discontinuous mode on injected channels disabled
   1: Discontinuous mode on injected channels enabled
   Note: Software is allowed to write this bit only when JADSTART=0 (which
   ensures that no injected conversion is ongoing).
   It is not possible to use both auto-injected mode and discontinuous mode
   simultaneously: the bits DISCEN and JDISCEN must be kept cleared by
   software when JAUTO is set. */

/* Bits [17..19] - Discontinuous mode channel count */
#define ADC_DISCNUM_MSK (0x7 << 17)
#define ADC_DISCNUM_SET(VAL) ((((VAL)- 1) << 17) & ADC_DISCNUM_MSK)
#define ADC_DISCNUM_GET(REG) ((((REG) & ADC_DISCNUM_MSK) >> 17) + 1)

/* These bits are written by software to define the number of regular channels
   to be converted in discontinuous mode, after receiving an external trigger.
   000: 1 channel
   001: 2 channels ...
   111: 8 channels
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 16 - Discontinuous mode for regular channels */
#define ADC_DISCEN (1 << 16)
/* This bit is set and cleared by software to enable/disable Discontinuous
   mode for regular channels.
   0: Discontinuous mode for regular channels disabled
   1: Discontinuous mode for regular channels enabled
   Note: It is not possible to have both discontinuous mode and continuous
   mode enabled: it is forbidden to set both DISCEN=1 and CONT=1.
   It is not possible to use both auto-injected mode and discontinuous mode
   simultaneously: the bits DISCEN and JDISCEN must be kept cleared by
   software when JAUTO is set.
   Software is allowed to write this bit only when ADSTART=0 (which ensures
   that no regular conversion is ongoing). */

/* Bit 15 - Reserved, must be kept at reset value. */

/* Bit 14 - Delayed conversion mode */
#define ADC_AUTDLY (1 << 14)
/* This bit is set and cleared by software to enable/disable the Auto Delayed
   Conversion mode..
   0: Auto-delayed conversion mode off
   1: Auto-delayed conversion mode on
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bit 13 - Single / continuous conversion mode for regular conversions */
#define ADC_CONT (1 << 13)
/* This bit is set and cleared by software. If it is set, regular conversion
   takes place continuously until it is cleared.
   0: Single conversion mode
   1: Continuous conversion mode
   Note: It is not possible to have both discontinuous mode and continuous
   mode enabled: it is forbidden to set both DISCEN=1 and CONT=1.
   Software is allowed to write this bit only when ADSTART=0 (which ensures
   that no regular conversion is ongoing). */

/* Bit 12 - Overrun mode */
#define ADC_OVRMOD (1 << 12)
/* This bit is set and cleared by software and configure the way data overrun
   is managed.
   0: ADC_DR register is preserved with the old data when an overrun is
   detected.
   1: ADC_DR register is overwritten with the last conversion result when an
   overrun is detected.
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bits [10..11] - External trigger enable and polarity selection for regular channels */
#define ADC_EXTEN (0x3 << 10)
#define ADC_EXTEN_SET(VAL) (((VAL) << 10) & ADC_EXTEN)
#define ADC_EXTEN_GET(REG) (((REG) & ADC_EXTEN) >> 10)
/* These bits are set and cleared by software to select the external trigger
   polarity and enable the trigger of a regular group.
   00: Hardware trigger detection disabled (conversions can be launched by
   software)
   01: Hardware trigger detection on the rising edge
   10: Hardware trigger detection on the falling edge
   11: Hardware trigger detection on both the rising and falling edges
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */
#define ADC_EXTEN_RISING  (1 << 10)
#define ADC_EXTEN_FALLING (2 << 10)
#define ADC_EXTEN_BOTH    (3 << 10)

/* Bits [6..9] - External trigger selection for regular group */
#define ADC_EXTSEL (0xf << 6)
#define ADC_EXTSEL_SET(VAL) (((VAL) << 6) & ADC_EXTSEL)
#define ADC_EXTSEL_GET(REG) (((REG) & ADC_EXTSEL) >> 6)
/* These bits select the external event used to trigger the start of
   conversion of a regular group:
   0000: Event 0
   0001: Event 1
   0010: Event 2
   0011: Event 3
   0100: Event 4
   0101: Event 5
   0110: Event 6
   0111: Event 7 ...
   1111: Event 15
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */
#define ADC_EXTSEL_TIM1_CH1   (0x0 << 6)
#define ADC_EXTSEL_TIM1_CH2   (0x1 << 6)
#define ADC_EXTSEL_TIM1_CH3   (0x2 << 6)
#define ADC_EXTSEL_TIM2_CH2   (0x3 << 6)
#define ADC_EXTSEL_TIM3_TRGO  (0x4 << 6)
#define ADC_EXTSEL_EXTI       (0x6 << 6)
#define ADC_EXTSEL_TIM1_TRGO  (0x9 << 6)
#define ADC_EXTSEL_TIM1_TRGO2 (0xa << 6)
#define ADC_EXTSEL_TIM2_TRGO  (0xb << 6)
#define ADC_EXTSEL_TIM6_TRGO  (0xd << 6)
#define ADC_EXTSEL_TIM15_TRGO (0xe << 6)

/* Bit 5 - Data alignment */
#define ADC_ALIGN (1 << 5)
/* This bit is set and cleared by software to select right or left alignment.
   Refer to Section : Data register, data alignment and offset (ADC_DR,
   OFFSETy, OFFSETy_CH, ALIGN)
   0: Right alignment
   1: Left alignment
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bits [3..4] - Data resolution */
#define ADC_RES (0x3 << 3)
#define ADC_RES_SET(VAL) (((VAL) << 3) & ADC_RES)
#define ADC_RES_GET(REG) (((REG) & ADC_RES) >> 3)
/* These bits are written by software to select the resolution of the
   conversion.
   00: 12-bit
   01: 10-bit
   10: 8-bit
   11: 6-bit
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

#define ADC_RES_12BIT (0 << 3)
#define ADC_RES_10BIT (1 << 3)
#define ADC_RES_8BIT  (2 << 3)
#define ADC_RES_6BIT  (3 << 3)

/* Bit 2 - DFSDM mode configuration */
#define ADC_DFSDMCFG (1 << 2)
/* This bit is set and cleared by software to enable the DFSDM mode. It is
   effective only when DMAEN=0.
   0: DFSDM mode disabled
   1: DFSDM mode enabled
   Note: To make sure no conversion is ongoing, the software is allowed to
   write this bit only when ADSTART= 0 and JADSTART= 0. */

/* Bit 1 - Direct memory access configuration */
#define ADC_DMACFG (1 << 1)
/* This bit is set and cleared by software to select between two DMA modes of
   operation and is effective only when DMAEN=1.
   0: DMA One Shot mode selected
   1: DMA Circular mode selected For more details, refer to Section : Managing
   conversions using the DMA
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */
#define ADC_DMACFG_CIRC (1 << 1)

/* Bit 0 - Direct memory access enable */
#define ADC_DMAEN (1 << 0)
/* This bit is set and cleared by software to enable the generation of DMA
   requests. This allows to use the GP-DMA to manage automatically the
   converted data. For more details, refer to Section : Managing conversions
   using the DMA.
   0: DMA disabled
   1: DMA enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC configuration register 2 - CFGR2 */
#define STM32_ADC_CFGR2 0x0010

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - Regular Oversampling mode */
#define ADC_ROVSM (1 << 10)
/* This bit is set and cleared by software to select the regular oversampling
   mode.
   0: Continued mode: When injected conversions are triggered, the
   oversampling is temporary stopped and continued after the injection
   sequence (oversampling buffer is maintained during injected sequence)
   1: Resumed mode: When injected conversions are triggered, the current
   oversampling is aborted and resumed from start after the injection sequence
   (oversampling buffer is zeroed by injected sequence start)
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no conversion is ongoing). */

/* Bit 9 - Triggered Regular Oversampling */
#define ADC_TROVS (1 << 9)
/* This bit is set and cleared by software to enable triggered oversampling
   0: All oversampled conversions for a channel are done consecutively
   following a trigger
   1: Each oversampled conversion for a channel needs a new trigger
   Note: Software is allowed to write this bit only when ADSTART=0 (which
   ensures that no conversion is ongoing). */

/* Bits [5..8] - Oversampling shift */
#define ADC_OVSS (0xf << 5)
#define ADC_OVSS_SET(VAL) (((VAL) << 5) & ADC_OVSS)
#define ADC_OVSS_GET(REG) (((REG) & ADC_OVSS) >> 5)
/* This bitfield is set and cleared by software to define the right shifting
   applied to the raw oversampling result.
   0000: No shift
   0001: Shift 1-bit
   0010: Shift 2-bits
   0011: Shift 3-bits
   0100: Shift 4-bits
   0101: Shift 5-bits
   0110: Shift 6-bits
   0111: Shift 7-bits
   1000: Shift 8-bits Other codes reserved
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no conversion is ongoing). */

/* Bits [2..4] - Oversampling ratio */
#define ADC_OVSR (0x7 << 2)
#define ADC_OVSR_SET(VAL) (((VAL) << 2) & ADC_OVSR)
#define ADC_OVSR_GET(REG) (((REG) & ADC_OVSR) >> 2)
/* This bitfield is set and cleared by software to define the oversampling
   ratio.
   000: 2x
   001: 4x
   010: 8x
   011: 16x
   100: 32x
   101: 64x
   110: 128x
   111: 256x
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no conversion is ongoing). */
#define ADC_OVSR_2   (0 << 2)
#define ADC_OVSR_4   (1 << 2)
#define ADC_OVSR_8   (2 << 2)
#define ADC_OVSR_16  (3 << 2)
#define ADC_OVSR_32  (4 << 2)
#define ADC_OVSR_64  (6 << 2)
#define ADC_OVSR_128 (6 << 2)
#define ADC_OVSR_256 (7 << 2)

/* Bit 1 - Injected Oversampling Enable */
#define ADC_JOVSE (1 << 1)
/* This bit is set and cleared by software to enable injected oversampling.
   0: Injected Oversampling disabled
   1: Injected Oversampling enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing) */

/* Bit 0 - Regular Oversampling Enable */
#define ADC_ROVSE (1 << 0)
/* This bit is set and cleared by software to enable regular oversampling.
   0: Regular Oversampling disabled
   1: Regular Oversampling enabled
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing) */

/* ------------------------------------------------------------------------- */
/* ADC sample time register 1 - SMPR1 */
#define STM32_ADC_SMPR1 0x0014

/* Bits [30..31] - Reserved, must be kept at reset value. */

/* Bits [0..29] - Channel x sampling time selection */
#define ADC_SMPX (0x3fffffff << 0)
#define ADC_SMPX_SET(VAL) (((VAL) << 0) & ADC_SMPX)
#define ADC_SMPX_GET(REG) (((REG) & ADC_SMPX) >> 0)
/* These bits are written by software to select the sampling time individually
   for each channel.
   During sample cycles, the channel selection bits must remain unchanged.
   000: 2.5 ADC clock cycles
   001: 6.5 ADC clock cycles
   010: 12.5 ADC clock cycles
   011: 24.5 ADC clock cycles
   100: 47.5 ADC clock cycles
   101: 92.5 ADC clock cycles
   110: 247.5 ADC clock cycles
   111: 640.5 ADC clock cycles
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

#define ADC_SMP_MSK(X) (0x7 << (3 * (X)))
#define ADC_SMP_SET(X, VAL) (((VAL) << (3 * (X))) & ADC_SMP_MSK(X))
#define ADC_SMP_GET(X, REG) (((REG) & ADC_SMP_MSK(X)) >> (3 * (X)))

#define ADC_SMP_3_CYC   0
#define ADC_SMP_7_CYC   1
#define ADC_SMP_13_CYC  2
#define ADC_SMP_25_CYC  3
#define ADC_SMP_48_CYC  4
#define ADC_SMP_93_CYC  5
#define ADC_SMP_248_CYC 6
#define ADC_SMP_641_CYC 7

/* ------------------------------------------------------------------------- */
/* ADC sample time register 2 - SMPR2 */
#define STM32_ADC_SMPR2 0x0018

/* Bits [27..31] - Reserved, must be kept at reset value. */
#if 0
/* Bits [0..26] - Channel x sampling time selection */
#define ADC_SMPX (0x7ffffff << 0)
#define ADC_SMPX_SET(VAL) (((VAL) << 0) & ADC_SMPX)
#define ADC_SMPX_GET(REG) (((REG) & ADC_SMPX) >> 0)
/* These bits are written by software to select the sampling time individually
   for each channel.
   During sampling cycles, the channel selection bits must remain unchanged.
   000: 2.5 ADC clock cycles
   001: 6.5 ADC clock cycles
   010: 12.5 ADC clock cycles
   011: 24.5 ADC clock cycles
   100: 47.5 ADC clock cycles
   101: 92.5 ADC clock cycles
   110: 247.5 ADC clock cycles
   111: 640.5 ADC clock cycles
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */
#endif

/* ------------------------------------------------------------------------- */
/* ADC watchdog threshold register 1 - TR1 */
#define STM32_ADC_TR1 0x0020

/* Bits [28..31] - Reserved, must be kept at reset value. */

/* Bits [16..27] - Analog watchdog 1 higher threshold */
#define ADC_HT1 (0xfff << 16)
#define ADC_HT1_SET(VAL) (((VAL) << 16) & ADC_HT1)
#define ADC_HT1_GET(REG) (((REG) & ADC_HT1) >> 16)
/* These bits are written by software to define the higher threshold for the
   analog watchdog 1.
   Refer to Section 16.4.29: Analog window watchdog (AWD1EN, JAWD1EN, AWD1SGL,
   AWD1CH, AWD2CH, AWD3CH, AWD_HTx, AWD_LTx, AWDx)
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bits [12..15] - Reserved, must be kept at reset value. */

/* Bits [0..11] - Analog watchdog 1 lower threshold */
#define ADC_LT1 (0xfff << 0)
#define ADC_LT1_SET(VAL) (((VAL) << 0) & ADC_LT1)
#define ADC_LT1_GET(REG) (((REG) & ADC_LT1) >> 0)
/* These bits are written by software to define the lower threshold for the
   analog watchdog 1.
   Refer to Section 16.4.29: Analog window watchdog (AWD1EN, JAWD1EN, AWD1SGL,
   AWD1CH, AWD2CH, AWD3CH, AWD_HTx, AWD_LTx, AWDx)
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC watchdog threshold register 2 - TR2 */
#define STM32_ADC_TR2 0x0024

/* Bits [24..31] - Reserved, must be kept at reset value. */

/* Bits [16..23] - Analog watchdog 2 higher threshold */
#define ADC_HT2 (0xff << 16)
#define ADC_HT2_SET(VAL) (((VAL) << 16) & ADC_HT2)
#define ADC_HT2_GET(REG) (((REG) & ADC_HT2) >> 16)
/* These bits are written by software to define the higher threshold for the
   analog watchdog 2.
   Refer to Section 16.4.29: Analog window watchdog (AWD1EN, JAWD1EN, AWD1SGL,
   AWD1CH, AWD2CH, AWD3CH, AWD_HTx, AWD_LTx, AWDx)
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bits [8..15] - Reserved, must be kept at reset value. */

/* Bits [0..7] - Analog watchdog 2 lower threshold */
#define ADC_LT2 (0xff << 0)
#define ADC_LT2_SET(VAL) (((VAL) << 0) & ADC_LT2)
#define ADC_LT2_GET(REG) (((REG) & ADC_LT2) >> 0)
/* These bits are written by software to define the lower threshold for the
   analog watchdog 2.
   Refer to Section 16.4.29: Analog window watchdog (AWD1EN, JAWD1EN, AWD1SGL,
   AWD1CH, AWD2CH, AWD3CH, AWD_HTx, AWD_LTx, AWDx)
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC watchdog threshold register 3 - TR3 */
#define STM32_ADC_TR3 0x0028

/* Bits [24..31] - Reserved, must be kept at reset value. */

/* Bits [16..23] - Analog watchdog 3 higher threshold */
#define ADC_HT3 (0xff << 16)
#define ADC_HT3_SET(VAL) (((VAL) << 16) & ADC_HT3)
#define ADC_HT3_GET(REG) (((REG) & ADC_HT3) >> 16)
/* These bits are written by software to define the higher threshold for the
   analog watchdog 3.
   Refer to Section 16.4.29: Analog window watchdog (AWD1EN, JAWD1EN, AWD1SGL,
   AWD1CH, AWD2CH, AWD3CH, AWD_HTx, AWD_LTx, AWDx)
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bits [8..15] - Reserved, must be kept at reset value. */

/* Bits [0..7] - Analog watchdog 3 lower threshold */
#define ADC_LT3 (0xff << 0)
#define ADC_LT3_SET(VAL) (((VAL) << 0) & ADC_LT3)
#define ADC_LT3_GET(REG) (((REG) & ADC_LT3) >> 0)
/* These bits are written by software to define the lower threshold for the
   analog watchdog 3.
   This watchdog compares the 8-bit of LT3 with the 8 MSB of the converted
   data.
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC regular sequence register 1 - SQR1 */
#define STM32_ADC_SQR1 0x0030

/* Bits [29..31] - Reserved, must be kept at reset value. */

/* Bits [24..28] - 4th conversion in regular sequence */
#define ADC_SQ4 (0x1f << 24)
#define ADC_SQ4_SET(VAL) (((VAL) << 24) & ADC_SQ4)
#define ADC_SQ4_GET(REG) (((REG) & ADC_SQ4) >> 24)
/* These bits are written by software with the channel number (0..18) assigned
   as the 4th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 23 - Reserved, must be kept at reset value. */

/* Bits [18..22] - 3rd conversion in regular sequence */
#define ADC_SQ3 (0x1f << 18)
#define ADC_SQ3_SET(VAL) (((VAL) << 18) & ADC_SQ3)
#define ADC_SQ3_GET(REG) (((REG) & ADC_SQ3) >> 18)
/* These bits are written by software with the channel number (0..18) assigned
   as the 3rd in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 17 - Reserved, must be kept at reset value. */

/* Bits [12..16] - 2nd conversion in regular sequence */
#define ADC_SQ2 (0x1f << 12)
#define ADC_SQ2_SET(VAL) (((VAL) << 12) & ADC_SQ2)
#define ADC_SQ2_GET(REG) (((REG) & ADC_SQ2) >> 12)
/* These bits are written by software with the channel number (0..18) assigned
   as the 2nd in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 11 - Reserved, must be kept at reset value. */

/* Bits [6..10] - 1st conversion in regular sequence */
#define ADC_SQ1 (0x1f << 6)
#define ADC_SQ1_SET(VAL) (((VAL) << 6) & ADC_SQ1)
#define ADC_SQ1_GET(REG) (((REG) & ADC_SQ1) >> 6)
/* These bits are written by software with the channel number (0..18) assigned
   as the 1st in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bits [4..5] - Reserved, must be kept at reset value. */

/* Bits [0..3] - Regular channel sequence length */
#define ADC_L (0xf << 0)
#define ADC_L_SET(VAL) ((((VAL) - 1) << 0) & ADC_L)
#define ADC_L_GET(REG) ((((REG) & ADC_L) >> 0) + 1)
/* These bits are written by software to define the total number of
   conversions in the regular channel conversion sequence.
   0000: 1 conversion
   0001: 2 conversions ...
   1111: 16 conversions
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC regular sequence register 2 - SQR2 */
#define STM32_ADC_SQR2 0x0034

/* Bits [29..31] - Reserved, must be kept at reset value. */

/* Bits [24..28] - 9th conversion in regular sequence */
#define ADC_SQ9 (0x1f << 24)
#define ADC_SQ9_SET(VAL) (((VAL) << 24) & ADC_SQ9)
#define ADC_SQ9_GET(REG) (((REG) & ADC_SQ9) >> 24)
/* These bits are written by software with the channel number (0..18) assigned
   as the 9th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 23 - Reserved, must be kept at reset value. */

/* Bits [18..22] - 8th conversion in regular sequence */
#define ADC_SQ8 (0x1f << 18)
#define ADC_SQ8_SET(VAL) (((VAL) << 18) & ADC_SQ8)
#define ADC_SQ8_GET(REG) (((REG) & ADC_SQ8) >> 18)
/* These bits are written by software with the channel number (0..18) assigned
   as the 8th in the regular conversion sequence
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 17 - Reserved, must be kept at reset value. */

/* Bits [12..16] - 7th conversion in regular sequence */
#define ADC_SQ7 (0x1f << 12)
#define ADC_SQ7_SET(VAL) (((VAL) << 12) & ADC_SQ7)
#define ADC_SQ7_GET(REG) (((REG) & ADC_SQ7) >> 12)
/* These bits are written by software with the channel number (0..18) assigned
   as the 7th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 11 - Reserved, must be kept at reset value. */

/* Bits [6..10] - 6th conversion in regular sequence */
#define ADC_SQ6 (0x1f << 6)
#define ADC_SQ6_SET(VAL) (((VAL) << 6) & ADC_SQ6)
#define ADC_SQ6_GET(REG) (((REG) & ADC_SQ6) >> 6)
/* These bits are written by software with the channel number (0..18) assigned
   as the 6th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 5 - Reserved, must be kept at reset value. */

/* Bits [0..4] - 5th conversion in regular sequence */
#define ADC_SQ5 (0x1f << 0)
#define ADC_SQ5_SET(VAL) (((VAL) << 0) & ADC_SQ5)
#define ADC_SQ5_GET(REG) (((REG) & ADC_SQ5) >> 0)
/* These bits are written by software with the channel number (0..18) assigned
   as the 5th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC regular sequence register 3 - SQR3 */
#define STM32_ADC_SQR3 0x0038

/* Bits [29..31] - Reserved, must be kept at reset value. */

/* Bits [24..28] - 14th conversion in regular sequence */
#define ADC_SQ14 (0x1f << 24)
#define ADC_SQ14_SET(VAL) (((VAL) << 24) & ADC_SQ14)
#define ADC_SQ14_GET(REG) (((REG) & ADC_SQ14) >> 24)
/* These bits are written by software with the channel number (0..18) assigned
   as the 14th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 23 - Reserved, must be kept at reset value. */

/* Bits [18..22] - 13th conversion in regular sequence */
#define ADC_SQ13 (0x1f << 18)
#define ADC_SQ13_SET(VAL) (((VAL) << 18) & ADC_SQ13)
#define ADC_SQ13_GET(REG) (((REG) & ADC_SQ13) >> 18)
/* These bits are written by software with the channel number (0..18) assigned
   as the 13th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 17 - Reserved, must be kept at reset value. */

/* Bits [12..16] - 12th conversion in regular sequence */
#define ADC_SQ12 (0x1f << 12)
#define ADC_SQ12_SET(VAL) (((VAL) << 12) & ADC_SQ12)
#define ADC_SQ12_GET(REG) (((REG) & ADC_SQ12) >> 12)
/* These bits are written by software with the channel number (0..18) assigned
   as the 12th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 11 - Reserved, must be kept at reset value. */

/* Bits [6..10] - 11th conversion in regular sequence */
#define ADC_SQ11 (0x1f << 6)
#define ADC_SQ11_SET(VAL) (((VAL) << 6) & ADC_SQ11)
#define ADC_SQ11_GET(REG) (((REG) & ADC_SQ11) >> 6)
/* These bits are written by software with the channel number (0..18) assigned
   as the 11th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 5 - Reserved, must be kept at reset value. */

/* Bits [0..4] - 10th conversion in regular sequence */
#define ADC_SQ10 (0x1f << 0)
#define ADC_SQ10_SET(VAL) (((VAL) << 0) & ADC_SQ10)
#define ADC_SQ10_GET(REG) (((REG) & ADC_SQ10) >> 0)
/* These bits are written by software with the channel number (0..18) assigned
   as the 10th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC regular sequence register 4 - SQR4 */
#define STM32_ADC_SQR4 0x003c

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bits [6..10] - 16th conversion in regular sequence */
#define ADC_SQ16 (0x1f << 6)
#define ADC_SQ16_SET(VAL) (((VAL) << 6) & ADC_SQ16)
#define ADC_SQ16_GET(REG) (((REG) & ADC_SQ16) >> 6)
/* These bits are written by software with the channel number (0..18) assigned
   as the 16th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* Bit 5 - Reserved, must be kept at reset value. */

/* Bits [0..4] - 15th conversion in regular sequence */
#define ADC_SQ15 (0x1f << 0)
#define ADC_SQ15_SET(VAL) (((VAL) << 0) & ADC_SQ15)
#define ADC_SQ15_GET(REG) (((REG) & ADC_SQ15) >> 0)
/* These bits are written by software with the channel number (0..18) assigned
   as the 15th in the regular conversion sequence.
   Note: Software is allowed to write these bits only when ADSTART=0 (which
   ensures that no regular conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC regular Data Register - DR */
#define STM32_ADC_DR 0x0040

/* Bits [16..31] - Reserved, must be kept at reset value. */

/* Bits [0..15] - Regular Data converted */
#define ADC_RDATA (0xffff << 0)
#define ADC_RDATA_SET(VAL) (((VAL) << 0) & ADC_RDATA)
#define ADC_RDATA_GET(REG) (((REG) & ADC_RDATA) >> 0)
/* These bits are read-only. They contain the conversion result from the last
   converted regular channel.
   The data are left- or right-aligned as described in Section 16.4.26: Data
   management. */

/* ------------------------------------------------------------------------- */
/* ADC injected sequence register - JSQR */
#define STM32_ADC_JSQR 0x004c

/* Bit 31 - Reserved, must be kept at reset value. */

/* Bits [26..30] - 4th conversion in the injected sequence */
#define ADC_JSQ4 (0x1f << 26)
#define ADC_JSQ4_SET(VAL) (((VAL) << 26) & ADC_JSQ4)
#define ADC_JSQ4_GET(REG) (((REG) & ADC_JSQ4) >> 26)
/* These bits are written by software with the channel number (0..18) assigned
   as the 4th in the injected conversion sequence.
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1). */

/* Bit 25 - Reserved, must be kept at reset value. */

/* Bits [20..24] - 3rd conversion in the injected sequence */
#define ADC_JSQ3 (0x1f << 20)
#define ADC_JSQ3_SET(VAL) (((VAL) << 20) & ADC_JSQ3)
#define ADC_JSQ3_GET(REG) (((REG) & ADC_JSQ3) >> 20)
/* These bits are written by software with the channel number (0..18) assigned
   as the 3rd in the injected conversion sequence.
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1). */

/* Bit 19 - Reserved, must be kept at reset value. */

/* Bits [14..18] - 2nd conversion in the injected sequence */
#define ADC_JSQ2 (0x1f << 14)
#define ADC_JSQ2_SET(VAL) (((VAL) << 14) & ADC_JSQ2)
#define ADC_JSQ2_GET(REG) (((REG) & ADC_JSQ2) >> 14)
/* These bits are written by software with the channel number (0..18) assigned
   as the 2nd in the injected conversion sequence.
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1). */

/* Bit 13 - Reserved, must be kept at reset value. */

/* Bits [8..12] - 1st conversion in the injected sequence */
#define ADC_JSQ1 (0x1f << 8)
#define ADC_JSQ1_SET(VAL) (((VAL) << 8) & ADC_JSQ1)
#define ADC_JSQ1_GET(REG) (((REG) & ADC_JSQ1) >> 8)
/* These bits are written by software with the channel number (0..18) assigned
   as the 1st in the injected conversion sequence.
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1). */

/* Bits [6..7] - External Trigger Enable and Polarity Selection for injected channels */
#define ADC_JEXTEN (0x3 << 6)
#define ADC_JEXTEN_SET(VAL) (((VAL) << 6) & ADC_JEXTEN)
#define ADC_JEXTEN_GET(REG) (((REG) & ADC_JEXTEN) >> 6)
/* These bits are set and cleared by software to select the external trigger
   polarity and enable the trigger of an injected group.
   00: If JQDIS=0 (queue enabled), Hardware and software trigger detection
   disabled
   00: If JQDIS=1 (queue disabled), Hardware trigger detection disabled
   (conversions can be launched by software)
   01: Hardware trigger detection on the rising edge
   10: Hardware trigger detection on the falling edge
   11: Hardware trigger detection on both the rising and falling edges
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1).
   If JQM=1 and if the Queue of Context becomes empty, the software and
   hardware triggers of the injected sequence are both internally disabled
   (refer to Section 16.4.21: Queue of context for injected conversions) */

/* Bits [2..5] - External Trigger Selection for injected group */
#define ADC_JEXTSEL (0xf << 2)
#define ADC_JEXTSEL_SET(VAL) (((VAL) << 2) & ADC_JEXTSEL)
#define ADC_JEXTSEL_GET(REG) (((REG) & ADC_JEXTSEL) >> 2)
/* These bits select the external event used to trigger the start of
   conversion of an injected
   group:
   0000: Event 0
   0001: Event 1
   0010: Event 2
   0011: Event 3
   0100: Event 4
   0101: Event 5
   0110: Event 6
   0111: Event 7 ...
   1111: Event 15
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1). */

/* Bits [0..1] - Injected channel sequence length */
#define ADC_JL (0x3 << 0)
#define ADC_JL_SET(VAL) (((VAL) << 0) & ADC_JL)
#define ADC_JL_GET(REG) (((REG) & ADC_JL) >> 0)
/* These bits are written by software to define the total number of
   conversions in the injected channel conversion sequence.
   00: 1 conversion
   01: 2 conversions
   10: 3 conversions
   11: 4 conversions
   Note: Software is allowed to write these bits at any time, once the ADC is
   enabled (ADEN=1). */

/* ------------------------------------------------------------------------- */
/* ADC offset register - OFRY */
#define STM32_ADC_OFRY 0x0080

/* Bit 31 - Offset y Enable */
#define ADC_OFFSETY_EN (1 << 31)
/* This bit is written by software to enable or disable the offset programmed
   into bits OFFSETy[11:0].
   Note: Software is allowed to write this bit only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bits [26..30] - Channel selection for the Data offset y */
#define ADC_OFFSETY_CH (0x1f << 26)
#define ADC_OFFSETY_CH_SET(VAL) (((VAL) << 26) & ADC_OFFSETY_CH)
#define ADC_OFFSETY_CH_GET(REG) (((REG) & ADC_OFFSETY_CH) >> 26)
/* These bits are written by software to define the channel to which the
   offset programmed into bits OFFSETy[11:0] will apply.
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing). */

/* Bits [12..25] - Reserved, must be kept at reset value. */

/* Bits [0..11] - Data offset y for the channel programmed into bits OFFSETy_CH[4:0] */
#define ADC_OFFSETY (0xfff << 0)
#define ADC_OFFSETY_SET(VAL) (((VAL) << 0) & ADC_OFFSETY)
#define ADC_OFFSETY_GET(REG) (((REG) & ADC_OFFSETY) >> 0)
/* These bits are written by software to define the offset y to be subtracted
   from the raw converted data when converting a channel (can be regular or
   injected). The channel to which applies the data offset y must be
   programmed in the bits OFFSETy_CH[4:0]. The conversion result can be read
   from in the ADC_DR (regular conversion) or from in the ADC_JDRyi registers
   (injected conversion).
   Note: Software is allowed to write these bits only when ADSTART=0 and
   JADSTART=0 (which ensures that no conversion is ongoing).
   If several offset (OFFSETy) point to the same channel, only the offset with
   the lowest x value is considered for the subtraction.
   Ex: if OFFSET1_CH[4:0]=4 and OFFSET2_CH[4:0]=4, this is OFFSET1[11:0] which
   is subtracted when converting channel 4.
   16.6.18     ADC injected data register (ADC_JDRy, y= 1..4) JDATA[15:0] */

/* Bits [16..31] - Reserved, must be kept at reset value. */

/* Bits [0..15] - Injected data */
#define ADC_JDATA (0xffff << 0)
#define ADC_JDATA_SET(VAL) (((VAL) << 0) & ADC_JDATA)
#define ADC_JDATA_GET(REG) (((REG) & ADC_JDATA) >> 0)
/* These bits are read-only. They contain the conversion result from injected
   channel y. The data are left -or right-aligned as described in Section
   16.4.26: Data management. */

/* ------------------------------------------------------------------------- */
/* ADC Analog Watchdog 2 Configuration Register - AWD2CR */
#define STM32_ADC_AWD2CR 0x00a0

/* Bits [19..31] - Reserved, must be kept at reset value. */

/* Bits [0..18] - Analog watchdog 2 channel selection */
#define ADC_AWD2CH (0x7ffff << 0)
#define ADC_AWD2CH_SET(VAL) (((VAL) << 0) & ADC_AWD2CH)
#define ADC_AWD2CH_GET(REG) (((REG) & ADC_AWD2CH) >> 0)
/* These bits are set and cleared by software. They enable and select the
   input channels to be guarded by the analog watchdog 2.
   AWD2CH[i] = 0: ADC analog input channel-i is not monitored by AWD2
   AWD2CH[i] = 1: ADC analog input channel-i is monitored by AWD2 When
   AWD2CH[18:0] = 000..0, the analog Watchdog 2 is disabled
   Note: The channels selected by AWD2CH must be also selected into the SQRi
   or JSQRi registers.
   Software is allowed to write these bits only when ADSTART=0 and JADSTART=0
   (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC Analog Watchdog 3 Configuration Register - AWD3CR */
#define STM32_ADC_AWD3CR 0x00a4

/* Bits [19..31] - Reserved, must be kept at reset value. */

/* Bits [0..18] - Analog watchdog 2 channel selection */
#define ADC_AWD2CH (0x7ffff << 0)
#define ADC_AWD2CH_SET(VAL) (((VAL) << 0) & ADC_AWD2CH)
#define ADC_AWD2CH_GET(REG) (((REG) & ADC_AWD2CH) >> 0)
/* These bits are set and cleared by software. They enable and select the
   input channels to be guarded by the analog watchdog 2.
   AWD2CH[i] = 0: ADC analog input channel-i is not monitored by AWD2
   AWD2CH[i] = 1: ADC analog input channel-i is monitored by AWD2 When
   AWD2CH[18:0] = 000..0, the analog Watchdog 2 is disabled
   Note: The channels selected by AWD2CH must be also selected into the SQRi
   or JSQRi registers.
   Software is allowed to write these bits only when ADSTART=0 and JADSTART=0
   (which ensures that no conversion is ongoing). */

/* ------------------------------------------------------------------------- */
/* ADC Differential mode Selection Register - DIFSEL */
#define STM32_ADC_DIFSEL 0x00b0

/* Bits [19..31] - Reserved, must be kept at reset value. */

/* Bits [16..18] - Differential mode for channels 18 to 16. */
#define ADC_DIFSEL (0x7 << 16)
#define ADC_DIFSEL_SET(VAL) (((VAL) << 16) & ADC_DIFSEL)
#define ADC_DIFSEL_GET(REG) (((REG) & ADC_DIFSEL) >> 16)
/* These bits are read only. These channels are forced to single-ended input
   mode (either connected to a single-ended I/O port or to an internal
   channel). */
#if 0
/* Bits [1..15] - Differential mode for channels 15 to 1 */
#define ADC_DIFSEL (0x7fff << 1)
#define ADC_DIFSEL_SET(VAL) (((VAL) << 1) & ADC_DIFSEL)
#define ADC_DIFSEL_GET(REG) (((REG) & ADC_DIFSEL) >> 1)
/* These bits are set and cleared by software. They allow to select if a
   channel is configured as single ended or differential mode.
   DIFSEL[i] = 0: ADC analog input channel-i is configured in single ended
   mode DIFSEL[i] = 1: ADC analog input channel-i is configured in
   differential mode
   Note: Software is allowed to write these bits only when the ADC is disabled
   (ADCAL=0, JADSTART=0, JADSTP=0, ADSTART=0, ADSTP=0, ADDIS=0 and ADEN=0).
   It is mandatory to keep cleared ADC1_DIFSEL[15] (connected to an internal
   single ended channel) Bit 0 DIFSEL[0]: Differential mode for channel 0 This
   bit is read only. This channel is forced to single-ended input mode
   (connected to an internal channel). */
#endif
/* ------------------------------------------------------------------------- */
/* ADC Calibration Factors - CALFACT */
#define STM32_ADC_CALFACT 0x00b4

/* Bits [23..31] - Reserved, must be kept at reset value. */

/* Bits [16..22] - Calibration Factors in differential mode */
#define ADC_CALFACT_D (0x7f << 16)
#define ADC_CALFACT_D_SET(VAL) (((VAL) << 16) & ADC_CALFACT_D)
#define ADC_CALFACT_D_GET(REG) (((REG) & ADC_CALFACT_D) >> 16)
/* These bits are written by hardware or by software.
   Once a differential inputs calibration is complete, they are updated by
   hardware with the calibration factors.
   Software can write these bits with a new calibration factor. If the new
   calibration factor is different from the current one stored into the analog
   ADC, it will then be applied once a new differential calibration is
   launched.
   Note: Software is allowed to write these bits only when ADEN=1, ADSTART=0
   and JADSTART=0 (ADC is enabled and no calibration is ongoing and no
   conversion is ongoing). */

/* Bits [7..15] - Reserved, must be kept at reset value. */

/* Bits [0..6] - Calibration Factors In single-ended mode */
#define ADC_CALFACT_S (0x7f << 0)
#define ADC_CALFACT_S_SET(VAL) (((VAL) << 0) & ADC_CALFACT_S)
#define ADC_CALFACT_S_GET(REG) (((REG) & ADC_CALFACT_S) >> 0)
/* These bits are written by hardware or by software.
   Once a single-ended inputs calibration is complete, they are updated by
   hardware with the calibration factors.
   Software can write these bits with a new calibration factor. If the new
   calibration factor is different from the current one stored into the analog
   ADC, it will then be applied once a new single-ended calibration is
   launched.
   Note: Software is allowed to write these bits only when ADEN=1, ADSTART=0
   and JADSTART=0 (ADC is enabled and no calibration is ongoing and no
   conversion is ongoing).
   16.7 ADC common registers 16.7.1 These registers define the control and
   status registers common to master and slave ADCs: */

/* ------------------------------------------------------------------------- */
/* ADC Common status register - CSR */
#define STM32_ADC_CSR 0x0000

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - Injected Context Queue Overflow flag of the master ADC */
#define ADC_JQOVF_MST (1 << 10)
/* This bit is a copy of the JQOVF bit in the corresponding ADC_ISR register. */

/* Bit 9 - Analog watchdog 3 flag of the master ADC */
#define ADC_AWD3_MST (1 << 9)
/* This bit is a copy of the AWD3 bit in the corresponding ADC_ISR register.
   16.7.2 Bit 8 AWD2_MST: Analog watchdog 2 flag of the master ADC This bit is
   a copy of the AWD2 bit in the corresponding ADC_ISR register. */

/* Bit 7 - Analog watchdog 1 flag of the master ADC */
#define ADC_AWD1_MST (1 << 7)
/* This bit is a copy of the AWD1 bit in the corresponding ADC_ISR register. */

/* Bit 6 - End of injected sequence flag of the master ADC */
#define ADC_JEOS_MST (1 << 6)
/* This bit is a copy of the JEOS bit in the corresponding ADC_ISR register. */

/* Bit 5 - End of injected conversion flag of the master ADC */
#define ADC_JEOC_MST (1 << 5)
/* This bit is a copy of the JEOC bit in the corresponding ADC_ISR register. */

/* Bit 4 - Overrun flag of the master ADC */
#define ADC_OVR_MST (1 << 4)
/* This bit is a copy of the OVR bit in the corresponding ADC_ISR register. */

/* Bit 3 - End of regular sequence flag of the master ADC */
#define ADC_EOS_MST (1 << 3)
/* This bit is a copy of the EOS bit in the corresponding ADC_ISR register. */

/* Bit 2 - End of regular conversion of the master ADC */
#define ADC_EOC_MST (1 << 2)
/* This bit is a copy of the EOC bit in the corresponding ADC_ISR register. */

/* Bit 1 - End of Sampling phase flag of the master ADC */
#define ADC_EOSMP_MST (1 << 1)
/* This bit is a copy of the EOSMP bit in the corresponding ADC_ISR register. */

/* Bit 0 - Master ADC ready */
#define ADC_ADRDY_MST (1 << 0)
/* This bit is a copy of the ADRDY bit in the corresponding ADC_ISR register. */

/* ------------------------------------------------------------------------- */
/* ADC common control register - CCR */
#define STM32_ADC_CCR 0x0008

/* Bits [25..31] - Reserved, must be kept at reset value. */

/* Bit 24 - CH18 selection */
#define ADC_CH18SEL (1 << 24)
/* This bit is set and cleared by software to control the channel 18 of ADC1
   0: VBAT channel disabled, DAC2_int selected.
   1: VBAT channel enabled */

/* Bit 23 - CH17 selection */
#define ADC_CH17SEL (1 << 23)
/* This bit is set and cleared by software to control the channel 17 of ADC1
   0: Temperature sensor channel disabled, DAC1_int selected
   1: Temperature sensor channel enabled */

/* Bit 22 - VREFINT enable */
#define ADC_VREFEN (1 << 22)
/* This bit is set and cleared by software to enable/disable the VREFINT
   channel.
   0: VREFINT channel disabled
   1: VREFINT channel enabled */

/* Bits [18..21] - ADC prescaler */
#define ADC_PRESC (0xf << 18)
#define ADC_PRESC_SET(VAL) (((VAL) << 18) & ADC_PRESC)
#define ADC_PRESC_GET(REG) (((REG) & ADC_PRESC) >> 18)
/* These bits are set and cleared by software to select the frequency of the
   clock to the ADC.
   The clock is common for all the ADCs.
   0000: input ADC clock not divided
   0001: input ADC clock divided by 2
   0010: input ADC clock divided by 4
   0011: input ADC clock divided by 6
   0100: input ADC clock divided by 8
   0101: input ADC clock divided by 10
   0110: input ADC clock divided by 12
   0111: input ADC clock divided by 16
   1000: input ADC clock divided by 32
   1001: input ADC clock divided by 64
   1010: input ADC clock divided by 128
   1011: input ADC clock divided by 256
   other: reserved
   Note: Software is allowed to write these bits only when the ADC is disabled
   (ADCAL=0, JADSTART=0, ADSTART=0, ADSTP=0, ADDIS=0 and ADEN=0). The ADC
   prescaler value is applied only when CKMODE[1:0] = 0b00. */
#define ADC_PRESC_1   (0 << 18)
#define ADC_PRESC_2   (1 << 18)
#define ADC_PRESC_4   (2 << 18)
#define ADC_PRESC_6   (3 << 18)
#define ADC_PRESC_8   (4 << 18)
#define ADC_PRESC_10  (5 << 18)
#define ADC_PRESC_12  (4 << 18)
#define ADC_PRESC_16  (7 << 18)
#define ADC_PRESC_32  (8 << 18)
#define ADC_PRESC_64  (9 << 18)
#define ADC_PRESC_128 (10 << 18)
#define ADC_PRESC_256 (11 << 18)

/* Bits [16..17] - ADC clock mode */
#define ADC_CKMODE (0x3 << 16)
#define ADC_CKMODE_SET(VAL) (((VAL) << 16) & ADC_CKMODE)
#define ADC_CKMODE_GET(REG) (((REG) & ADC_CKMODE) >> 16)
/* These bits are set and cleared by software to define the ADC clock scheme
   (which is common to both master and slave ADCs):
   00: CK_ADCx (x=123) (Asynchronous clock mode), generated at product level
   (refer to Section 6: Reset and clock control (RCC))
   01: HCLK/1 (Synchronous clock mode). This configuration must be enabled
   only if the AHB clock prescaler is set to 1 (HPRE[3:0] = 0xxx in RCC_CFGR
   register) and if the system clock has a 50% duty cycle.
   10: HCLK/2 (Synchronous clock mode)
   11: HCLK/4 (Synchronous clock mode) In all synchronous clock modes, there
   is no jitter in the delay from a timer trigger to the start of a
   conversion.
   Note: Software is allowed to write these bits only when the ADCs are
   disabled (ADCAL=0, JADSTART=0, ADSTART=0, ADSTP=0, ADDIS=0 and ADEN=0). */

#define ADC_CKMODE_ASYNC  (0 << 16)
#define ADC_CKMODE_HCLK_1 (1 << 16)
#define ADC_CKMODE_HCLK_2 (2 << 16)
#define ADC_CKMODE_HCLK_4 (3 << 16)

/* Bits [0..15] - Reserved, must be kept at reset value. */



#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_adc {
	volatile uint32_t isr; /* 0x00 */
	volatile uint32_t ier; /* 0x04 */
	volatile uint32_t cr; /* 0x08 */
	volatile uint32_t cfgr; /* 0x0c */

	volatile uint32_t cfgr2; /* 0x10 */
	volatile uint32_t smpr1; /* 0x14 */
	volatile uint32_t smpr2; /* 0x18 */
	uint32_t res0[1];

	volatile uint32_t tr1; /* 0x20 */
	volatile uint32_t tr2; /* 0x24 */
	volatile uint32_t tr3; /* 0x28 */
	uint32_t res1[1];

	volatile uint32_t sqr1; /* 0x30 */
	volatile uint32_t sqr2; /* 0x34 */
	volatile uint32_t sqr3; /* 0x38 */
	volatile uint32_t sqr4; /* 0x3c */

	volatile uint32_t dr; /* 0x40 */
	uint32_t res2[2];
	volatile uint32_t jsqr; /* 0x4c */
	uint32_t res3[12];
	volatile uint32_t ofry; /* 0x80 */
	uint32_t res4[7];
	volatile uint32_t awd2cr; /* 0xa0 */
	volatile uint32_t awd3cr; /* 0xa4 */
	uint32_t res5[2];
	volatile uint32_t difsel; /* 0xb0 */
	volatile uint32_t calfact; /* 0xb4 */
};

struct stm32_adcc {
	volatile uint32_t csr; /* 0x00 */
	uint32_t res0[1];
	volatile uint32_t ccr; /* 0x08 */
};

/* set the sample time for an specific ADC channel */
static inline void stm32_adc_smp_set(struct stm32_adc *adc,
				     unsigned int chan, unsigned int val)
{
	uint32_t smpr;

	if (chan > 9) {
		smpr = adc->smpr1 & ~ADC_SMP_MSK(chan - 10);
		adc->smpr1 = smpr | ADC_SMP_SET(chan - 10, val);
	} else {
		smpr = adc->smpr2 & ~ADC_SMP_MSK(chan);
		adc->smpr2 = smpr | ADC_SMP_SET(chan, val);
	}
}

/* set the conversion channel sequence */
static inline void stm32_adc_seq_set(struct stm32_adc *adc,
				     const uint8_t chan[], unsigned int len)
{
	uint32_t sqr1;
	uint32_t sqr2;
	uint32_t sqr3;
	uint32_t sqr4;
	unsigned int n;
	unsigned int i;

	sqr1 = ADC_L_SET(len);
	sqr2 = 0;
	sqr3 = 0;
	sqr4 = 0;

	/* Channels 1 to 4 */
	n = (len > 4) ? 4 : len;
	for (i = 0; i < n; i++)
		sqr1 |= chan[i] << (6 * (i + 1));

	/* Channels 5 to 9 */
	len -= n;
	n = (len > 5) ? 5 : len;
	for (i = 0; i < n; i++)
		sqr2 |= chan[i + 4] << (6 * i);

	/* Channels 10 to 14 */
	len -= n;
	n = (len > 5) ? 5 : len;
	for (i = 0; i < n; i++)
		sqr3 |= chan[i + 9] << (6 * i);

	/* Channels 15 to 16 */
	len -= n;
	n = (len > 2) ? 2 : len;
	for (i = 0; i < n; i++)
		sqr4 |= chan[i + 14] << (6 * i);

	adc->sqr1 = sqr1;
	adc->sqr2 = sqr2;
	adc->sqr3 = sqr3;
	adc->sqr4 = sqr4;
}

#endif				/* __ASSEMBLER__ */

#endif				/* __STM32L4_ADC_H__ */

