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

ff_table_s ff_generate_table(short irr_p)
{
	ff_table_s table	= {0};
	table.size	= FF_SIZE;
	
	ff_t x	= 1;
	for (size_t i = 0; i < FF_SIZE; i++)
	{
		table.antilogs[i]	= x;
		table.logs[x]	= i;

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

inline ff_t ff_multiply_lut(ff_table_s table, ff_t x, ff_t y)
{
	/*
	 * https://en.wikipedia.org/wiki/Finite_field_arithmetic#Implementation_tricks
	 */
	if (x == 0 || y == 0)
	{
		return 0;
	}
	return *(table.antilogs + (*(table.logs + x) + *(table.logs + y)));
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
			result.memory[i + j]	= FF_ADDITION(result.memory[i + j], 
				ff_multiply_lut(table, poly_a.memory[i], poly_b.memory[j]));
		}
		
	}
	
	return result;
}

inline ff_polynomial_s ff_polynomial_new(short size)
{
	ff_polynomial_s poly1 = {0};
	poly1.memory	= calloc(size, sizeof(ff_t));
	poly1.size	= size;
	return (poly1);
}

// ff_polynomial_s ff_polynomial_extend(ff_polynomial_s poly, short extend_by)
// {
// 	short new_size	= poly.size + (extend_by  * sizeof(ff_t));
// 	ff_t *memory	= realloc(poly.memory, new_size);
// 	if (memory == NULL)
// 	{	
// 		memory	= calloc(new_size, 1);
// 		memcpy(memory, poly.memory, poly.size);
// 		free(poly.memory);
// 	}
// 	poly.memory	= memory;
// 	poly.size	= new_size;

// 	return(poly);
// }

ff_polynomial_s ff_polynomial_extend(ff_polynomial_s poly, short extend_by)
{
	short new_size	= poly.size + (extend_by  * sizeof(ff_t));
	ff_polynomial_s poly2	= ff_polynomial_new(new_size);
	memcpy(poly2.memory, poly.memory, poly.size);
	
	return(poly2);
}

// ff_polynomial_s ff_append_polynomials(ff_polynomial_s poly_a
// 	, ff_polynomial_s poly_b)
// {
// 	ff_polynomial_s poly_result	= ff_polynomial_new(poly_a.size + poly_b.size);

// 	long poly_a_size	= poly_a.size * sizeof(ff_t)
// 		, poly_b_size	= poly_b.size * sizeof(ff_t);
// 	memcpy(poly_result.memory, poly_a.memory, poly_a_size);
// 	memcpy(poly_result.memory + poly_a_size, poly_b.memory, poly_b_size);

// 	return (poly_result);
// }


ff_polynomial_s ff_append_polynomials(ff_polynomial_s poly_a
	, ff_polynomial_s poly_b)
{
	if (poly_b.size > poly_a.size)
	{
		return (ff_polynomial_s){0};
	}
	memcpy(poly_a.memory + poly_a.size - poly_b.size, poly_b.memory, poly_b.size);

	return (poly_a);
}

inline ff_t ff_evaluate_polynomial(ff_table_s table, ff_polynomial_s poly, short x)
{
	ff_t *memory	= poly.memory;
	ff_t y	= memory[0];

	for (size_t i = 1; i < poly.size; i++)
	{
		ff_t lookup	= ff_multiply_lut(table, y, x);
		y	= FF_ADDITION(lookup, *(memory + i));
	}
	return (y);
}

ff_polynomial_s ff_polynomial_free(ff_polynomial_s poly)
{
	free(poly.memory);
}