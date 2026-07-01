#!julia
#

using Printf;
using Plots;

plotly()

global PCM_SAMPLE_RATE::UInt32 = 128000
global PCM_FRAME_LEN::UInt32 = 128
global PCM_FRAME_RATE::UInt32 = div(PCM_SAMPLE_RATE, PCM_FRAME_LEN)

ms_to_pcm_clock(ms) = div(UInt32(ms) * PCM_SAMPLE_RATE, UInt32(1000))
ms_to_pcm_frames(ms) = div(UInt32(ms) * PCM_SAMPLE_RATE, 
						   UInt32(1000 * PCM_FRAME_LEN))
# Frame aligned clock
ms_to_pcm_aligned_clock(ms) = ms_to_pcm_frames(ms) * PCM_FRAME_LEN

FP_TO_Q16(x::Real) = convert(UInt16, round(x * Float64(65535)))
FP_TO_Q32(x::Real) = convert(UInt32, round(x * Float64(4294967296.)))

Q31_TO_FP(x::Int) = Float64(x * (1.0 / (1 << 31)))
Q16_TO_FP(x::UInt32) = Float64(x * (1.0 / 0xffff))

Q16_TO_FP(x::UInt32) = Float64(x * (1.0 / 0xffff))

Q15Q15MUL(x::Int16, y::Int16) = Int32((Int32(x) * Int32(y)) >> 15)

Q16Q15MUL(x::UInt16, y::Int16) = Int32((UInt32(x) * Int32(y)) >> 16)

Q16_UMUL(x::UInt32, y::UInt32) = UInt32((UInt32(x) * UInt32(y)) >> 16)

Q16_UDIV(x::UInt32, y::UInt32) = UInt32(div(UInt32(x * 65536), y))

Q32_UDIV(x::UInt32, y::UInt32) = UInt32(div(UInt64(x * 4294967295), y))

function Q32Q16_UMUL(x32::UInt32, y16::UInt32) 
	low::UInt32 = y16*(x32 & 0xffff)
	high::UInt32 = y16*(x32 >> 16) + (low >> 16)
	return high
end

global Q16_MAX = UInt32(65535)
global Q16_MIN = UInt32(0)
global Q32_MAX = UInt32(4294967295)
	
Q15DIV(x::Int32, y::Int32) = Int32(div(x << 15, y))
Q16DIV(x::Int32, y::Int32) = Q16DIV(UInt32(x), UInt32(y))

mutable struct Smoothstep
	steps::UInt32
	x::Float32
	c1::Float32
	y::Float32
	dy::Float32

	Smoothstep() = new(0, 0, 0.0, 0.0, 0.0)
end

function set(smooth::Smoothstep, y0::Float32, y1::Float32, itv::Float32)
	steps = Int32(trunc(PCM_SAMPLE_RATE * itv))
	smooth.x = steps
	smooth.c1 = 1 / steps

	smooth.dy = (y0 - y1);
	smooth.y = y1;
end

function set(smooth::Smoothstep, y0::Float64, y1::Float64, itv::Float64)
	set(smooth, Float32(y0), Float32(y1), Float32(itv))
end

function step(smooth::Smoothstep)

	x = smooth.x

	x = (x >= 1) ? (x - 1) : x
	xs = x * smooth.c1

	y = (xs * xs) * (-2 * xs + 3) * smooth.dy + smooth.y

	smooth.x = x
	return y;
end

function step(smooth::Smoothstep, v::Vector{Number})
	for i = 1 : length(v)
		v[i] = step(smooth)
	end
end

function smooth_envelope(t)

	smooth = Smoothstep()

	t1 = 0.1
	t2 = 0.4
	t3 = t[end] - 0.5
	t4 = t[end]

	set(smooth, 0.0, 1.0, t1)

	n1 = Int64(round(t1 * PCM_SAMPLE_RATE))
	n2 = Int64(round(t2 * PCM_SAMPLE_RATE))
	n3 = Int64(round(t3 * PCM_SAMPLE_RATE))
	n4 = length(t)

	y = fill(Float32(0), length(t))

	for i = 1 : n1 - 1
		y[i] = step(smooth)
	end

	set(smooth, 1.0, 0.5, t2-t1)

	for i = n1 : n2 - 1
		y[i] = step(smooth)
	end

	for i = n2 : n3 - 1
		y[i] = step(smooth)
	end

	set(smooth, 0.5, 0.0, t4-t3)

	for i = n3 : n4 - 1
		y[i] = step(smooth)
	end

	y = y .* 65535
	return (y)
end

