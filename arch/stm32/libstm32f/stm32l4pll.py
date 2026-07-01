#!python
from math import *
import sys

if len(sys.argv) > 1:
        target_freq = int(sys.argv[1])
else:
        target_freq = 80000000

if len(sys.argv) > 2:
        hse = int(sys.argv[2])
else:
        hse = 11289600

print "Target Freq:", target_freq
print "hse:", hse

sai_freq = 44100 * 512
adc_freq = sai_freq


#
# STM32L4xxx RCC PLL
#


#
# ------------------------------------------------------------------
# PLL (Main PLL)
#
# Outputs:
# - PLLSAI2CLK (/P)
# - PLL48M1CLK (/Q)
# - PLLCLK     (/R)
#
#
clk_in = hse_freq
clk_ref = target_freq
err_min = 1000000
m_min = 1
r_min = 1
n_min = 1
m = 1
m_max = 8
n_max = 88
r_max = 8
while m <= m_max:
	n = 8
	while n <= n_max:
		r = 2
		while r <= r_max:
			fvco = (clk_in * n) / m 
			clk_out = fvco / r
			fvco_in_valid = ((clk_in / m) >= 4000000) and ((clk_in / m) <= 16000000) 
			fvco_out_valid = (fvco >= 64000000) and (fvco <= 344000000) 
			fclk_out_valid = (clk_out <= 80000000)
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

# Configuration 
fvco = clk_in * n_min / m_min 

PLLM = m_min
PLLN = n_min
PLLR = r_min
PLLQ = floor(fvco / 48000000) 
PLLPDIV = floor(fvco / sai_freq) 
PLLVCO = fvco

# Input
HSECLK = clk_in
# Outputs
PLLCLK = fvco / PLLR

if (PLLPDIV > 0):
	PLLSAI2CLK = PLLVCO / PLLPDIV
else:
	PLLSAI2CLK = 0

if (PLLQ > 0):
	PLL48M1CLK = PLLVCO / PLLQ
else:
	PLL48M1CLK = 0


print("----------------------------------------------------------------------")
print(" Main PLL configuration summary")
print("     HSE = %.6f MHz" % (HSECLK/1000000.0))
print("     VCO = %.6f MHz" % (PLLVCO/1000000.0))
print("  System = %.6f MHz" % (PLLCLK/1000000.0))
print("") 
print(" N = %d" % PLLN)
print(" M = %d" % PLLM)
print(" R = %d" % PLLR)
print(" Q = %d" % PLLQ)
print(" PDIV = %d" % PLLPDIV)
print("") 


#
# The two PLLs use the same predivisor M 
#
# ------------------------------------------------------------------
# PLLSAI1 (SAI PLL)
#
# Outputs:
# - PLLSAI1CLK (/P)
# - PLL48M2CLK (/Q)
# - PLLADC1CLK (/R)
#

clk_in = hse_freq
clk_ref = sai_freq
err_min = 1000000
m_min = 1
r_min = 1
n_min = 1
m = PLLM 
m_max = PLLM
n_max = 88
r_max = 8
while m <= m_max:
	n = 8
	while n <= n_max:
		r = 2
		while r <= r_max:
			fvco = (clk_in * n) / m 
			clk_out = fvco / r
			fvco_in_valid = ((clk_in / m) >= 4000000) and ((clk_in / m) <= 16000000) 
			fvco_out_valid = (fvco >= 64000000) and (fvco <= 344000000) 
			fclk_out_valid = (clk_out <= 80000000)
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

# Configuration 
fvco = clk_in * n_min / m_min 

PLLSAIN = n_min
PLLSAIPDIV = r_min
PLLSAIQ = round(fvco/48000000.0) 
PLLSAIR = round(fvco/adc_freq) 
PLLSAIVCO = fvco

# Outputs
if (PLLSAIPDIV > 0):
	PLLSAI1CLK = PLLSAIVCO / PLLSAIPDIV
else:
	PLLSAI1CLK = 0

if (PLLSAIQ > 0):
	PLL48M2CLK = PLLSAIVCO / PLLSAIQ
else:
	PLL48M2CLK = 0

if (PLLSAIR > 0):
	PLLADC1CLK = PLLSAIVCO / PLLSAIR
else:
	PLLADC1CLK = 0

print("----------------------------------------------------------------------")
print(" SAI PLL configuration summary")
print("     VCO = %.6f MHz" % (PLLSAIVCO/1000000.0))
print("") 
print(" N = %d" % PLLSAIN)
print(" M = %d" % PLLM)
print(" R = %d" % PLLSAIR)
print(" Q = %d" % PLLSAIQ)
print(" PDIV = %d" % PLLSAIPDIV)
print("") 


print("")
print("") 
print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
print("!! Add this block to your project's \"config.h\" !!")
print("")
print("/* -------------------------------------------------------------------------")
print("   RCC")
print("   ------------------------------------------------------------------------- */")
print("")
print("#define STM32_HCLK_HZ       %d /* HBA bus frequency */" % PLLCLK)
print("#define STM32_HSE_HZ        %d /* External oscillator frequency */" % HSECLK)
print("#define STM32_ENABLE_HSE    1 /* Use external oscillator */")
print("#define STM32_ENABLE_PLL    1 /* Enable PLL */")
print("#define STM32_ENABLE_HSI    0 /* Don't use internal oscillator */")
print("#define STM32_ENABLE_PLLSAI 1 /* Enable SAI PLL */")

print("")
print("")


print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
print("!! Add this block to \"stm2l4-init.c\" !!")
print("")
print("#if (STM32_HCLK_HZ == %d) && (STM32_HSE_HZ == %d)" %(PLLCLK,HSECLK))
print("  /* PLLVCO = %.6f MHz */" % (PLLVCO/1000000.0))
print("  #define PLLM       %3d" % PLLM)
print("  #define PLLN       %3d" % PLLN)
print("  #define PLLR       %3d /*     PLLCLK -> %10.6f MHz */" %(PLLR, PLLCLK/1000000.0))
print("  #define PLLQ       %3d /* PLL48M1CLK -> %10.6f MHz */" %(PLLQ, PLL48M1CLK/1000000.0))
print("  #define PLLPDIV    %3d /* PLLSAI2CLK -> %10.6f MHz */" %(PLLPDIV, PLLSAI2CLK/1000000.0))
print("")
print("  /* PLLSAIVCO = %.6f MHz */" % (PLLSAIVCO/1000000.0))
print("  #define PLLSAIN    %3d" % PLLSAIN)
print("  #define PLLSAIPDIV %3d /* PLLSAI2CLK -> %10.6f MHz */" %(PLLSAIPDIV, PLLSAI1CLK/1000000.0))
print("  #define PLLSAIQ    %3d /* PLL48M2CLK -> %10.6f MHz */" %(PLLSAIQ, PLL48M2CLK/1000000.0))
print("  #define PLLSAIR    %3d /*  PLLADCCLK -> %10.6f MHz */" %(PLLSAIR, PLLADC1CLK/1000000.0))
print("#endif")
print("")


