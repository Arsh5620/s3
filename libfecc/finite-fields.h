#ifndef FINITE_FIELDS_INCLUDE_GAURD
#define FINITE_FIELDS_INCLUDE_GAURD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/*
 * Please make sure to not pass a stack allocated array to a function 
 * you will be calling often as a new copy is created for each time 
 * the function is called. 
 */

/*
 * An important consideration is that here we are only working with finite
 * fields with p = 2 and m = 8, and the finite field arithmetic will be 
 * optimized for this special use case. 
 */ 

typedef unsigned char ff_t;

#define FF_SIZE	256
#define FF_SIZE_RSHIFT	8
#define FF_TABLE_LOOKUP(x, y) ((x << FF_SIZE_RSHIFT) | y)

#define SIMD_VECTOR_SIZE	sizeof(__m128)
#define MEMORY_ALIGNMENT	SIMD_VECTOR_SIZE
#define ALLOCATE_ALIGNMENT(x)	((x & MEMORY_ALIGNMENT) ? (x + 16) & ~(MEMORY_ALIGNMENT - 1): x)
#define SIMD_MULTIPLY_COMBINE(high, low) (high ^ low)
// byte mask high
#define BYTE_MH(x) (x << 4) & 0xF0
// byte mask low
#define BYTE_ML(x) (x & 0x0F)

typedef struct ff_table
{
	ff_t *full_table;
	ff_t *logs;
	ff_t *exponents;
	ff_t *stdmap;
	ff_t *power2;
} ff_table_s;

/*
 * In Finite Fields addition for p = 2, m = 8 is simple XOR
 */
#define FF_ADDITION(a, b) (a ^ b)
#define FF_ADDITION_INPLACE(a, b) (a ^= b)

/*
 * In Finite Fields subtraction is same as addition when p = 2, m = 8
 */
#define FF_SUBSTRACTION(a, b) (a ^ b)
#define FF_SUBSTRACTION_INPLACE(a, b) (a ^= b)

#define MAX(a, b) (a>b?a:b)
#define MIN(a, b) (a<b?a:b)

ff_t ff_multiply(ff_t a, ff_t b, short irr_p);
ff_t ff_divide_lut(ff_table_s table, ff_t x, ff_t y);
ff_t ff_raise_lut(ff_table_s table, ff_t x, short power);
ff_t ff_raise2_lut(ff_table_s table, short power);
ff_t ff_multiply_lut(ff_t *table, ff_t x, ff_t y);
__m128i ff_multiply_lut_sse(ff_table_s table, __m128i x, ff_t y);
ff_t ff_inverse_lut(ff_table_s table, ff_t x);

ff_table_s ff_table_new(short irr_p);
void ff_table_close(ff_table_s table);
#endif //FINITE_FIELDS_INCLUDE_GAURD