function exp_envelope(t)
	attack_itv_ms::Int32 = 30
	decay_itv_ms::Int32 =  100
	hold_itv_ms::Int32 = 100
	release_itv_ms::Int32 = 100

	nattack = ms_to_pcm_aligned_clock(attack_itv_ms)
	ndecay = ms_to_pcm_aligned_clock(decay_itv_ms)
	nhold = ms_to_pcm_aligned_clock(hold_itv_ms)
	nrelease = ms_to_pcm_aligned_clock(release_itv_ms)

    k1::Float32 = 1/nattack;
    k2::Float32 = 1/(nattack +ndecay);
    k3::Float32 = 1/nrelease;
	c1::Float32 = 1 - k1;
	c2::Float32 = 1 - k2;
	c3::Float32 = 1 - k3;
	e1::Float32 = 1
	e2::Float32 = 1
	e3::Float32 = 1
	x0::Float32 = 0
	x1::Float32 = 0
	g::Float32 = 0
	n::Int32 = 0
	dn::Int32 = 0

	N = length(t)
	y = fill(Float32(0), N)

	c3 = 0.999985
    while (true)
        x0 = x1;
        e1 = e1 * c1
        e2 = e2 * c2
        e3 = e3 * c3
		x1 = e3 + 3*e2 - 4*e1
		n += 1
    	if (x1 < x0)
			break
		end
	end

    g = 1/x0;


	e1 = 1
	e2 = 1
	e3 = 1
	n3 = n + nhold 
	for i = 1 : n3 - 1
        e1 = e1 * c1
        e2 = e2 * c2
        e3 = e3 * c3
		y[i] = (e3 + 3*e2 - 4*e1) * g
	end

	c3 =  1 - k3;
	for i = n3 : N
        e1 = e1 * c1
        e2 = e2 * c2
        e3 = e3 * c3
		y[i] = (e3 + 3*e2 - 4*e1) * g
	end

	y = y .* 65535

	return y
end

function exp_envelope_q32(t)

	N = length(t)
	y = fill(Int32(0), N)

	attack_itv_ms::Int32 = 30
	decay_itv_ms::Int32 = 100
	hold_itv_ms::Int32 = 1000
	release_itv_ms::Int32 = 100

	nattack = ms_to_pcm_aligned_clock(attack_itv_ms)
	nhold = ms_to_pcm_aligned_clock(hold_itv_ms)
	ndecay = ms_to_pcm_aligned_clock(decay_itv_ms)
	nrelease = ms_to_pcm_aligned_clock(release_itv_ms)

    @printf("envelope: a=%6u d=%6u r=%6u\n", nattack, ndecay, nrelease)

    k1::UInt32 = Q16_UDIV(Q16_MAX, nattack) >> 16;
    k2::UInt32 = Q16_UDIV(Q16_MAX, nattack+ndecay) >> 16;
    k3::UInt32 = Q16_UDIV(Q16_MAX, nrelease) >> 16;
	c1::UInt32 = Q16_MAX - k1;
	c2::UInt32 = Q16_MAX - k2;
	c3::UInt32 = Q16_MAX - k3;
	e1::UInt32 = Q32_MAX 
	e2::UInt32 = Q32_MAX 
	e3::UInt32 = Q32_MAX 
	x1::UInt32 = 0
    x0::UInt32 = 0
	g::UInt32 = 0
	n::UInt32 = 0

    @printf("envelope: k1=%6u k2=%6u k3=%6u\n", k1, k2, k3)
    @printf("envelope: c1=%6u c2=%6u c3=%6u", c1, c2, c3);

	c3 = Q16_MAX + 0

	n = 0 
    while (true)
        x0 = x1;
        e1 = Q32Q16_UMUL(e1, c1)
        e2 = Q32Q16_UMUL(e2, c2)
        e3 = Q32Q16_UMUL(e3, c3)
		x1 = UInt32((e3>>2) + 3*(e2>>2) - e1) 
		x1 = x1 >> 16
		n += 1
    	if (x1 < x0)
			break
		end
	end

    g = Q16_UDIV(Q16_MAX, x0);

    @printf(" gain=%6d pos=%d x0=%d\n", g, n, x0);

	e1 = Q32_MAX 
	e2 = Q32_MAX 
	e3 = Q32_MAX

	n3 = n + nhold 
	for i = 1 : n3 - 1
        e1 = Q32Q16_UMUL(e1, c1)
        e2 = Q32Q16_UMUL(e2, c2)
        e3 = Q32Q16_UMUL(e3, c3)
		x1 = UInt32((e3>>2) + 3*(e2>>2) - e1) 
 		y[i] = Q32Q16_UMUL(x1, g) >> 16
 #		y[i] = x >> 16
	end

	c3 = Q16_MAX - k3;
	for i = n3 : N
        e1 = Q32Q16_UMUL(e1, c1)
        e2 = Q32Q16_UMUL(e2, c2)
        e3 = Q32Q16_UMUL(e3, c3)
		x1 = UInt32((e3>>2) + 3*(e2>>2) - e1) 
 		y[i] = Q32Q16_UMUL(x1, g) >> 16
 #		y[i] = x >> 16
	end

	return y
