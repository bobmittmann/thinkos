

/* ------------------------------------------------------------------------- */
/* SAI - Serial audio interface */
/* ------------------------------------------------------------------------- */

#ifndef __STM32_SAI_H__
#define __STM32_SAI_H__


/* ------------------------------------------------------------------------- */
/* Global configuration register - GCR */
#define STM32_SAI_GCR 0x0000

/* Bits [6..31] - Reserved, always read as 0. */

/* Bits [4..5] - Synchronization outputs */
#define SAI_SYNCOUT_MSK (0x3 << 4)
#define SAI_SYNCOUT_SET(VAL) (((VAL) << 4) & SAI_SYNCOUT_MSK)
#define SAI_SYNCOUT_GET(REG) (((REG) & SAI_SYNCOUT_MSK) >> 4)
/* These bits are set and cleared by software.
   00: No synchronization output signals. SYNCOUT[1:0] should be configured as
   No synchronization output signals when audio block is configured as SPDIF
   01: Block A used for further synchronization for others SAI
   10: Block B used for further synchronization for others SAI
   11: Reserved. These bits must be set when both audio block (A and B) are
   disabled. */

/* Bits [2..3] - Reserved, always read as 0. */

/* Bits [0..1] - Synchronization inputs */
#define SAI_SYNCIN_MSK (0x3 << 0)
#define SAI_SYNCIN_SET(VAL) (((VAL) << 0) & SAI_SYNCIN_MSK)
#define SAI_SYNCIN_GET(REG) (((REG) & SAI_SYNCIN_MSK) >> 0)
/* These bits are set and cleared by software.
   Please refer to Table 168: External Synchronization Selection for
   information on how to program this field.
   These bits must be set when both audio blocks (A and B) are disabled.
   They are meaningful if one of the two audio block is defined to operate in
   synchronous mode with an external SAI (SYNCEN[1:0] = 01 in SAI_ACR1 or in
   SAI_BCR1 registers). */

/* ------------------------------------------------------------------------- */
/* Configuration register 1 - ACR1 */
#define STM32_SAI_ACR1 0x0004

/* Configuration register 1 - BCR1 */
#define STM32_SAI_BCR1 0x0024

/* Bits [24..31] - Reserved, always read as 0. */

/* Bits [20..23] - Master clock divider. */
#define SAI_MCKDIV_MSK (0xf << 20)
#define SAI_MCKDIV_SET(VAL) (((VAL) << 20) & SAI_MCKDIV_MSK)
#define SAI_MCKDIV_GET(REG) (((REG) & SAI_MCKDIV_MSK) >> 20)
/* These bits are set and cleared by software. These bits are meaningless when
   the audio block operates in slave mode. They have to be configured when the
   audio block is disabled.
   0000: Divides by 1 the master clock input.
   Others: the master clock frequency is calculated accordingly to the
   following formula: F_SCK = F_SAICLK / (MCKDIV * 2) */

/* Bit 19 - No divider. */
#define SAI_NODIV (1 << 19)
/* This bit is set and cleared by software.
   0: Master clock generator is enabled
   1: No divider used in the clock generator (in this case Master Clock
   Divider bit has no effect) */

/* Bit 18 - Reserved, always read as 0. */

/* Bit 17 - DMA enable. */
#define SAI_DMAEN (1 << 17)
/* This bit is set and cleared by software.
   0: DMA disabled
   1: DMA enabled
   Note: Since the audio block defaults to operate as a transmitter after
   reset, the MODE[1:0] bits must be configured before setting DMAEN to avoid
   a DMA request in receiver mode. */

/* Bit 16 - Audio block enable where x is A or B. */
#define SAI_SAIXEN (1 << 16)
/* This bit is set by software.
   To switch off the audio block, the application software must program this
   bit to 0 and poll the bit till it reads back 0, meaning that the block is
   completely disabled. Before setting this bit to 1, check that it is set to
   0, otherwise the enable command will not be taken into account.
   This bit allows to control the state of SAIx audio block. If it is disabled
   when an audio frame transfer is ongoing, the ongoing transfer completes and
   the cell is fully disabled at the end of this audio frame transfer.
   0: SAIx audio block disabled
   1: SAIx audio block enabled.
   Note: When SAIx block is configured in master mode, the kernel clock must
   be present on the input of SAIx before setting SAIXEN bit. */

/* Bits [14..15] - Reserved, always read as 0. */

