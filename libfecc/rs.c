#include "rs.h"

ff_polynomial_s rs_make_generator_polynomial(ff_table_s table
	, short number_ecc_symbols)
{
	ff_polynomial_s poly	= {0};
	poly.memory	= calloc(1, sizeof(ff_t));
	poly.size	= 1;
	poly.memory[0]	= 1;

	ff_polynomial_s init	= {0};
	init.memory	= calloc(2, sizeof(ff_t));
	init.size	= 2;
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

ff_polynomial_s ff_polynomial_division(ff_table_s table
	, ff_polynomial_s dividend, ff_polynomial_s divisor)
{
	long diff	= dividend.size - (divisor.size - 1);

	ff_polynomial_s dividend_copy	= dividend;
	dividend_copy.memory	= calloc(dividend.size, sizeof(ff_t));
	memcpy(dividend_copy.memory, dividend.memory
		, sizeof(ff_t) * dividend.size);

	ff_polynomial_s remainder;
	remainder.memory	= calloc(divisor.size -1, sizeof(ff_t));
	remainder.size	= divisor.size - 1;

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
						ff_subtraction(dividend_copy.memory[i + j]
						, ff_multiply_lut(table, divisor.memory[j], coef));
				}
			}
			
		}
	}

	ff_t *remainder_address	= dividend_copy.memory
		 + dividend_copy.size - remainder.size;
	memcpy(remainder.memory, remainder_address, remainder.size);

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

ff_polynomial_s rs_encode(ff_table_s table
	, ff_polynomial_s message_poly, short number_ecc_symbols)
{
	ff_polynomial_s generator	= 
		rs_make_generator_polynomial(table, number_ecc_symbols);


	ff_polynomial_s empty	= {0};
	empty.memory	= calloc(generator.size - 1, sizeof(ff_t));
	empty.size	= generator.size - 1;

	ff_polynomial_s message_poly_b = ff_append_polynomials(message_poly, empty);

	ff_polynomial_s remainder	= 
		ff_polynomial_division(table, message_poly_b, generator);

	ff_polynomial_s final_message	= 
		ff_append_polynomials(message_poly, remainder);
	
	free(empty.memory);
	free(generator.memory);
	free(message_poly_b.memory);
	free(remainder.memory);

	return (final_message);
}

/* using horner's theorm */
ff_polynomial_s rs_calculate_syndromes(ff_table_s table, ff_polynomial_s msg
	, short number_ecc_symbols)
{
	ff_polynomial_s syndromes	= {0};
	syndromes.memory	= calloc(number_ecc_symbols + 1, sizeof(ff_t));
	syndromes.size	= number_ecc_symbols + 1;

	for (size_t i = 0; i < number_ecc_symbols; i++)
	{
		syndromes.memory[i + 1]	= 
			ff_evaluate_polynomial(table, msg, ff_raise_lut(table, 2, i));
	}
	
	return (syndromes);
}

/*
 * In rs_calculate_delta what we are doing here is 
 * we are calculate the value of delta for berlekamp-massey algorithm
 * and we do that by multiplying the error locator by syndromes and 
 * calculating the addition value into delta
 */
