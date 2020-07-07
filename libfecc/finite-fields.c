#include "finite-fields.h"

inline ff_t ff_multiply(ff_t a, ff_t b, short irr_p)
{
	/* 
	 * We will be using Russian Peasant multiplication method
	 * https://en.wikipedia.org/wiki/Ancient_Egyptian_multiplication#Russian_peasant_multiplication
	 * The reason we are using this method, is because it allows for us to work
	 * in the FF(256)
	 */

	ff_t result	= 0;
	while (a && b)
	{
		if (b & 1)
		{
			result	= FF_ADDITION(result, a);
		}

		if (a & 0x80)
		{
			a	= FF_SUBSTRACTION(a << 1, irr_p);
		}
		else 
		{
			a	= a << 1;
		}

		b = b >> 1;
	}

	return (result);
}

void ff_table_close(ff_table_s table)
{
	if (table.full_table) 
	{
		free(table.full_table);
	}

	if (table.logs)
	{
		free(table.logs);
	}

	if (table.exponents)
	{
		free(table.exponents);
	}
}

/**
 * Here we would expect for table_stdmap to atleast be of size 
 * SIMD_VECTOR_SIZE * 2
 */
void ff_table_stdmap(ff_t *memory, ff_t y, short irr_p)
{
	for (size_t i = 0; i < SIMD_VECTOR_SIZE; i++)
	{
		*(memory + i)	= ff_multiply(y, BYTE_ML(i), irr_p);
		*(memory + i + SIMD_VECTOR_SIZE)	= ff_multiply(y, BYTE_MH(i), irr_p);
	}
}

ff_table_s ff_table_new(short irr_p)
{
	ff_table_s table	= {0};
	table.full_table	= calloc(FF_SIZE * FF_SIZE, sizeof(ff_t));
	table.logs	= calloc(FF_SIZE, sizeof(ff_t));
	table.exponents	= calloc(FF_SIZE * 2, sizeof(ff_t));
	table.stdmap	= calloc(FF_SIZE * SIMD_VECTOR_SIZE * 2, sizeof(ff_t));

	ff_t x	= 1;
	for (size_t i = 0; i < FF_SIZE; i++)
	{
		for (size_t j = 0; j < FF_SIZE; j++)
		{
			table.full_table[FF_TABLE_LOOKUP(i, j)] = ff_multiply(i, j, irr_p);
		}
		ff_table_stdmap(table.stdmap + (i * SIMD_VECTOR_SIZE * 2), i, irr_p);
		
		table.exponents[i]	= x;
		table.logs[x]	= i;

		x	= ff_multiply(x, 2, irr_p);
	}

	memcpy(table.exponents + FF_SIZE - 1, table.exponents, FF_SIZE);
	
	return (table);
}

/*
 * This function will not return a cannot-divide-by-zero error
 */
ff_t ff_divide_lut(ff_table_s table, ff_t x, ff_t y)
{
	if (y == 0 || x == 0)
	{
		return 0;
	}

	ff_t lut_index	= (table.logs[x] + (255 - table.logs[y])) % 255;
	return table.exponents[lut_index];
}

ff_t ff_raise_lut(ff_table_s table, ff_t x, short power)
{
	ff_t log	= (table.logs[x] * power) % 255;
	if (power < 0)
	{
		--log;
	}
	// compared to reference implementation in python where when a number is 
	// raised to a negative power and if the number is a fraction then the 
	// final result is moved towards the lower number (away from 0), c modulus
	// will instead move that number towards a bigger number (towards 0).
	// to compensate we will have to reduce it by one when the number is negative.

	return table.exponents[log];
}

ff_t ff_inverse_lut(ff_table_s table, ff_t x)
{
	return table.exponents[255 - table.logs[x]];
}

inline __attribute__((always_inline)) ff_t ff_multiply_lut(ff_t *full_table, ff_t x, ff_t y)
{
	/*
	 * https://en.wikipedia.org/wiki/Finite_field_arithmetic#Implementation_tricks
	 */
	// if (x == 0 || y == 0)
	// {
	// 	return 0;
	// }
	return full_table[FF_TABLE_LOOKUP(x, y)];
}

inline __m128i ff_multiply_lut_sse(ff_table_s table, __m128i x, ff_t y)
{
	__m128i mask1	= _mm_set1_epi8(0x0F);
	__m128i mask2	= _mm_set1_epi8(0xF0);

	__m128i *table_stdmap	= (__m128i*)(table.stdmap + (y * (SIMD_VECTOR_SIZE * 2)));

	__m128i v1	= *table_stdmap;
	// For now extra casts just to make sure we know what we are doing.
	__m128i v2	= *(__m128i*)((char*)table_stdmap + SIMD_VECTOR_SIZE);

	__m128i x1	= _mm_and_si128(mask1, x);
	__m128i x2	= _mm_and_si128(mask2, x);
	__m128i lookup1	= _mm_shuffle_epi8(v1, x1);
	x2	= _mm_srli_epi64(x2, 4);
	__m128i lookup2	= _mm_shuffle_epi8(v2, x2);
	__m128i xor	= _mm_xor_si128(lookup1, lookup2);
	return (xor);
}