/* Bit 13 - Output drive. */
#define SAI_OUTDRIV (1 << 13)
/* This bit is set and cleared by software.
   0: Audio block output driven when SAIXEN is set
   1: Audio block output driven immediately after the setting of this bit.
   Note: This bit has to be set before enabling the audio block and after the
   audio block configuration. */

/* Bit 12 - Mono mode. */
#define SAI_MONO (1 << 12)
/* This bit is set and cleared by software. It is meaningful only when the
   number of slots is equal to 2.
   When the mono mode is selected, slot 0 data are duplicated on slot 1 when
   the audio block operates as a transmitter. In reception mode, the slot1 is
   discarded and only the data received from slot 0 are stored. Refer to
   Section : Mono/stereo mode for more details.
   0: Stereo mode
   1: Mono mode. */

/* Bits [10..11] - Synchronization enable. */
#define SAI_SYNCEN_MSK (0x3 << 10)
#define SAI_SYNCEN_SET(VAL) (((VAL) << 10) & SAI_SYNCEN_MSK)
#define SAI_SYNCEN_GET(REG) (((REG) & SAI_SYNCEN_MSK) >> 10)
/* These bits are set and cleared by software. They must be configured when
   the audio sub-block is disabled.
   00: audio sub-block in asynchronous mode.
   01: audio sub-block is synchronous with the other internal audio sub-block.
   In this case, the audio sub-block must be configured in slave mode
   10: audio sub-block is synchronous with an external SAI embedded
   peripheral. In this case the audio sub-block should be configured in Slave
   mode.
   11: Reserved
   Note: The audio sub-block should be configured as asynchronous when SPDIF
   mode is enabled. */

/* Bit 9 - Clock strobing edge. */
#define SAI_CKSTR (1 << 9)
/* This bit is set and cleared by software. It must be configured when the
   audio block is disabled. This bit has no meaning in SPDIF audio protocol.
   0: Signals generated by the SAI change on SCK rising edge, while signals
   received by the SAI are sampled on the SCK falling edge.
   1: Signals generated by the SAI change on SCK falling edge, while signals
   received by the SAI are sampled on the SCK rising edge. */
#define SAI_CKSTR_RISE (0 << 9)
#define SAI_CKSTR_FALL (1 << 9)

/* Bit 8 - Least significant bit first. */
#define SAI_LSBFIRST (1 << 8)
/* This bit is set and cleared by software. It must be configured when the
   audio block is disabled. This bit has no meaning in AC’97 audio protocol
   since AC’97 data are always transferred with the MSB first. This bit has no
   meaning in SPDIF audio protocol since in SPDIF data are always transferred
   with LSB first.
   0: Data are transferred with MSB first
   1: Data are transferred with LSB first */

/* Bits [5..7] - Data size. */
#define SAI_DS_MSK (0x7 << 5)
#define SAI_DS_SET(VAL) (((VAL) << 5) & SAI_DS_MSK)
#define SAI_DS_GET(REG) (((REG) & SAI_DS_MSK) >> 5)
/* These bits are set and cleared by software. These bits are ignored when the
   SPDIF protocols are selected (bit PRTCFG[1:0]), because the frame and the
   data size are fixed in such case. When the companding mode is selected
   through COMP[1:0] bits, DS[1:0] are ignored since the data size is fixed to
   8 bits by the algorithm.
   These bits must be configured when the audio block is disabled.
   000: Reserved
   001: Reserved
   010: 8 bits
   011: 10 bits
   100: 16 bits
   101: 20 bits
   110: 24 bits
   111: 32 bits */
#define SAI_DS_8  (0x2 << 5)
#define SAI_DS_10 (0x3 << 5)
#define SAI_DS_16 (0x4 << 5)
#define SAI_DS_20 (0x5 << 5)
#define SAI_DS_24 (0x6 << 5)
#define SAI_DS_32 (0x7 << 5)

/* Bit 4 - Reserved, always read as 0. */

/* Bits [2..3] - Protocol configuration. */
#define SAI_PRTCFG_MSK (0x3 << 2)
#define SAI_PRTCFG_SET(VAL) (((VAL) << 2) & SAI_PRTCFG_MSK)
#define SAI_PRTCFG_GET(REG) (((REG) & SAI_PRTCFG_MSK) >> 2)
/* These bits are set and cleared by software. These bits have to be
   configured when the audio block is disabled.
   00: Free protocol. Free protocol allows to use the powerful configuration
   of the audio block to address a specific audio protocol (such as I2S,
   LSB/MSB justified, TDM, PCM/DSP...) by setting most of the configuration
   register bits as well as frame configuration register.
   01: SPDIF protocol
   10: AC’97 protocol
   11: Reserved */