ff_t rs_calculate_delta(ff_table_s table
	, short syndrome_i
	, ff_polynomial_s syndromes
	, ff_polynomial_s error_locator_poly)
{
	ff_t delta	= syndromes.memory[syndrome_i];
	for (long i = 1; i < error_locator_poly.size; ++i)
	{
		ff_t temp	= ff_multiply_lut(table
			, error_locator_poly.memory[error_locator_poly.size - (i + 1)]
			, syndromes.memory[syndrome_i - i]);

		delta	= ff_addition(delta, temp);
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
/**
 * Berlekamp-Massey Algorithm
 */
ff_polynomial_s rs_make_error_location_poly(ff_table_s table
	, ff_polynomial_s syndromes, short number_ecc_symbols)
{
	ff_polynomial_s error_locator_poly	= rs_polynomial_new(1, 1);
	ff_polynomial_s old_error_locator_poly	= rs_polynomial_new(1, 1);

	for (long i=0; i<syndromes.size; ++i)
	{
		ff_t delta = rs_calculate_delta(table, i, syndromes, error_locator_poly);
		old_error_locator_poly	= rs_polynomial_append(old_error_locator_poly, 0);

		if (delta != 0)
		{
			if (old_error_locator_poly.size > error_locator_poly.size)
			{
				ff_polynomial_s temp_poly	= 
					ff_polynomial_multiply_scalar(table, old_error_locator_poly, delta);
				old_error_locator_poly	= 
					ff_polynomial_multiply_scalar(table, error_locator_poly
					, ff_inverse_lut(table, delta));
				error_locator_poly	= temp_poly;
			}
			ff_polynomial_s scalar_mult1	= 
				ff_polynomial_multiply_scalar(table, old_error_locator_poly, delta);
	
			error_locator_poly	= 
				ff_polynomial_add(table, error_locator_poly
				, scalar_mult1);
		}
	}
	return (error_locator_poly);
}

ff_polynomial_s rs_polynomial_append(ff_polynomial_s polynomial, ff_t monomial)
{
	polynomial.memory	= realloc(polynomial.memory, polynomial.size + sizeof(ff_t));
	polynomial.memory[polynomial.size]	= monomial;
	polynomial.size++;
	return (polynomial);
}

ff_polynomial_s rs_polynomial_new(short size, ff_t def)
{
	ff_polynomial_s poly1 = {0};
	poly1.memory	= calloc(size, sizeof(ff_t));
	memset(poly1.memory, def, size);
	poly1.size	= size;
	return (poly1);
}

ff_polynomial_s rs_make_error_evaluator_poly(ff_table_s table
	, ff_polynomial_s syndromes
	, ff_polynomial_s error_locations
	, short ecc_symbols)
{
	// Omega(x) = Synd(x) * Error_loc(x) mod x ^ (n-k+1)
	ff_polynomial_s temp_poly	= ff_polynomial_multiply(table, syndromes, error_locations);
	ff_polynomial_s divisor	= rs_polynomial_new(ecc_symbols + 2, 0);
	divisor.memory[0]	= 1;
	ff_polynomial_s remainder	= ff_polynomial_division(table, temp_poly, divisor);
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
	for (size_t i = 0, j = 0; i < message_in_length; i++)
	{
		if (ff_evaluate_polynomial(table, invert_poly, ff_raise_lut(table, 2, i)) == 0)
		{
			error_locations.memory[j++]	= message_in_length - 1 - i;
		}
	}
	
	return (error_locations);
}

ff_polynomial_s rs_invert_poly(ff_polynomial_s poly)
{
	ff_polynomial_s poly2 = rs_polynomial_new(poly.size, 0);

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
	
	ff_polynomial_s X_poly	= rs_polynomial_new(error_positions.size, 0);

	for (size_t i = 0; i < error_positions.size; i++)
	{
		ff_t coff	= (message_in.size - 1 - error_positions.memory[i]);
		short l = 255 - coff;
		X_poly.memory[i]	= ff_raise_lut(table, 2, -l);
	}

	ff_polynomial_s error_magnitude_poly	= rs_polynomial_new(message_in.size, 0);
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
					ff_subtraction(1, ff_multiply_lut(table, X_inv, X_poly.memory[j])));
			}
		}
		rs_print_poly("evaluator", error_evaluator);
		printf("xinv: %d\n", X_inv);

		ff_t polynomial_eval	= ff_evaluate_polynomial(table, (error_evaluator), X_inv);
		polynomial_eval	= ff_multiply_lut(table, X_poly.memory[i], polynomial_eval);

		ff_t magnitude	= ff_divide_lut(table, polynomial_eval, err_loc_prime, NULL);


		error_magnitude_poly.memory[error_positions.memory[i]] = magnitude;
	}
	
	message_in	= ff_polynomial_add(table, message_in, error_magnitude_poly);
	return(message_in);
}