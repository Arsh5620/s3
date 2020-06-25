#include "tests.h"
#include "../libfecc/rs.h"
#include "../libfecc/finite-fields.h"

int test_fecc()
{
	short prime_poly	= 285;
	// We assume that the ff_generate_table is flawless for now. 
	ff_table_s table	= ff_generate_table(prime_poly);

	BEGIN_TEST;
	ASSERT_TEST_EQUALS(ff_multiply(129, 129, prime_poly), 18, "ff multiply 129 * 129");
	ASSERT_TEST_EQUALS(ff_multiply_lut(table, 129, 129), 18, "ff multiply lut assist 129 * 129");
	ASSERT_TEST_EQUALS(ff_divide_lut(table, 129, 127), 3, "ff divide lut assist 129 / 127");
	ASSERT_TEST_EQUALS(ff_divide_lut(table, 12, 115), 197, "ff divide lut assist 12 / 115");
	ASSERT_TEST_EQUALS(ff_raise_lut(table, 2, 8), 29, "ff raise lut assist 2 ^ 8");
	ASSERT_TEST_EQUALS(ff_raise_lut(table, 2, 8), 29, "ff raise lut assist 2 ^ 8");
	ASSERT_TEST_EQUALS(ff_raise_lut(table, 2, 8), 29, "ff raise lut assist 2 ^ 8");
	ff_t a_src[]	= {0, 1, 2, 3, 4, 5};
	ff_t b_src[]	= {7, 8, 9, 10, 11, 12};
	ff_t ab_src[]	= {7, 9, 11, 9, 15, 9};
	ff_polynomial_s a	= {.memory = a_src, 6, 6};
	ff_polynomial_s b	= {.memory = b_src, 6, 6};

	ff_polynomial_s c	= ff_polynomial_add(table, a, b);
	ASSERT_MEMTEST_EQUALS(ab_src, c.memory, sizeof(ab_src), c.size, "polynomial addition test");

	ff_t ac_src[]	= {0, 7, 6, 16, 28, 63, 8, 0, 26, 23, 60};
	c	=	ff_polynomial_multiply(table, a, b);
	ASSERT_MEMTEST_EQUALS(ac_src, c.memory, sizeof(ac_src), c.size, "polynomial multiplication test");

	ff_t acpy_src[]	= {0, 1, 2, 3, 4, 5};
	ff_polynomial_s a_cpy	= {.memory = acpy_src, 6, 6};
	ff_t ad_src[]	= {0, 12, 24, 20, 48, 60};
	ff_polynomial_multiply_scalar(table, &a_cpy, 12);
	ASSERT_MEMTEST_EQUALS(ad_src, a_cpy.memory, sizeof(ad_src), a_cpy.size, "polynomial scalar mult test");

	ASSERT_TEST_EQUALS(ff_evaluate_polynomial(table, a, 32), 225, "evaluate polynomial a @ 32");

	ff_t gen_poly_val[]	= {1, 29, 196, 111, 163, 112, 74, 10, 105, 105, 139, 132, 151, 32, 134, 26};
	ff_polynomial_s generator	= rs_make_generator_polynomial(table, 15);
	ASSERT_MEMTEST_EQUALS(gen_poly_val, generator.memory, sizeof(gen_poly_val), generator.size, "generator for rs @ ecc length 15");
	
	ff_polynomial_s remainder	= ff_polynomial_mod(table, b, a);
	ff_t poly_mod_test[]	= {15, 7, 3, 23, 23};
	ASSERT_MEMTEST_EQUALS(poly_mod_test, remainder.memory, sizeof(poly_mod_test), remainder.size, "polynomial mod test");

	rs_information_s rs_info	= rs_init(23, 15, 285);
	ff_t rs_init_memory[]	= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	memcpy(rs_info.message_in.memory, rs_init_memory, sizeof(rs_init_memory));

	rs_encode(&rs_info);
	ff_t actual_encode[]	= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 179, 193, 151, 124, 222, 249, 244, 69};
	ASSERT_MEMTEST_EQUALS(actual_encode, rs_info.message_out.memory
		, sizeof(actual_encode), rs_info.message_out.size, "message rs init test");
	
	rs_info.message_out.memory[3] = 0; // intentionally corrupt the data stream
	rs_info.message_out.memory[7] = 0; // intentionally corrupt the data stream
	rs_info.message_out.memory[8] = 0; // intentionally corrupt the data stream
	rs_calculate_syndromes(&rs_info.syndromes, table, rs_info.message_out, rs_info.field_ecc_length);	
	ff_t syndromes[] = {0, 12, 132, 92, 111, 248, 220, 26, 211};
	ASSERT_MEMTEST_EQUALS(syndromes, rs_info.syndromes.memory
		, sizeof(syndromes), rs_info.syndromes.size, "rs syndromes test");

	rs_make_error_location_poly(&rs_info);
	ff_t rs_err_locator_poly[]	= {0, 70, 89, 111, 1};
	ASSERT_MEMTEST_EQUALS(rs_err_locator_poly, rs_info.error_locator.memory
		, sizeof(rs_err_locator_poly), rs_info.error_locator.size, "rs error locator test");
	
	rs_info.error_locator.memory ++;
	rs_info.error_locator.size --;

	ff_polynomial_s locations	=
		rs_find_error_locations(rs_info.field_table, rs_info.error_locator, rs_info.message_out.size);
	ff_t rs_err_locations[]	= {8, 7, 3};
	ASSERT_MEMTEST_EQUALS(rs_err_locations, locations.memory
		, sizeof(rs_err_locations), locations.size, "rs locations test");

	ff_polynomial_s corrected	=
		rs_correct_errors(rs_info.field_table, rs_info.message_out
			, rs_info.syndromes, rs_info.error_locator, locations);
	ASSERT_MEMTEST_EQUALS(actual_encode, corrected.memory
		, sizeof(actual_encode), corrected.size, "rs error correction test");

	END_TEST;

	return (global_tests.failed_tests);
}