#define SAI_PRTCFG_FREE  (0x0 << 2)
#define SAI_PRTCFG_SPDIF (0x1 << 2)
#define SAI_PRTCFG_AC97E (0x2 << 2)

/* Bits [0..1] - SAIx audio block mode. */
#define SAI_MODE_MSK (0x3 << 0)
#define SAI_MODE_SET(VAL) (((VAL) << 0) & SAI_MODE_MSK)
#define SAI_MODE_GET(REG) (((REG) & SAI_MODE_MSK) >> 0)
/* These bits are set and cleared by software. They must be configured when
   SAIx audio block is disabled.
   00: Master transmitter
   01: Master receiver
   10: Slave transmitter
   11: Slave receiver
   Note: When the audio block is configured in SPDIF mode, the master
   transmitter mode is forced (MODE[1:0] = 00). In Master transmitter mode,
   the audio block starts generating the FS and the clocks immediately. */
#define SAI_MODE_MASTER_TX (0x0 << 0)
#define SAI_MODE_MASTER_RX (0x1 << 0)
#define SAI_MODE_SLAVE_TX  (0x2 << 0)
#define SAI_MODE_SLAVE_RX  (0x3 << 0)

/* ------------------------------------------------------------------------- */
/* Configuration register 2 - ACR2 */
#define STM32_SAI_ACR2 0x0008

/* Configuration register 2 - BCR2 */
#define STM32_SAI_BCR2 0x0028

/* Bits [16..31] - Reserved, always read as 0 */

/* Bits [14..15] - Companding mode. */
#define SAI_COMP_MSK (0x3 << 14)
#define SAI_COMP_SET(VAL) (((VAL) << 14) & SAI_COMP_MSK)
#define SAI_COMP_GET(REG) (((REG) & SAI_COMP_MSK) >> 14)
/* These bits are set and cleared by software. The µ-Law and the A-Law log are
   a part of the CCITT G.711 recommendation, the type of complement that will
   be used depends on CPL bit.
   The data expansion or data compression are determined by the state of bit
   MODE[0].
   The data compression is applied if the audio block is configured as a
   transmitter.
   The data expansion is automatically applied when the audio block is
   configured as a receiver.
   Refer to Section : Companding mode for more details.
   00: No companding algorithm
   01: Reserved.
   10: µ-Law algorithm
   11: A-Law algorithm
   Note: Companding mode is applicable only when TDM is selected. */
#define SAI_COMP_NONE (0x0 << 14)
#define SAI_COMP_ALAW (0x2 << 14)
#define SAI_COMP_ULAW (0x3 << 14)

/* Bit 13 - Complement bit. */
#define SAI_CPL (1 << 13)
/* This bit is set and cleared by software.
   It defines the type of complement to be used for companding mode
   0: 1’s complement representation.
   1: 2’s complement representation.
   Note: This bit has effect only when the companding mode is µ-Law algorithm
   or A-Law algorithm. */
#define SAI_CPL_1 (0 << 13)
#define SAI_CPL_2 (1 << 13)

/* Bits [7..12] - Mute counter. */
#define SAI_MUTECNT_MSK (0x3f << 7)
#define SAI_MUTECNT_SET(VAL) (((VAL) << 7) & SAI_MUTECNT_MSK)
#define SAI_MUTECNT_GET(REG) (((REG) & SAI_MUTECNT_MSK) >> 7)
/* These bits are set and cleared by software. They are used only in reception
   mode.
   The value set in these bits is compared to the number of consecutive mute
   frames detected in reception. When the number of mute frames is equal to
   this value, the flag MUTEDET will be set and an interrupt will be generated
   if bit MUTEDETIE is set.
   Refer to Section : Mute mode for more details. */

/* Bit 6 - Mute value. */
#define SAI_MUTEVAL (1 << 6)
/* This bit is set and cleared by software.It must be written before enabling
   the audio block: SAIXEN.
   This bit is meaningful only when the audio block operates as a transmitter,
   the number of slots is lower or equal to 2 and the MUTE bit is set.
   If more slots are declared, the bit value sent during the transmission in
   mute mode is equal to 0, whatever the value of MUTEVAL.
   if the number of slot is lower or equal to 2 and MUTEVAL = 1, the MUTE
   value transmitted for each slot is the one sent during the previous frame.
   Refer to Section : Mute mode for more details.
   0: Bit value 0 is sent during the mute mode.
   1: Last values are sent during the mute mode.
   Note: This bit is meaningless and should not be used for SPDIF audio
   blocks. */

