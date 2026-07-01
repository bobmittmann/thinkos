#!julia
#

using Printf;
using Plots;

# Number of entries in the table
N_SIN = 64

# Conversion form float to fixed point Q1.31 
#define Q31(F) ((int32_t)(double)(F) * (double)(1LL << 32))
function Q31(x::Float64)
	y::Int64 = 0
	y = Int(round(x * convert(Float64, (1 << 31))))
	return y
end

function Q31F(x::Int64)
	y::Float32 = 0
	y = x * (1.0 / (1 << 31))
	return y
end

function Q31F(x::Int32)
	return Q31F(convert(Int64, x))
end

function Q31SAT(x::Int64)
	y::Int32 = 0
	Q31_MIN = Int64(Q31(-1.0))
	Q31_MAX = Int64(Q31(1.0) - 1)

	y = (x < Q31_MIN) ? Q31_MIN : ((x > Q31_MAX) ? Q31_MAX : x);

	return y;
end

function f32sin(x::Int32, algo::Int=2)
	LOG2_N::Int32 = Int32(log2(N_SIN))
	MASK_N::Int32 = (1 << LOG2_N) - 1
	DX::Int32 = 1 << (31 - LOG2_N)
	dy10::Float32 = 0
	y2::Float32 = 0
	y1::Float32 = 0
	y::Float32 = 0
	x0::Int32 = 0
	x1::Int32 = 0
	x2::Int32 = 0
	x3::Int32 = 0
	qx0::Float32 = 0
	acc::Float32 = 0
	sin_x0::Float32 = 0
	cos_x0::Float32 = 0
	sin_x1::Float32 = 0
	cos_x1::Float32 = 0
	sin_fx::Float32 = 0
	cos_fx::Float32 = 0
	sin_fx0::Float32 = 0
	cos_fx0::Float32 = 0
	fx::Int32 = 0
	ffx::Int32 = 0
	i::Int32 = 0
	j::Int32 = 0

	i = x >>> (31 - LOG2_N)
	x0 = i << (31 - LOG2_N)
	sin_x0 = qsintab[(i & MASK_N) + 1] * ((x0 < 0) ? -1 : 1)

	if (algo == 1)
		dy20::Float32 = 0
		dy21::Float32 = 0
		qx1::Float32 = 0
		sin_x2::Float32 = 0

		x1 = x0 + DX
		sin_x1 = qsintab[((i + 1) & MASK_N) + 1] * ((x1 < 0) ? -1 : 1)

		x2 = x1 + DX
		sin_x2 = qsintab[((i + 2) & MASK_N) + 1] * ((x2 < 0) ? -1 : 1)

		qx0 = (x - x0) * Float32(1.0 / DX)
		qx1 = (x - x1) * Float32(1.0 / DX)

		dy10 = (sin_x1 - sin_x0)
		dy21 = (sin_x2 - sin_x1)
		dy20 = (dy21 - dy10) / 2

		acc = sin_x0
		acc += qx0 * dy10
		acc += qx0 * qx1 * dy20
	elseif (algo == 2)
		sin_fx1::Float32 = 0
		cos_fx1::Float32 = 0
		qfx::Float32 = 0
		sfx::Float32 = 0
		DFX::Int32 = 1 << (31 - 2*LOG2_N)
	

		x1 = x0 + Int32(Q31(0.5))
		j = x1 >>> (31 - LOG2_N)
		cos_x0 = qsintab[(j & MASK_N) + 1] * ((x1 < 0) ? -1 : 1)

		fx = (x - x0)
		i = fx >>> (31 - 2*LOG2_N)
		ffx = fx - (i << (31 - 2*LOG2_N))
		qfx = ffx * Float32(1.0 / DFX)

		sin_fx0 = fsintab[i + 1] 
		sin_fx1 = fsintab[i + 2] 
		sin_fx = sin_fx0 + (sin_fx1 - sin_fx0) * qfx

		cos_fx0 = fcostab[i + 1]
		cos_fx1 = fcostab[i + 2]
		cos_fx = cos_fx0 + (cos_fx1 - cos_fx0) * qfx

		acc = sin_x0 * cos_fx
		acc += cos_x0 * sin_fx
	else
		acc = sin_x0
	end

	return acc
end	

function f32sin(x::Int64, algo::Int=2)
	return f32sin(Q31SAT(x), algo)
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

function f32_table_entry(i::Int, x::Float64)
	y = convert(Float32, x)
	return @sprintf("\t%13.6a, /* %3d -> %13.10f */", y, i, x)
end

f32_qsin_table_entry(i::Int, n::Int) = f32_table_entry(i, sin(π*i/n))

f32_fsin_table_entry(i::Int, n::Int) = f32_table_entry(i, sin(π*i/(n*n)))

f32_fcos_table_entry(i::Int, n::Int) = f32_table_entry(i, cos(π*i/(n*n)))


