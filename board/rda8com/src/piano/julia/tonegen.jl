#!julia
#

using Printf;
using Plots;

# Number of entries in the table
global N_SIN = 64
global SAMPLERATE = 48000


# Conversion form float to fixed point Q1.31 
#define Q31(F) ((int32_t)(double)(F) * (double)(1LL << 32))
function Q31(x::Float64)
	y::Int32 = round(x * convert(Float64, ((1 << 31) - 1)))
	return y
end

function Q15(x::Float64)
	y::Int32 = round(x * convert(Float64, (1 << 15)))
	return y
end

FP_TO_Q15(x::Real) = convert(Int32, round(x * Float64(32767.)))
FP_TO_Q16(x::Real) = convert(Int32, round(x * Float64(65535.)))
FP_TO_Q31(x::Real) = convert(Int32, round(x * Float64(2147483647.)))
FP_TO_Q32(x::Real) = convert(UInt32, round(x * Float64(4294967295.)))

Q15_TO_FP(x::Int32) = Float64(x * Float64(1.0 / 32767.))
Q16_TO_FP(x::UInt32) = Float64(x * Float64(1.0 / 65535.))
Q16_TO_FP(x::Integer) = Q16_TO_FP(UInt32(x))

Q31_TO_FP(x::Int32) = Float64(x * Float64(1.0 / 2147483647.))
Q32_TO_FP(x::UInt32) = Float64(x * Float64(1.0 / 4294967295.))

#Q16MUL(x::UInt32, y::UInt32) = UInt32(((x * y) + 32768) >> 16)
Q16MUL(x::UInt32, y::UInt32) = UInt32((x * y) >>> 16)
Q16MUL(x::Vector{Int32}, y::Vector{Int32}) = Int32.((x .* y) .>>> 16)
Q16MUL(x::Vector{Int32}, y::Int32) = Int32.((x .* y) .>>> 16)

Q15DIV(x::Int32, y::Int32) = Int32(div(x << 15, y))
Q16DIV(x::UInt32, y::UInt32) = UInt32(div(x << 16, y))
Q16DIV(x::Number, y::Number) = Q16DIV(UInt32(x), UInt32(y))

function Q31(x::Float32)
	return Q31(convert(Float64, x))
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

function Q31MUL(x::Int32, y::Int32)
	xx::Int64 = x
	yy::Int64 = y
	
	return convert(Int32, (xx * yy) >> 31)
end

function Q15MUL(x::Number, y::Number)
	xx::Int32 = x
	yy::Int32 = y
	return convert(Int32, (xx * yy) >> 15)
end

function Q15MUL(x::Any, y::Any)
	return Int32.((x .* y) .>> 15)
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

function dss_test(freq::Float64, samplerate=SAMPLERATE)
	dp::Int32 = Q31(freq / samplerate)
	ph::UInt32 = 0
	k1::Int32 = 8
	k2::Int32 = 64
	c1::Float32 = 1 - k1 * (1 / samplerate)
	c2::Float32 = 1 - k2 * (1 / samplerate)
	e1::Float32 = 0
	e2::Float32 = 0

	@printf("DSS test: Freq=%.3fHz Samplerate=%.1f\n", freq, samplerate);
	@printf("dw: %d\n", dw);

	t = collect(0:1/samplerate:1)

	y = zeros(Float32, length(t))
	z = zeros(Float32, length(t))
	e1 = 1;
	e2 = 1;
	for i in 1:length(t)
		y[i] = f32sin(ph, 2)
		ph += dpw
		z[i] = (e1 - e2);
		e1 = e1 * c1;
		e2 = e2 * c2;
	end

	k = maximum(z)
	z = 1/k .* z
	
	@printf("k=%f 1/k=%f c1=%f c2=%f\n", k, 1/k, c1, c2);
	@printf("k1=%f 1/k1=%f k2=%f 1/k2=%f\n", k1, 1/k1, k2, 1/k2);
	@printf("k1/k2=%f k2/k1=%f\n", k1/k2, k2/k1);

	return (t, y, z, c1, c2)
