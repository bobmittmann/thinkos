#!julia
#

using Printf;
using Plots;


# Number of entries in the sine table
global N_SINE = 1024
# Number of entries in the interpolation table
global M_INTRP = 256

global LOG2_N::UInt32 = UInt32(log2(N_SINE))
global LOG2_M::UInt32 = UInt32(log2(M_INTRP))
global MASK_N::UInt32 = (1 << LOG2_N) - 1

#
# Binary Angle Measurement 32 bits
# 
# BAM32
# 
#

# Convert from a radians to a BAM32 format
# Angles smaller than 0 and larger than 2π are wrapped into the interval 0..2π
function rad_to_bam32(theta::Float64)
	omega::Float64 = theta / 2 * π
	wrap::Float64 = omega - floor(omega)
	bam32::UInt32 = UInt32(round(wrap * Float64(1 << 32)))
	return bam32
end

function degree_to_bam32(alpha::Float64)
	omega::Float64 = alpha / 360
	wrap::Float64 = omega - floor(omega)
	bam32::UInt32 = UInt32(round(wrap * Float64(1 << 32)))
	return bam32
end

# Convert from a normalized angle to a BAM32 format
function norm_to_bam32(omega::Float64)
	wrap::Float64 = omega - floor(omega)
	bam32::UInt32 = UInt32(round(wrap * Float64(1 << 32)))
	return bam32
end

function bam32_to_rad(bam32::UInt32)
	theta::Float64 = bam32 * (2 * π / (1 << 32))
	return theta
end

function bam32_to_degree(bam32::UInt32)
	theta::Float64 = bam32 * (360 / (1 << 32))
	return theta
end

function Q31(x::Float64)
	y::Int32 = 0
	y = Int32(round(x * convert(Float64, (1 << 31))))
	return y
end

function U32(x::Float64)
	y::UInt32 = UInt32(round(x * convert(Float64, (1 << 32))))
	return y
end

function Q31F(x::Int64)
	y::Float64 = 0
	y = x * (1.0 / (1 << 31))
	return y
end

function U32_TO_F(x::UInt32)
	y::Float64 = x * (1.0 / (1 << 32))
	return y
end

function Q31F(x::Int32)
	return Q31F(convert(Int64, x))
end

function Q15(x::Float64)
	y::Int32 = 0
	y = Int32(round(x * convert(Float64, 0x7fff)))
	return y
end

Q15_TO_FLOAT(x::Int32) = convert(Float64, x * (1.0 / 0x7fff))

FLOAT_TO_Q15(x::Float64) = convert(Int16, round(x * Float64(0x7fff)))

FLOAT_TO_Q16(x::Float64) = convert(UInt16, round(x * Float64(0xffff)))


function U16F(x::UInt64)
	y::Float64 = 0
	y = x * (1.0 / (1 << 16))
	return y
end

function U16F(x::Int32)
	return U16F(convert(UInt64, x))
end

function U16F(x::UInt32)
	return U16F(convert(UInt64, x))
end

function Q15SAT(x::Int32)
	y::Int32 = 0
	Q15_MIN = Int32(-32767)
	Q15_MAX = Int32(32767)

	y = (x < Q15_MIN) ? Q15_MIN : ((x > Q15_MAX) ? Q15_MAX : x);

	return y;
end

Q15DIV(x::Int32, y::Int32) = div(x << 15, y)

Q15MUL(x::Int32, y::Int32) = (x * y) >> 15

function Q31DIV(x::Int32, y::Int32)
	xx::Int64 = x
	yy::Int64 = y

	return convert(Int32, div(xx << 31, yy))
end

# Linear interpolation algorithm

function q15sin_lin_intrp(x::UInt32)
 # Linear interpolation algorithm
	sin_x0::Int32 = 0
	sin_x1::Int32 = 0
	x0::UInt32 = 0
	i::UInt32 = 0
	dx::Int32 = 0
	dy::Int32 = 0

	i = x >>> (32 - LOG2_N)
	x0 = i << (32 - LOG2_N)
	dx = (x - x0) >>> (17 - LOG2_N)

	sin_x0 = qsintab[i + 1]
	sin_x1 = qsintab[((i + 1) & MASK_N) + 1]

	dy = (sin_x1 - sin_x0)

	y = sin_x0 + (dx * dy) >> 15
	return y