function mk_c_qsintab(n::Int)
	txt1 = AbstractString[
	"const float qsintab[] = {"
	]

	txt2 = AbstractString[
	f32_qsin_table_entry(i, n) for i in 0:n-1
	]

	txt3 = AbstractString[
	"};"
	""
	"#define DX ((int32_t)(1 << (31 - LOG2_N)))"
	""
	]

	return vcat(txt1, txt2, txt3)
end

function mk_c_frac_tabs(n::Int)
	txt1 = AbstractString[
	"const float fsintab[] = {"
	]

	txt2 = AbstractString[
	f32_fsin_table_entry(i, n) for i in 0:n
	]

	txt3 = AbstractString[
	"};"
	""
	"const float fcostab[] = {"
	]

	txt4 = AbstractString[
	f32_fcos_table_entry(i, n) for i in 0:n
	]

	txt5 = AbstractString[
	"};"
	""
	"#define DFX ((int32_t)(1 << (31 - 2*LOG2_N)))"
	""
	]

	return vcat(txt1, txt2, txt3, txt4, txt5)
end

# Polynomial interpolation version
function mk_c_f32sin_poly(n::Int64)
	txt = AbstractString[
	""
	"#if (FIXPT_SINCOS_INTRPL_POLY)"
	"/* Polynomial interpolation version */"
	"float f32sin(int32_t x)"
	"{"
	"\tfloat y321;"
	"\tfloat y32;"
	"\tfloat y3;"
	"\tfloat x3;"
	"\tfloat qx2;"
	"\tfloat y21;"
	"\tfloat y2;"
	"\tfloat y1;"
	"\tfloat y;"
	"\tint32_t x2;"
	"\tint32_t x1;"
	"\tfloat qx1;"
	"\tint32_t i;"
	""
	"\ti = x >> (31 - LOG2_N);"
	""
	"\tx1 = i << (31 - LOG2_N);"
	"\ty1 = qsintab[i++ & MASK_N] * ((x1 < 0) ? -1 : 1);"
	""
	"\tx2 = x1 + DX;"
	"\ty2 = qsintab[i++ & MASK_N] * ((x2 < 0) ? -1 : 1);"
	""
	"\tx3 = x2 + DX;"
	"\ty3 = qsintab[i & MASK_N] * ((x3 < 0) ? -1 : 1);"
	""
	"\ty21 = (y2 - y1);"
	"\ty32 = (y3 - y2);"
	"\ty321 = (y32 - y21) / 2;"
	""
	"\tqx1 = (x - x1) * float(1.0 / DX);"
	"\tqx2 = (x - x2) * float(1.0 / DX);"
	""
	"\ty = y1 + y21 * qx1 + y321 * qx1 * qx2;"
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
	"float f32sin(int32_t x)"
	"{"
	"\tint32_t x0;"
	"\tint32_t x1;"
	"\tint32_t fx;"
	"\tint32_t ffx;"
	"\tfloat sin_x0;"
	"\tfloat cos_x0;"
	"\tfloat sin_fx0;"
	"\tfloat sin_fx1;"
	"\tfloat sin_fx;"
	"\tfloat cos_fx0;"
	"\tfloat cos_fx1;"
	"\tfloat cos_fx;"
	"\tfloat qfx;"
	"\tfloat y;"
	"\tint32_t i;"
	""
	"\ti = x >> (31 - LOG2_N);"
	""
	"\tx0 = i << (31 - LOG2_N);"
	"\tsin_x0 = qsintab[i & MASK_N] * ((x0 < 0) ? -1 : 1);"
	""
	"\tx1 = x0 + Q31(0.5);"
	"\ti = x1 >> (31 - LOG2_N);"
	"\tcos_x0 = qsintab[i & MASK_N] * ((x1 < 0) ? -1 : 1);"
	""
	"\tfx = (x - x0);"
	"\ti = fx >> (31 - 2*LOG2_N);"
	""
	"\tffx = fx - (i << (31 - 2*LOG2_N));"
	"\tqfx = ffx * (float)(1.0 / DFX);"
	""
	"\tsin_fx0 = fsintab[i];" 
	"\tsin_fx1 = fsintab[i + 1];" 
	"\tsin_fx = sin_fx0 + (sin_fx1 - sin_fx0) * qfx;"
	""
	"\tcos_fx0 = fcostab[i];"
	"\tcos_fx1 = fcostab[i + 1];"
	"\tcos_fx = cos_fx0 + (cos_fx1 - cos_fx0) * qfx;"
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
				   mk_c_qsintab(n),
				   mk_c_frac_tabs(n),
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

function float32_table_entry(i::Int, x::Float64)
	y = convert(Float32, x)
	return @sprintf("\t%a,", y)
end

float32_qsin_table_entry(i::Int, n::Int) = float32_table_entry(i, sin(π*i/n))

float32_fsin_table_entry(i::Int, n::Int) = float32_table_entry(i, sin(π*i/(n*n)))

float32_fcos_table_entry(i::Int, n::Int) = float32_table_entry(i, cos(π*i/(n*n)))

function mk_c_float32_qsintab(n::Int)
	txt1 = AbstractString[
	"const float sintab[] = {"
	]

	txt2 = AbstractString[
	float32_qsin_table_entry(i, n) for i in 0:n-1
	]

	txt3 = AbstractString[
	"};"
	""
	"#define DX ((int32_t)(1 << (31 - LOG2_N)))"
	""
	]

	return vcat(txt1, txt2, txt3)
end

function mk_c_float32_ftabs(n::Int)
	txt1 = AbstractString[
	"const float fsintab[] = {"
	]

	txt2 = AbstractString[
	float32_fsin_table_entry(i, n) for i in 0:n
	]

	txt3 = AbstractString[
	"};"
	""
	"const float fcostab[] = {"
	]

	txt4 = AbstractString[
	float32_fcos_table_entry(i, n) for i in 0:n
	]

	txt5 = AbstractString[
	"};"
	""
	"#define DFX ((int32_t)(1 << (31 - 2*LOG2_N)))"
	""
	]

	return vcat(txt1, txt2, txt3, txt4, txt5)
end


function mk_float32_sin(prefix, n::Int)

	c_lines = vcat(mk_c_head(n) , 
				   mk_c_float32_qsintab(n),
				   mk_c_float32_ftabs(n)
				   )

	out_dir = "."
	fname = @sprintf("%s/%s.c", out_dir, prefix)

	@printf("\nWriting to file: \"%s\"...\n", fname)

	f = open(fname, "w")
	for s in c_lines
		write(f, s * "\n")
	end

	close(f)
end

function dss_test(freq::Float64, samplerate=88200)
	dw::Int32 = Q31(freq / samplerate)
	w::Int32 = 0

	@printf("DSS test: Freq=%.3fHz Samplerate=%.1f\n", freq, samplerate);
	@printf("dw: %d\n", dw);

	t = collect(0:1/samplerate:16/freq)

	y = zeros(Float32, length(t))
	for i in 1:length(t)
		y[i] = f32sin(w, 2)
		w += dw
	end
	
	return (t, y)
end

qsintab = zeros(Float32, N_SIN)
fsintab = zeros(Float32, N_SIN + 1)
fcostab = zeros(Float32, N_SIN + 1)

for i in 0:N_SIN-1
	θ = π*i/N_SIN
	x = sin(θ)
	qsintab[i + 1] = convert(Float32, x)
end

for i in 0:N_SIN
	θ = π*i/(N_SIN * N_SIN)
	x = sin(θ)
	fsintab[i + 1] = convert(Float32, x)
	x = cos(θ)
	fcostab[i + 1] = convert(Float32, x)
end

println("Creating output file...");
mk_f32sin("f32sin", N_SIN)

println("Calculating error...");

w = collect(0.0 :1/(1024*32):0.5)
#w = [-.500003, -.5, -.499997]
y = sin.(π .* w)
x = [Q31(a) for a in w]
y0 = [f32sin(a, 0) for a in x]
y1 = [f32sin(a, 1) for a in x]
y2 = [f32sin(a, 2) for a in x]
#y3 = [f32sin(a, 3) for a in x]
#y4 = [f32sin(a, 4) for a in x]

dy0 = (y - y0)
dy1 = (y - y1)
dy2 = (y - y2)
#dy3 = (y - y3)
#dy4 = (y - y4)

ei0 = argmax(abs.(dy0))
ei1 = argmax(abs.(dy1))
ei2 = argmax(abs.(dy2))
#ei3 = argmax(abs.(dy3))
#ei4 = argmax(abs.(dy4))

@printf("Max errors:\n");
@printf(" - 0: %.10f @ %.10f\n", dy0[ei0], w[ei0])
@printf(" - 1: %.10f @ %.10f\n", dy1[ei1], w[ei1])
@printf(" - 2: %.10f @ %.10f\n", dy2[ei2], w[ei2])
#@printf(" - 3: %.10f @ %.10f\n", dy3[ei3], w[ei3])
#@printf(" - 4: %.10f @ %.10f\n", dy4[ei4], w[ei4])

#@printf("Error: %.9f %.9f %.9f...", y1, y2, y1-y2);
#i = argmax(dy2)
#@printf("Max error=%.9f @ %d = %.9f - %.9f...", dy[i], i, y1[i], y2[i])
#

plotly()

#=
p0 = plot((x, dy0.*1e9))
p1 = plot((x, dy1.*1e9), linetype = :steppre)
p2 = plot((x, dy2.*1e9), linetype = :steppre)
display(plot(p1, p2,
		  title = ["Lookup" "Poly2" "Trig2"],
		  titlefontsize = 9,
		  size = (1366,768),
		  layout = (2,1),
		  legend = false,
		  display = true
		 )
	   )

=#


#plot((x, [dy1, dy2]), linetype=:steppre, 
(t, s) = dss_test(999.45)

plot((t, [s]), linetype=:steppre, 
			 color = [:orange :royalblue], 
			 size=(1366,768),
			 display=true)
;

