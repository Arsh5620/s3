#include "rs.h"

// Reason for using stack instead of heap is because it
// will take more time for malloc to return compared to
// using stack to setup one time structures.
rs_encode_s
rs_init_encoder (ff_t n, ff_t k, short irr_p)
{
    rs_encode_s encoder = {0};
    ff_t data_length = k, ecc_length = (n - k);
    encoder.message_in_buffer = poly_new (k);
    encoder.message_out_buffer = poly_new (n);
    encoder.field_ecc_length = ecc_length;
    encoder.field_message_in_length = k;
    encoder.field_message_out_length = n;
    encoder.field_table = ff_table_new (irr_p);
    encoder.generator = rs_make_generator_polynomial (encoder.field_table, ecc_length);
    return (encoder);
}

// must match the rs_init_encoder to free all the memory used.
void
rs_close_encoder (rs_encode_s *s)
{
    poly_free (s->message_in_buffer);
    poly_free (s->message_out_buffer);
    ff_table_free (s->field_table);
}

rs_decode_s
rs_init_decoder (ff_t n, ff_t k, short irr_p)
{
    rs_decode_s decoder = {0};
    ff_t data_length = k, ecc_length = (n - k);
    decoder.message_in_buffer = poly_new (n);
    decoder.message_out_buffer = poly_new (n);
    decoder.field_ecc_length = ecc_length;
    decoder.field_message_in_length = n;
    decoder.field_message_out_length = k;
    decoder.field_table = ff_table_new (irr_p);
    decoder.generator = rs_make_generator_polynomial (decoder.field_table, ecc_length);
    short allocation_length = ecc_length;
    decoder.syndromes = poly_new (allocation_length);
    decoder.error_evaluator = poly_new (allocation_length);
    decoder.error_locator = poly_new (allocation_length);
    decoder.error_locator_temp = poly_new (allocation_length);
    decoder.error_locator_old = poly_new (allocation_length);
    decoder.error_locations = poly_new (allocation_length);
    decoder.working_set = poly_new (n);

    return (decoder);
}

void
rs_close_decoder (rs_decode_s *decoder)
{
    poly_free (decoder->syndromes);
    poly_free (decoder->error_evaluator);
    poly_free (decoder->error_locator);
    poly_free (decoder->error_locator_temp);
    poly_free (decoder->error_locator_old);
    poly_free (decoder->error_locations);
    poly_free (decoder->message_in_buffer);
    poly_free (decoder->message_out_buffer);
    poly_free (decoder->generator);
    poly_free (decoder->working_set);
    ff_table_free (decoder->field_table);
}

poly_s
rs_make_generator_polynomial (ff_table_s table, short ecc_length)
{
    poly_s poly = poly_new (1);
    poly.memory[0] = 1;

    poly_s init = poly_new (2);
    init.memory[0] = 1;

    for (size_t i = 0; i < ecc_length; i++)
    {
        init.memory[1] = ff_raise2_lut (&table, i);
        poly_s temp = poly_multiply (table, poly, init);
        poly_free (poly);
        poly = temp;
    }

    return (poly);
}

/**
 * Basically what we are doing is copying the src to dest
 * and if dest size is bigger than source, ZERO the rest of memory
 */
FECC_INLINE
void
rs_forward_copy (poly_s dest, poly_s src)
{
    memcpy (dest.memory, src.memory, src.size);
    memset (dest.memory + src.size, 0, dest.size - src.size);
}

/**
 * We are using Synthetic Division, read more at
 * https://en.wikipedia.org/wiki/Synthetic_division
 */
void
rs_encode (rs_encode_s *rs_info)
{
    rs_forward_copy (rs_info->message_out_buffer, rs_info->message_in_buffer);

    poly_s remainder
      = ff_polynomial_mod (rs_info->field_table, rs_info->message_out_buffer, rs_info->generator);

    poly_append (rs_info->message_out_buffer, remainder);

    poly_free (remainder);
}

FECC_INLINE
void
rs_decode (rs_decode_s *decode)
{
    rs_calculate_syndromes (decode);

    if (decode->result != DECODE_STATUS_NONE)
    {
        return;
    }

    rs_make_error_locator_poly (decode);
    rs_drop_leading_zeroes (&decode->error_locator);

    if ((decode->error_locator.size - 1) * 2 > decode->field_ecc_length)
    {
        decode->result = DECODE_STATUS_BERLEKAMP_MASSEY_FAILED;
        return;
    }

    rs_find_error_locations (decode);

    if (decode->result != DECODE_STATUS_NONE)
    {
        return;
    }

    rs_correct_errors (decode);
}