end

function q15sin_trig_intrp(x::UInt32)
 # Linear interpolation algorithm
	x0::UInt32 = 0
	x1::UInt32 = 0
	sin_x::Int32 = 0
	cos_x::Int32 = 0
	dx::Int32 = 0
	sin_dx::Int32 = 0
	cos_dx::Int32 = 0
	i::UInt32 = 0
	j::UInt32 = 0

	i = x >>> (32 - LOG2_N)
	x0 = i << (32 - LOG2_N)
	sin_x = qsintab[i + 1]

	x1 = x0 + UInt32(1 << 30)
	i = x1 >>> (32 - LOG2_N)
	cos_x = qsintab[i + 1]
	
	dx = (x - x0)
	j = dx >>> (32 - (LOG2_N + LOG2_M))

	sin_dx = fsintab[j + 1]
	cos_dx = fcostab[j + 1]

 # 	y = (sin_x * cos_dx + cos_x * sin_dx) >> 16
 	y = sin_x + ((cos_x * sin_dx) >> 16)
 #	@printf("<%.5f, %6d %6d %6d>\n", U32_TO_F(x), sin_x, cos_x, y)
	return y
end

function q15sin_lookup(x::UInt32)
	y::Int32 = 0
	i::UInt32 = 0

	i = (x + UInt32(1 << (31 - LOG2_N))) >>> (32 - LOG2_N)
	y = qsintab[i + 1]
 #	@printf("<%.5f, %5d>\n", U32_TO_F(x), y)
	return y
end

function mk_c_head(n::Int, m::Int)
	txt = AbstractString[
	"/*"
	" * WARNING: do not edit, this file was automatically generated."
	""
	"  - Sine and Cosine functions using lookup and interpolate method."
	"  - The input angles uses a Binary Angle Measurement (BAM) with 32 bits"
	"    resolution."
	"  - The results uses a Q15 format with 16 bits of resolutiuon."
	@sprintf("  - Size of the sine lookup table: N = %d.", n)
	@sprintf("  - Size of the residual lookup tables: M = %d.", m)
	"*/"
	""
	"#include <stdint.h>"
	""
	"/* Enable linear interpolation */"
	"#ifndef FIXPT_SINCOS_INTRPL_LIN"
	"#define FIXPT_SINCOS_INTRPL_LIN 1"
	"#endif"
	""
	"/* Enable trigonometric interpolation */"
	"#ifndef FIXPT_SINCOS_INTRPL_TRIG"
	"#define FIXPT_SINCOS_INTRPL_TRIG 0"
	"#endif"
	""
	@sprintf("#define N %d", n)
	@sprintf("#define LOG2_N %d", log2(n))
	@sprintf("#define MASK_N %d", n - 1)
	""
	@sprintf("#define M %d", m)
	@sprintf("#define LOG2_M %d", log2(m))
	""
	]
	return txt
end

function q16_table_entry(i::Int, x::Float64)
	y = convert(UInt32, round(x * Float64(0xffff)))
	return @sprintf("%6d, ", y & 0xffff)
end

function q15_table_entry(i::Int, x::Float64)
	y = convert(Int32, round(x * Float64(0x7fff)))
	return @sprintf("%6d, ", y)
end

function q16_fsin_table_line(i::Int, n::Int, m::Int)
	s = "\t"
	for j = i:i+7
		s = s * q16_table_entry(j, sin(2*π*j/(n*m)))
	end
	return s
end

function q16_fcos_table_line(i::Int, n::Int, m::Int)
	s = "\t"
	for j = i:i+7
		s = s * q16_table_entry(j, cos(2*π*j/(n*m)))
	end
	return s
end

function q15_qsin_table_line(i::Int, n::Int)
	s = "\t"
	for j = i:i+7
		s = s * q15_table_entry(j, sin(2*π*j/n))
	end
	return s
end

