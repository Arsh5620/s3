import galios

prime = 0x11d
galios.init_tables(prime)


def rs_print_poly(string, poly):
	print("poly: " + string)
	for ii in range(0, len(poly)):
		print(str(poly[ii]))
	print("")


def rs_find_error_locator(synd, nsym):
	err_loc = [1]
	old_loc = [1]

	for i in range(0, nsym):
		delta = synd[i]
		for j in range(1, len(err_loc)):
			delta ^= galios.gf_mul_lut(err_loc[-(j + 1)], synd[i - j])
		# delta = S_{i + v} + Λ1 S_{i + v − 1} + .... + Λ v S_{i} = 0 for i = 0, .... , 2t-v-1
		# d\gets S_{k}+C_{1}S_{k-1}+\cdots +C_{L}S_{k-L}.
		old_loc = old_loc + [0]

		if delta != 0:
			if len(old_loc) > len(err_loc):
				new_loc = galios.gf_poly_multiply_scalar(old_loc, delta)
				old_loc = galios.gf_poly_multiply_scalar(err_loc, galios.gf_inverse_lut(delta))
				err_loc = new_loc
			scalar = galios.gf_poly_multiply_scalar(old_loc, delta)
			err_loc = galios.gf_poly_add(err_loc, scalar)
	# Let C(x) be an instance of Λ(x), B(x) is C(x) for last iteration.
	# C(x) is defined as C(x) = (1 + C1x)(1 + C2x) ... (1 + Cvx)
	# C(x)=C_{L}x^{L}+C_{L-1}x^{L-1}+\cdots +C_{2}x^{2}+C_{1}x+1
	# then after we have calculated delta,
	# C(x) is initialized to 1x^0
	# L is the current number of assumed errors, and initialized to zero.
	# N is the total number of syndromes.
	# n is used as the main iterator and to index the syndromes from 0 to N−1.
	# B(x) is a copy of the last C(x) since L was updated and initialized to 1.
	# b is a copy of the last discrepancy d since L was updated and initialized to 1.
	# m is the number of iterations since L, B(x), and b were updated and initialized to 1.
	# **
	# C(x)\gets C(x)-(d/b)x^{m}B(x).
	# Here if we were to calculate C(x) after it is adjusted the delta should be zero.

	while len(err_loc) and err_loc[0] == 0:
		del err_loc[0]
	errs = len(err_loc) - 1
	if errs * 2 > nsym:
		raise Exception("Too many errors to correct")
	return err_loc

def rs_find_errors(err_loc, nmess):  # nmess is len(msg_in)
	'''Find the roots (ie, where evaluation = zero) of error polynomial by brute-force trial, this is a sort of Chien's search
    (but less efficient, Chien's search is a way to evaluate the polynomial such that each evaluation only takes constant time).'''
	errs = len(err_loc) - 1
	err_pos = []
	for i in range(
			nmess):  # normally we should try all 2^8 possible values, but here we optimize to just check the interesting symbols
		if galios.gf_poly_eval(err_loc, galios.gf_pow_lut(2,
														  i)) == 0:  # It's a 0? Bingo, it's a root of the error locator polynomial,
			# in other terms this is the location of an error
			err_pos.append(nmess - 1 - i)
	# Sanity check: the number of errors/errata positions found should be exactly the same as the length of the errata locator polynomial
	if len(err_pos) != errs:
		# couldn't find error locations
		raise Exception("Too many (or few) errors found by Chien Search for the errata locator polynomial!")
	return err_pos


def rs_find_errata_locator(e_pos):
	'''Compute the erasures/errors/errata locator polynomial from the erasures/errors/errata positions
       (the positions must be relative to the x coefficient, eg: "hello worldxxxxxxxxx" is tampered to "h_ll_ worldxxxxxxxxx"
       with xxxxxxxxx being the ecc of length n-k=9, here the string positions are [1, 4], but the coefficients are reversed
       since the ecc characters are placed as the first coefficients of the polynomial, thus the coefficients of the
       erased characters are n-1 - [1, 4] = [18, 15] = erasures_loc to be specified as an argument.'''

	e_loc = [
		1]  # just to init because we will multiply, so it must be 1 so that the multiplication starts correctly without nulling any term
	# erasures_loc = product(1 - x*alpha**i) for i in erasures_pos and where alpha is the alpha chosen to evaluate polynomials.
	for i in e_pos:
		e_loc = galios.gf_poly_mul(e_loc, galios.gf_poly_add([1], [galios.gf_pow_lut(2, i), 0]))
	return e_loc


def  rs_find_error_evaluator(synd, err_loc, nsym):
	'''Compute the error (or erasures if you supply sigma=erasures locator polynomial, or errata) evaluator polynomial Omega
       from the syndrome and the error/erasures/errata locator Sigma.'''

	# Omega(x) = [ Synd(x) * Error_loc(x) ] mod x^(n-k+1)
	_, remainder = galios.gf_poly_div(galios.gf_poly_mul(synd, err_loc),
							   ([1] + [0] * (nsym + 1)))  # first multiply syndromes * errata_locator, then do a
	# polynomial division to truncate the polynomial to the
	# required length

	# Faster way that is equivalent
	# remainder = gf_poly_mul(synd, err_loc) # first multiply the syndromes with the errata locator polynomial
	# remainder = remainder[len(remainder)-(nsym+1):] # then slice the list to truncate it (which represents the polynomial), which
	# is equivalent to dividing by a polynomial of the length we want

	return remainder


