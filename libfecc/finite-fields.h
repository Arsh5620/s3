#ifndef FINITE_FIELDS_INCLUDE_GAURD
#define FINITE_FIELDS_INCLUDE_GAURD
#include <stdlib.h>
#include <string.h>

// NEVER PASS A STACK-ALLOCATED-ARRAY TO A HOT FUNCTION YOU ARE GOING TO 
// CALL A MILLION TIMES, MEMCPY WILL EAT YOU. 

/*
 * An important consideration is that here we are only working with finite
 * fields with p = 2 and m = 8, and the finite field arithmetic will be 
 * optimized for this special use case. 
 */ 

typedef unsigned char ff_t;

#define FF_SIZE	256
#define FF_TABLE_LOOKUP(x, y) ((x * FF_SIZE) + y)

typedef struct ff_table
{
	ff_t *full_table;
	ff_t *logs;
	ff_t *exponents;
} ff_table_s;

typedef struct ff_polynomial
{
	ff_t *memory;
	long size;
	long max_size;
} ff_polynomial_s;

/*
 * In Finite Fields addition for p = 2, m = 8 is simple XOR
 */
#define FF_ADDITION(a, b) (a ^ b)

/*
 * In Finite Fields subtraction is same as addition when p = 2, m = 8
 */
#define FF_SUBSTRACTION(a, b) (a ^ b)

ff_t ff_multiply(ff_t a, ff_t b, short irr_p);
ff_table_s ff_generate_table(short irr_p);
ff_t ff_divide_lut(ff_table_s table, ff_t x, ff_t y);
ff_t ff_raise_lut(ff_table_s table, ff_t x, short power);
ff_polynomial_s ff_polynomial_multiply(ff_table_s table
	, ff_polynomial_s poly_a, ff_polynomial_s poly_b);
ff_t ff_multiply_lut(ff_table_s table, ff_t x, ff_t y);
ff_polynomial_s ff_append_polynomials(ff_polynomial_s poly_a
	, ff_polynomial_s poly_b);
ff_t ff_evaluate_polynomial(ff_table_s table, ff_polynomial_s poly, short x);
void ff_polynomial_multiply_scalar(ff_table_s table
	, ff_polynomial_s *poly, ff_t scalar);
ff_t ff_inverse_lut(ff_table_s table, ff_t x);
ff_polynomial_s ff_polynomial_add(ff_table_s table
	, ff_polynomial_s poly_a, ff_polynomial_s poly_b);
ff_polynomial_s ff_polynomial_new(short size);
ff_polynomial_s ff_polynomial_free(ff_polynomial_s poly);
ff_polynomial_s ff_polynomial_extend(ff_polynomial_s poly, short extend_by);

#endif //FINITE_FIELDS_INCLUDE_GAURD