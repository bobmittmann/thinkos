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

function dss_test(freq::Float64, samplerate=88200)
	dw::Int32 = Q31(freq / samplerate)
	w::Int32 = 0
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
		y[i] = f32sin(w, 2)
		w += dw
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


plotly()

(t, s, env, c1, c2) = dss_test(999.45, 22050)

plot((t, [env, s .* env]), linetype=:steppre, 
			 color = [:orange :royalblue], 
			 size=(1366,768),
			 display=true)
;

