/* 
 * File:	 midi.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2013 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __MIDI_H__
#define __MIDI_H__

/* ------------------------------------------------------------------------- 
   Keys/Notes
   ------------------------------------------------------------------------- */

/* Octave -1 */
#define MIDI_C      0
#define MIDI_CH     1
#define MIDI_D      2
#define MIDI_DH     3
#define MIDI_E      4
#define MIDI_F      5
#define MIDI_FH     6
#define MIDI_G      7
#define MIDI_GH     8
#define MIDI_A      9
#define MIDI_AH    10
#define MIDI_B     11
/* Octave 0 */
#define MIDI_C0    12
#define MIDI_CH0   13
#define MIDI_D0    14
#define MIDI_DH0   15
#define MIDI_E0    16
#define MIDI_F0    17
#define MIDI_FH0   18
#define MIDI_G0    19
#define MIDI_GH0   20
#define MIDI_A0    21
#define MIDI_AH0   22
#define MIDI_B0    23
/* Octave 1 */
#define MIDI_C1    24
#define MIDI_CH1   25
#define MIDI_D1    26
#define MIDI_DH1   27
#define MIDI_E1    28
#define MIDI_F1    29
#define MIDI_FH1   30
#define MIDI_G1    31
#define MIDI_GH1   32
#define MIDI_A1    33
#define MIDI_AH1   34
#define MIDI_B1    35
/* Octave 2 */
#define MIDI_C2    36
#define MIDI_CH2   37
#define MIDI_D2    38
#define MIDI_DH2   39
#define MIDI_E2    40
#define MIDI_F2    41
#define MIDI_FH2   42
#define MIDI_G2    43
#define MIDI_GH2   44
#define MIDI_A2    45
#define MIDI_AH2   46
#define MIDI_B2    47
/* Octave 3 */
#define MIDI_C3    48
#define MIDI_CH3   49
#define MIDI_D3    50
#define MIDI_DH3   51
#define MIDI_E3    52
#define MIDI_F3    53
#define MIDI_FH3   54
#define MIDI_G3    55
#define MIDI_GH3   56
#define MIDI_A3    57
#define MIDI_AH3   58
#define MIDI_B3    59
/* Octave 4 */
#define MIDI_C4    60
#define MIDI_CH4   61
#define MIDI_D4    62
#define MIDI_DH4   63
#define MIDI_E4    64
#define MIDI_F4    65
#define MIDI_FH4   66
#define MIDI_G4    67
#define MIDI_GH4   68
#define MIDI_A4    69
#define MIDI_AH4   70
#define MIDI_B4    71
/* Octave 5 */
#define MIDI_C5    72
#define MIDI_CH5   73
#define MIDI_D5    74
#define MIDI_DH5   75
#define MIDI_E5    76
#define MIDI_F5    77
#define MIDI_FH5   78
#define MIDI_G5    79
#define MIDI_GH5   80
#define MIDI_A5    81
#define MIDI_AH5   82
#define MIDI_B5    83
/* Octave 6 */
#define MIDI_C6    84
#define MIDI_CH6   85
#define MIDI_D6    86
#define MIDI_DH6   87
#define MIDI_E6    88
#define MIDI_F6    89
#define MIDI_FH6   90
#define MIDI_G6    91
#define MIDI_GH6   92
#define MIDI_A6    93
#define MIDI_AH6   94
#define MIDI_B6    95
/* Octave 7 */
#define MIDI_C7    96
#define MIDI_CH7   97
#define MIDI_D7    98
#define MIDI_DH7   99
#define MIDI_E7   100
#define MIDI_F7   101
#define MIDI_FH7  102
#define MIDI_G7   103
#define MIDI_GH7  104
#define MIDI_A7   105
#define MIDI_AH7  106
#define MIDI_B7   107
/* Octave 8 */
#define MIDI_C8   108
#define MIDI_CH8  109
#define MIDI_D8   110
#define MIDI_DH8  111
#define MIDI_E8   112
#define MIDI_F8   113
#define MIDI_FH8  114
#define MIDI_G8   115
#define MIDI_GH8  116
#define MIDI_A8   117
#define MIDI_AH8  118
#define MIDI_B8   119
/* Octave 9 */
#define MIDI_C9   120
#define MIDI_CH9  121
#define MIDI_D9   122
#define MIDI_DH9  123
#define MIDI_E9   124
#define MIDI_F9   125
#define MIDI_FH9  126
#define MIDI_G9   127