function mk_c_q15qsin_tab(n::Int)
	txt1 = AbstractString[
	"/* Sine/cosine lookup table. */"
		@sprintf("static const int16_t q15sintab[%d] = {", n)
	]

	txt2 = AbstractString[
		q15_qsin_table_line(i, n) for i in 0:8:n-8
	]

	txt3 = AbstractString[
		"};"
		""
	]

	return vcat(txt1, txt2, txt3)
end

function mk_c_q15intrp_tabs(n::Int, m::Int)
	txt1 = AbstractString[
	""
	"#if (FIXPT_SINCOS_INTRPL_TRIG)"
	""
	"static const uint16_t fsintab[] = {"
	]

	txt2 = AbstractString[
	q16_fsin_table_line(i, n, m) for i in 0:8:m-8
	]

	txt3 = AbstractString[
	"};"
	""
	"static const uint16_t fcostab[] = {"
	]

	txt4 = AbstractString[
	q16_fcos_table_line(i, n, m) for i in 0:8:m-8
	]

	txt5 = AbstractString[
	"};"
	""
	"#endif /* (FIXPT_SINCOS_INTRPL_TRIG) */"
	""
	]

	return vcat(txt1, txt2, txt3, txt4, txt5)
end

function mk_c_q15sin_lin(n::Int64)
	txt = AbstractString[
	""
	"#if (FIXPT_SINCOS_INTRPL_LIN)"
	"/* Linear interpolation method */"
	""
	"int32_t q15sin(uint32_t x)"
	"{"
	"\tuint32_t x0;"
	"\tint32_t sin_x0;"
	"\tint32_t sin_x1;"
	"\tint32_t dx;"
	"\tint32_t dy;"
	"\tint32_t y;"
	"\tuint32_t i;"
	""
	"\ti = x >> (32 - LOG2_N);"
	"\tx0 = i << (32 - LOG2_N);"
	"\tdx = (int32_t)(x - x0) >> (17 - LOG2_N);"
	""
	"\tsin_x0 = q15sintab[i];"
	"\ti = (i + 1) & MASK_N;"
	"\tsin_x1 = q15sintab[i];"
	""
	"\tdy = sin_x1 - sin_x0;"
	""
	"\ty = sin_x0 + ((dx * dy) >> 15);"
	"\treturn y;"
	"}"
	""
	"#endif /* (FIXPT_SINCOS_INTRPL_LIN) */"
	""
	]
end	

function mk_c_q15sin_trig(n::Int, m::Int)
	txt = AbstractString[
	""
	"#if (FIXPT_SINCOS_INTRPL_TRIG)"
	""

	"/* circular interpolation version, using the trigonometric "
	"   identity: sin(x + dx) = sin(x)*cos(dx) + cos(x)*sin(dx) "
	"*/"
	"int32_t q15sin(uint32_t x)"
	"{"
	"\tuint32_t x0;"
	"\tuint32_t x1;"
	"\tint32_t sin_x;"
	"\tint32_t cos_x;"
	"\tint32_t dx;"
	"\tint32_t sin_dx;"
	"\tint32_t cos_dx;"
	"\tint32_t y;"
	"\tuint32_t i;"
	"\tuint32_t j;"
	""
	"\ti = x >> (32 - LOG2_N);"
	"\tx0 = i << (32 - LOG2_N);"
	"\tsin_x = q15sintab[i];"
	""
	"/* Add 90 dg to get the cosine */"
	"\tx1 = x0 + (uint32_t)(1 << 30);"
	"\ti = x1 >> (32 - LOG2_N);"
	"\tcos_x = q15sintab[i];"
	""
	"\tdx = (x - x0);"
	"\tj = dx >> (32 - (LOG2_N + LOG2_M));"
	""
	"\tsin_dx = fsintab[j];"
	"\t/* cos_dx = fcostab[j];"
	"\ty = (sin_x * cos_dx + cos_x * sin_dx) >> 16; */"
	""
	"\ty = sin_x + ((cos_x * sin_dx) >> 16);"
	"\treturn y;"
	"}"
	""
	"#endif"
	""
	]
end	