end

function exp_envelope_q16(t)

	N = length(t)
	y = fill(Int32(0), N)

	attack_itv_ms::Int32 = 20
	decay_itv_ms::Int32 = 125
	hold_itv_ms::Int32 = 1000
	release_itv_ms::Int32 = 125

	nattack = ms_to_pcm_aligned_clock(attack_itv_ms)
	nhold = ms_to_pcm_aligned_clock(hold_itv_ms)
	ndecay = ms_to_pcm_aligned_clock(decay_itv_ms)
	nrelease = ms_to_pcm_aligned_clock(release_itv_ms)

    @printf("envelope: a=%6u d=%6u r=%6u\n", nattack, ndecay, nrelease)

    k1::UInt32 = Q16_UDIV(Q16_MAX, nattack) >> 16;
    k2::UInt32 = Q16_UDIV(Q16_MAX, ndecay + nattack) >> 16;
    k3::UInt32 = Q16_UDIV(Q16_MAX, nrelease) >> 16;
	c1::UInt32 = Q16_MAX - k1;
	c2::UInt32 = Q16_MAX - k2;
	c3::UInt32 = Q16_MAX - k3;
	e1::UInt32 = Q16_MAX 
	e2::UInt32 = Q16_MAX 
	e3::UInt32 = Q16_MAX 
	x1::UInt32 = 0
    x0::UInt32 = 0
	g::UInt32 = 0
	n::UInt32 = 0

#	yn = c3^n + c2^n - c1^n
#
#	c2 = (yn + c1^n - c3^n) ^ 1/n
#   e1 = c1^n
#   e2 = c2^n
#   e3 = c3^n
#	c3 = e3^1/n
#
#   x^1/n ~= 1 - (1-x)/n
#   c3 ~= 1 - (1-e3)/n
#   k3 = (1-e3)/n
#   
#   ex for e3==1/2
#	c3 ~= 1 - 0.5/n
#	k3 = 0.5/n
#   n == number of steps on a period of time
#   n is in proportion to the samplerate 
#
#   k3 = 
#
    @printf("envelope: k1=%6u k2=%6u k3=%6u\n", k1, k2, k3)


    @printf("envelope: c1=%6u c2=%6u c3=%6u", c1, c2, c3);

	c3 = Q16_MAX + 1

	n = 0 
    while (true)
        x0 = x1;
        e1 = Q16_UMUL(e1, c1)
        e2 = Q16_UMUL(e2, c2)
        e3 = Q16_UMUL(e3, c3)
		x1 = UInt32(e3 + 3*e2 - 4*e1)
		n += 1
    	if (x1 < x0)
			break
		end
	end

    g = Q16_UDIV(Q16_MAX, x0);

    @printf(" gain=%6d pos=%d x0=%d\n", g, n, x0);

	e1 = Q16_MAX 
	e2 = Q16_MAX 
	e3 = Q16_MAX

 #=	n2 = div(N, 4) * 1
	for i = 1 : n2 - 1
        e1 = Q16_UMUL(e1, c1)
        e2 = Q16_UMUL(e2, c2)
        e3 = Q16_UMUL(e3, c3)
		x = UInt32(e3 + 4*e2 - 5*e1)
		y[i] = Q16_UMUL(x, g)
	end
=#
 #	n3 = div(N, 4) * 3
	n3 = n + nhold 
	for i = 1 : n3 - 1
        e1 = Q16_UMUL(e1, c1)
        e2 = Q16_UMUL(e2, c2)
        e3 = Q16_UMUL(e3, c3)
		x = UInt32(e3 + 3*e2 - 4*e1)
		y[i] = Q16_UMUL(x, g)
	end

	c3 = Q16_MAX - k3;
	for i = n3 : N
        e1 = Q16_UMUL(e1, c1)
        e2 = Q16_UMUL(e2, c2)
        e3 = Q16_UMUL(e3, c3)
		x = UInt32(e3 + 3*e2 - 4*e1)
		y[i] = Q16_UMUL(x, g)
	end

	return y
end

time_end = 2
t = collect(0:1/PCM_SAMPLE_RATE:time_end-1/PCM_SAMPLE_RATE)

y1 = exp_envelope(t)
y2 = exp_envelope_q16(t)
y3 = smooth_envelope(t)
y4 = exp_envelope_q32(t)

plot((t, [y1 y2 y3 y4]),
	 size = (1280, 256),
	 display = true
	)
	;

