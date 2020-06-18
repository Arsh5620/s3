
def gf_add(x, y):
	return x ^ y


def gf_sub(x, y):
	return x ^ y


def gf_mul(x, y, prime):
	result = 0
	while x and y:
		if y & 1:
			# Here we are doing multiplication by addition
			result = gf_add(result, x)

		if x & 0x80:
			# If we know the number is going to overflow,
			# here we will be doing division by subtraction,
			# and result will contain the remainder (mod operation)
			x = gf_sub((x << 1), prime)
		else:
			x = x << 1

		y = y >> 1

	return result


"""
How does log/anti-log works?

"""

gf_exp = [0] * 512  # anti-log
gf_log = [0] * 256  # logs


def init_tables(prime):
	global gf_exp, gf_log
	x = 1
	for i in range(0, 255):
		gf_exp[i] = x  # compute anti-log for this value and store it in a table
		gf_log[x] = i  # compute log at the same time

		# 2 is the generator for the field.
		x = gf_mul(x, 2, prime)

	for i in range(255, 512):
		gf_exp[i] = gf_exp[i - 255]

	return [gf_log, gf_exp]


def gf_mul_lut(x, y):
	"""https://en.wikipedia.org/wiki/Finite_field_arithmetic#Implementation_tricks"""
	if x == 0 or y == 0:
		return 0
	return gf_exp[gf_log[x] + gf_log[y]]


def gf_div_lut(x, y):
	if y == 0:
		raise Exception("Division by Zero error")
	if x == 0:
		return 0
	return gf_exp[(gf_log[x] + (255 - gf_log[y])) % 255]


def gf_pow_lut(x, power):
	return gf_exp[(gf_log[x] * power) % 255]


def gf_inverse_lut(x):
	return gf_exp[255 - gf_log[x]]


def gf_poly_multiply_scalar(p, x):
	r = [0] * len(p)
	for i in range(0, len(p)):
		r[i] = gf_mul_lut(p[i], x)
	return r


def gf_poly_add(p, q):

	r = [0] * max(len(p), len(q))
	for i in range(0, len(p)):
		r[i + len(r) - len(p)] = p[i]
	for i in range(0, len(q)):
		r[i + len(r) - len(q)] ^= q[i]
	return r


def gf_poly_mul(p, q):
	r = [0] * (len(p) + len(q) - 1)
	# Compute the polynomial multiplication (just like the outer product of two vectors,
	# we multiply each coefficients of p with all coefficients of q)
	for j in range(0, len(q)):
		for i in range(0, len(p)):
			r[i + j] ^= gf_mul_lut(p[i], q[j])

	return r


def rs_generator_poly(nsym):
	g = [1]
	for i in range(0, nsym):
		g = gf_poly_mul(g, [1, gf_pow_lut(2, i)])
	return g

def gf_poly_div(dividend, divisor):
    '''Fast polynomial division by using Extended Synthetic Division and optimized for GF(2^p) computations
    (doesn't work with standard polynomials outside of this galois field, see the Wikipedia article for generic algorithm).'''
    # CAUTION: this function expects polynomials to follow the opposite convention at decoding:
    # the terms must go from the biggest to lowest degree (while most other functions here expect
    # a list from lowest to biggest degree). eg: 1 + 2x + 5x^2 = [5, 2, 1], NOT [1, 2, 5]

    msg_out = list(dividend) # Copy the dividend
    #normalizer = divisor[0] # precomputing for performance
    for i in range(0, len(dividend) - (len(divisor)-1)):
        #msg_out[i] /= normalizer # for general polynomial division (when polynomials are non-monic), the usual way of using
                                  # synthetic division is to divide the divisor g(x) with its leading coefficient, but not needed here.
        coef = msg_out[i] # precaching
        if coef != 0: # log(0) is undefined, so we need to avoid that case explicitly (and it's also a good optimization).
            for j in range(1, len(divisor)): # in synthetic division, we always skip the first coefficient of the divisior,
                                              # because it's only used to normalize the dividend coefficient
                if divisor[j] != 0: # log(0) is undefined
                    msg_out[i + j] ^= gf_mul_lut(divisor[j], coef) # equivalent to the more mathematically correct
                                                               # (but xoring directly is faster): msg_out[i + j] += -divisor[j] * coef

    # The resulting msg_out contains both the quotient and the remainder, the remainder being the size of the divisor
    # (the remainder has necessarily the same degree as the divisor -- not length but degree == length-1 -- since it's
    # what we couldn't divide from the dividend), so we compute the index where this separation is, and return the quotient and remainder.
    separator = -(len(divisor)-1)
    return msg_out[:separator], msg_out[separator:]


