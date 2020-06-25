#include "rs.h"

rs_information_s rs_init(ff_t n, ff_t k, short irr_p)
{
	rs_information_s info	= {0};
	ff_t data_length	= k
		, ecc_length	= (n-k);
	info.message_in	= ff_polynomial_new(data_length);
	info.message_out	= ff_polynomial_new(n);
	info.field_ecc_length	= ecc_length;
	info.field_message_length	= data_length;
	info.field_table	=	ff_generate_table(irr_p);
	info.generator	= 
		rs_make_generator_polynomial(info.field_table, ecc_length);
	info.syndromes	= ff_polynomial_new(ecc_length + 1);

	info.error_evaluator	= ff_polynomial_new(ecc_length + 1);
	info.error_locator	= ff_polynomial_new(ecc_length + 1);
	info.error_locator_temp	= ff_polynomial_new(ecc_length + 1);
	info.error_locator_old	= ff_polynomial_new(ecc_length + 1);
	info.error_locations	= ff_polynomial_new(ecc_length + 1);
	return (info);
}

ff_polynomial_s rs_make_generator_polynomial(ff_table_s table
	, short number_ecc_symbols)
{
	ff_polynomial_s poly	= ff_polynomial_new(1);
	poly.memory[0]	= 1;

	ff_polynomial_s init	= ff_polynomial_new(2);
	init.memory[0]	= 1;
	
	for (size_t i = 0; i < number_ecc_symbols; i++)
	{
		init.memory[1]	= ff_raise_lut(table, 2, i);

		ff_polynomial_s temp	= ff_polynomial_multiply(table, poly, init);
		
		free(poly.memory);
		poly	= temp;
	}

	return (poly);
}

ff_polynomial_s ff_polynomial_copy(ff_polynomial_s poly)
{
	ff_polynomial_s copy	= ff_polynomial_new(poly.size);
	memcpy(copy.memory, poly.memory, sizeof(ff_t) * poly.size);
	return (copy);
}

