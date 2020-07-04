#include "rs.h"

// Reason for using stack instead of heap is because it 
// will take more time for malloc to return compared to 
// using stack to setup one time structures. 
rs_encode_s rs_init_encoder(ff_t n, ff_t k, short irr_p)
{
	rs_encode_s encoder	= {0};
	ff_t data_length	= k
		, ecc_length	= (n-k);
	encoder.message_in_buffer	= poly_new(k);
	encoder.message_out_buffer	= poly_new(n);
	encoder.field_ecc_length	= ecc_length;
	encoder.field_message_in_length	= k;
	encoder.field_message_out_length	= n;
	encoder.field_table	=	ff_table_new(irr_p);
	encoder.generator	= 
		rs_make_generator_polynomial(encoder.field_table, ecc_length);
	return(encoder);
}

// must match the rs_init_encoder to free all the memory used. 
void rs_close_encoder(rs_encode_s *s)
{
	poly_free(s->message_in_buffer);
	poly_free(s->message_out_buffer);
	ff_table_close(s->field_table);
}

rs_decode_s rs_init_decoder(ff_t n, ff_t k, short irr_p)
{
	rs_decode_s decoder	= {0};
	ff_t data_length	= k
		, ecc_length	= (n-k);
	decoder.message_in_buffer	= poly_new(n);
	decoder.message_out_buffer	= poly_new(n);
	decoder.field_ecc_length	= ecc_length;
	decoder.field_message_in_length	= n;
	decoder.field_message_out_length	= k;
	decoder.field_table	=	ff_table_new(irr_p);
	decoder.generator	= 
		rs_make_generator_polynomial(decoder.field_table, ecc_length);
	short allocation_length	= ecc_length + 1;
	decoder.syndromes	= poly_new(allocation_length);
	decoder.error_evaluator	= poly_new(allocation_length);
	decoder.error_locator	= poly_new(allocation_length);
	decoder.error_locator_temp	= poly_new(allocation_length);
	decoder.error_locator_old	= poly_new(allocation_length);
	decoder.error_locations	= poly_new(allocation_length);

	return(decoder);
}

void rs_close_decoder(rs_decode_s *decoder)
{
	poly_free(decoder->syndromes);
	poly_free(decoder->error_evaluator);
	poly_free(decoder->error_locator);
	poly_free(decoder->error_locator_temp);
	poly_free(decoder->error_locator_old);
	poly_free(decoder->error_locations);
	poly_free(decoder->message_in_buffer);
	poly_free(decoder->message_out_buffer);
	poly_free(decoder->generator);
	ff_table_close(decoder->field_table);
}

poly_s rs_make_generator_polynomial(ff_table_s table, short ecc_length)
{
	poly_s poly	= poly_new(1);
	poly.memory[0]	= 1;

	poly_s init	= poly_new(2);
	init.memory[0]	= 1;
	
	for (size_t i = 0; i < ecc_length; i++)
	{
		init.memory[1]	= ff_raise_lut(table, 2, i);
		poly_s temp	= poly_new(poly.size + init.size - 1);
		poly_multiply(table, &temp, poly, init);
		free(poly.memory);
		poly	= temp;
	}

	return (poly);
}

/**
 * Basically what we are doing is copying the src to dest
 * and if dest size is bigger than source, ZERO the rest of memory
 */
inline void rs_forward_copy(poly_s dest, poly_s src)
{
	memcpy(dest.memory, src.memory, src.size);
	memset(dest.memory + src.size, 0, dest.size - src.size);
}

/**
 * How does the synthetic polynomial division works?
 * We take a copy of dividend, and here the length of the dividend is
 * "Actual dividend" + "Zero bytes filled in for divisor"
 * 
 * That is why we only operate on the actual divident length. 
 * We take the value at i from message m as coff
 * Now if coff == 0, we don't do anything because log(0) is undefined
 * 
 * Otherwise we iterate through the length of the divisor
 * And we take value of divisor at j from divisor m as coff
 * 
 * We then multiply the values at i, j and we subtract it from the value of the 
 * dividend at [i + j]
 * 
 * https://en.wikipedia.org/wiki/Synthetic_division
 */

