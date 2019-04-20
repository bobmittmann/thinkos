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

from __future__ import print_function
import sys
import os
import re
import codecs
import subprocess

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
'''

__info = '''
/** 
 * @file {:s}
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */
'''

def log(level, s):
  if (level > 5):
    return
  try:
    print(s, file=sys.stderr)
  except:
    print(u'?????', file=sys.stderr)
  sys.stderr.flush()

# -------------------------------------------------------------------------

class Comment(object):
  def __init__(self):
    self.line = ''
    self.buf = ''

  def append(self, s):
    s = s.rstrip()
    s = s.lstrip()
    line = self.line

    if (line == ''):
      if (self.buf == ''):
        line = '/* ' + s
      else:
        line = s
    elif (line[-1] == '.'):
      self.buf = self.buf + line + '\n'
      line = '   ' + s
    elif re.match(r'^[0-9A-Za-z]+:', s):
      self.buf = self.buf + line + '\n'
      line = '   ' + s
    else:
      line = line + ' ' + s

    if (len(line) > 78):
      lst = line.split()
      n = 0
      line = u''
      for w in lst:
        n = n + 1 + len(w)
        if (n > 78):
           self.buf = self.buf + line + '\n'
           line = '   ' + w
           n = 3 + len(w)
        elif (w == '/*'):
           line = w
           n = len(w)
        elif (line == ''):
           line = '   ' + w
           n = 3 + len(w)
        else:
          line = line + ' ' + w

    self.line = line

  def text(self):
    s = self.buf + self.line
    if (s != ''):
      s = s + ' */\n\n'
    return s
    
# -------------------------------------------------------------------------

class Field(object):
  def __init__(self, name, mod, begin, end, desc = ''):
    self.mod = mod.upper()
    self.name = name.upper()
    self.begin = int(begin)
    self.end = int(end) 
    if (self.begin > self.end):
      log(10, '{:s}: begin({:d}) > end({:d})'.format(self.name, 
        self.begin, self.end))
      self.begin = int(end)
      self.end = int(begin)
    self.desc = desc
    self.comment = Comment()
    if (self.begin == self.end):
      log(4, u'     - {:s}_{:s} Bit {:d}'.format(self.mod, 
        self.name, self.begin))
    else:
      log(4, u'     - {:s}_{:s} Bits {:d}..{:d}'.format(self.mod,
        self.name, self.begin, self.end))

  def append(self, s):
    self.comment.append(s)

  def text(self):
    return self.buf

  def format(self, mod):
    begin = self.begin
    end = self.end
    if (self.name == ''):
      if (begin == end):
        # Comment reserved single bits 
        s = u'/* Bit {:d}'.format(begin)
      else:
        # Comment reserved multiple bits 
        s = u'/* Bits [{:d}..{:d}]'.format(begin, end)
      s = s + ' - ' + self.desc + ' */\n\n'
    else:
      sym = mod + '_' + self.name
      shift = '{:d}'.format(begin)
      if (begin == end):
        # single bit field
        s = '/* Bit {:d}'.format(begin) + ' - ' + self.desc + ' */\n'
        s = s + '#define ' + sym + ' (1 << ' + shift + ')\n'
      else:
        s = u'/* Bits [{:d}..{:d}] - '.format(begin, end) + self.desc + ' */\n'
        mask = (1 << (end - begin + 1)) - 1
        s = s + '#define ' + sym + '_MSK (0x{:x}'.format(mask)
        s = s + ' << ' + shift + ')\n'
        s = s + '#define ' + sym + '_SET(VAL) (((VAL) << ' + shift + ') & '
        s = s + sym + '_MSK)\n'
        s = s + '#define ' + sym + '_GET(REG) (((REG) & ' + sym + '_MSK) >> '
        s = s + shift + ')\n'
      # append the comment
      s = s + self.comment.text()
    return s


# -------------------------------------------------------------------------

class Register(object):
  def __init__(self, mod, name, desc = ''):
    self.mod = mod.upper() 
    self.name = name
    self.offs = [] 
    self.desc = desc
    self.reset = ''
    self.access = ''
    self.fields = []
    self.comment = Comment()
    if (name == []):
      log(0, 'Empty name list')
      raise 'hello'
    log(1, u'Register: {:s}_{:s} - {:s}'.format(self.mod, 
      self.name[0], self.desc))

  def format(self, mod):
      mod = self.mod
      name = self.name
      desc = self.desc
      if len(self.name) != len(self.offs):
        log(0, 'len(name({:d}) != offs({:d}))'.format(len(self.name), 
          len(self.offs)))
        for n in self.name:
          log(0, '  name= "' + n + '"')
        for o in self.offs:
          log(0, '  offs= 0x{:x}'.format(o))
        raise Exception('Offsets != names')
      s = u'/* --------------------------------------'
      s = s + '----------------------------------- */\n'
      if (len(self.name) == 1):
        offs = self.offs[0]
        s = s + '/* ' + desc + ' - ' + name[0] + ' */\n'
        s = s + '#define STM32_' + mod + '_' + name[0]
        s = s + ' 0x{:04x}\n\n'.format(offs)
      else:
        try:
          i = 0
          for offs in self.offs:
            s = s + '/* ' + desc + ' - ' + name[i] + ' */\n'
            s = s + '#define STM32_' + mod + '_' + name[i]
            s = s + ' 0x{:04x}\n\n'.format(offs)
            i = i + 1
        except:
          log(0, '!! Error !!!')
          for name in self.name:
            print('=====', file=sys.stderr)
            print(self.name, file=sys.stderr)
#          log(0, 'name=' + ' '.join(self.name))
#          log(0, 'offs=' + ' '.join(self.offs))
          raise
      # append the comment if any 
      # s = s + self.comment.text()
      for f in self.fields:
        s = s + f.format(mod)
      return s

  def append(self, s, num):

    # address offset
    m = re.search(r'[Aa]ddress[ a-zA-Z]*:[ ]*0x([0-9A-F]+)', s)
    if (m):
      x = m.groups()
      offs = int(x[0], 16)
      self.offs.append(offs)
      name = self.name[0]
      log(2, u'   * Offs: {:s}_{:s} 0x{:x}'.format(self.mod, name, offs))
      return

    # address offset alternate form
    m = re.search(r'[Aa]ddress[ a-zA-Z]*:' + 
        r'[ ]*([0-9A-Za-z][0-9A-Za-z _-]+):' +
        r'[ ]*0x([0-9A-F]+)', s)
    if (m):
      x = m.groups()
      offs = int(x[1], 16)
      self.offs.append(offs)
#     i = len(self.offs) - 1
      name = self.name[0]
      log(2, u'   * Offs(Alt): {:s}_{:s} 0x{:x}'.format(self.mod, name, offs))
      return

    # reset value
    m = re.match(r'[Rr]eset.*:[ ]*0x([0-9A-FX]+)[ ]+([0-9A-FX]+)', s)
    if (m):
      x = m.groups()
      self.reset = x[0] + ' ' + x[1]
      log(2, u'   * Reset: {:s}'.format(self.reset))
      return

    # access
    m = re.match(r'[A]ccess:[ ]*([0-9A-za-z].+)', s)
    if (m):
      x = m.groups()
      self.access = x[0]
      log(2, u'   * Access: ' + self.access)
      return

    # reserved multiple bits 
    m = re.match(r'Bit[s]* ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])' + 
      r'[ ]*[\n]*(Reserved.*)', s)
    if (m):
      x = m.groups()
      begin = x[1]
      end = x[0]
      desc = x[2]
      field = Field('', '', begin, end, desc)
      self.fields.append(field)
      return

    # reserved single bits 
    m = re.match(r'Bit[s]* ([1-3]{,1}[0-9])[ ]*[\n]*(Reserved.*)', s)
    if (m):
      x = m.groups()
      begin = x[0]
      end = x[0]
      desc = x[1]
      field = Field('', '', begin, end, desc)
      self.fields.append(field)
      return 

    # multiple bits field
    m = re.match(r'Bit[s]* ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])[ ]*' +
      r'([A-Z_][A-Za-z_0-9]*)\[([1-3]{,1}[0-9])[:.]+' + 
      r'([1-3]{,1}[0-9])\]:[ ]+(.*)', s)
    if (m):
      x = m.groups()
      name = x[2]
      begin = x[1]
      end = x[0]
      desc = x[5]
      field = Field(name, self.mod, begin, end, desc)
      self.fields.append(field)
      return 

    # multiple bits field
    m = re.match(r'Bit[s]* ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])' + 
      r'[ ]+([A-Z_][A-Za-z_0-9]*):[ ]+(.*)', s)
    if (m):
      x = m.groups()
      name = x[2]
      begin = x[1]
      end = x[0]
      desc = x[3]
      field = Field(name, self.mod, begin, end, desc)
      self.fields.append(field)
      return 

    # multiple bits field
    m = re.match(r'Bits ([1-3]{,1}[0-9]):([1-3]{,1}[0-9])' + 
      r'[ ]+([A-Z_][A-Za-z_0-9]*)[:]?', s)
    if (m):
      x = m.groups()
      name = x[2]
      begin = x[1]
      end = x[0]
      desc = u''
      field = Field(name, self.mod, begin, end, desc)
      self.fields.append(field)
      return 


    # single bit field
    m = re.match('Bit[s]* ([1-3]{,1}[0-9])[ ]+' + 
      r'([A-Z_][A-Za-z_0-9]*):[ ]+(.*)', s)
    if (m):
      x = m.groups()
      name = x[1]
      begin = x[0]
      end = x[0]
      desc = x[2]
      field = Field(name, self.mod, begin, end, desc)
      self.fields.append(field)
      return 

    # single bit field
    m = re.match('Bit ([1-3]{,1}[0-9])[ ]+' + 
      r'([A-Z_][A-Za-z_0-9]*)[:]?', s)
    if (m):
      x = m.groups()
      name = x[1]
      begin = x[0]
      end = x[0]
      desc = u''
      field = Field(name, self.mod, begin, end, desc)
      self.fields.append(field)
      return 

    # try to remove some garbage generated by the PDF tables decoding
    if (re.match('^[0-9rws ]+$', s)):
      return 
    if (re.match('^(Res.+){2,}$', s)):
      return 

    if (len(self.fields) > 0):
      # all other lines are comments to the current field if one exists
      self.fields[-1].append(s)
    else:
      log(3, u' {:5d} "'.format(num) + s + u'"')
      # otherwise are considered general register comments
      self.comment.append(s)

# -------------------------------------------------------------------------

class Peripheral(object):
  def __init__(self, name, desc = ''):
    self.name = name.upper()
    self.desc = desc
    self.regs = []
    self.lines = []
    self.foot = False
    self.foot_re = re.compile(r'.*DocID[A-Za-z 0-9].*')
    self.mod_re = re.compile(r'' + self.desc + '[ ]+\(' + self.name + '\)')
    self.head_re = re.compile(r'RM[0-9].*')

  def append(self, line, num):
    s = line.rstrip()
    s = s.lstrip()

    # skip empty lines
    if (s == ''):
      return

    # skip page headers and footers
    if (self.foot):
      if self.mod_re.search(s):
        self.foot = False
      elif self.head_re.search(s):
        self.foot = False
    else:
      if self.foot_re.search(s):
        self.foot = True
      else:
        self.lines.append((s, num))

  def filter(self):
    lines = self.lines

    n = len(lines)
    i = 0
    reg = None
    while (i < n):
      s = lines[i][0]
      num = lines[i][1]
      i = i + 1

      # append to the next line in case the register section was split in 
      # two lines
      if (i < n):
        ss = s + ' ' + lines[i][0]
        if ((i + 1) < n):
          ss = ss + ' ' + lines[i + 1][0]
      else:
        ss = s
      log(6, u'{:d}: "{:s}"'.format(num, ss))

      # Find register entries
      m = re.match(r'^([0-9.]*)[ ]*' +
        r'([A-Z][A-Za-z0-9_ /&-]+[A-Za-z0-9])' +
        r'[ ]?\(([A-Z][A-Za-z0-9]+)_([A-Za-z0-9_]+)' +
        r'([ /]+(([A-Z][A-Za-z0-9]+)_([A-Za-z0-9_]+)))*\)[ ]' +
        r'.*[Aa]ddress[ a-zA-Z:]*:[ ]+0x[0-9A-F]+', ss)
      if (m):
        x = m.groups()
        try:
          desc = x[1]
          mod = x[2]
          name = []
          j = 3
          while (j < len(x)):
            sym = x[j]
            if (sym != None):
               name.append(sym)
            j = j + 4
          reg = Register(mod, name, desc)
          self.regs.append(reg)
        except:
          log(1, '\nParse error at line: {:d}\n'.format(num))
          log(1, '  "' + s + '"\n')
          raise
      else:
        m = re.match(r'^([0-9.]*)[ ]*' +
          r'([A-Z][A-Za-z0-9_ /&-]+[A-Za-z0-9])' +
          r'[ ]+\(([A-Z][A-Z0-9]+)\)' +
          r'.*[Aa]ddress[ a-zA-Z:]*:[ ]+0x[0-9A-F]+', ss)
        if (m):
          x = m.groups()
          try:
            desc = x[1]
            mod = self.name
            name = [x[2]]
            reg = Register(mod, name, desc)
            self.regs.append(reg)
          except:
            log(1, '\nParse error at line: {:d}\n'.format(num))
            log(1, '  "' + s + '"\n')
            raise

      if (reg):
        # add line to the current register
        reg.append(s, num)
      else:
        # log the remaining lines for debugging
        log(4, u' {:5d} "'.format(num) + s + u'"')

  def struct(self):
    s = u'struct stm32_' + self.name.lower() + ' {\n'
    offs = 0
    res = 0
    for r in self.regs:
      if (r.offs[0] > offs):
        cnt = int((r.offs[0] - offs) / 4)
        s = s + '\tuint32_t res{:d}[{:d}];\n'.format(res, cnt)
        res = res + 1

      sym = r.name[0].lower()

      if (r.offs[0] < offs):
        res = 0
        s = s + '};\n\n'
        s = s + 'struct stm32_' + self.name.lower() + '_XYZ {\n'
        s = s + '\tvolatile uint32_t '
        s = s + sym + '; /* 0x{:02x} */\n'.format(r.offs[0])
      else:
        s = s + '\tvolatile uint32_t '
        s = s + sym + '; /* 0x{:02x} */\n'.format(r.offs[0])

      offs = r.offs[0] + 4

    s = s + '};\n\n'
    return s

  def dump(self, f):
    for line in self.lines:
      f.write(line[0] + '\n')


  def format(self):
    s = u''

    mod = self.name
    s = s + '\n\n'
    s = s + '/* --------------------------------------'
    s = s + '----------------------------------- */\n'
    s = s + '/* ' + mod + ' - ' + self.desc + ' */\n'
    s = s + '/* --------------------------------------'
    s = s + '----------------------------------- */\n\n'

    s = s + '#ifndef __STM32_' + mod + '_H__\n'
    s = s + '#define __STM32_' + mod + '_H__\n'
    s = s + '\n\n'

    for r in self.regs:
      s = s + r.format(mod)

    s = s+ '\n\n'
    s = s + '#ifndef __ASSEMBLER__\n\n'
    s = s + '#include <stdint.h>\n\n'

    s = s + self.struct()

    s = s + '#endif /* __ASSEMBLER__ */\n\n'
    s = s + '#endif /* __STM32_' + mod + '_H__ */\n'
    s = s + '\n'

    return s

# -------------------------------------------------------------------------
# Parse the PDF text and extract the Peripherals description sections
# Return a list of peripherals
def extract(f):
  # Regular expression to mach a section header in the document
  section_re = re.compile(r'^([1-9][0-9]*)[ ]+' + 
    r'([A-Z][(A-Za-z0-9() _/-]+[A-Za-z0-9)])$')
  # Regular expression to mach a peripheral section header
  peripheral_re = re.compile(r'^([1-9][0-9]*)[ ]+' +
    r'([A-Za-z][A-Za-z0-9 _/-]+[A-Za-z0-9])[ ]+' + 
    r'\(([A-Za-z][A-Za-z0-9_]+)([/]([A-Za-z][A-Za-z0-9_]+))*\)')
  # Regular expression to mach a 'registers' subsection
  regsub_re = re.compile(r'([1-9][0-9]?[.][1-9]+)[ ]+([A-Z][A-Za-z0-9_/ ]*)' + 
    r'[ ]+[Rr]egisters')
  # Regular expression to mach a 'register map' subsection
  mapsub_re = re.compile(r'Table [1-9][0-9.]*[ ]+' + 
    r'([A-Za-z][A-Za-z0-9_/]+)' + 
    r'[ -]+[Rr]egister map')

  mod = None
  flag = False
  done = False
  lst = []
  num = 0

  while not done:
    # find section
    for line in f:
      if (line == '\14'):
        continue
      if (line == '\13'):
        continue
      num = num + 1
      m = section_re.match(line)
      if (m):
        log(8, '{:d}: "'.format(num) + line + '"')
        sec = m.groups()[0]
        desc = m.groups()[1]
        log(4, '(' + sec + ') - ' + desc)
        # merge with the next line as the module symbol may be there
        line = line + f.readline()
        # remove CR or LF
        line = re.sub('[\r\n]','', line)
        # replace double spaces 
        line = re.sub('[ ]+',' ', line)
        num = num + 1
        # try to match a peripheral section
        m = peripheral_re.match(line)
        if (m):
          x = m.groups()
          sec = x[0]
          name = x[2]
          desc = x[1]
          mod = Peripheral(name, desc)
          log(1, 'Peripheral: ' + mod.name + ' - ' + mod.desc)
        else:
          log(5, '{:d}: "'.format(num) + line + '"')
      elif (mod):
        # find 'registers' subsection
        m = regsub_re.search(line)
        if (m):
          x = m.groups()
          lst.append(mod)
          log(3, '{:d}: ({:s}) {:s} registers'.format(num, x[0], mod.name))
          break
      else:
        log(8, '{:d}: "'.format(num) + line + '"')

    if mod == None:
      done = True

    # a 'register map' subsection marks the end of the module
    for line in f:
      num = num + 1
      if mapsub_re.search(line):
        mod = None
        break
      # add the lines to the module
      mod.append(line, num)

  return lst

# -------------------------------------------------------------------------
# Print the peripherals list content
def dump(f, lst):
  for mod in lst:
    mod.dump(f)

def print_module(f, mod):
   s = mod.format() 
   f.write(s)

# -------------------------------------------------------------------------
# Script main function
if __name__ == '__main__':
  from optparse import OptionParser
  parser = OptionParser(usage='''
  %prog txtfile
  %prog -n [name]''', version=__version__)
  parser.add_option("-o", "--output", dest="outfname", 
          help="write result to FILE", metavar="FILE")
  parser.add_option("-n", "--name", dest="name", 
          help="write module NAME", metavar="NAME")
  parser.add_option("-p", "--pdf", dest="pdf", default=False,
          action="store_true", help="read PDF file")

  options, args = parser.parse_args()

  if not args:
    parser.print_help()
    sys.exit(-1)

  if options.name:
    name = options.name
  else:
    name = 'xxx'

  infile = args[0]

  if options.outfname:
    fname = os.path.basename(options.outfname)
    fout = codecs.open(options.outfname, 'w+', encoding='utf8')
  else:
    fname = 'stm32_xxx.h'
    fout = codecs.getwriter('utf8')(sys.stdout)

  if options.pdf:
    log(0, 'Converting PDF to TXT ...')
    subprocess.check_call(["pdftotext", "-layout", 
      "-enc", "UTF-8", "-eol", "unix", infile])
    infile = os.path.splitext(infile)[0]+'.txt'

  log(0, 'Processin file: ' + infile)
  fin = codecs.open(infile, 'r', encoding='utf8')
  modules = extract(fin)
  log(0, '{:d} Modules found'.format(len(modules)))

  if options.name:
    for m in modules: 
      if (m.name == options.name):
        log(0, '+++ Module: ' + m.name)
        m.filter() 
        print_module(fout, m)
  else:
    for m in modules: 
      log(0, '+++ Module: ' + m.name)
      m.filter() 
      print_module(fout, m)

