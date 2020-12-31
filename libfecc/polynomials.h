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

#define POLY_RESET(poly)                                                                           \
    {                                                                                              \
        poly->memory[0] = 1;                                                                       \
        poly->size = 1;                                                                            \
    }

poly_s
poly_new (short size);
poly_s
poly_free (poly_s poly);
void
poly_append (poly_s poly_a, poly_s poly_b);
void
poly_print (char *string, poly_s poly);
void
poly_copy (poly_s *dest, poly_s *source);
void
poly_add (poly_s *result, poly_s poly_a, poly_s poly_b);
void
poly_add_inplace (poly_s *dest, poly_s *src);
poly_s
ff_polynomial_mod (ff_table_s table, poly_s dividend, poly_s divisor);
void
ff_polynomial_trim_x (ff_table_s table, poly_s *poly, short x_degree);
poly_s
poly_multiply (ff_table_s table, poly_s poly_a, poly_s poly_b);
ff_t
poly_evaluate_sse (ff_table_s *table, poly_s poly, ff_t root_log);
ff_t
poly_evaluate_modified (ff_t *modified_multiply_table, poly_s poly);
void
poly_multiply_scalar_sse (ff_table_s table, poly_s *poly, ff_t scalar);
#endif // _POLYNOMIALS_INCLUDE_GAURD_
