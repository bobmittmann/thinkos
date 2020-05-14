#!julia
#

using Printf;
using Plots;

plotly()

# Number of entries in the table
N_SIN = 4096

qtab = fill((Float32(0), Float32(0)), N_SIN + 2)
ftab = fill((Float32(0), Float32(0)), N_SIN + 2)

for i in 0:N_SIN+1
	θ = 2*π*i/N_SIN
	p = convert(Float32, sin(θ))
	q = convert(Float32, cos(θ))
	qtab[i + 1] = ( p, q ) 
end

for i in 0:N_SIN+1
	θ = 2*π*i/(N_SIN * N_SIN)
	p = convert(Float32, sin(θ))
	q = convert(Float32, cos(θ))
	ftab[i + 1] = ( p, q ) 
end

# Conversion form float to fixed point Q1.31 
#define Q31(F) ((int32_t)(double)(F) * (double)(1LL << 32))
function Q31(x::Float64)
	y::Int64 = 0
	y = Int64(round(x * convert(Float64, (1 << 31))))
	return y
end

function mk_c_head(n::Int)
	txt = AbstractString[
	"/*"
	" * WARNING: do not edit, this file was automatically generated."
	"*/"
	"/* Sine and Cosine functions using Lookup and interpolate method."
	@sprintf("  - size of the lookup tables: N = %d", n)
	" */"
	""
	"/* Enable polynomial interpolation */"
	"#ifndef FIXPT_SINCOS_INTRPL_POLY"
	"#define FIXPT_SINCOS_INTRPL_POLY 0"
	"#endif"
	""
	"/* Enable trigonometric interpolation */"
	"#ifndef FIXPT_SINCOS_INTRPL_TRIG"
	"#define FIXPT_SINCOS_INTRPL_TRIG 1"
	"#endif"
	""
	"#include <fixpt.h>"
	""
	@sprintf("#define LOG2_N %d", log2(n))
	@sprintf("#define MASK_N %d", n - 1)
	""
	]
	return txt
end

function f32_table_entry(i::Int, p::Float64, q::Float64)
	return @sprintf("\t{%13.6a, %13.6a}, /* %3d */", 
					convert(Float32, p), convert(Float32, q), i)
end

f32_pqtable_entry(i::Int, n::Int) = f32_table_entry(i, sin(2*π*i/n), 
												   cos(2*π*i/n))

function mk_c_qtab(n::Int)
	txt1 = AbstractString[
	"const float qtab[][2] = {"
	]

	txt2 = AbstractString[
	f32_pqtable_entry(i, n) for i in 0:n+1
	]

	txt3 = AbstractString[
	"};"
	""
	"#define DX ((int32_t)(1 << (32 - LOG2_N)))"
	"#define DX_MSK ((uint32_t)(DX - 1))"
	"#define DX_RCP ((float)1.0 / DX)"
	""
	]

	return vcat(txt1, txt2, txt3)
end

function mk_c_ftab(n::Int)
	txt1 = AbstractString[
	"const float ftab[][2] = {"
	]

	txt2 = AbstractString[
	f32_pqtable_entry(i, n*n) for i in 0:n
	]

	txt3 = AbstractString[
	"};"
	""
	"#define DFX ((int32_t)(1 << (32 - 2*LOG2_N)))"
	"#define DFX_MSK ((uint32_t)(DFX - 1))"
	"#define DFX_RCP ((float)1.0 / DX)"
	@sprintf("#define Q31_RAD ((float)%13.6a)", Float32(π / (1 << 31)))
	]

	return vcat(txt1, txt2, txt3)
end

function f32_quad_sintable_entry(i::Int, n::Int)
	x1 = Float32(sin(2*π*(i + 0)/n))
	x2 = Float32(sin(2*π*(i + 1)/n))
	x3 = Float32(sin(2*π*(i + 2)/n))
	x4 = Float32(sin(2*π*(i + 3)/n))
	return @sprintf("\t%13.6a, %13.6a, %13.6a, %13.6a,", x1, x2, x3, x4)
end

function mk_c_sintab(n::Int)
	txt1 = AbstractString[
	"const float sintab[] = {"
	]

	txt2 = AbstractString[
	f32_quad_sintable_entry(i, n) for i in 0:4:n+1
	]

	txt3 = AbstractString[
	"};"
	""
	"#define DX ((int32_t)(1 << (32 - LOG2_N)))"
	"#define DX_MSK ((uint32_t)(DX - 1))"
	"#define DX_RCP ((float)1.0 / DX)"
	""
	]

	return vcat(txt1, txt2, txt3)