/* Bit 5 - Mute. */
#define SAI_MUTE (1 << 5)
/* This bit is set and cleared by software. It is meaningful only when the
   audio block operates as a transmitter. The MUTE value is linked to value of
   MUTEVAL if the number of slots is lower or equal to 2, or equal to 0 if it
   is greater than 2.
   Refer to Section : Mute mode for more details.
   0: No mute mode.
   1: Mute mode enabled.
   Note: This bit is meaningless and should not be used for SPDIF audio
   blocks. */

/* Bit 4 - Tristate management on data line. */
#define SAI_TRIS (1 << 4)
/* This bit is set and cleared by software. It is meaningful only if the audio
   block is configured as a transmitter. This bit is not used when the audio
   block is configured in SPDIF mode. It should be configured when SAI is
   disabled.
   Refer to Section : Output data line management on an inactive slot for more
   details.
   0: SD output line is still driven by the SAI when a slot is inactive.
   1: SD output line is released (HI-Z) at the end of the last data bit of the
   last active slot if the next one is inactive. */

/* Bit 3 - FIFO flush. */
#define SAI_FFLUSH (1 << 3)
/* This bit is set by software. It is always read as 0. This bit should be
   configured when the SAI is disabled.
   0: No FIFO flush.
   1: FIFO flush. Programming this bit to 1 triggers the FIFO Flush. All the
   internal FIFO pointers (read and write) are cleared. In this case data
   still present in the FIFO are lost (no more transmission or received data
   lost). Before flushing SAI, DMA stream/interruption must be disabled */

/* Bits [0..2] - FIFO threshold. */
#define SAI_FTH_MSK (0x7 << 0)
#define SAI_FTH_SET(VAL) (((VAL) << 0) & SAI_FTH_MSK)
#define SAI_FTH_GET(REG) (((REG) & SAI_FTH_MSK) >> 0)
/* This bit is set and cleared by software.
   000: FIFO empty
   001: ¼ FIFO
   010: ½ FIFO
   011: ¾ FIFO
   100: FIFO full
   101: Reserved
   110: Reserved
   111: Reserved */
#define SAI_FTH_EMPTY (0x0 << 0)
#define SAI_FTH_HALF  (0x2 << 0)
#define SAI_FTH_FULL  (0x4 << 0)

/* ------------------------------------------------------------------------- */
/* Frame configuration register - AFRCR */
#define STM32_SAI_AFRCR 0x000c

/* Frame configuration register - BFRCR */
#define STM32_SAI_BFRCR 0x002c

/* Bits [19..31] - Reserved, always read as 0. */

/* Bit 18 - Frame synchronization offset. */
#define SAI_FSOFF (1 << 18)
/* This bit is set and cleared by software. It is meaningless and is not used
   in AC’97 or SPDIF audio block configuration. This bit must be configured
   when the audio block is disabled.
   0: FS is asserted on the first bit of the slot 0.
   1: FS is asserted one bit before the first bit of the slot 0. */

/* Bit 17 - Frame synchronization polarity. */
#define SAI_FSPOL (1 << 17)
/* This bit is set and cleared by software. It is used to configure the level
   of the start of frame on the FS signal. It is meaningless and is not used
   in AC’97 or SPDIF audio block configuration.
   This bit must be configured when the audio block is disabled.
   0: FS is active low (falling edge)
   1: FS is active high (rising edge) */
#define SAI_FSPOL_LO (0 << 17)
#define SAI_FSPOL_HI (1 << 17)

/* Bit 16 - Frame synchronization definition. */
#define SAI_FSDEF (1 << 16)
/* This bit is set and cleared by software.
   0: FS signal is a start frame signal
   1: FS signal is a start of frame signal + channel side identification When
   the bit is set, the number of slots defined in the SAI_xSLOTR register has
   to be even. It means that half of this number of slots will be dedicated to
   the left channel and the other slots for the right channel (e.g: this bit
   has to be set for I2S or MSB/LSB-justified protocols...).
   This bit is meaningless and is not used in AC’97 or SPDIF audio block
   configuration. It must be configured when the audio block is disabled. */

