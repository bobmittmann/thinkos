#!julia

function psi2pascal(x::Number)
	return x * 6894.757
end

function pascal2psi(x::Number)
	return x / 6894.757
end

function inch2meter(x::Float64)
	return x * 0.0254
end

function galon2cubicmeter(x::Float64)
	return x * 0.00454609
end

function ms2kmh(x::Float64)
	return x * 3.6
end

function main()

	if (isinteractive())
	else
	end

# Tank pressure (pascals)
	t_p = psi2pascal(150)
# Tank volume
	t_v = galon2cubicmeter(2.1)

# Barrel diameter (meters)
	b_d = inch2meter(3/4)
# Barrel length (meters)
	b_l = inch2meter(12*3.0)
# Barrel volume (meters^2)
	b_v = b_d * b_l

# Initial Barrel pressure (pascals)
	p0 = t_p

# Rocket diameter (meters)
	r_d = b_d
# Rocket length (meters)
	r_l = 0.5
# Rocket area (meters^2)
	r_a = π * (r_d / 2)^2
# Rocket mass (kg)
	r_m = 0.1

# Pressure = Force / Area
	f0 = p0 * r_a;
# Force = Mass * Acceleration
	g = 9.8
	a0 = (f0 / r_m) - g

# a = dv/dt
# a --> constant
# dv = v1 - v0
# v(t) = a*dt + v(0)
# x(t) = x(0) + v(t) * dt
# dt = t1 - t0
# x(t1) = x(0) + v(t) * (t1 - t0)
# x(t) = x(0) + 1/2 * a * t^2 + v0*dt
# x(t) = x(0) + a * t^2 + v0
# v(1)^2 = v(0)^2 + 2 * a * dx

    v1 = sqrt(2* a0 * r_l)


	@printf("Diameter=%.3f [m], Length=%.3f [m]\n", r_d, r_l) 
	@printf("Area=%.5f [m^2]\n", r_a)
	@printf("Mass=%.3f [kg]\n", r_m)
	@printf("Tank Pressure=%.2f [kPa] (%.2f psi)\n", p0/1000, pascal2psi(p0))
	@printf("Initial Pressure=%.2f [kPa]\n", p0/1000)
	@printf("Initial Force=%.2f [kN]\n", f0/1000)
	@printf("Initial Acceleration=%.6f [m/s^2]\n", a0)
	@printf("Final Velocity=%.6f [m/s] , %3f [km/h]\n", v1, ms2kmh(v1))
end

main()

