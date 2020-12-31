#include "finite-fields.h"

FECC_INLINE
ff_t
ff_multiply (ff_t a, ff_t b, short irr_p)
{
    /*
     * We will be using Russian Peasant multiplication method
     * https://en.wikipedia.org/wiki/Ancient_Egyptian_multiplication#Russian_peasant_multiplication
     * The reason we are using this method, is because it allows for us to work
     * in the FF(256)
     */

    ff_t result = 0;
    while (a && b)
    {
        if (b & 1)
        {
            result = FF_ADDITION (result, a);
        }

        if (a & 0x80)
        {
            a = FF_SUBSTRACTION (a << 1, irr_p);
        }
        else
        {
            a = a << 1;
        }

        b = b >> 1;
    }

    return (result);
}

/**
 * For us to be able to SSE for multiplication, we will be using SSE shuffle
 * instruction for which the maximum table size has to be sizeof(__m128i).
 * So we instead break down the multiplication into two parts for 8 bit vectors.
 * First we shuffle the lower 4 bit half, and then we shuffle the upper 4 bit
 * half, and then we do an add which is basically a XOR in finite fields.
 * General mathematical proof:
 * a * b = c, a1a2 * b = c = a1*b + a2*b = c, here a1 = a & 0xF0, a2 = a & 0x0F
 * example: 11 * 9 = 99, 10 * 9 + 01 * 9 = 90 + 9 = 99
 */
FECC_INLINE
void
ff_table_ssemap (ff_t *memory, ff_t val, short irr_p)
{
    for (size_t i = 0; i < SIMD_VECTOR_SIZE; i++)
    {
        memory[i] = ff_multiply (val, BYTE_MASKL (i), irr_p);
        memory[i + SIMD_VECTOR_SIZE] = ff_multiply (val, BYTE_MASKH (i), irr_p);
    }
}

/**
 * Since we are going to be targetting for speed over memory usage,
 * here we try to perform as much pre-compute as required since the data
 * sets we are going to be working on are expected to be huge.
 * And the precompute only needs to be done once the library is setting up.
 */
ff_table_s
ff_table_new (short irr_p)
{
    ff_table_s table = {0};
    size_t total_allocation = (FF_SIZE * FF_SIZE)                 // For multiply table
                              + (FF_SIZE * 2)                     // For Logs and exponents
                              + (FF_SIZE * SIMD_VECTOR_SIZE * 2); // For sse map

    size_t memory_index = 0;
    table.buffer_alloc
      = aligned_alloc (MEMORY_ALIGNMENT, ALLOCATE_ALIGNMENT (total_allocation * sizeof (ff_t)));

    // We only do one big memory allocation and distribute it across
    // different tables here.
    TABLE_INDEX (table.multiply_table, table.buffer_alloc, memory_index, FF_SIZE * FF_SIZE);
    TABLE_INDEX (table.logs, table.buffer_alloc, memory_index, FF_SIZE);
    TABLE_INDEX (table.exponents, table.buffer_alloc, memory_index, FF_SIZE);
    TABLE_INDEX (table.ssemap, table.buffer_alloc, memory_index, (FF_SIZE * SIMD_VECTOR_SIZE * 2));
    table.powers2 = table.exponents;

    /**
     * For more information as to why using logarithms and exponents work with
     * precomputed tables goto https://en.wikiversity.org/wiki/Reed%E2%80%93
     * Solomon_codes_for_coders#Multiplication_with_logarithms for more info-
     * rmation.
     *
     * In general when we do x^i, it is possible for us to get the result of
     * the compute. Say 2^2 = 4. But in finite fields for a given value for
     * example say (212 = 2 ^ i) cannot be solved in constant time for i.
     *
     * Now if we are going to multiply (2 ^ i) * (2 ^ j) = 2 ^ (i + j), and
     * the division will be (2 ^ i) / (2 ^ j) = 2 ^ (i - j).
     * Addition and substraction are already solved as simple XOR operation.
     */
    ff_t x = 1;
    for (size_t i = 0; i < FF_SIZE; i++)
    {
        for (size_t j = 0; j < FF_SIZE; j++)
        {
            table.multiply_table[FF_TABLE_LOOKUP (i, j)] = ff_multiply (i, j, irr_p);
        }

        ff_table_ssemap (table.ssemap + (i * SIMD_VECTOR_SIZE * 2), i, irr_p);

        table.exponents[i] = x;
        table.logs[x] = i;

        x = ff_multiply (x, 2, irr_p);
    }

    return (table);
}

void
ff_table_free (ff_table_s table)
{
    if (table.buffer_alloc)
    {
        free (table.buffer_alloc);
    }
}

/*
 * This function will not return a cannot-divide-by-zero error
 */
FECC_INLINE
ff_t
ff_divide_lut (ff_table_s table, ff_t x, ff_t y)
{
    if (y == 0 || x == 0)
    {
        return 0;
    }

    ff_t lut_index = (table.logs[x] + (255 - table.logs[y])) % 255;
    return table.exponents[lut_index];
}

FECC_INLINE
ff_t
ff_raise_lut (ff_table_s table, ff_t x, short power)
{
    ff_t log = (table.logs[x] * power) % 255;
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

FECC_INLINE
ff_t
ff_raise2_lut (ff_table_s *table, short power)
{
    return *(table->powers2 + (power % 255));
}

FECC_INLINE
ff_t
ff_inverse_lut (ff_table_s table, ff_t x)
{
    return table.exponents[255 - table.logs[x]];
}

FECC_INLINE
ff_t
ff_multiply_lut (ff_t *full_table, ff_t x, ff_t y)
{
    /*
     * https://en.wikipedia.org/wiki/Finite_field_arithmetic#Implementation_tricks
     */

    return full_table[FF_TABLE_LOOKUP (x, y)];
}

/**
 * Before using partial modified lut, full lut segmentation should be done.
 * Basically make sure that you have the full FF_SIZE lut table already
 * setup using FF_TABLE_MULT_MODIFIED for the appropriate value of x
 */
FECC_INLINE
ff_t
ff_multiply_modified_lut (ff_t *partial_table, ff_t y)
{
    return partial_table[y];
}

/**
 * Please refer to the article
 * http://web.eecs.utk.edu/~jplank/plank/papers/FAST-2013-GF.pdf
 * for more information on how this works
 */
FECC_INLINE
__m128i
ff_multiply_lut_sse (ff_t *ssemap_table, __m128i vector, ff_t scalar)
{
    /**
     * The reason we are not creating a global variable here is
     * because its much faster to execute instruction _mm_set1_epi8
     * than to do a memory read for the entire sizeof(__m128i) table.
     */

    __m128i mask1 = _mm_set1_epi8 (0x0F);
    __m128i mask2 = _mm_set1_epi8 (0xF0);

    __m128i *ssemap = (__m128i *) (ssemap_table + (scalar * SIMD_VECTOR_SIZE * 2));

    __m128i v1 = *(ssemap + 0);
    __m128i v2 = *(ssemap + 1);

    __m128i x1 = _mm_and_si128 (mask1, vector);
    __m128i x2 = _mm_srli_epi64 (_mm_and_si128 (mask2, vector), 4);

    __m128i lookup1 = _mm_shuffle_epi8 (v1, x1);
    __m128i lookup2 = _mm_shuffle_epi8 (v2, x2);

    __m128i xor = _mm_xor_si128 (lookup1, lookup2); // Horizontal addition
    return (xor);
}