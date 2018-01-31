N_SIN = 128

function mk_q31sin(fname, n::Int64)
	f = open(fname, "w")

	@printf(f, "#include <fixpt.h>\n")
	@printf(f, "\n")

	@printf(f, "#define SIN_N %d\n\n", n)
	@printf(f, "const int32_t sintab[] = {\n")

	for i in 0:n+1
		θ = π*i/n
		x = sin(θ)
		@printf(f, "\tQ31_SAT(Q31(%12.9f)), /* %3d - θ = %9.6f*π */\n", x, i, i/n)
	end
	@printf(f, "};\n")

	@printf(f, "\n")

	@printf(f, "int32_t q31sin(int32_t x)\n")
	@printf(f, "{\n")
	@printf(f, "\tint32_t y321;\n")
	@printf(f, "\tint32_t y32;\n")
	@printf(f, "\tint32_t y21;\n")
	@printf(f, "\tint32_t y3;\n")
	@printf(f, "\tint32_t y2;\n")
	@printf(f, "\tint32_t y1;\n")
	@printf(f, "\tint32_t y;\n")
	@printf(f, "\tint32_t x1;\n")
	@printf(f, "\tint32_t dx1;\n")
	@printf(f, "\tint32_t dx2;\n")
	@printf(f, "\tint32_t i;\n")

#	θ = π/2*i/N_SIN
#	i = θ * 2/π * N_SIN

	@printf(f, "\ti = ((int64_t)x * SIN_N) >> 31;\n")
	@printf(f, "\ty1 = sintab[i];\n")
	@printf(f, "\ty2 = sintab[i + 1];\n")
	@printf(f, "\ty3 = sintab[i + 2];\n")
	@printf(f, "\ty21 = (y2 - y1) * 2;\n")
	@printf(f, "\ty32 = (y3 - y2) * 2;\n")
	@printf(f, "\ty321 = y32 - y21;\n")
	@printf(f, "\tx1 = ((int64_t)i << 31) / SIN_N;\n")
	@printf(f, "\tdx1 = (x - x1) * (SIN_N / 2);\n")
	@printf(f, "\tdx2 = dx1 + Q31(-1.0);\n")
	@printf(f, "\ty = y1 + Q31_MUL(y21, dx1) + Q31_MUL(y321, Q31_MUL(dx1, dx2));\n")
	@printf(f, "\n")
	@printf(f, "\treturn y;\n")
	@printf(f, "}\n")

	close(f)
end

# Conversion form float to fixed point Q1.31 
#define Q31(F) ((int32_t)(double)(F) * (double)(1LL << 32))
function Q31(x::Float64)
	y::Int64 = 0
	y = Int(round(x * convert(Float64, (1 << 31))))
	if (y >= (1 << 31))
		y = (1 << 31) - 1
	elseif (y < (-1 * (1 << 31)))
		y = -1 * (1 << 31)
	end
	return convert(Int32, y)
end

function Q31F(x::Int32)
	y::Float64 = 0
	y = convert(Float64, x) * (1.0 / (1 << 31))
	return y
end

function Q31MUL(x::Int32, y::Int32)
	xx::Int64 = x
	yy::Int64 = y
	z::Int64 = 0

	z = (xx * yy + (1 << 30)) >> 31
	if (z >= (1 << 31))
		z = (1 << 31) - 1
	elseif (z < (-1 * (1 << 31)))
		z = -1 * (1 << 31)
	end
	return convert(Int32, z)
end

sintab = zeros(Int32, N_SIN+2)
for i in 0:N_SIN+1
	θ = π*i/N_SIN
	sintab[i + 1] = Q31(sin(θ))
end

function q31sin(x::Int32)
	y321::Int32 = 0
	y32::Int32 = 0
	y21::Int32 = 0
	y2::Int32 = 0
	y1::Int32 = 0
	y::Int32 = 0
	x1::Int32 = 0
	x2::Int32 = 0
	dx1::Int32 = 0
	dx2::Int32 = 0
	i::Int32 = 0
	n::Int32 = N_SIN

	i = (convert(Int64, x) * N_SIN) >> 31
	y1 = sintab[i + 1]
	y2 = sintab[i + 2]
	y3 = sintab[i + 3]
	y21 = (y2 - y1) * 2
	y32 = (y3 - y2) * 2
	y321 = y32 - y21
#	println("i=", i)

	x1 = convert(Int32, ((convert(Int64, (i)) << 31) / N_SIN))
	x2 = x1 + Q31(1.0)
	dx1 = convert(Int32, (x - x1) * (n/2))
	dx2 = dx1 + Q31(-1.0)
	println(" -> dx1=", Q31F(dx1), " dx2=", Q31F(dx2))
	y = y1 + Q31MUL(y21, dx1) + Q31MUL(y321, Q31MUL(dx1, dx2))

	return y
end	

