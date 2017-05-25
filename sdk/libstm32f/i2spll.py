#!python
from math import *

target_fs = 44100
i2sclk = 192000000
hse = 12000000
r = 3
n = 357
m = 11

err_min = 1000000
m_min = 1
r_min = 1
n_min = 1
m = 2
while m < 64:
	n = 50
	while n < 433:
		r = 1
		while r < 8:
			fvco = (hse * n) / m 
			i2sclk = fvco / r
			if ((hse / m) > 1000000) and ((hse / m) < 2000000) and (fvco < 432000000) and (i2sclk <= 192000000):
				target_mclk = target_fs * 256;
				ratio = (i2sclk + (target_mclk / 2)) / target_mclk;

				odd = ratio & 1
				div = ratio / 2
				if div > 1:
					fs = i2sclk / ((32*2)*((2*div)+odd)*4)
					err = fs - target_fs
					if err < 0:
						err = -err
					if err < err_min:
						m_min = m
						r_min = r
						n_min = n
						err_min = err
			r = r + 1
		n = n + 1
	m = m + 1

m = m_min
r = r_min
n = n_min
print("m = %d" % m)
print("n = %d" % n)
print("r = %d" % r)

fvco = hse * n / m 
i2sclk = fvco / r
target_mclk = target_fs * 256;
ratio = (i2sclk + (target_mclk / 2)) / target_mclk;

odd = ratio & 1
div = ratio / 2
fs = i2sclk / ((32*2)*((2*div)+odd)*4)
mclk = fs * 256



f = mclk/1000000.0
print("MCLK = %.6f MHz" % f)
f = i2sclk/1000000.0
print("i2sclk = %.6f MHz" % f)
print("div = %d" % div)
f = fs/1000.0
print("Fs = %.3f KHz" % f)

