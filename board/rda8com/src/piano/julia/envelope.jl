#!julia
#

using Printf;
using Plots;

plotly()

SAMPLERATE = 11025

mutable struct Smoothstep
	steps::UInt32
	x::Float32
	c1::Float32
	y::Float32
	dy::Float32

	Smoothstep() = new(0, 0, 0.0, 0.0, 0.0)
end

function set(smooth::Smoothstep, y0::Float32, y1::Float32, itv::Float32)
	steps = Int32(trunc(SAMPLERATE * itv))
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

function envelope(samplerate, time_end)

	t = collect(0:1/samplerate:time_end)

	smooth = Smoothstep()

	t1 = 0.05
	t2 = 0.085

	set(smooth, 0.0, 1.0, t1)

	n1 = Int64(round(t1 * samplerate))
	n2 = Int64(round(t2 * samplerate))
	n3 = length(t)

	y = fill(Float32(0), length(t))

	e1::Float32 = 1
	e2::Float32 = 1
	c1::Float32 = 1
	c2::Float32 = 1 - 1/64

	for i = 1 : n1 - 1
		y[i] = step(smooth)
	end

	set(smooth, 1.0, 0.5, t2-t1)

	c2 = 1 - 1/4;
	c1 = 1 - 1/128;

	for i = n1 : n2 - 1
		y[i] = step(smooth)
	end

	for i = n2 : n3 - 1
		y[i] = step(smooth)
	end

	return (t, y)
end

ms2samples(ms, sr, fs) = Int32(round(ms * sr / (1000 * fs))) * fs

function exp_envelope(samplerate, time_end)

	t = collect(0:1/samplerate:time_end)

	smooth = Smoothstep()

	attack_itv_ms::Int32 = 125
	hold_itv_ms::Int32 = 0
	decay_itv_ms::Int32 = 125
	release_itv_ms::Int32 = 2500
	framesize::Int32 = 128
	n::Int32 = 1
	dn::Int32 = 0

	nattack = ms2samples(attack_itv_ms, samplerate, framesize)
	nhold = ms2samples(hold_itv_ms, samplerate, framesize)
	ndecay = ms2samples(decay_itv_ms, samplerate, framesize)
	nrelease = ms2samples(release_itv_ms, samplerate, framesize)

	N = length(t)
	y = fill(Float32(0), N)

	e1::Float32 = 1
	e2::Float32 = 1
	c1::Float32 = 1
	c2::Float32 = 1

#	yn = e1 - e2 * c2 ^ n
#	yn = e1 * c1^n - e2 * c2^n
#
#	c1^n = (yn + e2 * c2^n) / e1
#
#	c2^n = (e1 - yn) / e2
#   n*log(c2) = log((e1 - yn) / e2)
#   log(c2) = log((e1 - yn) / e2) / n 
#   log(c2) = log(((e1 - yn) / e2) ^ 1/n)
#   c2 = ((e1 - yn) / e2) ^ 1/n

	dn = nattack
	yn = 0.90
	c2 = ((e1 - yn) / e2) .^ (1 / dn)
	@printf("attack: dn=%d c1=%15.12f c2=%15.12f\n", dn, c1, c2)

	for i = 1 : n + dn
		y[i] = e1 - e2
		e2 = e2 * c2
		e1 = e1 * c1
	end
	n += dn

	dn = nhold
	yn = 0.5
	c1 = ((e2.*c2^dn + yn) / e1) .^ (1 / dn)
	@printf("hold: dn=%d c1=%15.12f c2=%15.12f\n", dn, c1, c2)

	for i = n + 1 : n + dn
		y[i] = e1 - e2
		e2 = e2 * c2
		e1 = e1 * c1
	end
	n += dn

	dn = ndecay
	yn = 0.45
#	c2 = ((e1.*c1^dn - yn) / e2) .^ (1 / dn)
	c1 = ((e2.*c2^dn + yn) / e1) .^ (1 / dn)
	@printf("decay: dn=%d c1=%15.12f c2=%15.12f\n", dn, c1, c2)

	for i = n + 1 : n + dn
		y[i] = e1 - e2
		e2 = e2 * c2
		e1 = e1 * c1
	end
	n += dn

	dn = nrelease
	yn = 0
#	c2 = ((e1.*c1^dn - yn) / e2) .^ (1 / dn)
	c1 = ((e2.*c2^dn + yn) / e1) .^ (1 / dn)
	@printf("release: dn=%d c1=%15.12f c2=%15.12f\n", dn, c1, c2)

	for i = n + 1 : N
		y[i] = e1 - e2
		e2 = e2 * c2
		e1 = e1 * c1
	end


	return (t, y)
end

(t, y) = exp_envelope(44100, 3)

plot((t, [y]),
	 size = (1024, 640),
	 display = true
	)
	;

