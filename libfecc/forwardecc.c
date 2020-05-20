#include "forwardecc.h"
#include "rs.h"

int main()
{
	ff_t a = 100, b = 121;
	short irr_p = 285;

	printf("Addition, %d + %d = %d\n", a, b, ff_addition(a, b));
	printf("Subtraction, %d - %d = %d\n", a, b, ff_subtraction(a, b));
	printf("Multiplication, %d * %d = %d\n", a, b, ff_multiply(a, b, irr_p));

	ff_table_s table	= ff_generate_table(irr_p);

	printf("Division by LUT, %d / %d = %d\n", a, b, ff_divide_lut(table, a, b, NULL));

	ff_polynomial_s message	= {0};
	message.memory	= "\x40\xd2\x75\x47\x76\x17\x32\x06\x27\x26\x96\xc6\xc6\x96\x70\xec";
	message.size	= strlen(message.memory);

	ff_polynomial_s rs_gen	= rs_encode(table, message, 10);

	rs_gen.memory[0]	= 0;
	ff_polynomial_s syndromes	= rs_calculate_syndromes(table, rs_gen, 10);


	for (size_t i = 0; i < rs_gen.size; i++)
	{
		printf("%x, ", rs_gen.memory[i]);
	}
	puts("");
	for (size_t i = 0; i < syndromes.size; i++)
	{
		printf("%x, ", syndromes.memory[i]);
	}
}