end

# Polynomial interpolation version
function mk_c_f32sin_poly(n::Int64)
	txt = AbstractString[
	""
	"#if (FIXPT_SINCOS_INTRPL_POLY)"
	"/* Polynomial interpolation version */"
	"float f32sin(int32_t x)"
	"{"
	"\tfloat dy20;"
	"\tfloat dy21;"
	"\tfloat dy10;"
	"\tfloat y2;"
	"\tfloat y1;"
	"\tfloat y0;"
	"\tfloat y;"
	"\tfloat qx1;"
	"\tfloat qx0;"
	"\tint32_t dx1;"
	"\tint32_t dx0;"
	"\tint32_t i;"
	""
	"\ti = x >> (32 - LOG2_N);"
	""
	"\ty0 = qtab[i++][0];"
	"\ty1 = qtab[i++][0];"
	"\ty2 = qtab[i][0];"
	""
	"\tdx0 = x & DX_MSK;"
	"\tdx1 = dx0 - DX;"
	"\tqx0 = dx0 * DX_RCP;"
	"\tqx1 = dx1 * DX_RCP;"
	""
	"\tdy10 = (y1 - y0);"
	"\tdy21 = (y2 - y1);"
	"\tdy20 = (dy21 - dy10) / 2;"
	""
	"\ty = y0 + qx0 * dy10 + qx0 * qx1 * dy20;"
	""
	"\treturn y;"
	"}"
	"#endif /* (FIXPT_SINCOS_INTRPL_POLY) */"
	]
	return txt
end

function mk_c_f32sin_trig(n::Int64)
	txt = AbstractString[
	""
	"#if (FIXPT_SINCOS_INTRPL_TRIG)"
	"/* Circular interpolation version, using the trigonometric "
	"   identity: sin(a + b) = sin(a)*cos(b) + cos(a)*sin(b) "
	"*/"
	"float f32sin(uint32_t x)"
	"{"
	"\tfloat sin_x0;"
	"\tfloat cos_x0;"
	"\tfloat sin_fx0;"
	"\tfloat sin_fx1;"
	"\tfloat sin_fx;"
	"\tfloat cos_fx0;"
	"\tfloat cos_fx1;"
	"\tfloat cos_fx;"
	"\tint32_t dfx0;"
	"\tfloat qfx0;"
	"\tfloat y;"
	"\tuint32_t i;"
	"\tuint32_t j;"
	""
	"\ti = (x >> (32 - LOG2_N));"
	"\tj = (x >> (32 - 2*LOG2_N)) & MASK_N;"
	""
	"\tsin_x0 = qtab[i][0];"
	"\tcos_x0 = qtab[i][1];"
	""
	"\tdfx0 = x & DFX_MSK;"
	"\tqfx0 = dfx0 * DFX_RCP;"
	""
	"\tsin_fx0 = ftab[j][0];" 
	"\tcos_fx0 = ftab[j][1];"
	"\tsin_fx1 = ftab[j + 1][0];" 
	"\tcos_fx1 = ftab[j + 1][1];" 
	""
	"\tsin_fx = sin_fx0 + (sin_fx1 - sin_fx0) * qfx0;"
	"\tcos_fx = cos_fx0 + (cos_fx1 - cos_fx0) * qfx0;"
	""
	"\ty = sin_x0 * cos_fx + cos_x0 * sin_fx;"
	""
	"\treturn y;"
	"}"
	"#endif /* (FIXPT_SINCOS_INTRPL_TRIG) */"
	]
end	

function mk_f32sin(prefix, n::Int)

	c_lines = vcat(mk_c_head(n) , 
				   mk_c_qtab(n),
				   mk_c_ftab(n),
				   mk_c_sintab(n),
				   mk_c_f32sin_poly(n),
				   mk_c_f32sin_trig(n)
				   )

	out_dir = ".."
	fname = @sprintf("%s/%s.c", out_dir, prefix)

	@printf("\nWriting to file: \"%s\"...\n", fname)

	f = open(fname, "w")
	for s in c_lines
		write(f, s * "\n")
	end

	close(f)
end

