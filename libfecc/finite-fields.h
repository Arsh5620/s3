#ifndef FINITE_FIELDS_INCLUDE_GAURD
#define FINITE_FIELDS_INCLUDE_GAURD
#include <stdlib.h>
#include <string.h>

/*
 * An important consideration is that here we are only working with finite
 * fields with p = 2 and m = 8, and the finite field arithmetic will be 
 * optimized for this special use case. 
 */ 

typedef unsigned char ff_t;

#define FF_SIZE	(1 << 8)

typedef struct ff_table
{
	int size;
	ff_t antilogs[FF_SIZE * 2];
	ff_t logs[FF_SIZE];
} ff_table_s;

typedef struct ff_polynomial
{
	ff_t *memory;
	long size;
} ff_polynomial_s;

ff_t ff_addition(ff_t a, ff_t b);
ff_t ff_subtraction(ff_t a, ff_t b);
ff_t ff_multiply(ff_t a, ff_t b, short irr_p);
ff_table_s ff_generate_table(short irr_p);
ff_t ff_divide_lut(ff_table_s lut, ff_t x, ff_t y, char *error);
ff_t ff_raise_lut(ff_table_s table, ff_t x, short power);
ff_polynomial_s ff_polynomial_multiply(ff_table_s table
	, ff_polynomial_s poly_a, ff_polynomial_s poly_b);
ff_t ff_multiply_lut(ff_table_s table, ff_t x, ff_t y);
ff_polynomial_s ff_append_polynomials(ff_polynomial_s poly_a
	, ff_polynomial_s poly_b);
ff_t ff_evaluate_polynomial(ff_table_s table, ff_polynomial_s poly, short x);
ff_polynomial_s ff_polynomial_multiply_scalar(ff_table_s table
	, ff_polynomial_s poly, ff_t scalar);
ff_t ff_inverse_lut(ff_table_s table, ff_t x);
ff_polynomial_s ff_polynomial_add(ff_table_s table
	, ff_polynomial_s poly_a, ff_polynomial_s poly_b);
#endif //FINITE_FIELDS_INCLUDE_GAURD