void rs_encode(rs_encode_s *rs_info)
{
	rs_forward_copy(rs_info->message_out_buffer, rs_info->message_in_buffer);

	poly_s remainder	= ff_polynomial_mod(rs_info->field_table
		, rs_info->message_out_buffer, rs_info->generator);

	poly_append(rs_info->message_out_buffer, remainder);
	
	poly_free(remainder);
}

/**
 * Using Horner's Theorm, we are calculating syndromes at ecc_length locations
 * Calculating syndromes is very straight forward and more information at
 * https://en.wikipedia.org/wiki/Horner%27s_method
 */
inline void rs_calculate_syndromes(rs_decode_s *rs_info)
{
	for (size_t i = 0; i < rs_info->field_ecc_length; i++)
	{
		rs_info->syndromes.memory[i + 1]	= 
			poly_evaluate
				(rs_info->field_table.full_table
				, rs_info->message_in_buffer
				, ff_raise_lut(rs_info->field_table, 2, i));
	}
}

/*
 * In rs_calculate_delta what we are doing here is 
 * we are calculate the value of delta for berlekamp-massey algorithm
 * and we do that by multiplying the error locator by syndromes and 
 * calculating the addition value into delta
 * for j in range(1, len(err_loc)):
 * delta ^= galios.gf_mul_lut(err_loc[-(j + 1)], synd[i - j])
 */
ff_t rs_calculate_delta(rs_decode_s *rs_info , short i)
{
	ff_t delta	= rs_info->syndromes.memory[i];

	for (long j = 1; j < rs_info->error_locator.size; ++j)
	{
		ff_t temp	= ff_multiply_lut(rs_info->field_table.full_table
			, rs_info->error_locator.memory[rs_info->error_locator.size - (j + 1)]
			, rs_info->syndromes.memory[i - j]);
		FF_ADDITION_INPLACE(delta, temp);
	}
	
	return (delta);
}

/**
 * Berlekamp-Massey Algorithm
 */
void rs_make_error_location_poly(rs_decode_s *rs_info)
{
	poly_s *err_poly	= &rs_info->error_locator;
	POLY_RESET(err_poly);
	poly_s *old_err_poly	=  &rs_info->error_locator_old;
	POLY_RESET(old_err_poly);

	poly_s *temp_poly	= &rs_info->error_locator_temp;
	for (long i=0; i < rs_info->syndromes.size; ++i)
	{
		ff_t delta	= rs_calculate_delta(rs_info, i);
		rs_polynomial_append(old_err_poly, 0);

		if (delta != 0)
		{
			if (old_err_poly->size > err_poly->size)
			{
				poly_copy(temp_poly, old_err_poly);
				poly_multiply_scalar(rs_info->field_table, temp_poly, delta);
				ff_t poly_inverse	= ff_inverse_lut(rs_info->field_table, delta);
				poly_copy(old_err_poly, err_poly);
				poly_multiply_scalar(rs_info->field_table, old_err_poly
					, poly_inverse);
				poly_copy(err_poly, temp_poly);
			}
			poly_copy(temp_poly, old_err_poly);
			poly_multiply_scalar(rs_info->field_table, temp_poly, delta);
	
			poly_s	t1	= poly_new(MAX(err_poly->size, temp_poly->size));
			poly_add(rs_info->field_table, &t1, *err_poly, *temp_poly);
						
			poly_copy(err_poly, &t1);
			free(t1.memory);
		}
	}
}

void rs_polynomial_append(poly_s *polynomial, ff_t monomial)
{
	assert(polynomial->size <= polynomial->max_size);
	polynomial->memory[polynomial->size]	= monomial;
	polynomial->size++;
}