end

function q15sin(x::Int32, algo::Int=2)
	return Q31(f32sin(x, algo)) >> 16
end	

function dss_test_q15(freq, t_attack_ms::Integer, t_decay_ms::Integer, 
					  tmax=2, samplerate=SAMPLERATE)
	dph::Int32 = Q31(freq / samplerate)
	mask:: Int32 = 0x7
	ph::Int32 = 0
	k1::Int32 = div(4000, t_attack_ms)
	k2::Int32 = div(4000, t_decay_ms)
 	c1::UInt32 = FP_TO_Q16(1) - (k1 + k2)
 	c2::UInt32 = FP_TO_Q16(1) - k2
 #	c1::UInt32 = FP_TO_Q16(1) - Q16DIV(k1 + k2, samplerate)
 #	c2::UInt32 = FP_TO_Q16(1) - Q16DIV(k2, samplerate)
	g ::UInt32 = FP_TO_Q16(1)
	e1::UInt32 = FP_TO_Q16(1)
	e2::UInt32 = FP_TO_Q16(1)

	@printf("DSS test: Freq=%.3fHz Samplerate=%.1f\n", freq, samplerate);
	@printf("dphase: %d\n", dph);
	@printf("k1=%d k2=%d\n", k1, k2);
	@printf("c1=%.6f c2=%.6f\n", Q16_TO_FP(c1), Q16_TO_FP(c2));

	t = collect(0:1/samplerate:tmax-1/samplerate)
	n = length(t)
	@printf("Number of samples: %d\n", n);

	x1::UInt32 = 0
	x0::UInt32 = 0
	while true
		x0 = x1
		x1 = e2 - e1
		e1 = Q16MUL(e1, c1)
		e2 = Q16MUL(e2, c2)
		if (x1 < x0)
			break;
		end
	end

	g = Q16DIV(65535, x0);
	@printf("xmax: %d gain=%f\n", x0, Q16_TO_FP(g));

	x = zeros(Int32, length(t))
	y = zeros(Int32, length(t))
	z = zeros(Int32, length(t))
	w = zeros(Int32, length(t))
	e1 = FP_TO_Q16(1)
	e2 = FP_TO_Q16(1)
	for i in 1:length(t)
		x[i] = q15sin(ph, 2)
		ph += dph
		if (e1 > 0) || ((i & mask) == 0) 
			e2 = Q16MUL(e2, c2)
		end
		e1 = Q16MUL(e1, c1)
 		y[i] = Q16MUL(e2 - e1, g)
 		z[i] = e1
		w[i] = e2
	end

 #	k = maximum(z)
 #	@printf("k: %d gain=%f\n", k, 65535/k);

 #	for i in 1:length(z)
 #		z[i] = trunc(32767 .* z[i] / k)
 #	end
	
	return (t, x, y, z, w, c1, c2, g)
end

qsintab = zeros(Float32, N_SIN)
fsintab = zeros(Float32, N_SIN + 1)
fcostab = zeros(Float32, N_SIN + 1)

for i in 0:N_SIN-1
	θ = π*i/N_SIN
	local x = sin(θ)
	qsintab[i + 1] = convert(Float32, x)
end

for i in 0:N_SIN
	θ = π*i/(N_SIN * N_SIN)
	local x = sin(θ)
	fsintab[i + 1] = convert(Float32, x)
	local x = cos(θ)
	fcostab[i + 1] = convert(Float32, x)
end

function mk_exp_tab_q16(base=0.995370, plateau=5)
	n::Int64 = 0
	y::UInt16 = 65535
	
	exp_vec = UInt16[]

	while (y >= plateau) 
		append!(exp_vec, y)
		y = trunc((base^n) * 65535)
		n = n + 1
	end

	m::Int32 = 1 << Int32(ceil(log2(n)))
	@printf("Exponential table to zero length n=%d\n", n);
	@printf("EXtended to the next base 2 length m=%d\n", m)

	append!(exp_vec, zeros(UInt16, m-n))

	global exp_tab_q16 = exp_vec

	return exp_vec