ff_polynomial_s ff_polynomial_mod(ff_table_s table
	, ff_polynomial_s dividend, ff_polynomial_s divisor)
{
	long diff	= dividend.size - (divisor.size - 1);
	ff_polynomial_s dividend_copy	= ff_polynomial_copy(dividend);
	ff_polynomial_s remainder	= ff_polynomial_new(divisor.size - 1);

	for (size_t i = 0; i < diff; i++)
	{
		ff_t coef	= dividend_copy.memory[i];
		
		if (coef != 0)
		{
			for (size_t j = 1; j < divisor.size; j++)
			{
				if (divisor.memory[j] != 0)
				{
					dividend_copy.memory[i + j] = 
						FF_SUBSTRACTION(dividend_copy.memory[i + j]
						, ff_multiply_lut(table, divisor.memory[j], coef));
				}
			}
			
		}
	}

	ff_t *remainder_address	= dividend_copy.memory
		 + dividend_copy.size - remainder.size;
	memcpy(remainder.memory, remainder_address, remainder.size);

	ff_polynomial_free(dividend_copy);
	return (remainder);
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

void rs_encode(rs_information_s *rs_info)
{
	memcpy(rs_info->message_out.memory
		, rs_info->message_in.memory, rs_info->message_in.size);
	memset(rs_info->message_out.memory + rs_info->message_in.size
		, 0, rs_info->field_ecc_length);

	ff_polynomial_s remainder	= 
		ff_polynomial_mod(rs_info->field_table, rs_info->message_out, rs_info->generator);

	ff_append_polynomials(rs_info->message_out, remainder);
	
	ff_polynomial_free(remainder);
}

/* using horner's theorm */
inline void rs_calculate_syndromes(ff_polynomial_s *syndromes
	, ff_table_s table, ff_polynomial_s message_in, short field_ecc_length)
{
	for (size_t i = 0; i < field_ecc_length; i++)
	{
		syndromes->memory[i + 1]	= 
			ff_evaluate_polynomial(table
				, message_in, ff_raise_lut(table, 2, i));
	}
}

// for (size_t i = 0; i < rs_info->field_ecc_length; i++)
// 	{
// 		rs_info->syndromes.memory[i + 1]	= 
// 			ff_evaluate_polynomial(rs_info->field_table
// 				, rs_info->message_in, ff_raise_lut(rs_info->field_table, 2, i));
// 	}
	
// 	return (rs_info->syndromes);
/*
 * In rs_calculate_delta what we are doing here is 
 * we are calculate the value of delta for berlekamp-massey algorithm
 * and we do that by multiplying the error locator by syndromes and 
 * calculating the addition value into delta
 */
ff_t rs_calculate_delta(rs_information_s *rs_info , short syndrome_i)
{
	ff_t delta	= rs_info->syndromes.memory[syndrome_i];
	for (long i = 1; i < rs_info->error_locator.size; ++i)
	{
		ff_t temp	= ff_multiply_lut(rs_info->field_table
			, rs_info->error_locator.memory[rs_info->error_locator.size - (i + 1)]
			, rs_info->syndromes.memory[syndrome_i - i]);

		delta	= FF_ADDITION(delta, temp);
	}
	/**
	 * 		for j in range(1, len(err_loc)):
	 *		delta ^= galios.gf_mul_lut(err_loc[-(j + 1)], synd[i - j])
	*/
	return (delta);
}

void rs_print_poly(char *string, ff_polynomial_s poly)
{
	puts(string);
	puts("poly: ");
	for (size_t i = 0; i < poly.size; i++)
	{
		printf("%d, ", poly.memory[i]);
	}
	puts("");
}

void rs_copy_poly(ff_polynomial_s *dest, ff_polynomial_s *source)
{
	memcpy(dest->memory, source->memory, source->size);
	dest->size	= source->size;
}
/**
 * Berlekamp-Massey Algorithm
 */
void rs_make_error_location_poly(rs_information_s *rs_info)
{
	ff_polynomial_s *error_locator_poly	= &rs_info->error_locator;
	error_locator_poly->memory[0]	= 1;
	error_locator_poly->size	= 1;
	ff_polynomial_s *old_error_locator_poly	=  &rs_info->error_locator_old;
	old_error_locator_poly->memory[0]	= 1;
	old_error_locator_poly->size	= 1;
	ff_polynomial_s *temp_poly	= &rs_info->error_locator_temp;

	for (long i=0; i < rs_info->syndromes.size; ++i)
	{
		ff_t delta	= rs_calculate_delta(rs_info, i);
		rs_polynomial_append(old_error_locator_poly, 0);

		if (delta != 0)
		{
			if (old_error_locator_poly->size > error_locator_poly->size)
			{
				rs_copy_poly(temp_poly, old_error_locator_poly);
				ff_polynomial_multiply_scalar(rs_info->field_table, temp_poly, delta);
				ff_t poly_inverse	= ff_inverse_lut(rs_info->field_table, delta);
				rs_copy_poly(old_error_locator_poly, error_locator_poly);
				ff_polynomial_multiply_scalar(rs_info->field_table, old_error_locator_poly
					, poly_inverse);
				rs_copy_poly(error_locator_poly, temp_poly);
			}
			rs_copy_poly(temp_poly, old_error_locator_poly);
			ff_polynomial_multiply_scalar(rs_info->field_table, temp_poly, delta);
	
			ff_polynomial_s	t1	= 
				ff_polynomial_add(rs_info->field_table, *error_locator_poly
				, *temp_poly);
			
			rs_copy_poly(error_locator_poly, &t1);
			free(t1.memory);
		}
	}
}

void rs_polynomial_append(ff_polynomial_s *polynomial, ff_t monomial)
{
	assert(polynomial->size <= polynomial->max_size);
	polynomial->memory[polynomial->size]	= monomial;
	polynomial->size++;
}

ff_polynomial_s rs_make_error_evaluator_poly(ff_table_s table
	, ff_polynomial_s syndromes
	, ff_polynomial_s error_locations
	, short ecc_symbols)
{
	// Omega(x) = Synd(x) * Error_loc(x) mod x ^ (n-k+1)
	ff_polynomial_s temp_poly	= ff_polynomial_multiply(table, syndromes, error_locations);
	ff_polynomial_s divisor	= ff_polynomial_new(ecc_symbols + 2);
	divisor.memory[0]	= 1;
	ff_polynomial_s remainder	= ff_polynomial_mod(table, temp_poly, divisor);
	free(divisor.memory);
	return (remainder);
}

ff_polynomial_s rs_find_error_locations(ff_table_s table
	, ff_polynomial_s error_locator_poly, short message_in_length)
{
	ff_polynomial_s error_locations	= {0};
	error_locations.size	= error_locator_poly.size - 1;
	error_locations.memory	= calloc(error_locations.size, sizeof(ff_t));

	ff_polynomial_s invert_poly	= {0};
	invert_poly	= rs_invert_poly(error_locator_poly);
	if (error_locations.size > 0){
		
	for (size_t i = 0, j = 0; i < message_in_length; i++)
	{
		if (ff_evaluate_polynomial(table, invert_poly, ff_raise_lut(table, 2, i)) == 0)
		{
			error_locations.memory[j++]	= message_in_length - 1 - i;
		}
	}
	
	}
	return (error_locations);
}

ff_polynomial_s rs_invert_poly(ff_polynomial_s poly)
{
	ff_polynomial_s poly2 = ff_polynomial_new(poly.size);

	for (size_t i = 1; i <= poly.size; i++)
	{
		poly2.memory[poly.size - i]	= poly.memory[i - 1];
	}
	return(poly2);
}

ff_polynomial_s rs_correct_errors(ff_table_s table
	, ff_polynomial_s message_in 
	, ff_polynomial_s syndromes
	, ff_polynomial_s error_locator_poly
	, ff_polynomial_s error_positions)
{
	ff_polynomial_s reverse1	= rs_invert_poly(syndromes);
	ff_polynomial_s error_evaluator	= rs_make_error_evaluator_poly(table, reverse1, error_locator_poly, error_locator_poly.size - 1);
	
	if (error_evaluator.size <=0 )
	{
		return (message_in);
	}
	ff_polynomial_s X_poly	= ff_polynomial_new(error_positions.size);

	for (size_t i = 0; i < error_positions.size; i++)
	{
		ff_t coff	= (message_in.size - 1 - error_positions.memory[i]);
		short l = 255 - coff;
		X_poly.memory[i]	= ff_raise_lut(table, 2, -l);
	}

	ff_polynomial_s error_magnitude_poly	= ff_polynomial_new(message_in.size);
	short message_len	= message_in.size;


	for (size_t i = 0; i < X_poly.size; i++)
	{
		ff_t X_inv	= ff_inverse_lut(table, X_poly.memory[i]);
		ff_t err_loc_prime = 1;

		for (size_t j = 0; j < X_poly.size; j++)
		{
			if (i != j)
			{
				// in case of i = j then the inverse will cancel out the X and the final 
				// result will be 1. 
				err_loc_prime = ff_multiply_lut(table, err_loc_prime, 
					FF_SUBSTRACTION(1, ff_multiply_lut(table, X_inv, X_poly.memory[j])));
			}
		}

		ff_t polynomial_eval	= ff_evaluate_polynomial(table, (error_evaluator), X_inv);
		polynomial_eval	= ff_multiply_lut(table, X_poly.memory[i], polynomial_eval);

		ff_t magnitude	= ff_divide_lut(table, polynomial_eval, err_loc_prime);


		error_magnitude_poly.memory[error_positions.memory[i]] = magnitude;
	}
	
	message_in	= ff_polynomial_add(table, message_in, error_magnitude_poly);
	return(message_in);
}