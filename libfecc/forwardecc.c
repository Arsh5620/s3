#include "forwardecc.h"
#include "rs.h"

int main()
{
	ff_t a = 100, b = 121;

	short irr_p = 285;

	ff_table_s table	= ff_generate_table(irr_p);

	ff_polynomial_s message	= {0};
	short message_size = 32;
	short number_ecc_symbols	= 8;
	message.memory	= calloc(message_size, 1);
	for(int i=1; i< message_size; ++i)
	{
		message.memory[i-1] = i;
	}
	message.size	= message_size;

	ff_polynomial_s generator	= 
		rs_make_generator_polynomial(table, number_ecc_symbols);


	ff_polynomial_s rs_gen	= rs_encode(table, message, generator);
	rs_gen.memory[0] = 123;
	rs_gen.memory[1] = 111;

	for (int kl=0; kl < 100000; kl++)
	{
	
	ff_polynomial_s syndromes	= rs_calculate_syndromes(table, rs_gen, 10);

	ff_polynomial_s error_locator	= rs_make_error_location_poly(table, syndromes, 10);
	error_locator.memory++;
	error_locator.size --;
	ff_polynomial_s error_positions	= rs_find_error_locations(table, error_locator, rs_gen.size);
	ff_polynomial_s corrected = rs_correct_errors(table, rs_gen, syndromes, error_locator, error_positions);	
	// rs_print_poly("corrected", corrected);
	}
	return (0);
}