def rs_encode_msg(msg_in, nsym):
	'''Reed-Solomon main encoding function, using polynomial division (algorithm Extended Synthetic Division)'''
	if (len(msg_in) + nsym) > 255: raise ValueError("Message is too long (%i when max is 255)" % (len(msg_in) + nsym))
	gen = rs_generator_poly(nsym)

	# Init msg_out with the values inside msg_in and pad with len(gen)-1 bytes (which is the number of ecc symbols).
	msg_out = [0] * (len(msg_in) + len(gen) - 1)
	# Initializing the Synthetic Division with the dividend (= input message polynomial)
	msg_out[:len(msg_in)] = msg_in

	# Synthetic division main loop
	for i in range(len(msg_in)):
		# Note that it's msg_out here, not msg_in. Thus, we reuse the updated value at each iteration
		# (this is how Synthetic Division works: instead of storing in a temporary register the intermediate values,
		# we directly commit them to the output).
		coef = msg_out[i]

		# log(0) is undefined, so we need to manually check for this case. There's no need to check
		# the divisor here because we know it can't be 0 since we generated it.
		if coef != 0:
			# in synthetic division, we always skip the first coefficient of the divisior, because it's only used to normalize the dividend coefficient (which is here useless since the divisor, the generator polynomial, is always monic)
			for j in range(1, len(gen)):
				msg_out[i + j] ^= gf_mul_lut(gen[j], coef)  # equivalent to msg_out[i+j] += gf_mul(gen[j], coef)

	# At this point, the Extended Synthetic Divison is done, msg_out contains the quotient in msg_out[:len(msg_in)]
	# and the remainder in msg_out[len(msg_in):]. Here for RS encoding, we don't need the quotient but only the remainder
	# (which represents the RS code), so we can just overwrite the quotient with the input message, so that we get
	# our complete codeword composed of the message + code.
	msg_out[:len(msg_in)] = msg_in

	return msg_out


def gf_poly_eval(p, x):
	y = p[0]
	for i in range(1, len(p)):
		y = gf_mul_lut(y, x) ^ p[i]
	return y

# basically, here is all calculate syndromes work
# Take the message, This is regular message appended with the RS codes
# for an ecc number 2, where we can correct at most one error, it would be :
# {{ msg: 0x40 0xd2 0x75 0x47 0x76 0x17 0x32 0x6 0x27 0x26 0x96 0xc6 0xc6 0x96
# 0x70 0xec}, { RS: 0x14 0x7c}}
# Now syndromes calculation means that we are trying to find if any of the
# data is corrupted, and if it is then what is the information about the corruption
# How do we confirm if the data is corrupted?
# So if the ECC number was 2 then we have two syndromes to find
# and for each syndrome we find them by synd[i] = gf_poly_eval(msg, gf_pow_lut(2, i))
# in mathematical sense s(i) = GFPE(m, 2^i), here i is the syndrome position and m
# is the entire message.
# GFPE is a function to find the syndrome for a given message and i
# GFPE is defined as
# 	y = p[0]
# 	for i in range(1, len(p)):
# 		y = gf_mul_lut(y, x) ^ p[i]
# 	return y
# in more mathematical sense
# y = p[0] + (p[0] * 2 ^ i + p[1]) + ((p[0] * 2 ^ i + p[1]) * 2 ^ i + p[2]) +
# 		(((p[0] * 2 ^ i + p[1]) * 2 ^ i + p[2]) * 2 ^ i + p[3]) ....
#
def rs_calc_syndromes(msg, nsym):
	'''Given the received codeword msg and the number of error correcting symbols (nsym), computes the syndromes polynomial.
    Mathematically, it's essentially equivalent to a Fourrier Transform (Chien search being the inverse).
    '''
	# Note the "[0] +" : we add a 0 coefficient for the lowest degree (the constant). This effectively shifts the syndrome, and will shift every computations depending on the syndromes (such as the errors locator polynomial, errors evaluator polynomial, etc. but not the errors positions).
	# This is not necessary, you can adapt subsequent computations to start from 0 instead of skipping the first iteration (ie, the often seen range(1, n-k+1)),
	synd = [0] * nsym
	for i in range(0, nsym):
		synd[i] = gf_poly_eval(msg, gf_pow_lut(2, i))
	return [0] + synd

def rs_find_errata_locator(e_pos):
    '''Compute the erasures/errors/errata locator polynomial from the erasures/errors/errata positions
       (the positions must be relative to the x coefficient, eg: "hello worldxxxxxxxxx" is tampered to "h_ll_ worldxxxxxxxxx"
       with xxxxxxxxx being the ecc of length n-k=9, here the string positions are [1, 4], but the coefficients are reversed
       since the ecc characters are placed as the first coefficients of the polynomial, thus the coefficients of the
       erased characters are n-1 - [1, 4] = [18, 15] = erasures_loc to be specified as an argument.'''

    e_loc = [1] # just to init because we will multiply, so it must be 1 so that the multiplication starts correctly without nulling any term
    # erasures_loc = product(1 - x*alpha**i) for i in erasures_pos and where alpha is the alpha chosen to evaluate polynomials.
    for i in e_pos:
        e_loc = gf_poly_mul( e_loc, gf_poly_add([1], [gf_pow(2, i), 0]) )
    return e_loc
