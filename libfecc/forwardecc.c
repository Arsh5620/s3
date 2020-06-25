#include "forwardecc.h"
#include "rs.h"

int main()
{
	rs_information_s inf	= rs_init(255, 223, 285);

	for(int i=0; i< inf.message_in.size; ++i)
	{
		inf.message_in.memory[i] = i;
	}

	rs_encode(&inf);
	rs_print_poly("encoded", inf.message_out);
	inf.message_out.memory[3]=0;
	inf.message_out.memory[7]=0;
	inf.message_out.memory[8]=0;

	for (int kl=0; kl < 1000000; kl++)
	{	
		rs_calculate_syndromes(&inf.syndromes, inf.field_table, inf.message_out, inf.field_ecc_length);
		rs_make_error_location_poly(&inf);
		inf.error_locator.memory ++;
		inf.error_locator.size --;
		ff_polynomial_s error_positions	= rs_find_error_locations(inf.field_table, inf.error_locator, inf.message_out.size);
		ff_polynomial_s corrected = rs_correct_errors(inf.field_table, inf.message_out, inf.syndromes, inf.error_locator, error_positions);	
		// rs_print_poly("corrected", corrected);
	}
	return (0);
}