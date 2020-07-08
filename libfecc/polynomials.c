#include "polynomials.h"

inline poly_s poly_new(short size)
{
	poly_s poly = {0};
	size_t size_allocated	= ALLOCATE_ALIGNMENT(size);
	poly.memory	= aligned_alloc(MEMORY_ALIGNMENT, size_allocated);
	memset(poly.memory, 0, size_allocated);
	poly.size	= size;
	poly.max_size	= size_allocated;
	return (poly);
}

// The size of memory allocated for the result should atleast be of the size
// poly_a.size + poly_b.size - 1, and it must be ZEROED
inline void poly_multiply(ff_table_s table, poly_s *result
	, poly_s poly_a, poly_s poly_b)
{
	for (size_t i = 0; i < poly_a.size; i++)
	{
		for (size_t j = 0; j < poly_b.size; j++)
		{
			FF_ADDITION_INPLACE(result->memory[i + j], 
				ff_multiply_lut(table.full_table, poly_a.memory[i], poly_b.memory[j]));
		}	
	}
}

// This is using Horner's Scheme for evaluating a polynomial, 
// read more about this @ https://en.wikipedia.org/wiki/Horner%27s_method
inline ff_t poly_evaluate(ff_t *full_table, poly_s poly, short evaluate_at)
{
	ff_t eval	= poly.memory[0];

	for (size_t i = 1; i < poly.size; i++)
	{
		ff_t lookup	= ff_multiply_lut(full_table, eval, evaluate_at);
		eval	= FF_ADDITION(lookup, *(poly.memory + i));
	}
	return (eval);
}

/**
 * For polynomials size under 16 degrees, poly_evaluate_sse **MUST NOT** be used
 * for two reasons, first the performance tanks when the polynomials are not large
 * enough, and second is that the evaluation result will be incorrect. 
 * The extra setup required for evaluating using SSE is usable at degree 36. 
 */
__attribute__((optimize("unroll-loops")))
inline ff_t poly_evaluate_sse(ff_table_s *table, poly_s poly, ff_t root)
{
	ff_t root_powers[SIMD_VECTOR_SIZE]	= {0};
	for (long i = 0; i < SIMD_VECTOR_SIZE; i++)
	{
		root_powers[i]	= ff_raise2_lut(*table, root * i);
	}

	__m128i eval_results	= _mm_load_si128((__m128i*)(poly.memory));
	ff_t mult_x	= ff_raise2_lut(*table, root * SIMD_VECTOR_SIZE);

	for (size_t i = SIMD_VECTOR_SIZE; i < poly.size; i += SIMD_VECTOR_SIZE)
	{
		eval_results	= ff_multiply_lut_sse(*table, eval_results, mult_x);
		__m128i temp_vector	= _mm_load_si128((__m128i*)(poly.memory + i));
		eval_results	= _mm_xor_si128(eval_results, temp_vector);
	}

	ff_t *eval_results_v	= (ff_t*)&eval_results;
	for (size_t i = 0, j = SIMD_VECTOR_SIZE - i - 1; i < SIMD_VECTOR_SIZE; i++, j--)
	{
		root_powers[i]	= ff_multiply_lut(table->full_table, eval_results_v[j], root_powers[i]);
	}

	ff_t eval_value	= 0;
	long int v1	= 0;
	long int *v2	= (long int*) root_powers;
	for (size_t i = 0; i < SIMD_VECTOR_SIZE / sizeof(v1); i++)
	{
		FF_ADDITION_INPLACE(v1, v2[i]);
	}
	
	for (size_t i = 0; i < sizeof(v1); i++)
	{
		FF_ADDITION_INPLACE(eval_value, ((ff_t*)(&v1))[i]);
	}
	return (eval_value);
}

