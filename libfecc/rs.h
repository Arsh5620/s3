#ifndef RS_INCLUDE_GAURD
#define RS_INCLUDE_GAURD
#include <stdio.h>
#include <stdlib.h>

#include "./finite-fields.h"

ff_polynomial_s rs_make_generator_polynomial(ff_table_s table
	, short number_ecc_symbols);
ff_polynomial_s rs_encode(ff_table_s table
	, ff_polynomial_s message_poly, short number_ecc_symbols);
ff_polynomial_s rs_calculate_syndromes(ff_table_s table, ff_polynomial_s msg
	, short number_ecc_symbols);
ff_polynomial_s rs_polynomial_new(short size, ff_t def);
ff_polynomial_s rs_polynomial_append(ff_polynomial_s polynomial, ff_t monomial);
ff_polynomial_s rs_make_error_location_poly(ff_table_s table
	, ff_polynomial_s syndromes, short number_ecc_symbols);
#endif // RS_INCLUDE_GAURD