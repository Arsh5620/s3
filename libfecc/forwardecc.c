#include "forwardecc.h"
#include "rs.h"
#include <time.h>

int main()
{
	short n = 255, k = 249;
	rs_encode_s inf	= rs_init_encoder(n, k, 285);

	for(int i=0; i< inf.message_in_buffer.size; ++i)
	{
		inf.message_in_buffer.memory[i] = i;
	}

	rs_encode(&inf);
	poly_print("encoded", inf.message_out_buffer);
	
	memset(inf.message_out_buffer.memory + 1, 0,(n-k)/2);

	size_t iteration	= 1000000;
	rs_decode_s decode	= rs_init_decoder(n, k, 285);
	decode.message_in_buffer	= inf.message_out_buffer;
	clock_t clock_time1	= clock();
	for (int kl=0; kl < iteration; kl++)
	{	
		rs_decode(&decode);
		// printf("corrected code %d", decode.result);
	}
	clock_t clock_time2	= clock();
	clock_t time_taken	= clock_time2 - clock_time1;
	printf("Total time taken %d seconds, %d ms, %d micro-s\n"
		, time_taken / CLOCKS_PER_SEC
		, (time_taken % CLOCKS_PER_SEC) / 1000
		, (time_taken % 1000));
	printf("Decoding speed is almost %.3f Mbit/s\n"
		, (((float)(k * iteration)) / (1024 * 1024 * ((float)time_taken/CLOCKS_PER_SEC))) * 8);
	
	return (0);
}