/* Bit 15 - Reserved, always read as 0. */

/* Bits [8..14] - Frame synchronization active level length. */
#define SAI_FSALL_MSK (0x7f << 8)
#define SAI_FSALL_SET(VAL) (((VAL) << 8) & SAI_FSALL_MSK)
#define SAI_FSALL_GET(REG) (((REG) & SAI_FSALL_MSK) >> 8)
/* These bits are set and cleared by software. They specify the length in
   number of bit clock (SCK) + 1 (FSALL[6:0] + 1) of the active level of the
   FS signal in the audio frame These bits are meaningless and are not used in
   AC’97 or SPDIF audio block configuration.
   They must be configured when the audio block is disabled. */
#define SAI_FSALL(LEN) ((((LEN) - 1) & 0x7f) << 8)

/* Bits [0..7] - Frame length. */
#define SAI_FRL_MSK (0xff << 0)
#define SAI_FRL_SET(VAL) (((VAL) << 0) & SAI_FRL_MSK)
#define SAI_FRL_GET(REG) (((REG) & SAI_FRL_MSK) >> 0)
/* These bits are set and cleared by software. They define the audio frame
   length expressed in number of SCK clock cycles: the number of bits in the
   frame is equal to FRL[7:0] + 1.
   The minimum number of bits to transfer in an audio frame must be equal to
   8, otherwise the audio block will behaves in an unexpected way. This is the
   case when the data size is 8 bits and only one slot 0 is defined in
   NBSLOT[4:0] of SAI_xSLOTR register (NBSLOT[3:0] = 0000).
   In master mode, if the master clock (available on MCLK_x pin) is used, the
   frame length should be aligned with a number equal to a power of 2, ranging
   from 8 to 256. When the master clock is not used (NODIV = 1), it is
   recommended to program the frame length to an value ranging from 8 to 256.
   These bits are meaningless and are not used in AC’97 or SPDIF audio block
   configuration. */
#define SAI_FRL(LEN) ((((LEN) - 1) & 0xff) << 0)

/* ------------------------------------------------------------------------- */
/* Slot register - ASLOTR */
#define STM32_SAI_ASLOTR 0x0010

/* Slot register - BSLOTR */
#define STM32_SAI_BSLOTR 0x0030

/* Bits [16..31] - Slot enable. */
#define SAI_SLOTEN_MSK (0xffff << 16)
#define SAI_SLOTEN_SET(VAL) (((VAL) << 16) & SAI_SLOTEN_MSK)
#define SAI_SLOTEN_GET(REG) (((REG) & SAI_SLOTEN_MSK) >> 16)
/* These bits are set and cleared by software.
   Each SLOTEN bit corresponds to a slot position from 0 to 15 (maximum 16
   slots).
   0: Inactive slot.
   1: Active slot.
   The slot must be enabled when the audio block is disabled.
   They are ignored in AC’97 or SPDIF mode. */

/* Bits [12..15] - Reserved, always read as 0. */

/* Bits [8..11] - Number of slots in an audio frame. */
#define SAI_NBSLOT_MSK (0xf << 8)
#define SAI_NBSLOT_SET(VAL) (((VAL) << 8) & SAI_NBSLOT_MSK)
#define SAI_NBSLOT_GET(REG) (((REG) & SAI_NBSLOT_MSK) >> 8)
/* These bits are set and cleared by software.
   The value set in this bitfield represents the number of slots + 1 in the
   audio frame (including the number of inactive slots). The maximum number of
   slots is 16.
   The number of slots should be even if FSDEF bit in the SAI_xFRCR register
   is set.
   The number of slots must be configured when the audio block is disabled.
   They are ignored in AC’97 or SPDIF mode. */
#define SAI_NBSLOT(SLOTS) ((((SLOTS) - 1) & 0xf) << 8)

/* Bits [6..7] - Slot size */
#define SAI_SLOTSZ_MSK (0x3 << 6)
#define SAI_SLOTSZ_SET(VAL) (((VAL) << 6) & SAI_SLOTSZ_MSK)
#define SAI_SLOTSZ_GET(REG) (((REG) & SAI_SLOTSZ_MSK) >> 6)
/* This bits is set and cleared by software.
   The slot size must be higher or equal to the data size. If this condition
   is not respected, the behavior of the SAI will be undetermined.
   Refer to Section : Output data line management on an inactive slot for
   information on how to drive SD line.
   These bits must be set when the audio block is disabled.
   They are ignored in AC’97 or SPDIF mode.
   00: The slot size is equivalent to the data size (specified in DS[3:0] in
   the SAI_xCR1 register).
   01: 16-bit
   10: 32-bit
   11: Reserved */
