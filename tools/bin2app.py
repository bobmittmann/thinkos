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
import gzip
import zlib
import codecs
import crcmod
import io
import struct

__version__ = '0.1'

# --- constants
PY3 = sys.version_info >= (3, 0)

class FlatHdr(object):
  def __init__(self, data):
    hdr = struct.unpack('<IIII8s8s', data[0:32])
    self.entry = hdr[0]
    self.stack = hdr[1]
    self.stksz = hdr[2]
    self.size = hdr[3]
    self.os_tag = hdr[4]
    self.sys_tag = hdr[5]

    tab = struct.unpack('<IIIIIIII', data[32:64])
    self.text_start = tab[0]
    self.text_end = tab[1]
    self.data_start = tab[2]
    self.data_end = tab[3]
    self.bss_start = tab[4]
    self.bss_end = tab[5]
    self.ctor_start = tab[6]
    self.ctor_end = tab[7]

  def print(self):
    os_tag = self.os_tag.decode('ascii') 
    os_key = struct.unpack('<II', self.os_tag)
    sys_tag = self.sys_tag.decode('ascii') 
    sys_key = struct.unpack('<II', self.sys_tag)

    print('  -      Start: 0x{:08x}'.format(self.entry))
    print('  -      Stack: 0x{:08x}'.format(self.stack))
    print('  - Stack size: {:d}'.format(self.stksz))
    print('  -  File size: {:d}'.format(self.size))
    print('  -         Os: \"' + os_tag + '\" (0x{:08x} 0x{:08x})'.format(
                                  os_key[0], os_key[1]))
    print('  -        Sys: \"' + sys_tag + '\" (0x{:08x} 0x{:08x})'.format(
         sys_key[0], sys_key[1]))


    n = self.text_end - self.text_start
    print('  - .text: 0x{:08x} - 0x{:08x} : {:d}'.format(
                             self.text_start, self.text_end, n))

    n = self.data_end - self.data_start
    print('  - .data: 0x{:08x} - 0x{:08x} : {:d}'.format(
                             self.data_start, self.data_end, n))

    n = self.bss_end - self.bss_start
    print('  -  .bss: 0x{:08x} - 0x{:08x} : {:d}'.format(
                             self.bss_start, self.bss_end, n))

    n = self.ctor_end - self.ctor_start
    print('  - .ctor: 0x{:08x} - 0x{:08x} : {:d}'.format(
                             self.ctor_start, self.ctor_end, n))

def crctest(f, tag, bsize, data, size):
  crc32 = crcmod.predefined.mkCrcFun('crc-32')
  crc32_jamcrc = crcmod.predefined.mkCrcFun('jamcrc')
  crc32_mpeg = crcmod.predefined.mkCrcFun('crc-32-mpeg')
  crc32_posix = crcmod.predefined.mkCrcFun('posix')
  crc32_bzip2 = crcmod.predefined.mkCrcFun('crc-32-bzip2')

  crc32_stm32 = crcmod.mkCrcFun(0x104c11db7, initCrc=0, xorOut=0x00000000)

  crc0 = crc32(data)
  crc1 = crc32_jamcrc(data)
  crc2 = crc32_mpeg(data)
  crc3 = crc32_posix(data)
  crc4 = crc32_bzip2(data)
  crc5 = crc32_stm32(data)

  crc = crc2;

  print('  - CRC32: 0x{:08x} ... '.format(crc))
  print('  -   CRC: 0x{:08x} 0x{:08x} 0x{:08x} 0x{:08x} 0x{:08x}'.format(
                                   crc0, crc1, crc2, crc3, crc4))
  crc_stm32 = 0x0d1871bf
  dat = bytearray([0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80])
  i = 0
  while i < 8:
    crc0 = crc32_mpeg(dat[i:i+4])
    crc1 = crc32(dat[i:i+4])
    crc2 = crc32_jamcrc(dat[i:i+4])
    crc3 = crc32_posix(dat[i:i+4])
    crc4 = crc32_bzip2(dat[i:i+4])
    crc5 = crc32_stm32(dat[i:i+4])
    print('{:2d} - 0x{:08x} 0x{:08x} 0x{:08x} 0x{:08x} 0x{:08x} 0x{:08x} '.format(
           i, crc0, crc1, crc2, crc3, crc4, crc5))
    i += 4


  trail = (crc).to_bytes(4, byteorder='little') 