end

function exp_q16_vec(itv_ms::Int=1000, tmax_ms::Int=10000, samplerate::Int=SAMPLERATE)
	EXP_TAB_LEN::Int32 = length(exp_tab_q16)
	LOG2_LEN::Int32 = log2(EXP_TAB_LEN) 
 #	exp_tab_k = (0x100000000 / (EXP_TAB_LEN * 1000))
 #	dt::Float32 = (EXP_TAB_LEN) / (itv_ms/1000 * samplerate)
	nsamples::UInt32 = div(samplerate * itv_ms, 1000)
	inc::UInt32 = div(0xffffffff, nsamples)
	ntmax::UInt32 = div(samplerate * tmax_ms, 1000)
 #	inc::UInt32 = FP_TO_Q32(dt)
	acc::UInt32 = 0
	n::UInt32 = 0
	y0::UInt32 = 0x0000ffff

	@printf("exp_q16_vec(): nsamples=%d inc=%d nmax=%d\n", nsamples, inc, ntmax);
	nsamples = (nsamples > ntmax) ? ntmax : nsamples 

	expvec = zeros(UInt16, nsamples)
	
	if (inc <= 8)
		return
	end
	while (n < nsamples)
		idx::UInt32 = (acc >> (32 - LOG2_LEN))
		y::UInt16 = exp_tab_q16[idx + 1]
		acc += inc
		n += 1
		expvec[n]=y
	end

	return expvec
end

function envelope_q16_vec(attack_itv::Int=1000, samplerate::Int=SAMPLERATE)
	EXP_TAB_LEN::Int32 = length(exp_tab_q16)
	LOG2_LEN::Int32 = log2(EXP_TAB_LEN) 
	nsamples::UInt32 = div(samplerate * itv_ms, 1000)
	inc1::UInt32 = div(0xffffffff, nsamples)
	acc1::UInt32 = 0
	n::UInt32 = 0
	y0::UInt32 = 0x0000ffff
	vec = zeros(UInt16, nsamples)

	@printf("nsamples=%d inc: %d\n", nsamples, inc);
	
	if (inc <= 8)
		return
	end
	while (n < nsamples)
		idx::UInt32 = (acc >> (32 - LOG2_LEN))
		y::UInt16 = exp_tab_q16[idx + 1]
		acc += inc
		n += 1
		vec[n]=y
	end

	return vec
end

 #(t, s, env, c1, c2) = dss_test(999.45, 22050)
(t, x, y, z, w, c1, c2, g) = dss_test_q15(500, 200, 4000, 4)

cb::Float64 = c2 / 65536	
ca::Float64 = c1 / 65536	
 #e = convert(Vector{Int32}, env)
nmax = log(log(ca)^2/log(cb)^2)/log(cb/ca)
ymax = (cb.^nmax .- ca.^nmax)
gain = 65536 / ymax

 mk_exp_tab_q16(0.995370)
 # mk_exp_tab_q16(0.998200, 2048)

e1 = convert(Vector{Int32}, exp_q16_vec(20000, 4000))
e2 = convert(Vector{Int32}, exp_q16_vec(50))

append!(e2, zeros(Int32, length(e1) - length(e2)))


env = (e1 .- e2)
emax = maximum(env)

ke::Int32 = Q16DIV(65535, emax)
env = Q16MUL(env, ke)

@printf("emax=%d ke=%.f\n", emax, Q16_TO_FP(ke));

 #t = collect(1:length(e1))

plotly()

plot((t, [y z w]), linetype=:steppre, 
 #			 color = [:orange :royalblue :red :green], 
			 size=(1366,768),
			 display=true)

;

