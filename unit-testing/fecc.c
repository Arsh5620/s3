#include "tests.h"
#include "../libfecc/rs.h"
#include "../libfecc/finite-fields.h"
#include "../libfecc/polynomials.h"

int test_fecc()
{
	short prime_poly	= 285;
	// We assume that the ff_generate_table is flawless for now. 
	ff_table_s table	= ff_table_new(prime_poly);

	BEGIN_TEST;
	ASSERT_TEST_EQUALS(ff_multiply(129, 129, prime_poly), 18, "ff multiply 129 * 129");
	ASSERT_TEST_EQUALS(ff_multiply_lut(table.full_table, 129, 129), 18, "ff multiply lut assist 129 * 129");
	ASSERT_TEST_EQUALS(ff_divide_lut(table, 129, 127), 3, "ff divide lut assist 129 / 127");
	ASSERT_TEST_EQUALS(ff_divide_lut(table, 12, 115), 197, "ff divide lut assist 12 / 115");
	ASSERT_TEST_EQUALS(ff_raise_lut(table, 2, 8), 29, "ff raise lut assist 2 ^ 8");
	ASSERT_TEST_EQUALS(ff_raise_lut(table, 2, 8), 29, "ff raise lut assist 2 ^ 8");
	ASSERT_TEST_EQUALS(ff_raise_lut(table, 2, 8), 29, "ff raise lut assist 2 ^ 8");
	ff_t a_src[]	= {0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	ff_t b_src[]	= {7, 8, 9, 10, 11, 12};
	ff_t ab_src[]	= {7, 9, 11, 9, 15, 9};
	poly_s a	= {.memory = a_src, 6, 6};
	poly_s b	= {.memory = b_src, 6, 6};

	poly_s c	= poly_new(MAX(a.size, b.size));
	poly_add(table, &c, a, b);
	ASSERT_MEMTEST_EQUALS(ab_src, c.memory, sizeof(ab_src), c.size, "polynomial addition test");

	ff_t ac_src[]	= {0, 7, 6, 16, 28, 63, 8, 0, 26, 23, 60};
	c	= poly_new(a.size + b.size - 1);
	poly_multiply(table, &c, a, b);
	ASSERT_MEMTEST_EQUALS(ac_src, c.memory, sizeof(ac_src), c.size, "polynomial multiplication test");

	ff_t *acopy_src	= aligned_alloc(16, 16);
	for (size_t i = 0; i < 16; i++)
	{
		*(acopy_src + i)	= i;
	}

	poly_s a_cpy	= {.memory = acopy_src, 16, 16};
	ff_t ad_src[]	= {0, 12, 24, 20, 48, 60, 40, 36, 96, 108, 120, 116, 80, 92, 72, 68};
	poly_multiply_scalar(table, &a_cpy, 12);
	ASSERT_MEMTEST_EQUALS(ad_src, a_cpy.memory, sizeof(ad_src), a_cpy.size, "polynomial scalar mult test");

	ff_t nsrc[]	= {7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9
	, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 
	9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7,
	 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7
	 , 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 
	 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9,
	  11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9, 11, 9, 15, 9, 7, 9};
	poly_s nsrc_poly	= {.memory = nsrc, 191, 191};

	poly_s invert	= rs_invert_poly(nsrc_poly);
	ASSERT_TEST_EQUALS(31, poly_evaluate(table.full_table, invert, 42),"evaluate polynomial a @ 42");

	poly_s setup	= rs_setup_eval_poly_sse(nsrc_poly);
	ff_t retvalue = poly_evaluate_sse(&table, setup, 42);
	ASSERT_TEST_EQUALS(31, retvalue, "poly evaluate sse @ 42");

	ff_t gen_poly_val[]	= {1, 29, 196, 111, 163, 112, 74, 10, 105, 105, 139, 132, 151, 32, 134, 26};
	poly_s generator	= rs_make_generator_polynomial(table, 15);
	ASSERT_MEMTEST_EQUALS(gen_poly_val, generator.memory, sizeof(gen_poly_val), generator.size, "generator for rs @ ecc length 15");
	
	poly_s remainder	= ff_polynomial_mod(table, b, a);
	ff_t poly_mod_test[]	= {15, 7, 3, 23, 23};
	ASSERT_MEMTEST_EQUALS(poly_mod_test, remainder.memory, sizeof(poly_mod_test), remainder.size, "polynomial mod test");

	rs_encode_s encode	= rs_init_encoder(23, 15, 285);
	ff_t rs_init_memory[]	= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	memcpy(encode.message_in_buffer.memory, rs_init_memory, sizeof(rs_init_memory));

	rs_encode(&encode);
	ff_t actual_encode[]	= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 179, 193, 151, 124, 222, 249, 244, 69};
	ASSERT_MEMTEST_EQUALS(actual_encode, encode.message_out_buffer.memory
		, sizeof(actual_encode), encode.message_out_buffer.size, "message rs init test");
	
	encode.message_out_buffer.memory[3] = 0; // intentionally corrupt the data stream
	encode.message_out_buffer.memory[7] = 0; // intentionally corrupt the data stream
	encode.message_out_buffer.memory[8] = 0; // intentionally corrupt the data stream

	rs_decode_s decode	= rs_init_decoder(23, 15, 285);
	decode.message_in_buffer	= encode.message_out_buffer;

	rs_calculate_syndromes(&decode);	
	ff_t syndromes[] = {0, 12, 132, 92, 111, 248, 220, 26, 211};
	ASSERT_MEMTEST_EQUALS(syndromes, decode.syndromes.memory
		, sizeof(syndromes), decode.syndromes.size, "rs syndromes test");

	rs_make_error_location_poly(&decode);
	ff_t rs_err_locator_poly[]	= {0, 70, 89, 111, 1};
	ASSERT_MEMTEST_EQUALS(rs_err_locator_poly, decode.error_locator.memory
		, sizeof(rs_err_locator_poly), decode.error_locator.size, "rs error locator test");
	
	decode.error_locator.memory ++;
	decode.error_locator.size --;

	rs_find_error_locations(&decode);
	ff_t rs_err_locations[]	= {8, 7, 3};
	ASSERT_MEMTEST_EQUALS(rs_err_locations, decode.error_locations.memory
		, sizeof(rs_err_locations), decode.error_locations.size, "rs locations test");

	rs_correct_errors(&decode);
	ASSERT_MEMTEST_EQUALS(actual_encode, decode.message_out_buffer.memory
		, sizeof(actual_encode), decode.message_out_buffer.size, "rs error correction test");

	END_TEST;
	#include <time.h>
	#define COUNT_ITERATE 10000000
	clock_t time1 = clock();
	for (size_t i = 0; i < COUNT_ITERATE; i++)
	{
			volatile ff_t in1 = poly_evaluate(table.full_table, nsrc_poly, 42);
	}

	clock_t time2	= clock();
	printf("Time elapsed regular loop: %ld\n", time2 - time1);
	for (size_t i = 0; i < COUNT_ITERATE; i++)
	{
			volatile ff_t jn1 = poly_evaluate_sse(&table, nsrc_poly, 42);
	}

	clock_t time3	= clock();
	printf("Time elapsed SSE loop: %ld\n", time3 - time2);

	return (global_tests.failed_tests);
}