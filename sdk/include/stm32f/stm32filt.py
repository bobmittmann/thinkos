#!/usr/bin/env python

#
# Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
# 
# This file is part of the YARD-ICE.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You can receive a copy of the GNU Lesser General Public License from 
# http://www.gnu.org/

#
#    File: bin2hex.py
# Comment: binary to C hex array utility
#  Author: Robinson Mittmann <bobmittmann@gmail.com>
# 

import binascii  
import sys
import os
import re
import codecs
import io
from subprocess import call

__version__ = '0.2'

__header = '''
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
 * @file stm32f-comp.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */
'''

def desc_append(desc, s):
  s = s.rstrip()
  s = s.lstrip()
  if (len(s) > 78):
    lst = s.split()
    n = 0
    s = ''
    for w in s:
      if (n + len(w) + 1) > 78:
        s = s + '\n'
        n = 0
      else:
        n = n + len(w)
        s = s + ' '
      s = s + w
  desc = desc + s

def filter(f, name, data, size):

  lines = data.split('\n')

  mod = name.upper()

  i = 0
  j = 0
  n = len(lines)
  while (i < n):
    s = lines[i]
    i = i + 1
    # remove CR
    s = re.sub('\r','', s)
    # skip empty lines
    if re.match(r'[ \t]*$', s):
	  continue
    lines[j] = s
    j = j + 1

  f.write(__header)
  f.write('\n\n')
  f.write('#ifndef __STM32_' + name.upper() + '_H__\n')
  f.write('#define __STM32_' + name.upper() + '_H__\n')
  f.write('\n\n')

  desc = False
  n = j
  i = 0
  while (i < n):
    s = lines[i]
    i = i + 1

    # Handle registers
    m = re.search(r'[ \t]*[A-Za-z_0-9]+.*register.*[ ]+\([A-Z_][A-Za-z_0-9]*\)', s)
    if (m):
      s = s + ' ' + lines[i]	
      i = i + 1;
      x = re.split(r'[ \t]*([A-Za-z_0-9]+)[ ]+([A-Za-z_0-9]+.*)[ ]+\(([A-Z_][A-Za-z_0-9]*)\).*Address offset: (0x[0-9a-fA-F]+).*', s)
      s = ''
      if (desc):
        s = ' */\n\n'
        desc = False
      s = s + '/* ' + x[1] + ' ' + x[2] + ' */\n'
      s = s + '#define STM32_' + x[3].upper() + ' ' + x[4] + '\n'
      mod = x[1].upper()
      f.write(s)
      continue

	# Comment reserved multiple bits 
    m = re.search(r'[ \t]*Bit[s]* ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])[ ]*[\n]*(Reserved.*)', s)
    if (m):
      x = m.groups()
      s = ''
      if (desc):
        s = ' */\n\n'
        desc = False
      s = s + '/* [' + x[0] + '..' + x[1] + '] ' + x[2] + ' */\n'
      f.write(s)
      continue

    # Comment reserved single bits 
    m = re.search('[ \t]*Bit[s]* ([1-3]{,1}[0-9])[ ]*[\n]*(Reserved.*)', s)
    if (m):
      x = m.groups()
      s = ''
      if (desc):
        s = ' */\n\n'
        desc = False
      s = s + '/* Bit ' + x[0] + ' ' + x[1] + ' */\n'
      f.write(s)
      continue

    # Handle multiple bits field
    m = re.search('[ \t]*Bit[s]* ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])[ ]+([A-Z_][A-Za-z_0-9]*)\[([1-3]{,1}[0-9]):([1-3]{,1}[0-9])\]:[ ]+(.*)', s)
    if (m):
      x = m.groups()
      s = ''
      if (desc):
        s = ' */\n\n'
        desc = False
      s = s + '/* Bits [' + x[0] + '..' + x[1] + '] - ' + x[5] + ' */\n'
      sym = mod + '_' + x[2].upper()
      mask = (1 << (int(x[3]) + 1)) - 1
      shift = '{:d}'.format(int(x[1]))

      s = s + '#define ' + sym + '_MSK ' + '(0x{:x}'.format(mask) + ' << ' + shift + ')\n'
      s = s + '#define ' + sym + '_SET(VAL) (((VAL) << ' + shift + ') & ' + sym + '_MSK)\n'
      s = s + '#define ' + sym + '_GET(REG) (((REG) & ' + sym + '_MSK) >> ' + shift + ')\n'

      s = s + '/* '
      desc = True
      f.write(s)
      continue

    # Handle multiple bits field
    m = re.search('[ \t]*Bit[s]* ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])[ ]+([A-Z_][A-Za-z_0-9]*):[ ]+(.*)', s)
    if (m):
      x = m.groups()
      s = ''
      if (desc):
        s = ' */\n\n'
        desc = False
      s = s + '/* Bits [' + x[0] + '..' + x[1] + '] - ' + x[3] + ' */\n'
      sym = mod + '_' + x[2].upper()
      mask = (1 << (int(x[1]) - int(x[0]) + 1)) - 1
      shift = '{:d}'.format(int(x[1]))

      s = s + '#define ' + sym + '(0x{:x}'.format(mask) + ' << ' + shift + ')\n'
      s = s + '#define ' + sym + '_SET(VAL) (((VAL) << ' + shift + ') & ' + sym + '_MSK)\n'
      s = s + '#define ' + sym + '_GET(REG) (((REG) & ' + sym + '_MSK) >> ' + shift + ')\n'

      s = s + '/* '
      desc = True
      f.write(s)
      continue

    # Handle single bit field
    m = re.search('[ \t]*Bit[s]* ([1-3]{,1}[0-9])[ ]+([A-Z_][A-Za-z_0-9]*):[ ]+(.*)', s)
    if (m):
      x = m.groups()
      s = ''
      if (desc):
        s = ' */\n\n'
        desc = False
      s = s + '/* Bit ' + x[0] + ' - ' + x[2] + ' */\n'
      sym = mod + '_' + x[1].upper()
      shift = '{:d}'.format(int(x[0]))
      s = s + '#define ' + sym + ' (1 << ' + shift + ')\n'
      s = s + '/* '
      desc = True
      f.write(s)
      continue

    f.write(s.encode('utf-8'))
#    f.write(s)

  if (desc):
    s = ' */\n\n'
    f.write(s)

  f.write('\n\n')
  f.write('#endif /* __STM32_' + name.upper() + '_H__ */\n')
  f.write('\n')


if __name__ == '__main__':
  from optparse import OptionParser
  parser = OptionParser(usage='''
  %prog txtfile
  %prog -n [name]''', version=__version__)
  parser.add_option("-o", "--output", dest="outfname", 
					help="write result to FILE", metavar="FILE")
  parser.add_option("-n", "--name", dest="name", 
					help="use module NAME", metavar="NAME")

  options, args = parser.parse_args()

  if not args:
    parser.print_help()
    sys.exit(-1)

  if options.name:
    name = options.name
  else:
    name = 'MODULE'

  infile = args[0]

  if options.outfname:
    fout = codecs.open(options.outfname, 'w+', encoding='utf8')
  else:
    fout = sys.stdout

  fin = codecs.open(infile, 'r', encoding='utf8')
  data = fin.read();
  fin.close()

  size = len(data);
  filter(fout, name, data, size)
  fout.close()

  if options.outfname:
	call(["indent", "-linux", "-fc1", "-fca", options.outfname])

