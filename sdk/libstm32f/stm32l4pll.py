#!python
from math import *

target_freq = 80000000
hse = 11289600
r = 3
n = 357
m = 11

err_min = 1000000
m_min = 1
r_min = 1
n_min = 1
m = 1
while m < 9:
	n = 8
	while n < 87:
		r = 2
		while r < 9:
			fvco = (hse * n) / m 
			mainclk = fvco / r
			fvco_in_valid = ((hse / m) >= 4000000) and ((hse / m) <= 16000000) 
			fvco_out_valid = (fvco >= 64000000) and (fvco <= 344000000) 
			fsys_valid = (mainclk <= 80500000)
			if fvco_in_valid and fvco_out_valid and fsys_valid:
				err = mainclk - target_freq
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

m = m_min
r = r_min
n = n_min

fvco = hse * n / m 
mainclk = fvco / r

print(" ----------------------")

f = hse/1000000.0
print("     HSE = %.6f MHz" % f)
f = fvco/1000000.0
print("     VCO = %.6f MHz" % f)
f = mainclk/1000000.0
print("  System = %.6f MHz" % f)

print("") 
print(" N = %d" % n)
print(" M = %d" % m)
print(" R = %d" % r)

print(" ----------------------")
