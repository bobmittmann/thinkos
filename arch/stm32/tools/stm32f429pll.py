#!python

from math import *
import sys
import os
import getopt
import time
from pll import *
#
# STM32F42xx RCC PLL
#

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

class Stm32f4Pll(object):
    """Abstract parent class of PLL classes."""
    def __init__(self, vco, p, q, r):
        self.vco = vco
        self.m = vco.clkin
        self.mux = self.m.clkin
        self.hsi = self.mux.clk[0]
        self.hse = self.mux.clk[1]
        self.p = p
        self.q = q
        self.r = r

    def info(self):
        print(" PLL configuration summary")
        print(" VCO = %.6f MHz" % self.vco.freq_mhz())
        print("") 
        print("   mux = %d" % self.mux.sel)
        print("   N = %d" % self.vco.get())
        print("   M = %d" % self.m.get())
        print("   R = %d -> %.6f MHz" % (self.r.get(), self.r.freq_mhz()))
        print("   Q = %d -> %.6f MHz" % (self.q.get(), self.q.freq_mhz()))
        print("   P = %d -> %.6f MHz" % (self.p.get(), self.p.freq_mhz()))

    def solve_pq(self, p_freq, p_tol=1e-3, q_freq=None, q_tol=1e-3):

        p_ref = p_freq
        if (q_freq == None):
            q_freq = 48000000

        q_ref = q_freq

        p_w = 1/p_tol
        q_w = 1/q_tol

        err_min = 10000000000
        m_min = 1
        n_min = 1
        p_min = 1
        q_min = 1
        
        in_freq = self.m.clkin.freq_get()
        print("  PLL Input = %.6f MHz" % (in_freq * 1e-6))
        print("  Target = %.6f MHz, %.6f MHz" % (p_ref * 1e-6, q_ref * 1e-6))

        for m in self.m.range():
            print(" m=", m)
            self.m.set(m)
            for n in self.vco.range():
                self.vco.set(n)
                for p in self.p.range():
                    self.p.set(p)
                    for q in self.q.range():
                        self.q.set(q)

                        if self.q.is_valid() and self.p.is_valid():
                            p_freq = self.p.freq_get()
                            q_freq = self.q.freq_get()

                            dp = ((p_freq - p_ref) / p_ref) * p_w
                            dq = ((q_freq - q_ref) / q_ref) * q_w
                            err = sqrt(dp**2 + dq**2)
                            if err < err_min:
                                print("p=", p, " n=", n, " m=", m, " q=", q, \
                                  " err=", err)
                                m_min = m
                                p_min = p
                                n_min = n
                                q_min = q
                                err_min = err

        self.m.set(m_min)
        self.vco.set(n_min)
        self.p.set(p_min)
        self.q.set(q_min)



    def print_h(self):

        PLLCLK = self.p.freq_get()
        HSECLK = self.hse.freq_get()

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
        print("")


    def print_c(self):

        PLLVCO = self.vco.freq_get()
        PLLCLK = self.p.freq_get()
        PLLQCLK = self.q.freq_get()
        PLLRCLK = self.r.freq_get()
        PLLM = self.m.get()
        PLLN = self.vco.get()
        PLLP = self.p.get()
        PLLQ = self.q.get()
        PLLR = self.r.get()
        HSECLK = self.hse.freq_get()

        print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        print("!! Add this block to \"stm2f4-init.c\" !!")
        print("")

        print("#if (STM32_HCLK_HZ == %d) && (STM32_HSE_HZ == %d)" %(PLLCLK,HSECLK))
        print("  /* PLLVCO = %.6f MHz */" % (PLLVCO/1000000.0))
        print("  #define PLLM       %3d" % PLLM)
        print("  #define PLLN       %3d" % PLLN)
        print("  #define PLLR       %3d /*     PLLCLK -> %10.6f MHz */" %(PLLR, PLLRCLK*1e-6))
        print("  #define PLLQ       %3d /* PLL48M1CLK -> %10.6f MHz */" %(PLLQ, PLLQCLK*1e-6))
        print("  #define PLLPDIV    %3d /* PLLSAI2CLK -> %10.6f MHz */" %(PLLP, PLLCLK*1e-6))
        print("")
        print("#endif")
        print("")