#define SAI_SLOTSZ_DS (0x0 << 6)
#define SAI_SLOTSZ_16 (0x1 << 6)
#define SAI_SLOTSZ_32 (0x2 << 6)

/* Bit 1 - Reserved, always read as 0. */

/* Bits [0..4] - First bit offset */
#define SAI_FBOFF_MSK (0x1f << 0)
#define SAI_FBOFF_SET(VAL) (((VAL) << 0) & SAI_FBOFF_MSK)
#define SAI_FBOFF_GET(REG) (((REG) & SAI_FBOFF_MSK) >> 0)
/* These bits are set and cleared by software.
   The value set in this bitfield defines the position of the first data
   transfer bit in the slot. It represents an offset value. In transmission
   mode, the bits outside the data field are forced to 0. In reception mode,
   the extra received bits are discarded.
   These bits must be set when the audio block is disabled.
   They are ignored in AC’97 or SPDIF mode. */

/* ------------------------------------------------------------------------- */
/* Interrupt mask register 2 - AIM */
#define STM32_SAI_AIM 0x0014

/* Interrupt mask register 2 - BIM */
#define STM32_SAI_BIM 0x0034

/* Bits [7..31] - Reserved, always read as 0. */

/* Bit 6 - Late frame synchronization detection interrupt enable. */
#define SAI_LFSDETIE (1 << 6)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled When this bit is set, an interrupt will be
   generated if the LFSDET bit is set in the SAI_xSR register.
   This bit is meaningless in AC’97, SPDIF mode or when the audio block
   operates as a master. */

/* Bit 5 - Anticipated frame synchronization detection interrupt enable. */
#define SAI_AFSDETIE (1 << 5)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled When this bit is set, an interrupt will be
   generated if the AFSDET bit in the SAI_xSR register is set.
   This bit is meaningless in AC’97, SPDIF mode or when the audio block
   operates as a master. */

/* Bit 4 - Codec not ready interrupt enable (AC’97). */
#define SAI_CNRDYIE (1 << 4)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled When the interrupt is enabled, the audio block
   detects in the slot 0 (tag0) of the AC’97 frame if the Codec connected to
   this line is ready or not. If it is not ready, the CNRDY flag in the
   SAI_xSR register is set and an interruption i generated.
   This bit has a meaning only if the AC’97 mode is selected through
   PRTCFG[1:0] bits and the audio block is operates as a receiver. */

/* Bit 3 - FIFO request interrupt enable. */
#define SAI_FREQIE (1 << 3)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled When this bit is set, an interrupt is generated if
   the FREQ bit in the SAI_xSR register is set.
   Since the audio block defaults to operate as a transmitter after reset, the
   MODE bit must be configured before setting FREQIE to avoid a parasitic
   interruption in receiver mode, */

/* Bit 2 - Wrong clock configuration interrupt enable. */
#define SAI_WCKCFGIE (1 << 2)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled This bit is taken into account only if the audio
   block is configured as a master (MODE[1] = 0) and NODIV = 0.
   It generates an interrupt if the WCKCFG flag in the SAI_xSR register is
   set.
   Note: This bit is used only in TDM mode and is meaningless in other modes. */

/* Bit 1 - Mute detection interrupt enable. */
#define SAI_MUTEDETIE (1 << 1)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled When this bit is set, an interrupt is generated if
   the MUTEDET bit in the SAI_xSR register is set.
   This bit has a meaning only if the audio block is configured in receiver
   mode. */

/* Bit 0 - Overrun/underrun interrupt enable. */
#define SAI_OVRUDRIE (1 << 0)
/* This bit is set and cleared by software.
   0: Interrupt is disabled
   1: Interrupt is enabled When this bit is set, an interrupt is generated if
   the OVRUDR bit in the SAI_xSR register is set. */

/* ------------------------------------------------------------------------- */
/* Status register - ASR */
#define STM32_SAI_ASR 0x0018

/* Status register - BSR */
#define STM32_SAI_BSR 0x0038

/* Bits [19..31] - Reserved, always read as 0. */