function mk_c_q15sin_lookup(n::Int64)
	txt = AbstractString[
	""
	"#if !((FIXPT_SINCOS_INTRPL_LIN) || (FIXPT_SINCOS_INTRPL_TRIG))"
	""
	"/* Simple lookup method */"
	"int32_t q15sin(uint32_t x)"
	"{"
	"\tint32_t sin_x;"
	"\tint32_t i;"
	""
	"/* Round the angle to the closest table entry. */"
	"\ti = ((uint32_t)(x + (1 << (31 - LOG2_N)))) >> (32 - LOG2_N);"
	"\tsin_x = q15sintab[i];"
	""
	"\treturn sin_x;"
	"}"
	""
	"#endif /* !((FIXPT_SINCOS_INTRPL_LIN) || "
	"            (FIXPT_SINCOS_INTRPL_TRIG)) */"
	]
end	

function mk_q15sincos(prefix)

 	n::Int = N_SINE
 	m::Int = M_INTRP

	c_lines = vcat(mk_c_head(n, m) , 
				   mk_c_q15qsin_tab(n),
				   mk_c_q15sin_lin(n),
				   mk_c_q15intrp_tabs(n, m),
				   mk_c_q15sin_trig(n, m),
				   mk_c_q15sin_lookup(n)
				   )

	out_dir = ".."
	fname = @sprintf("%s/%s.c", out_dir, prefix)

	@printf("Writing to file: \"%s\"...\n", fname)

	f = open(fname, "w")
	for s in c_lines
		write(f, s * "\n")
	end

	close(f);
end

@printf("Creating sine lookup table: %d entries...\n", N_SINE)
qsintab = zeros(Int16, N_SINE)
@printf("Creating secondary cos/sin lookup tables: %d entries...\n", M_INTRP)
fsintab = zeros(UInt16, M_INTRP + 1)
fcostab = zeros(UInt16, M_INTRP + 1)

# Generate the sine table
for i in 0:N_SINE-1
	θ = 2*π*i/N_SINE
	qsintab[i + 1] = FLOAT_TO_Q15(sin(θ))
end

# Generate the sine and cosine interpolation tables
#
# The trigonometric interpolation uses the identity: 
#    sin(a + b) = sin(a)*cos(b) + cos(a)*sin(b) "
# the 'a' term used as index lookup in the sine table 
# the 'b' term is used as index to the sine and cosine tables
for i in 0:M_INTRP
	θ = 2*π*i/(N_SINE * M_INTRP)
	fsintab[i + 1] = FLOAT_TO_Q16(sin(θ))
	fcostab[i + 1] = FLOAT_TO_Q16(cos(θ))
end


println("Creating output file...");
mk_q15sincos("q15sincos")

println("Calculating error...");

w = collect(-1.0 :1/10000:0)
#w = [-.500003, -.5, -.499997]
y = sin.(2 * π .* w)
x = [norm_to_bam32(a) for a in w]
y0 = [Q15_TO_FLOAT(q15sin_lookup(a)) for a in x]
y1 = [Q15_TO_FLOAT(q15sin_lin_intrp(a)) for a in x]
y2 = [Q15_TO_FLOAT(q15sin_trig_intrp(a)) for a in x]

dy0 = (y - y0)
dy1 = (y - y1)
dy2 = (y - y2)

ei0 = argmax(abs.(dy0))
ei1 = argmax(abs.(dy1))
ei2 = argmax(abs.(dy2))

@printf("Max sine errors:\n");
@printf(" - 0: %.10f @ %.10f\n", abs.(dy0[ei0]), w[ei0])
@printf(" - 1: %.10f @ %.10f\n", abs.(dy1[ei1]), w[ei1])
@printf(" - 2: %.10f @ %.10f\n", abs.(dy2[ei2]), w[ei2])


 #plotly()
p0 = plot((x, dy0.*32768), linetype = :steppre)
p1 = plot((x, dy1.*32768), linetype = :steppre)
p2 = plot((x, dy2.*32768))

display(plot(p0, p1, p2,
		  title = ["Direct lookup" "Linear Interp" "Trig Interp"],
		  titlefontsize = 9,
		  size = (1366,768),
		  layout = (2,2),
		  legend = false,
		  display = true
		 )
	   )
;

