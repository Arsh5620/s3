#ifndef _POLYNOMIALS_INCLUDE_GAURD_
#define _POLYNOMIALS_INCLUDE_GAURD_
#include "finite-fields.h"
#include <string.h>

typedef struct poly
{
	ff_t *memory;
	long size;
	long max_size;
} poly_s;

#define POLY_RESET(poly) {poly->memory[0] = 1; poly->size = 1;}
 
poly_s poly_new(short size);
poly_s poly_free(poly_s poly);
void poly_multiply(ff_table_s table, poly_s *result
	, poly_s poly_a, poly_s poly_b);
ff_t poly_evaluate(ff_t *full_table, poly_s poly, short evaluate_at);
void poly_add(ff_table_s table, poly_s *result, poly_s poly_a, poly_s poly_b);
// size_t poly_add_sse(ff_table_s table, poly_s *result, poly_s poly_a, poly_s poly_b);
void poly_append(poly_s poly_a, poly_s poly_b);
void poly_multiply_scalar(ff_table_s table, poly_s *poly, ff_t scalar);
void poly_print(char *string, poly_s poly);
void poly_copy(poly_s *dest, poly_s *source);
poly_s poly_make_copy(poly_s poly);
poly_s ff_polynomial_mod(ff_table_s table, poly_s dividend, poly_s divisor);

ff_t poly_evaluate_sse(ff_table_s *table, poly_s poly, ff_t root_log);
void poly_multiply_scalar_sse(ff_table_s table, poly_s *poly, ff_t scalar);
#endif // _POLYNOMIALS_INCLUDE_GAURD_