/* Bits [16..18] - FIFO level threshold. */
#define SAI_FLTH_MSK (0x7 << 16)
#define SAI_FLTH_SET(VAL) (((VAL) << 16) & SAI_FLTH_MSK)
#define SAI_FLTH_GET(REG) (((REG) & SAI_FLTH_MSK) >> 16)
/* This bit is read only. The FIFO level threshold flag is managed only by
   hardware and its setting depends on SAI block configuration (transmitter or
   receiver mode).
   If the SAI block is configured as transmitter:
   000: FIFO empty
   001: FIFO <= ¼ but not empty
   010: ¼ < FIFO <= ½
   011: ½ < FIFO <= ¾
   100: ¾ < FIFO but not full
   101: FIFO full If SAI block is configured as receiver:
   000: FIFO empty
   001: FIFO < ¼ but not empty
   010: ¼ <= FIFO < ½
   011: ½ =< FIFO < ¾
   100: ¾ =< FIFO but not full
   101: FIFO full */

/* Bits [7..15] - Reserved, always read as 0. */

/* Bit 6 - Late frame synchronization detection. */
#define SAI_LFSDET (1 << 6)
/* This bit is read only.
   0: No error.
   1: Frame synchronization signal is not present at the right time.
   This flag can be set only if the audio block is configured in slave mode.
   It is not used in AC’97 or SPDIF mode.
   It can generate an interrupt if LFSDETIE bit is set in the SAI_xIM
   register.
   This flag is cleared when the software sets bit CLFSDET in SAI_xCLRFR
   register */

/* Bit 5 - Anticipated frame synchronization detection. */
#define SAI_AFSDET (1 << 5)
/* This bit is read only.
   0: No error.
   1: Frame synchronization signal is detected earlier than expected.
   This flag can be set only if the audio block is configured in slave mode.
   It is not used in AC’97or SPDIF mode.
   It can generate an interrupt if AFSDETIE bit is set in SAI_xIM register.
   This flag is cleared when the software sets CAFSDET bit in SAI_xCLRFR
   register. */

/* Bit 4 - Codec not ready. */
#define SAI_CNRDY (1 << 4)
/* This bit is read only.
   0: External AC’97 Codec is ready
   1: External AC’97 Codec is not ready This bit is used only when the AC’97
   audio protocol is selected in the SAI_xCR1 register and configured in
   receiver mode.
   It can generate an interrupt if CNRDYIE bit is set in SAI_xIM register.
   This flag is cleared when the software sets CCNRDY bit in SAI_xCLRFR
   register. */

/* Bit 3 - FIFO request. */
#define SAI_FREQ (1 << 3)
/* This bit is read only.
   0: No FIFO request.
   1: FIFO request to read or to write the SAI_xDR.
   The request depends on the audio block configuration: – If the block is
   configured in transmission mode, the FIFO request is related to a write
   request operation in the SAI_xDR.
   – If the block configured in reception, the FIFO request related to a read
   request operation from the SAI_xDR.
   This flag can generate an interrupt if FREQIE bit is set in SAI_xIM
   register. */

/* Bit 2 - Wrong clock configuration flag. */
#define SAI_WCKCFG (1 << 2)
/* This bit is read only.
   0: Clock configuration is correct
   1: Clock configuration does not respect the rule concerning the frame
   length specification defined in Section 28.3.5: Frame synchronization
   (configuration of FRL[7:0] bit in the SAI_xFRCR register) This bit is used
   only when the audio block operates in master mode (MODE[1] = 0) and NODIV =
   0.
   It can generate an interrupt if WCKCFGIE bit is set in SAI_xIM register.
   This flag is cleared when the software sets CWCKCFG bit in SAI_xCLRFR
   register. */

/* Bit 1 - Mute detection. */
#define SAI_MUTEDET (1 << 1)
/* This bit is read only.
   0: No MUTE detection on the SD input line
   1: MUTE value detected on the SD input line (0 value) for a specified
   number of consecutive audio frame This flag is set if consecutive 0 values
   are received in each slot of a given audio frame and for a consecutive
   number of audio frames (set in the MUTECNT bit in the SAI_xCR2 register).
   It can generate an interrupt if MUTEDETIE bit is set in SAI_xIM register.
   This flag is cleared when the software sets bit CMUTEDET in the SAI_xCLRFR
   register. */