def rs_correct_errata(msg_in, synd, err_pos):  # err_pos is a list of the positions of the errors/erasures/errata
	'''Forney algorithm, computes the values (error magnitude) to correct the input message.'''
	# calculate errata locator polynomial to correct both errors and erasures (by combining the errors positions given by the error locator polynomial found by BM with the erasures positions given by caller)
	coef_pos = [len(msg_in) - 1 - p for p in
				err_pos]  # need to convert the positions to coefficients degrees for the errata locator algo to work (eg: instead of [0, 1, 2] it will become [len(msg)-1, len(msg)-2, len(msg) -3])
	err_loc = rs_find_errata_locator(coef_pos)
	# calculate errata evaluator polynomial (often called Omega or Gamma in academic papers)
	err_eval = rs_find_error_evaluator(synd[::-1], err_loc, len(err_loc) - 1)[::-1]

	# Second part of Chien search to get the error location polynomial X from the error positions in err_pos (the roots of the error locator polynomial, ie, where it evaluates to 0)
	X = []  # will store the position of the errors

	for i in range(0, len(coef_pos)):
		l = 255 - coef_pos[i]
		X.append(galios.gf_pow_lut(2, -l))

	# Forney algorithm: compute the magnitudes
	E = [0] * (len(
		msg_in))  # will store the values that need to be corrected (substracted) to the message containing errors. This is sometimes called the error magnitude polynomial.
	Xlength = len(X)
	for i, Xi in enumerate(X):

		Xi_inv = galios.gf_inverse_lut(Xi)

		# Compute the formal derivative of the error locator polynomial (see Blahut, Algebraic codes for data transmission, pp 196-197).
		# the formal derivative of the errata locator is used as the denominator of the Forney Algorithm, which simply says that the ith error value is given by error_evaluator(gf_inverse(Xi)) / error_locator_derivative(gf_inverse(Xi)). See Blahut, Algebraic codes for data transmission, pp 196-197.
		# err_loc_prime_tmp = []
		# for j in range(0, Xlength):
		# 	if j != i:
		# 		err_loc_prime_tmp.append(galios.gf_sub(1, galios.gf_mul_lut(Xi_inv, X[j])))
		# # compute the product, which is the denominator of the Forney algorithm (errata locator derivative)
		# err_loc_prime = 1
		# for coef in err_loc_prime_tmp:
		# 	err_loc_prime = galios.gf_mul_lut(err_loc_prime, coef)

		err_loc_prime = 1
		for j in range(0, Xlength):
			if j != i:
				err_loc_prime = galios.gf_mul_lut(err_loc_prime,galios.gf_sub(1, galios.gf_mul_lut(Xi_inv, X[j])))
		# compute the product, which is the denominator of the Forney algorithm (errata locator derivative)
		# equivalent to: err_loc_prime = functools.reduce(gf_mul, err_loc_prime_tmp, 1)
		rs_print_poly("err eval", err_eval)
		print("xinv is " + str(Xi_inv))

		# Compute y (evaluation of the errata evaluator polynomial)
		# This is a more faithful translation of the theoretical equation contrary to the old forney method. Here it is an exact reproduction:
		# Yl = omega(Xl.inverse()) / prod(1 - Xj*Xl.inverse()) for j in len(X)
		y = galios.gf_poly_eval(err_eval[::-1], Xi_inv)  # numerator of the Forney algorithm (errata evaluator evaluated)
		y = galios.gf_mul_lut(Xi, y)
		# Check: err_loc_prime (the divisor) should not be zero.
		if err_loc_prime == 0:
			raise Exception("Could not find error magnitude")  # Could not find error magnitude

		# Compute the magnitude
		magnitude = galios.gf_div_lut(y,
						   err_loc_prime)  # magnitude value of the error, calculated by the Forney algorithm (an equation in fact): dividing the errata evaluator with the errata locator derivative gives us the errata magnitude (ie, value to repair) the ith symbol
		E[err_pos[i]] = magnitude  # store the magnitude for this error into the magnitude polynomial

	# Apply the correction of values to get our message corrected! (note that the ecc bytes also gets corrected!)
	# (this isn't the Forney algorithm, we just apply the result of decoding here)
	msg_in = galios.gf_poly_add(msg_in,
						 E)  # equivalent to Ci = Ri - Ei where Ci is the correct message, Ri the received (senseword) message, and Ei the errata magnitudes (minus is replaced by XOR since it's equivalent in GF(2^p)). So in fact here we substract from the received message the errors magnitude, which logically corrects the value to what it should be.
	return msg_in


msg_in = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]
fecc = 8

message = galios.rs_encode_msg(msg_in, fecc)

print(str(message))
message[3] = 0
message[7] = 0
message[8] = 0

syndrome = galios.rs_calc_syndromes(message, fecc)
print(str(syndrome))
error_locator_poly = rs_find_error_locator(syndrome, fecc)
rs_print_poly("error locator", error_locator_poly)
error_poly = rs_find_errors(error_locator_poly[::-1], len(message))

actual = rs_correct_errata(message, syndrome, error_poly)

a=[0, 1, 2, 3, 4, 5]
b=[7, 8, 9, 10, 11, 12]

rs_print_poly("remainder", galios.gf_poly_div(b, a))