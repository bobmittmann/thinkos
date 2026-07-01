#!python
from math import *
import sys

# --- constants
PY3 = sys.version_info >= (3, 0)


# ----------------------------------------------------------------------
# PLL Exceptions
# ----------------------------------------------------------------------

class PllException(Exception):
	"""Parent class of all PLL exceptions."""
	pass

class Vco(object):
	def __init__(self, f_min, f_max):

		self.f_min = f_min 
		self.f_max = f_max

	def is_freq_valid(self, f):

		self.f_min = f_min 
		self.f_max = f_max

class Pll(object):

	def __init__(self, in_freq):

		self.in_div = 1
		self.out_div = 1
		self.in_freq = in_freq
		self.vco = Vco()

