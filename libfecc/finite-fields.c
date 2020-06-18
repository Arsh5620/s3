#include "finite-fields.h"

/*
 * In Finite Fields addition for p = 2, m = 8 is simple XOR
 */
inline ff_t ff_addition(ff_t a, ff_t b)
{
	// refer to FF.odt for more information
	return (a ^ b);
}

/*
 * In Finite Fields subtraction is same as addition when p = 2, m = 8
 */
inline ff_t ff_subtraction(ff_t a, ff_t b)
{
	return (a ^ b);
}

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
			result	= ff_addition(result, a);
		}

		if (a & 0x80)
		{
			a	= ff_subtraction(a << 1, irr_p);
		}
		else 
		{
			a	= a << 1;
		}

		b = b >> 1;
	}

	return (result);
}

ff_table_s ff_generate_table(short irr_p)
{
	ff_table_s table	= {0};
	table.size	= FF_SIZE;
	
	ff_t x	= 1;
	for (size_t i = 0; i < FF_SIZE; i++)
	{
		table.antilogs[i]	= x;
		table.logs[(unsigned int)x]	= i;

		x	= ff_multiply(x, 2, irr_p);
	}
	memcpy(table.antilogs + 255, table.antilogs, 256);

	return (table);
}

ff_t ff_divide_lut(ff_table_s lut, ff_t x, ff_t y, char *error)
{
	if (y == 0)
	{
		if (error != NULL)
		{
			*error	= 1;
		}
		return 0;
	}
	
	if (x == 0)
	{
		return 0;
	}

	ff_t lut_index	= (lut.logs[x] + (255 - lut.logs[y])) % 255;
	return lut.antilogs[lut_index];
}

ff_t ff_raise_lut(ff_table_s table, ff_t x, short power)
{
	ff_t alog	= (table.logs[x] * power) % 255;
	if (power < 0)
		alog--;
	return table.antilogs[alog];
}

ff_t ff_inverse_lut(ff_table_s table, ff_t x)
{
	return table.antilogs[255 - table.logs[x]];
}

ff_t ff_multiply_lut(ff_table_s table, ff_t x, ff_t y)
{
	/*
	 * https://en.wikipedia.org/wiki/Finite_field_arithmetic#Implementation_tricks
	 */
	if (x == 0 || y == 0)
	{
		return 0;
	}
	return table.antilogs[table.logs[x] + table.logs[y]];
}

ff_polynomial_s ff_polynomial_multiply_scalar(ff_table_s table
	, ff_polynomial_s poly, ff_t scalar)
{
	ff_polynomial_s result	= {0};
	result.memory	= calloc(poly.size, sizeof(ff_t));
	result.size	= poly.size;

	for (size_t i = 0; i < poly.size; i++)
	{
		result.memory[i]	= ff_multiply_lut(table, poly.memory[i], scalar);
	}
	
	return(result);	
}

ff_polynomial_s ff_polynomial_add(ff_table_s table
	, ff_polynomial_s poly_a, ff_polynomial_s poly_b)
{
	size_t result_length = sizeof(ff_t) * (poly_a.size > poly_b.size ? poly_a.size : poly_b.size);
	ff_polynomial_s result;
	result.memory	= calloc(result_length, sizeof(ff_t));
	result.size	= result_length;

	for (size_t i = 0; i < poly_a.size; i++)
	{
		result.memory[i + result_length - poly_a.size]	= poly_a.memory[i];
	}
	
	for (size_t i = 0; i < poly_b.size; i++)
	{
		result.memory[i + result_length - poly_b.size]	^= poly_b.memory[i];
	}
	
	return result;
}

ff_polynomial_s ff_polynomial_multiply(ff_table_s table
	, ff_polynomial_s poly_a, ff_polynomial_s poly_b)
{
	ff_polynomial_s result	= {0};
	result.size	= poly_a.size + poly_b.size - 1;
	result.memory	= calloc(result.size, sizeof(ff_t));
	
	for (size_t i = 0; i < poly_a.size; i++)
	{
		for (size_t j = 0; j < poly_b.size; j++)
		{
			result.memory[i + j]	= ff_addition(result.memory[i + j], 
				ff_multiply_lut(table, poly_a.memory[i], poly_b.memory[j]));
		}
		
	}
	
	return result;
}

ff_polynomial_s ff_append_polynomials(ff_polynomial_s poly_a
	, ff_polynomial_s poly_b)
{
	ff_polynomial_s poly_result	= {0};
	poly_result.size	= poly_a.size + poly_b.size;
	poly_result.memory	= calloc(poly_result.size, sizeof(ff_t));

	long poly_a_size	= poly_a.size * sizeof(ff_t)
		, poly_b_size	= poly_b.size * sizeof(ff_t);
	memcpy(poly_result.memory, poly_a.memory, poly_a_size);
	memcpy(poly_result.memory + poly_a_size, poly_b.memory, poly_b_size);

	return (poly_result);
}

ff_t ff_evaluate_polynomial(ff_table_s table, ff_polynomial_s poly, short x)
{
	ff_t y	= poly.memory[0];

	for (size_t i = 1; i < poly.size; i++)
	{
		y	= ff_addition(ff_multiply_lut(table, y, x), poly.memory[i]);
	}
	return (y);
}