def bin2app(f, tag, bsize, data, size):
  app = FlatHdr(data)
  app.print()

  if app.size != size:
    print(' file size mismatch {:d} != {:d}. '.format(app.size, size))
    sys.exit(-1)

  if app.entry != (app.text_start + 65):
    print(' invalid entry point {:08x} != {:08x}. '.format(app.entry,
                               app.text_start + 64))
    sys.exit(-1)

  os_tag = ('ThinkOS\000').encode('ascii')
  if app.os_tag != os_tag:
    print(' invalid OS tag: \"' + os_tag.decode('ascii') + '\" != \"' + 
      os_tag.decode('ascii') + '\"')
#    print(' invalid OS name: \"' + app.os_tag + '\" != \"' + os_tag + '\"')

  rem = (size % bsize)
  if  rem != 0:
    n = bsize - rem
    print('   - Padding {:d} bytes...'.format(n))
    pad = bytearray(n)
    data += pad 
    size += n

  crc32_fun = crcmod.predefined.mkCrcFun('crc-32')
  crc32_mpeg = crcmod.predefined.mkCrcFun('crc-32-mpeg')
# crc32_stm32 = crcmod.mkCrcFun(0x104c11db7, initCrc=0xffffffff, rev=False, 
#                                xorOut=0x00000000)

  crc32_stm32 = crcmod.mkCrcFun(0x104c11db7, initCrc=0xffffffff, rev=False, 
                                xorOut=0x00000000)

  crc0 = crc32_stm32(data)
  crc1 = crc32_mpeg(data)

  crc = crc0;

  print('  - CRC32: 0x{:08x} '.format(crc))
  print('  -   CRC: 0x{:08x} '.format(crc1))

  trail = (crc).to_bytes(4, byteorder='little') 
  f.write(data)
  f.write(trail)
  f.close

def replacelst(s, lst, val):
  for c in lst:
      s = s.replace(c, val)
  return s

if __name__ == '__main__':
  from optparse import OptionParser
  parser = OptionParser(usage='''
  %prog binfile
  %prog -t [tag]''', version=__version__)
  parser.add_option("-o", "--output", dest="outfname", 
          help="write result to FILE", metavar="FILE")
  parser.add_option("-t", "--tag", dest="tag", 
          help="use tagname TAG", metavar="TAG")
  parser.add_option("-b", "--block-size", dest="bsize", type="int", default=4, 
          help="set block size to BSIZE bytes", metavar="BSIZE")
  parser.add_option("-z", "--zcompress", dest="compress", default=False,
          action="store_true", help="compress before converting")
  parser.add_option("-H", "--header", dest="header", 
          help="write header file", metavar="HEADER")

  options, args = parser.parse_args()

  if not args:
    parser.print_help()
    sys.exit(-1)

  infile = args[0]

  if options.tag:
    tag = options.tag
  else:
    tag = replacelst(os.path.basename(infile), [' ', '-', '.'], '_') 

  if options.bsize:
    bsize = options.bsize
  else:
    bsize = 4

  if (bsize % 4) != 0:
    print('Invalid block size: {:d}, must be multiple of 4!'.format(bsize))
    sys.exit(-1)
    

  if options.outfname:
    outfile = options.outfname
  else:
    outfile = os.path.splitext(os.path.basename(infile))[0] + '.app'

  fin = open(infile, 'rb')
  data = fin.read();
  fin.close()

  fout = open(outfile, 'wb+')

  if options.compress:
    zcompress = zlib.compressobj(level-12, startegy=Z_RLE)
    zdata = zcompress.compress(data);

  size = len(data);

  bin2app(fout, tag, bsize, data, size)

