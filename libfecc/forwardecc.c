#include "forwardecc.h"
#include "rs.h"

int main()
{
	rs_encode_s inf	= rs_init_encoder(255, 223, 285);

	for(int i=0; i< inf.message_in_buffer.size; ++i)
	{
		inf.message_in_buffer.memory[i] = i;
	}

	rs_encode(&inf);
	poly_print("encoded", inf.message_out_buffer);
	inf.message_out_buffer.memory[3]=0;
	inf.message_out_buffer.memory[7]=0;
	inf.message_out_buffer.memory[8]=0;

	rs_decode_s decode	= rs_init_decoder(255, 233, 285);
	decode.message_in_buffer	= inf.message_out_buffer;
	for (int kl=0; kl < 1000000; kl++)
	{	
		rs_calculate_syndromes(&decode);
		rs_make_error_location_poly(&decode);
		decode.error_locator.memory ++;
		decode.error_locator.size --;
		rs_find_error_locations(&decode);
		rs_correct_errors(&decode);
		// poly_print("corrected", decode.message_out_buffer);
	}
	return (0);
}