function f32sin(x::UInt32, algo::Int=2)
	LOG2_N::UInt32 = UInt32(log2(N_SIN))
	MASK_N::UInt32 = (1 << LOG2_N) - 1
	DX::Int32 = 1 << (32 - LOG2_N)
	DX_MSK::UInt32 = DX - 1
	DX_RCP::Float32 = (1.0 / DX)
	DFX::Int32 = 1 << (32 - 2*LOG2_N)
	DFX_MSK::UInt32 = DFX - 1
	DFX_RCP::Float32 = (1.0 / DFX)
	Q31_RAD::Float32 = (π / (1 << 31))
	dy10::Float32 = 0
	y2::Float32 = 0
	y1::Float32 = 0
	y::Float32 = 0
	x0::UInt32 = 0
	x1::UInt32 = 0
	x2::UInt32 = 0
	x3::UInt32 = 0
	acc::Float32 = 0
	sin_x0::Float32 = 0
	cos_x0::Float32 = 0
	sin_x1::Float32 = 0
	cos_x1::Float32 = 0
	sin_fx::Float32 = 0
	cos_fx::Float32 = 0
	sin_fx0::Float32 = 0
	cos_fx0::Float32 = 0
	dfx0::Int32 = 0
	dx0::Int32 = 0
	dx1::Int32 = 0
	qx0::Float32 = 0
	qx1::Float32 = 0
	rdx0::Float32 = 0
	i::UInt32 = 0
	j::UInt32 = 0

	i = x >>> (32 - LOG2_N)
	j = (x >>> (32 - 2*LOG2_N) & MASK_N)

	dx0 = x & DX_MSK
	qx0 = dx0 * DX_RCP
	dx1 = dx0 - DX
	qx1 = dx1 * DX_RCP

	dfx0 = x & DFX_MSK
	rdx0 = dx0 * Q31_RAD

	(sin_x0, cos_x0) = qtab[i + 1]
	(sin_x1, cos_x1) = qtab[i + 2]

	if (algo == 1)

		dy10 = (sin_x1 - sin_x0)

		acc = sin_x0 + qx0 * dy10
	elseif (algo == 2)
		dy20::Float32 = 0
		dy21::Float32 = 0
		sin_x2::Float32 = 0
		cos_x2::Float32 = 0

		(sin_x2, cos_x2) = qtab[i + 3]

		dy10 = (sin_x1 - sin_x0)
		dy21 = (sin_x2 - sin_x1)
		dy20 = (dy21 - dy10) / 2

		acc = sin_x0 + qx0 * dy10
		acc += qx0 * qx1 * dy20 
	elseif (algo == 3)
		sin_fx1::Float32 = 0
		cos_fx1::Float32 = 0
		qfx0::Float32 = 0
	
		qfx0 = dfx0 * DFX_RCP

		(sin_fx0, cos_fx0) = ftab[j + 1] 
		(sin_fx1, cos_fx1) = ftab[j + 2] 

		sin_fx = rdx0
		cos_fx = 1 - rdx0 * rdx0 / 2
#		sin_fx = sin_fx0 + (sin_fx1 - sin_fx0) * qfx0
#		cos_fx = cos_fx0 + (cos_fx1 - cos_fx0) * qfx0

		acc = sin_x0 * cos_fx + cos_x0 * sin_fx
	else
		acc = sin_x0
	end

	return acc
end	

function f32sin(x::Int64, algo::Int=2)
	return f32sin(convert(UInt32, x), algo)
end	

println("Creating output file...");
mk_f32sin("f32sin", N_SIN)

K = 1024
w = collect(0 : 1/(1024*K) : 0.5-1/(1024*K))
#w = [-.500003, -.5, -.499997]
y = sin.(π .* w)
x = [Q31(a) for a in w]
y0 = [f32sin(a, 0) for a in x]
y1 = [f32sin(a, 1) for a in x]
y2 = [f32sin(a, 2) for a in x]
y3 = [f32sin(a, 3) for a in x]

dy0 = log2.(abs.(y - y0)) 
dy1 = log2.(abs.(y - y1))
dy2 = log2.(abs.(y - y2))
dy3 = log2.(abs.(y - y3)) 

ei0 = argmax(dy0)
ei1 = argmax(dy1)
ei2 = argmax(dy2)
ei3 = argmax(dy3)

@printf("Max errors:\n");
@printf(" - 0: %.10f @ %.10f\n", dy0[ei0], w[ei0])
@printf(" - 1: %.10f @ %.10f\n", dy1[ei1], w[ei1])
@printf(" - 2: %.10f @ %.10f\n", dy2[ei2], w[ei2])
@printf(" - 3: %.10f @ %.10f\n", dy3[ei3], w[ei3])

plot((x, [dy0 dy1 dy2 dy3]),
		  title = ["Lookup" "Poly1" "Poly2" "Trig2"],
		  titlefontsize = 9,
		  size = (1024, 768),
		  legend = false,
		  display = true
		 )
;