/**
 * Using Horner's Theorm, we are calculating syndromes at ecc_length locations
 * Calculating syndromes is very straight forward and more information is at
 * https://en.wikipedia.org/wiki/Horner%27s_method
 */
FECC_INLINE
void
rs_calculate_syndromes (rs_decode_s *rs_info)
{
    rs_setup_poly_sse (&rs_info->working_set, rs_info->message_in_buffer);
    char no_error = 1;
    for (size_t i = 0; i < rs_info->field_ecc_length; i++)
    {
        ff_t syndrome = poly_evaluate_sse (&rs_info->field_table, rs_info->working_set, i);
        rs_info->syndromes.memory[i] = syndrome;

        if (syndrome != 0)
        {
            no_error = 0;
        }
    }

    if (no_error == 1)
    {
        rs_info->result = DECODE_STATUS_NO_ERRORS_FOUND;
    }
}

/*
 * In rs_calculate_delta what we are doing here is
 * we are calculating the value of delta for berlekamp-massey algorithm
 * and we do that by multiplying the error locator by syndromes and
 * calculating the horizontal sum into delta
 * for j in range(1, len(err_loc)):
 * 		delta ^= galios.gf_mul_lut(err_loc[-(j + 1)], synd[i - j])
 */
FECC_INLINE
ff_t
rs_calculate_delta (rs_decode_s *rs_info, short i)
{
    ff_t delta = rs_info->syndromes.memory[i];

    for (long j = 1; j < rs_info->error_locator.size; ++j)
    {
        ff_t temp = ff_multiply_lut (
          rs_info->field_table.multiply_table,
          rs_info->error_locator.memory[rs_info->error_locator.size - (j + 1)],
          rs_info->syndromes.memory[i - j]);
        FF_ADDITION_INPLACE (delta, temp);
    }
    return (delta);
}

FECC_INLINE
void
rs_drop_leading_zeroes (poly_s *poly)
{
    int index = 0;
    for (; index < poly->size && poly->memory[index] == 0; index++)
        ;
    if (index)
    {
        memmove (poly->memory, poly->memory + index, poly->size - index);
        poly->size -= index;
    }
}

/**
 * Berlekamp-Massey Algorithm
 */
void
rs_make_error_locator_poly (rs_decode_s *rs_info)
{
    poly_s *err_poly = &rs_info->error_locator;
    poly_s *old_poly = &rs_info->error_locator_old;
    poly_s *temp_poly = &rs_info->error_locator_temp;
    POLY_RESET (err_poly);
    POLY_RESET (old_poly);

    for (long i = 0; i < rs_info->field_ecc_length; ++i)
    {
        // ff_t syndrome_shift	= (rs_info->syndromes.size > rs_info->field_ecc_length
        // 	? rs_info->syndromes.size - rs_info->field_ecc_length : 0 );

        // ff_t delta	= rs_calculate_delta(rs_info, i + syndrome_shift);
        ff_t delta = rs_calculate_delta (rs_info, i);
        rs_polynomial_append (old_poly, 0);

        if (delta != 0)
        {
            if (old_poly->size > err_poly->size)
            {
                poly_copy (temp_poly, old_poly);
                poly_multiply_scalar_sse (rs_info->field_table, temp_poly, delta);
                ff_t poly_inverse = ff_inverse_lut (rs_info->field_table, delta);
                poly_copy (old_poly, err_poly);
                poly_multiply_scalar_sse (rs_info->field_table, old_poly, poly_inverse);
                poly_copy (err_poly, temp_poly);
            }

            poly_copy (temp_poly, old_poly);
            poly_multiply_scalar_sse (rs_info->field_table, temp_poly, delta);
            poly_add_inplace (err_poly, temp_poly);
        }
    }
}

void
rs_polynomial_append (poly_s *polynomial, ff_t monomial)
{
    assert (polynomial->size <= polynomial->max_size);
    polynomial->memory[polynomial->size] = monomial;
    polynomial->size++;
}

