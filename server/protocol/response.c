#include "protocol.h"

int dbp_response_write(dbp_response_s *response)
{
	network_s *connection	= 
		&((dbp_protocol_s*)response->instance)->connection;
	ulong header_len	= dbp_response_header_length(response);
	ulong size	= sizeof(ulong) + header_len + response->data_string.length;

	response->header_info.data_length	= response->data_string.length;
	response->header_info.magic	= DBP_PROTOCOL_MAGIC;
	// we have to go with the next multiple of 16
	response->header_info.header_length	= header_len;
		
	ulong i	= dbp_response_make_magic(response);

	char *address	= m_calloc(size, MEMORY_FILE_LINE);
	memcpy(address, (char*)&i, sizeof(ulong));
	dbp_response_make_header(response, address + sizeof(ulong), header_len);
	memcpy(address + header_len + sizeof(ulong)
		, response->data_string.address
		, response->data_string.length);
	network_write_stream(connection, address, size);
	m_free(address, MEMORY_FILE_LINE);
	return(SUCCESS);
}

ulong dbp_response_header_length(dbp_response_s *response)
{
	ulong header_length	= sizeof(DBP_RESPONSE_FORMAT);
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
	return(header_length);
}

void dbp_response_make_header(dbp_response_s *response
	, char *buffer, ulong header_length)
{
	ulong dbp_response_len	= sizeof(DBP_RESPONSE_FORMAT);
	if (buffer)
	{
		snprintf(buffer, dbp_response_len + 1
			, DBP_RESPONSE_FORMAT, response->response_code);
	
		ulong index = dbp_response_len - 1;
		for (size_t i = 0; i < response->header_list.count; i++)
		{
			key_value_pair_s pair	= 
				*(key_value_pair_s*)my_list_get(response->header_list, i);

			int written	= snprintf(buffer + index, header_length - index
				, "%.*s=\"%.*s\"\r\n"
				, pair.key_length, pair.key
				, pair.value_length, pair.value);
			index	+= written;
		}
	}
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