#define MIDI_C0_FREQ  (float)16.35160 /* 12 */
#define MIDI_CH0_FREQ (float)17.32391 /* 13 */
#define MIDI_D0_FREQ  (float)18.35405 /* 14 */
#define MIDI_DH0_FREQ (float)19.44544 /* 15 */
#define MIDI_E0_FREQ  (float)20.60172 /* 16 */
#define MIDI_F0_FREQ  (float)21.82676 /* 17 */
#define MIDI_FH0_FREQ (float)23.12465 /* 18 */
#define MIDI_G0_FREQ  (float)24.49971 /* 19 */
#define MIDI_GH0_FREQ (float)25.95654 /* 20 */
#define MIDI_A0_FREQ  (float)27.50000 /* 21 */
#define MIDI_AH0_FREQ (float)29.13524 /* 22 */
#define MIDI_B0_FREQ  (float)30.86771 /* 23 */
/* Octave 1 */
#define MIDI_C1_FREQ  (float)32.70320
#define MIDI_CH1_FREQ (float)
#define MIDI_D1_FREQ  (float)36.70810
#define MIDI_DH1_FREQ (float)
#define MIDI_E1_FREQ  (float)41.20344
#define MIDI_F1_FREQ  (float)43.65353
#define MIDI_FH1_FREQ (float)
#define MIDI_G1_FREQ  (float)48.99943
#define MIDI_GH1_FREQ (float)
#define MIDI_A1_FREQ  (float)55.00000	
#define MIDI_AH1_FREQ (float)
#define MIDI_B1_FREQ  (float)61.73541
/* Octave 2 */
#define MIDI_C2_FREQ  (float)65.40639
#define MIDI_CH2_FREQ (float)
#define MIDI_D2_FREQ  (float)73.41619
#define MIDI_DH2_FREQ (float)
#define MIDI_E2_FREQ  (float)82.40689
#define MIDI_F2_FREQ  (float)87.30706
#define MIDI_FH2_FREQ (float)
#define MIDI_G2_FREQ  (float)97.99886
#define MIDI_GH2_FREQ (float)
#define MIDI_A2_FREQ  (float)110.0000
#define MIDI_AH2_FREQ (float)
#define MIDI_B2_FREQ  (float)123.4708
/* Octave 3 */
#define MIDI_C3_FREQ  (float)130.8128 /*  48 */
#define MIDI_CH3_FREQ (float)
#define MIDI_D3_FREQ  (float)146.8324
#define MIDI_DH3_FREQ (float)
#define MIDI_E3_FREQ  (float)164.8138
#define MIDI_F3_FREQ  (float)174.6141
#define MIDI_FH3_FREQ (float)
#define MIDI_G3_FREQ  (float)195.9977
#define MIDI_GH3_FREQ (float)
#define MIDI_A3_FREQ  (float)220.0000
#define MIDI_AH3_FREQ (float)
#define MIDI_B3_FREQ  (float)246.9417
/* Octave 4 */
#define MIDI_C4_FREQ  (float)261.6265
#define MIDI_CH4_FREQ (float)277.1826
#define MIDI_D4_FREQ  (float)293.6648
#define MIDI_DH4_FREQ (float)311.1270
#define MIDI_E4_FREQ  (float)329.6276
#define MIDI_F4_FREQ  (float)349.2282
#define MIDI_FH4_FREQ (float)369.9944
#define MIDI_G4_FREQ  (float)391.9954
#define MIDI_GH4_FREQ (float)
#define MIDI_A4_FREQ  (float)440.0000
#define MIDI_AH4_FREQ (float)
#define MIDI_B4_FREQ  (float)493.8833
/* Octave 5 */
#define MIDI_C5_FREQ  (float)523.2511
#define MIDI_CH5_FREQ (float)
#define MIDI_D5_FREQ  (float)587.3295
#define MIDI_DH5_FREQ (float)
#define MIDI_E5_FREQ  (float)659.2551
#define MIDI_F5_FREQ  (float)698.4565
#define MIDI_FH5_FREQ (float)
#define MIDI_G5_FREQ  (float)783.9909
#define MIDI_GH5_FREQ (float)
#define MIDI_A5_FREQ  (float)880.0000
#define MIDI_AH5_FREQ (float)
#define MIDI_B5_FREQ  (float)987.7666
/* Octave 6 */
#define MIDI_C6_FREQ  (float)1046.502
#define MIDI_CH6_FREQ (float)
#define MIDI_D6_FREQ  (float)1174.659
#define MIDI_DH6_FREQ (float)
#define MIDI_E6_FREQ  (float)1318.510
#define MIDI_F6_FREQ  (float)1396.913
#define MIDI_FH6_FREQ (float)
#define MIDI_G6_FREQ  (float)1567.982
#define MIDI_GH6_FREQ (float)
#define MIDI_A6_FREQ  (float)1760.000
#define MIDI_AH6_FREQ (float)
#define MIDI_B6_FREQ  (float)1975.655 /*  95 */
/* Octave 7 */
#define MIDI_C7_FREQ  (float)2093.005 /*  96 */
#define MIDI_CH7_FREQ (float)2217.461 /*  97 */
#define MIDI_D7_FREQ  (float)2349.318 /*  98 */
#define MIDI_DH7_FREQ (float)2489.016 /*  99 */
#define MIDI_E7_FREQ  (float)2637.020 /* 100 */
#define MIDI_F7_FREQ  (float)2793.826 /* 101 */
#define MIDI_FH7_FREQ (float)2959.955 /* 102 */
#define MIDI_G7_FREQ  (float)3135.963 /* 103 */
#define MIDI_GH7_FREQ (float)3322.438 /* 104 */
#define MIDI_A7_FREQ  (float)3520.000 /* 105 */
#define MIDI_AH7_FREQ (float)3729.310 /* 106 */
#define MIDI_B7_FREQ  (float)3951.066 /* 107 */
/* Octave 8 */
#define MIDI_C8_FREQ  (float)4186.009 /* 108 */
#define MIDI_CH8_FREQ (float)4434.922 /* 109 */
#define MIDI_D8_FREQ  (float)4698.636 /* 110 */
#define MIDI_DH8_FREQ (float)4978.032 /* 111 */
#define MIDI_E8_FREQ  (float)5274.041 /* 112 */
#define MIDI_F8_FREQ  (float)5587.652 /* 113 */
#define MIDI_FH8_FREQ (float)5919.911 /* 114 */
#define MIDI_G8_FREQ  (float)6271.927 /* 115 */
#define MIDI_GH8_FREQ (float)6644.875 /* 116 */
#define MIDI_A8_FREQ  (float)7040.000 /* 117 */
#define MIDI_AH8_FREQ (float)7458.620 /* 118 */
#define MIDI_B8_FREQ  (float)7902.133 /* 119 */
/* Octave 9 */
#define MIDI_C9_FREQ  (float)8372.018 /* 120 */
#define MIDI_CH9_FREQ (float)8869.844 /* 121 */
#define MIDI_D9_FREQ  (float)9397.273 /* 122 */
#define MIDI_DH9_FREQ (float)9956.063 /* 123 */
#define MIDI_E9_FREQ  (float)10548.08 /* 124 */
#define MIDI_F9_FREQ  (float)11175.30 /* 125 */
#define MIDI_FH9_FREQ (float)11839.82 /* 126 */
#define MIDI_G9_FREQ  (float)12543.85 /* 127 */


#define MIDI_NOTE_OFF          0x80
#define MIDI_NOTE_ON           0x90
#define MIDI_PRESSURE          0xA0
#define MIDI_CONTROLLER_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE    0xC0
#define MIDI_CHANNEL_PRESSURE  0xD0
#define MIDI_PITCH_BEND        0xE0

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif	

#endif /* __IO_H__ */

