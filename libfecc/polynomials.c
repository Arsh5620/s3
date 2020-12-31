#include "polynomials.h"

FECC_INLINE
poly_s
poly_new (short size)
{
    poly_s poly = {0};
    size_t size_allocated = ALLOCATE_ALIGNMENT (size);
    poly.memory = aligned_alloc (MEMORY_ALIGNMENT, size_allocated);
    memset (poly.memory, 0, size_allocated);
    poly.size = size;
    poly.max_size = size_allocated;
    return (poly);
}

/**
 * This is the same polynomial multiplication taught in schools,
 * and has a runtime of O ^ 2, unless the polynomials size is more
 * than a couple thousands of degrees, there is no advantage of other
 * algorithms such as Karatsuba.
 */
FECC_INLINE
poly_s
poly_multiply (ff_table_s table, poly_s poly_a, poly_s poly_b)
{
    poly_s result = poly_new (poly_a.size + poly_b.size - 1);
    for (size_t i = 0; i < poly_a.size; i++)
    {
        ff_t *mod_table = table.multiply_table + FF_TABLE_MULT_MODIFIED (poly_a.memory[i]);
        for (size_t j = 0; j < poly_b.size; j++)
        {
            FF_ADDITION_INPLACE (
              result.memory[i + j], ff_multiply_modified_lut (mod_table, poly_b.memory[j]));
        }
    }
    return (result);
}

/**
 * Horner's Scheme for polynomial evaluation is the fastest known
 * algorithm with unknown polynomials of degree usually less than 40.
 * For polynomials with a higher level of degree Estrin's Scheme allows
 * for parallel compute of polynomials with some setup overhead.
 * See Horner's Method at https://en.wikipedia.org/wiki/Horner%27s_method
 * See Estrin's Method at https://en.wikipedia.org/wiki/Estrin%27s_scheme
 */
FECC_INLINE
ff_t
poly_evaluate_modified (ff_t *modified_multiply_table, poly_s poly)
{
    ff_t eval = poly.memory[0];

    for (size_t i = 1; i < poly.size; i++)
    {
        eval
          = FF_ADDITION (ff_multiply_modified_lut (modified_multiply_table, eval), poly.memory[i]);
    }

    return (eval);
}

/**
 * For polynomials degree under 16 this function **MUST NOT** be used.
 * Using Estrin's Scheme the setup required for parallel polynomial compute
 * is not advantageous until the polynomial degree is higher than 40.
 * And also it will yeild incorrect results for degree less than 16 and the
 * extra steps required have been left out due to optimizations.
 */
FECC_UNROLL_LOOPS FECC_INLINE ff_t
poly_evaluate_sse (ff_table_s *table, poly_s poly, ff_t root_log)
{
    /**
     * Unlike the regular poly evaluate, you might notice that the last
     * variable is named root_log. This is because we don't expect this
     * variable to be root = ff_raise_lut(table, 2, i)
     *
     * Instead we just want to receive the (i) which is the root log
     * itself, and since we are going to multiply it later anyways.
     * We will do regular multiplication and save ourselves some
     * memory accesses.
     */

    ff_t scalar_x = ff_raise2_lut (table, root_log * SIMD_VECTOR_SIZE);
    __m128i evaluation = _mm_load_si128 ((__m128i *) (poly.memory));

    /**
     * We would expect and assume for the polynomial to be aligned on
     * sizeof(__m128i) memory address with the size also aligned at the same
     */
    for (size_t i = SIMD_VECTOR_SIZE; i < poly.size; i += SIMD_VECTOR_SIZE)
    {
        evaluation = ff_multiply_lut_sse (table->ssemap, evaluation, scalar_x);
        evaluation = _mm_xor_si128 (evaluation, *((__m128i *) (poly.memory + i)));
    }

    ff_t eval_value = 0;
    ff_t *evaluation_p = (ff_t *) &evaluation;
    for (size_t i = 0, j = SIMD_VECTOR_SIZE - 1; i < SIMD_VECTOR_SIZE; i++, j--)
    {
        FF_ADDITION_INPLACE (
          eval_value,
          ff_multiply_lut (
            table->multiply_table, evaluation_p[j], ff_raise2_lut (table, root_log * i)));
    }

    return (eval_value);
}