poly_s
rs_make_error_evaluator_poly (rs_decode_s *decode)
{
    // Omega(x) = Synd(x) * Error_loc(x) mod x ^ (n-k+1)
    rs_invert_poly (&decode->working_set, decode->syndromes);
    decode->working_set.memory[decode->working_set.size] = 0;
    decode->working_set.size++;

    poly_s temp_poly
      = poly_multiply (decode->field_table, decode->working_set, decode->error_locator);
    ff_polynomial_trim_x (decode->field_table, &temp_poly, decode->error_locator.size + 1);
    poly_copy (&decode->error_evaluator, &temp_poly);
    poly_free (temp_poly);
    return (decode->error_evaluator);
}

void
rs_find_error_locations (rs_decode_s *rs_info)
{
    rs_invert_poly (&rs_info->working_set, rs_info->error_locator);
    rs_info->error_locations.size = rs_info->error_locator.size - 1;

    if (rs_info->error_locations.size > 0)
    {
        long i = 0, j = 0;
        for (; i < rs_info->message_in_buffer.size; i++)
        {
            ff_t *multiply_table	= (rs_info->field_table.multiply_table 
				+ FF_TABLE_MULT_MODIFIED(ff_raise2_lut(&rs_info->field_table, i)));

            if (poly_evaluate_modified (multiply_table, rs_info->working_set) == 0)
            {
                rs_info->error_locations.memory[j++] = rs_info->message_in_buffer.size - 1 - i;

                if (j == rs_info->error_locations.size)
                {
                    break;
                }
            }
        }

        if (j != rs_info->error_locations.size)
        {
            rs_info->result = DECODE_STATUS_CHIEN_SEARCH_FAILED;
        }
    }
    else
    {
        rs_info->result = DECODE_STATUS_NO_ERRORS_FOUND;
    }
}

FECC_INLINE
void
rs_invert_poly (poly_s *dest, poly_s poly)
{
    dest->size = poly.size;
    for (size_t i = 1; i <= poly.size; i++)
    {
        dest->memory[poly.size - i] = poly.memory[i - 1];
    }
}

FECC_INLINE
void
rs_setup_poly_sse (poly_s *dest, poly_s src)
{
    short alignment = ALLOCATE_ALIGNMENT (src.size);
    short diff = alignment - src.size;
    dest->size = alignment;

    if (diff)
    {
        memset (dest->memory, 0, diff);
    }
    memcpy (dest->memory + diff, src.memory, src.size);
}

void
rs_correct_errors (rs_decode_s *rs_info)
{
    poly_s error_evaluator = rs_make_error_evaluator_poly (rs_info);
    if (error_evaluator.size <= 0)
    {
        rs_info->result = DECODE_STATUS_ERROR_EVAL_FAILED;
        return;
    }

    poly_s X_poly = rs_info->working_set;
    X_poly.size = rs_info->error_locations.size;

    for (long i = 0; i < rs_info->error_locations.size; i++)
    {
        ff_t coff = (rs_info->message_in_buffer.size - rs_info->error_locations.memory[i] - 1);
        short l = 255 - coff;
        X_poly.memory[i] = ff_raise_lut (rs_info->field_table, 2, -l);
    }

    short message_len = rs_info->message_in_buffer.size;
    poly_copy (&rs_info->message_out_buffer, &rs_info->message_in_buffer);

    for (long i = 0; i < X_poly.size; i++)
    {
        ff_t *multiply_table = rs_info->field_table.multiply_table;
        ff_t X_inverse = ff_inverse_lut (rs_info->field_table, X_poly.memory[i]);
        ff_t err_loc_prime = 1;

        for (size_t j = 0; j < X_poly.size; j++)
        {
            if (i != j)
            {
                ff_t temp1 = FF_SUBSTRACTION (
                  1, ff_multiply_lut (multiply_table, X_inverse, X_poly.memory[j]));

                err_loc_prime = ff_multiply_lut (multiply_table, err_loc_prime, temp1);
            }
        }
        ff_t *modified_lut = multiply_table + FF_TABLE_MULT_MODIFIED (X_inverse);

        ff_t eval = poly_evaluate_modified (modified_lut, error_evaluator);
        eval = ff_multiply_lut (multiply_table, eval, X_poly.memory[i]);

        ff_t magnitude = ff_divide_lut (rs_info->field_table, eval, err_loc_prime);

        ff_t err_location = rs_info->error_locations.memory[i];
        FF_ADDITION_INPLACE (rs_info->message_out_buffer.memory[err_location], magnitude);
    }
}