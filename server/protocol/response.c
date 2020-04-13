#include "protocol.h"

int dbp_response_write(dbp_response_s *response)
{
	network_s *connection	= 
		&((dbp_protocol_s*)response->instance)->connection;

	string_s header	= dbp_response_make_header(response);
	response->header_info.data_length	= response->data_string.length;
	response->header_info.magic	= DBP_PROTOCOL_MAGIC;
	
	// we have to go with the next multiple of 16
	response->header_info.header_length	= header.length;
		
	ulong i	= dbp_response_make_magic(response);

	network_write_stream(connection, (char*)&i , sizeof(ulong));
	network_write_stream(connection, header.address ,header.length);

	network_write_stream(connection
		, response->data_string.address
		, response->data_string.length);

	return(SUCCESS);
}

string_s dbp_response_make_header(dbp_response_s *response)
{
	int dbp_response_attrib_len	= sizeof(DBP_RESPONSE_FORMAT);
	ulong header_length	= dbp_response_attrib_len;
	my_list_s header_list	= response->header_list;

	for (size_t i = 0; i < header_list.count; i++)
	{
		key_value_pair_s pair	= 
			*(key_value_pair_s*)my_list_get(header_list, i);

		// for each line the length of the line is 
		// key + value + assignment-sign + 
		// (quotations-for-value + new-line + carriage-return) = 5
		header_length += (pair.key_length + pair.value_length + 5);
	}

	header_length	= header_length & 0XF 
		? (header_length + 0XF) & ~0XF 
		: header_length;

	string_s header	= { 0 };
	header.address	= header_length 
		? m_calloc(header_length, MEMORY_FILE_LINE) 
		: 0;
	header.length = header_length;

	snprintf(header.address, dbp_response_attrib_len + 1
		, DBP_RESPONSE_FORMAT, response->response_code);

	ulong index = dbp_response_attrib_len - 1;
	for (size_t i = 0; i < header_list.count; i++)
	{
		key_value_pair_s pair	= 
			*(key_value_pair_s*)my_list_get(header_list, i);

		int written	= snprintf(header.address, header.length - index
			, "%.*s=\"%.*s\"\r\n"
			, pair.key_length, pair.key
			, pair.value_length, pair.value);
		index	+= written;
	}
	return(header);
}

ulong dbp_response_make_magic(dbp_response_s *response)
{
	// first we shift the magic header by 7 bytes
	ulong i	= ((ulong)DBP_PROTOCOL_MAGIC << (7*8));
	
	// second then we divide header_length by 16 and right shift by 6 bytes
	i |= ((ulong)response->header_info.header_length >> 4) << (6*8);

	i |= (response->header_info.data_length & 0x0000FFFFFFFFFF);
	return(i);
}