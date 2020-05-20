import galios

prime = 0x11d
galios.init_tables(prime)
#
# msg_in = [0x40, 0xd2, 0x75, 0x47, 0x76, 0x17, 0x32, 0x06, 0x27, 0x26, 0x96, 0xc6, 0xc6, 0x96, 0x70, 0xec]
# iiii = galios.rs_encode_msg(msg_in, 2)
# for i in range(0, len(iiii)):
# 	print(hex(iiii[i]), end=' ')
#
# result = galios.rs_calc_syndromes(iiii, 2)
# print(str(result))
#
# msg_in[0] = 0
# err = galios.rs_find_errata_locator(iiii, 10)

for i in range(1, 255):
	print(str(galios.gf_pow_lut(2, i)));