/* Bit 0 - Overrun / underrun. */
#define SAI_OVRUDR (1 << 0)
/* This bit is read only.
   0: No overrun/underrun error.
   1: Overrun/underrun error detection.
   The overrun and underrun conditions can occur only when the audio block is
   configured as a receiver and a transmitter, respectively.
   It can generate an interrupt if OVRUDRIE bit is set in SAI_xIM register.
   This flag is cleared when the software sets COVRUDR bit in SAI_xCLRFR
   register. */

/* ------------------------------------------------------------------------- */
/* Clear flag register - ACLRFR */
#define STM32_SAI_ACLRFR 0x001c

/* Clear flag register - BCLRFR */
#define STM32_SAI_BCLRFR 0x003c

/* Bits [7..31] - Reserved, always read as 0. */

/* Bit 6 - Clear late frame synchronization detection flag. */
#define SAI_CLFSDET (1 << 6)
/* This bit is write only.
   Programming this bit to 1 clears the LFSDET flag in the SAI_xSR register.
   This bit is not used in AC’97or SPDIF mode Reading this bit always returns
   the value 0.
   Bit 5 .CAFSDET: Clear anticipated frame synchronization detection flag.
   This bit is write only.
   Programming this bit to 1 clears the AFSDET flag in the SAI_xSR register.
   It is not used in AC’97or SPDIF mode.
   Reading this bit always returns the value 0. */

/* Bit 4 - Clear Codec not ready flag. */
#define SAI_CCNRDY (1 << 4)
/* This bit is write only.
   Programming this bit to 1 clears the CNRDY flag in the SAI_xSR register.
   This bit is used only when the AC’97 audio protocol is selected in the
   SAI_xCR1 register.
   Reading this bit always returns the value 0. */

/* Bit 3 - Reserved, always read as 0. */

/* Bit 2 - Clear wrong clock configuration flag. */
#define SAI_CWCKCFG (1 << 2)
/* This bit is write only.
   Programming this bit to 1 clears the WCKCFG flag in the SAI_xSR register.
   This bit is used only when the audio block is set as master (MODE[1] = 0)
   and NODIV = 0 in the SAI_xCR1 register.
   Reading this bit always returns the value 0. */

/* Bit 1 - Mute detection flag. */
#define SAI_CMUTEDET (1 << 1)
/* This bit is write only.
   Programming this bit to 1 clears the MUTEDET flag in the SAI_xSR register.
   Reading this bit always returns the value 0. */

/* Bit 0 - Clear overrun / underrun. */
#define SAI_COVRUDR (1 << 0)
/* This bit is write only.
   Programming this bit to 1 clears the OVRUDR flag in the SAI_xSR register.
   Reading this bit always returns the value 0.
   0x002C 0x0028 or A write to this register loads the FIFO provided the FIFO
   is not full.
   0x0024 A read from this register empties the FIFO if the FIFO is not empty. */

/* ------------------------------------------------------------------------- */
/* SAI_xFRCR                       SAI_xCR2                                  SAI_xCR1     Reset value          SAI_GCR         Register                                                    SAI register map                                                                                                           rw rw rw rw rw                                   rw rw rw rw rw                                                                             Data register - ADR */
#define STM32_SAI_ADR 0x0020

/* SAI_xFRCR                       SAI_xCR2                                  SAI_xCR1     Reset value          SAI_GCR         Register                                                    SAI register map                                                                                                           rw rw rw rw rw                                   rw rw rw rw rw                                                                             Data register - BDR */
#define STM32_SAI_BDR 0x0040



#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_sai {
	volatile uint32_t gcr; /* 0x00 */
	volatile uint32_t acr1; /* 0x04 */
	volatile uint32_t acr2; /* 0x08 */
	volatile uint32_t afrcr; /* 0x0c */
	volatile uint32_t aslotr; /* 0x10 */
	volatile uint32_t aim; /* 0x14 */
	volatile uint32_t asr; /* 0x18 */
	volatile uint32_t aclrfr; /* 0x1c */
	volatile uint32_t adr; /* 0x20 */
	volatile uint32_t bcr1; /* 0x24 */
	volatile uint32_t bcr2; /* 0x28 */
	volatile uint32_t bfrcr; /* 0x2c */
	volatile uint32_t bslotr; /* 0x30 */
	volatile uint32_t bim; /* 0x34 */
	volatile uint32_t bsr; /* 0x38 */
	volatile uint32_t bclrfr; /* 0x3c */
	volatile uint32_t bdr; /* 0x40 */
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32_SAI_H__ */

