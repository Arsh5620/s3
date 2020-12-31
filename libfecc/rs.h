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

enum decode_status
{
    DECODE_STATUS_NONE,
    DECODE_STATUS_NO_ERRORS_FOUND,
    DECODE_STATUS_CHIEN_SEARCH_FAILED,
    DECODE_STATUS_BERLEKAMP_MASSEY_FAILED,
    DECODE_STATUS_ERROR_EVAL_FAILED
};

typedef struct rs_decode
{
    poly_s message_in_buffer;
    poly_s message_out_buffer;
    ff_table_s field_table;
    short field_message_in_length; // in mathematics literature for RS(n,k), this is the "n"
    short field_message_out_length;
    short field_ecc_length; // and this will be the k
    poly_s generator;       // generator for the (n-k) field
    poly_s syndromes;       // syndromes for upto the size of ecc code length + 1
    poly_s error_evaluator; // will not be bigger than syndromes
    poly_s error_locator;
    poly_s error_locator_old;
    poly_s error_locator_temp;
    poly_s error_locations;
    poly_s working_set;
    enum decode_status result;
} rs_decode_s;

rs_encode_s
rs_init_encoder (ff_t n, ff_t k, short irr_p);
rs_decode_s
rs_init_decoder (ff_t n, ff_t k, short irr_p);
void
rs_close_encoder (rs_encode_s *s);
void
rs_close_decoder (rs_decode_s *decoder);
void
rs_encode (rs_encode_s *rs_info);
void
rs_decode (rs_decode_s *decode);
poly_s
ff_polynomial_copy (poly_s poly);
void
rs_calculate_syndromes (rs_decode_s *rs_info);
void
rs_make_error_locator_poly (rs_decode_s *rs_info);
void
rs_correct_errors (rs_decode_s *rs_info);
void
rs_find_error_locations (rs_decode_s *rs_info);
void
rs_invert_poly (poly_s *dest, poly_s poly);
void
rs_drop_leading_zeroes (poly_s *poly);
void
rs_setup_poly_sse (poly_s *dest, poly_s src);
void
rs_forward_copy (poly_s dest, poly_s src);
void
rs_polynomial_append (poly_s *polynomial, ff_t monomial);
poly_s
ff_polynomial_mod (ff_table_s table, poly_s dividend, poly_s divisor);
poly_s
rs_make_generator_polynomial (ff_table_s table, short number_ecc_symbols);
ff_t
rs_calculate_delta (rs_decode_s *rs_info, short syndrome_i);
#endif // RS_INCLUDE_GAURD