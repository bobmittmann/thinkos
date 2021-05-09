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

  def print_info(self):
    os_tag = self.os_tag.decode('ascii') 
    os_key = struct.unpack('<II', self.os_tag)
    sys_tag = self.sys_tag.decode('ascii') 
    sys_key = struct.unpack('<II', self.sys_tag)

    print('  - Start addr: 0x{:08x}'.format(self.entry))
    print('  - Stack addr: 0x{:08x}'.format(self.stack))
    print('  - Stack size: {:-10d}'.format(self.stksz))
    print('  -  File size: {:-10d}'.format(self.size))
    print('  -     Os tag: \"' + os_tag + '\" (0x{:08x} 0x{:08x})'.format(
                                  os_key[0], os_key[1]))
    print('  -    Sys tag: \"' + sys_tag + '\" (0x{:08x} 0x{:08x})'.format(
         sys_key[0], sys_key[1]))

    print('  - Init table: ')
    print('    Idx  Name    Sart addr   End addr       Size ')
    n = self.text_end - self.text_start
    print('      0  .text   0x{:08x}  0x{:08x} {:-8d}'.format(
                             self.text_start, self.text_end, n))
    n = self.data_end - self.data_start
    print('      1  .data   0x{:08x}  0x{:08x} {:-8d}'.format(
                             self.data_start, self.data_end, n))
    n = self.bss_end - self.bss_start
    print('      2  .bss    0x{:08x}  0x{:08x} {:-8d}'.format(
                             self.bss_start, self.bss_end, n))
    n = self.ctor_end - self.ctor_start
    print('      3  .ctor   0x{:08x}  0x{:08x} {:-8d}'.format(
                             self.ctor_start, self.ctor_end, n))

  def print_magic(self):
    hdr_bin = struct.pack('<IIII8s8s', self.entry, self.stack, self.stksz,
              self.size, self.os_tag, self.sys_tag)

    hdr = struct.unpack('<IIIIIIII', hdr_bin)

    print('\t.rec = {')
    for u in hdr:
      print('\t\t {{ 0x{:08x}, 0x{:08x} }},'.format(0xffffffff, u))
    print('\t}')


def bin2app(f, tag, bsize, data, size):
  app = FlatHdr(data)
  app.print_info()

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

  rem = (size % bsize)
  if  rem != 0:
    n = bsize - rem
    print('   - Padding {:d} bytes...'.format(n))
    pad = bytearray(n)
    data += pad 
    size += n

  crc32_mpeg = crcmod.predefined.mkCrcFun('crc-32-mpeg')
  crc = crc32_mpeg(data)

  print('  - CRC32: 0x{:08x} '.format(crc))

  trail = struct.pack('<I', crc)
# trail = (crc).to_bytes(4, byteorder='little') 
  f.write(data)
  f.write(trail)
  f.close

  app.print_magic()


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

