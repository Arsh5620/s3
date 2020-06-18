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
void rs_print_poly(char *string, ff_polynomial_s poly);
ff_polynomial_s rs_correct_errors(ff_table_s table
	, ff_polynomial_s message_in 
	, ff_polynomial_s syndromes
	, ff_polynomial_s error_locator_poly
	, ff_polynomial_s error_positions);
ff_polynomial_s rs_find_error_locations(ff_table_s table
	, ff_polynomial_s error_locator_poly, short message_in_length);
ff_polynomial_s rs_invert_poly(ff_polynomial_s poly);
#endif // RS_INCLUDE_GAURD