// This function will return the length of the data written, but result must have 
// enough storage space for MAX(poly_a, poly_b) and the memory must be ZEROED
inline size_t poly_add(ff_table_s table, poly_s *result, poly_s poly_a, poly_s poly_b)
{
	size_t result_length =  MAX(poly_a.size, poly_b.size);

	// since memory is zeroed, all we are doing is we are right aligning the poly
	// to the memory allocated for result, and then we will just do the add. 
	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	
	for (size_t i = 0; i < poly_b.size; i++)
	{
		FF_ADDITION_INPLACE(result->memory[i + result_length - poly_b.size]
			, poly_b.memory[i]);
	}
}

// Requirement for poly add sse is that the result, poly_a and poly_b must be aligned
// on the 16 byte boundary and the size must also be a multiple of 16 bytes.  
/*
inline size_t poly_add_sse(ff_table_s table, poly_s *result, poly_s poly_a, poly_s poly_b)
{
	size_t result_length =  MAX(poly_a.size, poly_b.size);
	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	
	size_t size_diff	= result_length - poly_b.size;
	for (size_t i = 0; i < poly_b.size; i+=MEMORY_ALIGNMENT)
	{
		__m128i *mem_ptr	= (__m128i*)(result->memory + i + size_diff);
		__m128i v1	= _mm_load_si128((__m128i*)(poly_b.memory));
		__m128i v2	= _mm_load_si128(mem_ptr);
		*mem_ptr	= _mm_xor_si128(v1, v2);
	}
}
*/

inline void poly_append(poly_s poly_a, poly_s poly_b)
{
	size_t memcpy_size	= MIN(poly_a.size, poly_b.size);
	memcpy(poly_a.memory + poly_a.size - memcpy_size, poly_b.memory, memcpy_size);
}

void poly_multiply_scalar(ff_table_s table, poly_s *poly, ff_t scalar)
{
	for (size_t i = 0; i < poly->size; i++)
	{
		poly->memory[i]	= ff_multiply_lut(table.full_table, poly->memory[i], scalar);
	}
}

inline poly_s poly_free(poly_s poly)
{
	free(poly.memory);
}

void poly_print(char *string, poly_s poly)
{
	printf("{ poly name: \"%s\", values: { ", string);
	for (size_t i = 0; i < poly.size; i++)
	{
		printf("%d, ", poly.memory[i]);
	}
	puts("}}");
}

void poly_copy(poly_s *dest, poly_s *source)
{
	memcpy(dest->memory, source->memory, source->size);
	dest->size	= source->size;
}

poly_s poly_make_copy(poly_s poly)
{
	poly_s copy	= poly_new(poly.size);
	memcpy(copy.memory, poly.memory, sizeof(ff_t) * poly.size);
	return (copy);
}

poly_s ff_polynomial_mod(ff_table_s table, poly_s dividend, poly_s divisor)
{
	long diff	= dividend.size - (divisor.size - 1);
	poly_s dividend_copy	= poly_make_copy(dividend);
	poly_s remainder	= poly_new(divisor.size - 1);

	for (size_t i = 0; i < diff; i++)
	{
		ff_t coef	= dividend_copy.memory[i];
		if (coef != 0)
		{
			for (size_t j = 1; j < divisor.size; j++)
			{
				FF_SUBSTRACTION_INPLACE(dividend_copy.memory[i + j]
					, ff_multiply_lut(table.full_table, divisor.memory[j], coef));
			}
		}
	}

	ff_t *remainder_address	= dividend_copy.memory
		 + dividend_copy.size - remainder.size;
	memcpy(remainder.memory, remainder_address, remainder.size);
	poly_free(dividend_copy);
	return (remainder);
}

void poly_multiply_scalar_sse(ff_table_s table, poly_s *poly, ff_t scalar)
{
	for (size_t i = 0; i < poly->size; i+=SIMD_VECTOR_SIZE)
	{
		__m128i filter = _mm_load_si128((__m128i*)((char*)poly->memory + i));
		__m128i temp = ff_multiply_lut_sse(table, filter, scalar);
		*(__m128i*)((char*)poly->memory + i) = temp;
	}
}