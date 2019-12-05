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
# PLL Exceptions
# ----------------------------------------------------------------------

class ClkException(Exception):
	"""Parent class of all Clk exceptions."""
	pass

class Clk(object):
	"""Abstract parent class of Clock classes."""
	def __init__(self):

	def freq_get():
		raise NotImplementedError("Abstract method")

	def freq_set(freq):
		raise NotImplementedError("Abstract method")

	def is_freq_valid(self, freq):
		raise NotImplementedError("Abstract method")


class Osc(Clk):
	def __init__(self, freq, fmin=None, fmax=None):
		if (fmin == None)
			fmin = freq
		if (fmax == None)
			fmax = freq
		self.fmin = fmin
		self.fmax = fmax
		self.freq = fmin

	def freq_get():
		return self.freq

	def freq_set(freq):
		if ((freq < self.fmin) or (freq > freq.fmax))
			raise ClkException("Frequency invalid {:d}".format(freq))
		self.freq = freq

class Div(Clk):
	def __init__(self, clkin, m=1):
		self.clkin = clkin
		self.m = m

	def freq_get():
		return self.clkin.freq_get() * m


class Vco(Clk):
	def __init__(self, clkin, n=1, nmin=None, nmax=None, fmin=None, fmax=None):
		self.clkin = clkin
		self.n = n
		if (nmin == None)
			nmin = n
		if (nmax = None)
			nmax = n
		if (fmin == None)
			fmin = 1 * clkin.freq_get()
		if (fmax = None)
			fmax = n * clkin.freq_get()
		self.nmin = nmin
		self.nmax = nmax
		self.fmin = fmin
		self.fmax = fmax
		self.n = n

	def freq_get():
		return self.clkin.freq_get() * self.n

	def n_set(self, n):
		self.n = n

class Pll(object):
	"""Abstract parent class of PLL classes."""
	def __init__(self, clkin, vco, div):

		self.in_div = 1
		self.out_div = 1
		self.in_freq = 1
		self.out_fmax = 1
		self.vco = Vco(1, 1)

class Stm32f4Pll(Pll):
	"""STM32F4 PLL class."""
	def __init__(self, fmax=180000000):
		Pll.__init__(self)
		self.out_fmax = fmax
		self.p = 1
		self.q = 1
		self.r = 1
		self.n = 1
		self.vco.f_min = 1
		self.vco.f_max = 1
		self.n_min = 8
		self.n_max = 512
		self.err_max = 1000

	def evaluate_p(target_freq):
		clk_in = hse_freq
		clk_ref = target_freq
		err_min = 1000000
		m_min = 1
		r_min = 1
		n_min = 1
		m = 1
		m_max = 8
		n_max = 512
		r_max = 8
		while m <= m_max:
			n = self.n_min
			while n <= n_max:
				r = 2
				while r <= r_max:
					fvco = (fin * n) / m 
					fout = fvco / p
					in_valid = ((fin / m) >= 1000000) and \
						((fin / m) <= 2400000) 
					out_valid = (fvco >= 100000000) and (fvco <= 432000000) 
					fclk_out_valid = (clk_out <= 180000000)
					if fvco_in_valid and fvco_out_valid and fclk_out_valid:
						err = clk_out - clk_ref
						if err < 0:
							err = -err
						if err < err_min:
							m_min = m
							r_min = r
							n_min = n
							err_min = err
					r = r + 2
				n = n + 1
			m = m + 1
#
#       |\                               +----+
# HSI-->| |   +----+      +-----+    +-->| /P | 
#       | |-->| /M |----->|     |    |   +----+
# HSE-->| |   +----+      | VCO |-+--+
#       |/             +->|     | |  |   +----+
#                      |  +-----+ |  +-->| /Q |
#                      |          |  |   +----+
#                      |  +----+  |  |
#                      +--| xN |<-+  |   +----+
#                         +----+     +-->| /R |
#                                        +----+

class Stm32f4PllMain(Pll):
	"""STM32F4 PLL class."""
	def __init__(self, fmax=180000000):
		Stm32f4Pll.__init__(self)

# ------------------------------------------------------------------
# PLL (Main PLL)
#
# Outputs:
# - PLLSAI2CLK (/P)
# - PLL48M1CLK (/Q)
# - PLLCLK     (/R)
#
#