def do_pll():
    if len(sys.argv) > 1:
            target_freq = int(sys.argv[1])
    else:
            target_freq = 180000000

    if len(sys.argv) > 2:
            hse_freq = int(sys.argv[2])
    else:
            hse_freq = 8000000

    print("Target freqency:", target_freq)
    print("HSE freqency:", hse_freq)

    sai_freq = 44100 * 512
    adc_freq = sai_freq


#
# ------------------------------------------------------------------
# PLL (Main PLL)
#
# Outputs:
# - PLLCLK     (/P)
# - PLL48M1CLK (/Q)
# - PLLSAI2CLK (/P)
#
#


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



def show_usage():
    print("Usage: {0} [OPTION] FILE".format(progname))
    print("Transfer FILE using the TFTP protocol")
    print("")
    print("  -a, --addr     upload address."\
        " Default to: 0x{0:08x}".format(def_addr))
    print("  -h, --host     remote host addr."\
        " Default to: '{0}'".format(def_host))
    print("  -t, --target   target platform."\
        " Default to: '{0}'".format(def_target))
    print("  -r, --reset    exec the reset script")
    print("  -n, --nrst     force reset using the nRST pin (hardware)")
    print("  -i, --init     exec init script")
    print("  -q, --quiet    silent mode (no beeps)")
    print("      --help     display this help and exit")
    print("  -V, --version  output version information and exit")
    print("")

def show_version():
    print("{0} - version {1:d}.{2:d}".format(progname, ver_major, ver_minor))
    print("")
    print("Writen by Bob Mittmann - bobmittmann@gmail.com")
    print("(C) Cpyright 2014 - Bob Mittmann")
    print("")

def show_error(msg):
    print("")
    print("{0}: error: {1}".format(progname, msg))
    print("")
    sys.exit(2)

def main():
    global progname 

    progname = sys.argv[0]

    # Oscillators 
    lsi_osc = Osc("LSI", 32000)
    hsi_osc = Osc("HSI", 16e6)
    lse_osc = Osc("LSE", 32768)
    hse_osc = Osc("HSE", 8000000, fmin=4e6, fmax=26e6)
    m_mux = ClkMux([hsi_osc, hse_osc])
    m_div = ClkDiv("M", m_mux, 1, qmin=2, qmax=63)
    m_vco = Vco("N", m_div, nmin=15, nmax=512, fmin=10e6, fmax=432e6)
    p_div = ClkDiv("PLLCLK", m_vco, 1, qmin=2, qmax=8, qstep=2, \
                   fmax=180e6*1.067)
    q_div = ClkDiv("PLL48M1CLK", m_vco, 2, qmin=2, qmax=15, fmax=48e6*1.01)
    r_div = ClkDiv("R", m_vco, 1)

    m_pll = Stm32f4Pll(m_vco, p_div, q_div, r_div)

    m_mux.select(1)
    pllclk_hz = 180000000
    pll48clk_hz = 48000000

    quiet = False
    verbose = False

    try:
        opts, args = getopt.getopt(sys.argv[1:], "?Vvqh:p:", \
            ["help", "version", "verbose", "quiet", "hse=", "pll="])
    except err:
        show_error(str(err))

    for o, a in opts:
        if o in ("-?", "--help"):
            show_usage()
            sys.exit()
        elif o in ("-V", "--version"):
            show_version()
            sys.exit()
        elif o in ("-v", "--verbose"):
            verbose = True
        elif o in ("-q", "--quiet"):
            quiet = True
        elif o in ("-h", "--hse"):
            hse_hz = int(a, 0)
            hse_osc.freq_set(hse_hz)
            m_mux.select(1)
        elif o in ("-p", "--pll"):
            pllclk_hz = int(a, 0)
        else:
            assert False, "unhandled option"

    if len(args) > 1:
        error("too many arguments")

    m_pll.solve_pq(pllclk_hz, 0.1, pll48clk_hz, 0.001)
    m_pll.info()
    m_pll.print_h()
    m_pll.print_c()


if __name__ == "__main__":
    main()