poly_s rs_make_error_evaluator_poly(ff_table_s table
	, poly_s syndromes
	, poly_s error_locations
	, short ecc_symbols)
{
	// Omega(x) = Synd(x) * Error_loc(x) mod x ^ (n-k+1)
	poly_s temp_poly	= poly_new(syndromes.size + error_locations.size - 1);
	poly_multiply(table, &temp_poly, syndromes, error_locations);
	poly_s divisor	= poly_new(ecc_symbols + 2);
	divisor.memory[0]	= 1;
	poly_s remainder	= ff_polynomial_mod(table, temp_poly, divisor);
	free(divisor.memory);
	return (remainder);
}

void rs_find_error_locations(rs_decode_s *rs_info)
{
	poly_s invert_poly	= rs_invert_poly(rs_info->error_locator);
	rs_info->error_locations.size	= rs_info->error_locator.size - 1;
	// printf("error locations size is %d\n", rs_info->error_locations.size);
	if (rs_info->error_locations.size > 0)
	{
		
	for (size_t i = 0, j = 0; i < rs_info->message_in_buffer.size; i++)
	{
		if (poly_evaluate(rs_info->field_table.full_table, invert_poly, ff_raise_lut(rs_info->field_table, 2, i)) == 0)
		{
			rs_info->error_locations.memory[j++]	= rs_info->message_in_buffer.size - 1 - i;
			
	// printf("j is %d, and i is %d\n", j, i);
			if(j == rs_info->error_locations.size)
			{
				// printf("break entered");
				break;
			}
		}
	}
	poly_free(invert_poly);
	}
}

poly_s rs_invert_poly(poly_s poly)
{
	poly_s poly2 = poly_new(poly.size);

	for (size_t i = 1; i <= poly.size; i++)
	{
		poly2.memory[poly.size - i]	= poly.memory[i - 1];
	}
	return(poly2);
}

void rs_correct_errors(rs_decode_s *rs_info)
{
	poly_s reverse1	= rs_invert_poly(rs_info->syndromes);
	poly_s error_evaluator	= rs_make_error_evaluator_poly(rs_info->field_table, reverse1, rs_info->error_locator, rs_info->error_locator.size - 1);
	// poly_free(reverse1);
	if (error_evaluator.size <=0 )
	{
		return ;
	}
	poly_s X_poly	= poly_new(rs_info->error_locations.size);

	for (size_t i = 0; i < rs_info->error_locations.size; i++)
	{
		ff_t coff	= (rs_info->message_in_buffer.size - 1 - rs_info->error_locations.memory[i]);
		short l = 255 - coff;
		X_poly.memory[i]	= ff_raise_lut(rs_info->field_table, 2, -l);
	}

	poly_s error_magnitude_poly	= poly_new(rs_info->message_in_buffer.size);
	short message_len	= rs_info->message_in_buffer.size;


	for (size_t i = 0; i < X_poly.size; i++)
	{
		ff_t X_inv	= ff_inverse_lut(rs_info->field_table, X_poly.memory[i]);
		ff_t err_loc_prime = 1;

		for (size_t j = 0; j < X_poly.size; j++)
		{
			if (i != j)
			{
				// in case of i = j then the inverse will cancel out the X and the final 
				// result will be 1. 
				err_loc_prime = ff_multiply_lut(rs_info->field_table.full_table, err_loc_prime, 
					FF_SUBSTRACTION(1, ff_multiply_lut(rs_info->field_table.full_table, X_inv, X_poly.memory[j])));
			}
		}

		ff_t polynomial_eval	= poly_evaluate(rs_info->field_table.full_table, (error_evaluator), X_inv);
		polynomial_eval	= ff_multiply_lut(rs_info->field_table.full_table, X_poly.memory[i], polynomial_eval);

		ff_t magnitude	= ff_divide_lut(rs_info->field_table, polynomial_eval, err_loc_prime);


		error_magnitude_poly.memory[rs_info->error_locations.memory[i]] = magnitude;
	}
	
	poly_add(rs_info->field_table, &rs_info->message_out_buffer
		, rs_info->message_in_buffer, error_magnitude_poly);
}