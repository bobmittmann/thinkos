#include <stdio.h>
#include <midi.h>

#include "keyboard.h"

const struct keyboard_cfg keyboard_xilophone_cfg = {
	.keymap_cnt = 18,
	.keymap = {
		 [0] = { .key =  1, .code = MIDI_A4},
		 [1] = { .key =  2, .code = MIDI_B4},
		 [2] = { .key =  3, .code = MIDI_C5},
		 [3] = { .key =  4, .code = MIDI_D5},
		 [4] = { .key =  5, .code = MIDI_E5},
		 [5] = { .key =  6, .code = MIDI_F5},
		 [6] = { .key =  7, .code = MIDI_G5},
		 [7] = { .key =  8, .code = MIDI_A5},
		 [8] = { .key =  9, .code = MIDI_B5},
		 [9] = { .key = 10, .code = MIDI_C6},
		[10] = { .key = 11, .code = MIDI_D6},
		[11] = { .key = 12, .code = MIDI_E6},
		[12] = { .key = 13, .code = MIDI_F6},
		[13] = { .key = 14, .code = MIDI_G6},
		[14] = { .key = 15, .code = MIDI_A6},
		[15] = { .key = 16, .code = MIDI_B6},

		[16] = { .key = 17, .code = KBD_SWITCH(0) },
		[17] = { .key = 18, .code = KBD_SWITCH(1) }
	}
};

const struct keyboard_cfg keyboard_piano_cfg = {
	.keymap_cnt = 18,
	.keymap = {
		 [0] = { .key =  1, .code = MIDI_C4},
		 [1] = { .key =  2, .code = MIDI_D4},
		 [2] = { .key =  3, .code = MIDI_E4},
		 [3] = { .key =  4, .code = MIDI_F4},
		 [4] = { .key =  5, .code = MIDI_G4},
		 [5] = { .key =  6, .code = MIDI_A4},
		 [6] = { .key =  7, .code = MIDI_B4},
		 [7] = { .key =  8, .code = MIDI_C5},
		 [8] = { .key =  9, .code = MIDI_D5},
		 [9] = { .key = 10, .code = MIDI_E5},
		[10] = { .key = 11, .code = MIDI_F5},
		[11] = { .key = 12, .code = MIDI_G5},
		[12] = { .key = 13, .code = MIDI_A5},
		[13] = { .key = 14, .code = MIDI_B5},

		[14] = { .key = 15, .code = MIDI_C6},
		[15] = { .key = 16, .code = MIDI_D6},

		[16] = { .key = 17, .code = KBD_SWITCH(0) },
		[17] = { .key = 18, .code = KBD_SWITCH(1) }
	}
};

