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
			, error_locator_poly.memory[error_locator_poly.size - 1 - i]
			, syndromes.memory[syndrome_i - i]);

		delta	= ff_addition(delta, temp);
	}
	return (delta);
}

/**
 * Berlekamp-Massey Algorithm
 */
ff_polynomial_s rs_make_error_location_poly(ff_table_s table
	, ff_polynomial_s syndromes, short number_ecc_symbols)
{
	ff_polynomial_s error_location_poly1	= rs_polynomial_new(1, 1);
	ff_polynomial_s error_location_poly2	= rs_polynomial_new(1, 1);

	for (long i=0; i<syndromes.size; ++i)
	{
		error_location_poly2	= rs_polynomial_append(error_location_poly2, 0);
		ff_t delta	= syndromes.memory[i];

		delta	= rs_calculate_delta(table, i, syndromes, error_location_poly1);

		printf("Delta is %d\n", delta);
		if (delta != 0)
		{
			if (error_location_poly2.size > error_location_poly1.size)
			{
				ff_polynomial_s temp_poly	= 
					ff_polynomial_multiply_scalar(table, error_location_poly2, delta);
				error_location_poly2	= 
					ff_polynomial_multiply_scalar(table, error_location_poly1
					, ff_inverse_lut(table, delta));
				error_location_poly1	= temp_poly;
			}
			ff_polynomial_s scalar_mult1	= 
				ff_polynomial_multiply_scalar(table, error_location_poly2, delta);
			error_location_poly1	= 
				ff_polynomial_add(table, error_location_poly1
				, scalar_mult1);
			
			for (size_t i = 0; i < scalar_mult1.size; i++)
			{
				printf("POLY: %x, ", scalar_mult1.memory[i]);
			}
		}
	}
	return (error_location_poly1);
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
	poly1.memory	= malloc(size * sizeof(ff_t));
	memset(poly1.memory, def, size);
	poly1.size	= size;
	return (poly1);
}