#ifndef RS_INCLUDE_GAURD
#define RS_INCLUDE_GAURD
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "./finite-fields.h"

typedef struct rs_information
{
	ff_polynomial_s message_in;
	ff_polynomial_s message_out;
	ff_table_s field_table;
	short field_message_length;	// in mathematics literature for RS(n,k), this is the "n"
	short field_ecc_length;	// and this will be the k
	ff_polynomial_s generator;	// generator for the (n-k) field
	ff_polynomial_s syndromes;	// syndromes for upto the size of ecc code length + 1
	ff_polynomial_s error_evaluator;	// cannot be bigger than syndromes
	ff_polynomial_s error_locator;
	ff_polynomial_s error_locator_old;
	ff_polynomial_s error_locator_temp;
	ff_polynomial_s error_locations;
} rs_information_s;

rs_information_s rs_init(ff_t n, ff_t k, short irr_p);
ff_polynomial_s rs_make_generator_polynomial(ff_table_s table
	, short number_ecc_symbols);
void rs_encode(rs_information_s *rs_info);
ff_polynomial_s ff_polynomial_copy(ff_polynomial_s poly);
void rs_calculate_syndromes(ff_polynomial_s *syndromes
	, ff_table_s table, ff_polynomial_s message_in, short field_ecc_length);
void rs_polynomial_append(ff_polynomial_s *polynomial, ff_t monomial);
void rs_make_error_location_poly(rs_information_s *rs_info);
void rs_print_poly(char *string, ff_polynomial_s poly);
ff_polynomial_s rs_correct_errors(ff_table_s table
	, ff_polynomial_s message_in 
	, ff_polynomial_s syndromes
	, ff_polynomial_s error_locator_poly
	, ff_polynomial_s error_positions);
ff_polynomial_s rs_find_error_locations(ff_table_s table
	, ff_polynomial_s error_locator_poly, short message_in_length);
ff_polynomial_s rs_invert_poly(ff_polynomial_s poly);
ff_polynomial_s ff_polynomial_mod(ff_table_s table
	, ff_polynomial_s dividend, ff_polynomial_s divisor);
ff_t rs_calculate_delta(rs_information_s *rs_info , short syndrome_i);
#endif // RS_INCLUDE_GAURD