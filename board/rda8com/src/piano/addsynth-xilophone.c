/* 
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the usb-serial converter.
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
 * @file addsynth_xilophone.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "addsynth.h"

#define RELEASE_ITV(__F) ((30 * DAC_SAMPLE_RATE) / (__F))


const struct addsynth_instrument_cfg addsynth_xilophone_cfg = {
	.tag = "Xilophone",
	.note_cnt = 16,
	.note = {
		[0] = {
			.tag = "A4",
			.midi = MIDI_A4,
			.osc = { 
				[0] = { .freq = MIDI_A4_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_A4_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_A4_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_A4_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_A4_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_A4_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 23,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_A4_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[1] = {
			.tag = "B4",
			.midi = MIDI_B4,
			.osc = { 
				[0] = { .freq = MIDI_B4_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_B4_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_B4_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_B4_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_B4_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_B4_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_B4_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[2] = {
			.tag = "C5",
			.midi = MIDI_C5,
			.osc = { 
				[0] = { .freq = MIDI_C5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_C5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_C5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_C5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_C5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_C5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_C5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[3] = {
			.tag = "D5",
			.midi = MIDI_D5,
			.osc = { 
				[0] = { .freq = MIDI_D5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_D5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_D5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_D5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_D5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_D5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_D5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[4] = {
			.tag = "E5",
			.midi = MIDI_E5,
			.osc = { 
				[0] = { .freq = MIDI_E5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_E5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_E5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_E5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_E5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_E5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_E5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[5] = {
			.tag = "F5",
			.midi = MIDI_F5,
			.osc = { 
				[0] = { .freq = MIDI_F5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_F5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_F5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_F5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_F5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_F5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.id = 6,
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_F5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[6] = {
			.tag = "G5",
			.midi = MIDI_G5,
			.osc = { 
				[0] = { .freq = MIDI_G5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_G5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_G5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_G5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_G5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_G5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_G5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[7] = {
			.tag = "A5",
			.midi = MIDI_A5,
			.osc = { 
				[0] = { .freq = MIDI_A5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_A5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_A5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_A5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_A5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_A5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_A5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[8] = {
			.tag = "B5",
			.midi = MIDI_B5,
			.osc = { 
				[0] = { .freq = MIDI_B5_FREQ, .ampl = 0.500 },
				[1] = { .freq = MIDI_B5_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_B5_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_B5_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_B5_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_B5_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_B5_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[9] = {
			.tag = "C6",
			.midi = MIDI_C6,
			.osc = { 
				[0] = { .freq = MIDI_C6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_C6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_C6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_C6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_C6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_C6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_C6_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[10] = {
			.tag = "D6",
			.midi = MIDI_D6,
			.osc = { 
				[0] = { .freq = MIDI_D6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_D6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_D6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_D6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_D6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_D6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_D6_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[11] = {
			.tag = "E6",
			.midi = MIDI_E6,
			.osc = { 
				[0] = { .freq = MIDI_E6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_E6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_E6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_E6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_E6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_E6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_E6_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[12] = {
			.tag = "F6",
			.midi = MIDI_F6,
			.osc = { 
				[0] = { .freq = MIDI_F6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_F6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_F6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_F6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_F6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_F6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_F6_FREQ),
				.supress_itv_ms = 92,
				.sustain_lvl = 0.50
			}

		},
		[13] = {
			.tag = "G6",
			.midi = MIDI_G6,
			.osc = { 
				[0] = { .freq = MIDI_G6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_G6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_G6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_G6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_G6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_G6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_G6_FREQ),
				.supress_itv_ms = 64,
				.sustain_lvl = 0.50
			}

		},
		[14] = {
			.tag = "A6",
			.midi = MIDI_A6,
			.osc = { 
				[0] = { .freq = MIDI_A6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_A6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_A6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_A6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_A6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_A6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_A6_FREQ),
				.supress_itv_ms = 64,
				.sustain_lvl = 0.50
			}

		},
		[15] = {
			.tag = "B6",
			.midi = MIDI_B6,
			.osc = { 
				[0] = { .freq = MIDI_B6_FREQ, .ampl = 0.333 },
				[1] = { .freq = MIDI_B6_FREQ * 2, .ampl = 0 },
				[2] = { .freq = MIDI_B6_FREQ * 3, .ampl = 0 },
				[3] = { .freq = MIDI_B6_FREQ * 4, .ampl = 0 },
				[4] = { .freq = MIDI_B6_FREQ * 5, .ampl = 0 },
				[5] = { .freq = MIDI_B6_FREQ * 6, .ampl = 0 }
			},
			.env = {
				.guard_itv_ms = 0,
				.delay_itv_ms = 0,
				.attack_itv_ms = 17,
				.hold_itv_ms = 0,
				.decay_itv_ms = 46,
				.release_itv_ms = RELEASE_ITV(MIDI_B6_FREQ),
				.supress_itv_ms = 64,
				.sustain_lvl = 0.50
			}

		},
		/* end of record */
		[16] = {
			.midi = -1,
		}
	}
};

