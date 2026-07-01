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
#    File: fftp.py
# Comment: JTAG tftp upload utility
#  Author: Robinson Mittmann <bobmittmann@gmail.com>
# 

from math import *
import sys

# --- constants
PY3 = sys.version_info >= (3, 0)


# ----------------------------------------------------------------------
# Clock Exceptions
# ----------------------------------------------------------------------

class ClkException(Exception):
    """Parent class of all Clk exceptions."""
    pass

class Clk(object):
    """Abstract parent class of Clock classes."""
    def __init__(self, tag=""):
        self.freq = 0
        self.tag = tag

    def freq_get():
        raise NotImplementedError("Abstract method")

    def freq_set(freq):
        raise NotImplementedError("Abstract method")

    def is_freq_valid(self, freq):
        raise NotImplementedError("Abstract method")

    def freq_hz(self):
        fhz = self.freq_get()
        return fhz * 1.0

    def freq_khz(self):
        fhz = self.freq_get()
        return fhz * 1e-3

    def freq_mhz(self):
        fhz = self.freq_get()
        return fhz * 1e-6

class Osc(Clk):
    def __init__(self, tag, fmin, fmax):
        Clk.__init__(self, tag)
        self.fmin = fmin
        self.fmax = fmax

    def __init__(self, tag, freq, fmin=None, fmax=None):
        Clk.__init__(self, tag)
        if (fmin == None):
            fmin = freq
        if (fmax == None):
            fmax = freq
        self.fmin = fmin
        self.fmax = fmax
        self.freq = freq

    def is_valid(self):
        fout = self.freq
        return (fout <= self.fmax) and (fout >= self.fmin)

    def freq_get(self):
        return self.freq

    def freq_set(self, freq):
        if ((freq < self.fmin) or (freq > self.fmax)):
            raise ClkException("Frequency invalid {:d}".format(freq))
        self.freq = freq

class ClkDiv(Clk):
    def __init__(self, tag, clkin, q=1, qmin=None, qmax=None, qstep=1, fmin=None, fmax=None):
        Clk.__init__(self, tag)
        self.clkin = clkin
        self.q = q
        if (qmin == None):
            qmin = q
        if (qmax == None):
            qmax = q
        if (fmin == None):
            fmin = 0
        if (fmax == None):
            fmax = 1e9
        self.qmin = qmin
        self.qmax = qmax
        self.qstep = qstep
        self.fmin = fmin
        self.fmax = fmax

    def get(self):
        return self.q

    def set(self, q):
        self.q = q
        
    def is_valid(self):
        f = self.freq_get()
        q = self.q
        return self.clkin.is_valid() and (q <= self.qmax) and (q >= self.qmin) and (f <= self.fmax) and (f >= self.fmin)

    def freq_get(self):
        return self.clkin.freq_get() / self.q

    def range(self):
        return range(self.qmin, self.qmax + self.qstep, self.qstep)

    def clk(self):
        return self;

class ClkMux(Clk):
    def __init__(self, lst, tag=""):
        Clk.__init__(self, tag)
        self.clk = []
        for x in lst:
            self.clk.append(x)
        self.sel = 0

    def freq_get(self):
        return self.clk[self.sel].freq_get() 

    def is_valid(self):
        return self.clk[self.sel].is_valid()

    def select(self, sel):
        selmin = 0
        selmax = len(self.clk) - 1
        if (sel < selmin):
            sel = selmin
        if (sel > selmax):
            sel = selmax
        self.sel = sel    

    def clk(self):
        return self.clk[self.sel]

class Vco(Clk):
    def __init__(self, tag, clkin, n=1, nmin=None, nmax=None, fmin=None, fmax=None):
        Clk.__init__(self, tag)
        self.clkin = clkin
        self.n = n
        if (nmin == None):
            nmin = n
        if (nmax == None):
            nmax = n
        if (fmin == None):
            fmin = 1 * clkin.freq_get()
        if (fmax == None):
            fmax = n * clkin.freq_get()
        self.nmin = nmin
        self.nmax = nmax
        self.fmin = fmin
        self.fmax = fmax
        self.n = n

    def freq_get(self):
        return self.clkin.freq_get() * self.n

    def is_valid(self):
        fin = self.clkin.freq_get()
        n = self.n
        fout = fin * n
        return self.clkin.is_valid() and (fout <= self.fmax) and \
            (fout >= self.fmin) and (n <= self.nmax) and (n >= self.nmin)

    def get(self):
        return self.n

    def set(self, n):
        self.n = n

    def range(self):
        return range(self.nmin, self.nmax + 1)

class Pll(object):
    """Abstract parent class of PLL classes."""
    def __init__(self, in_div, vco, out_div):
        self.in_div = in_div
        self.out_div = out_div
        self.vco = vco

    def info(self):
        raise NotImplementedError("Abstract method")

