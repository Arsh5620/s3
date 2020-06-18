#include "forwardecc.h"
#include "rs.h"

int main()
{
	ff_t a = 100, b = 121;
	short irr_p = 285;
	ff_table_s table	= ff_generate_table(irr_p);
	ff_polynomial_s message	= {0};
	message.memory	= (ff_t*)"\x1\x2\x3\x4\x5\x6\x7\x8\x9\xB\xC\xD\xE\xF";
	message.size	= strlen((char*)message.memory);

	ff_polynomial_s rs_gen	= rs_encode(table, message, 10);

	rs_gen.memory[0]	= 0;
	rs_gen.memory[4] 	= 0;
	rs_gen.memory[7] 	= 0;
	rs_gen.memory[9] 	= 0;
	ff_polynomial_s syndromes	= rs_calculate_syndromes(table, rs_gen, 10);

	ff_polynomial_s error_locator	= rs_make_error_location_poly(table, syndromes, 10);
	error_locator.memory ++;
	error_locator.size --;
	ff_polynomial_s error_positions	= rs_find_error_locations(table, error_locator, rs_gen.size);
	ff_polynomial_s corrected = rs_correct_errors(table, rs_gen, syndromes, error_locator, error_positions);	
	rs_print_poly("corrected", corrected);
	return (0);
}