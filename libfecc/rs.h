#ifndef RS_INCLUDE_GAURD
#define RS_INCLUDE_GAURD
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "./polynomials.h"

typedef struct rs_encode
{
	poly_s message_in_buffer;
	poly_s message_out_buffer;
	ff_table_s field_table;
	short field_message_in_length;
	short field_message_out_length;
	short field_ecc_length;	
	poly_s generator;
} rs_encode_s;

typedef struct rs_decode
{
	poly_s message_in_buffer;
	poly_s message_out_buffer;
	ff_table_s field_table;
	short field_message_in_length;	// in mathematics literature for RS(n,k), this is the "n"
	short field_message_out_length;
	short field_ecc_length;	// and this will be the k
	poly_s generator;	// generator for the (n-k) field
	poly_s syndromes;	// syndromes for upto the size of ecc code length + 1
	poly_s error_evaluator;	// will not be bigger than syndromes
	poly_s error_locator;
	poly_s error_locator_old;
	poly_s error_locator_temp;
	poly_s error_locations;
} rs_decode_s;

rs_encode_s rs_init_encoder(ff_t n, ff_t k, short irr_p);
void rs_close_encoder(rs_encode_s *s);
rs_decode_s rs_init_decoder(ff_t n, ff_t k, short irr_p);
void rs_close_decoder(rs_decode_s *decoder);
poly_s rs_make_generator_polynomial(ff_table_s table
	, short number_ecc_symbols);
void rs_encode(rs_encode_s *rs_info);
poly_s ff_polynomial_copy(poly_s poly);
void rs_calculate_syndromes(rs_decode_s *rs_info);
void rs_polynomial_append(poly_s *polynomial, ff_t monomial);
void rs_make_error_location_poly(rs_decode_s *rs_info);
void rs_correct_errors(rs_decode_s *rs_info);
void rs_find_error_locations(rs_decode_s *rs_info);
poly_s rs_invert_poly(poly_s poly);
poly_s ff_polynomial_mod(ff_table_s table
	, poly_s dividend, poly_s divisor);
ff_t rs_calculate_delta(rs_decode_s *rs_info , short syndrome_i);
void rs_forward_copy(poly_s dest, poly_s src);
#endif // RS_INCLUDE_GAURD