/**
 * Simple elementary level polynomial addition.
 * When optimizing GCC can automatically vectorize the code.
 */
FECC_INLINE
void
poly_add (poly_s *result, poly_s poly_a, poly_s poly_b)
{
    size_t result_length = MAX (poly_a.size, poly_b.size);

    // Copy the poly_a to the result.
    memcpy (result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);

    // Do an inplace addition in finite field.
    for (size_t i = 0; i < poly_b.size; i++)
    {
        FF_ADDITION_INPLACE (result->memory[i + result_length - poly_b.size], poly_b.memory[i]);
    }
}

FECC_INLINE
void
poly_add_inplace (poly_s *dest, poly_s *src)
{
    ff_t *memory_dest = dest->memory + (dest->size - src->size);
    // Do an inplace addition in finite field.
    for (size_t i = 0; i < src->size; i++)
    {
        FF_ADDITION_INPLACE (memory_dest[i], src->memory[i]);
    }
}

FECC_INLINE
void
poly_append (poly_s poly_a, poly_s poly_b)
{
    size_t memcpy_size = MIN (poly_a.size, poly_b.size);

    // Just copy poly_b to the end of poly_a
    memcpy (poly_a.memory + poly_a.size - memcpy_size, poly_b.memory, memcpy_size);
}

FECC_INLINE
poly_s
poly_free (poly_s poly)
{
    free (poly.memory);
}

void
poly_print (char *string, poly_s poly)
{
    printf ("{ poly name: \"%s\", values: { ", string);
    for (size_t i = 0; i < poly.size; i++)
    {
        printf ("%d, ", poly.memory[i]);
    }
    puts ("}}");
}

FECC_INLINE
void
poly_copy (poly_s *dest, poly_s *source)
{
    memcpy (dest->memory, source->memory, source->size);
    dest->size = source->size;
}

/**
 * TODO
 */
FECC_UNROLL_LOOPS FECC_INLINE poly_s
ff_polynomial_mod (ff_table_s table, poly_s dividend, poly_s divisor)
{
    long diff = dividend.size - (divisor.size - 1);
    poly_s dividend_copy = poly_new (dividend.size);
    poly_copy (&dividend_copy, &dividend);

    poly_s remainder = poly_new (divisor.size - 1);

    for (size_t i = 0; i < diff; i++)
    {
        ff_t coef = dividend_copy.memory[i];
        if (coef != 0)
        {
            for (size_t j = 1; j < divisor.size; j++)
            {
                FF_SUBSTRACTION_INPLACE (
                  dividend_copy.memory[i + j],
                  ff_multiply_lut (table.multiply_table, divisor.memory[j], coef));
            }
        }
    }

    ff_t *remainder_address = dividend_copy.memory + dividend_copy.size - remainder.size;
    memcpy (remainder.memory, remainder_address, remainder.size);
    poly_free (dividend_copy);
    return (remainder);
}

FECC_INLINE
void
ff_polynomial_trim_x (ff_table_s table, poly_s *poly, short x_degree)
{
    memmove (poly->memory, poly->memory + (poly->size - x_degree), x_degree);
    poly->size = x_degree;
}

/**
 * Load Multiply Store using SSE shuffle multiply.
 */
void
poly_multiply_scalar_sse (ff_table_s table, poly_s *poly, ff_t scalar)
{
    for (size_t i = 0; i < poly->size; i += SIMD_VECTOR_SIZE)
    {
        *(__m128i *) (poly->memory + i)
          = ff_multiply_lut_sse (table.ssemap, *(__m128i *) (poly->memory + i), scalar);
    }
}