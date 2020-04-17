#include "protocol.h"

size_t dbp_header_code_compare(void *memory, char *str, size_t strlen)
{
	dbp_header_keys_s *s    = (dbp_header_keys_s*) memory;

	int cmp = memcmp(str, s->string, strlen); // order does matter it is a - b

	if (cmp == 0 && s->strlen > strlen)
	{
		cmp--;
	}
	else if (cmp == 0 && s->strlen < strlen)
	{
		cmp++;
	}
	return(cmp);
}

ulong dbp_request_read_headers(dbp_protocol_s protocol, dbp_request_s *request)
{
	network_data_atom_s header_read	= network_read_long(&protocol.connection);
	request->header_info = dbp_header_parse8(header_read._u.long_t);
	
	if (request->header_info.magic != DBP_PROTOCOL_MAGIC)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_ABORTED_CORRUPTION
			, request->header_info.magic);
		return(DBP_RESPONSE_CORRUPTED_PACKET);
	}

	network_data_s header_raw	= network_read_stream(&protocol.connection
		, request->header_info.header_length);
	
	if (header_raw.error_code)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_READ_HEADERS_FAILED);
		return(DBP_RESPONSE_ERROR_READ);
	}

	lexer_status_s status	= {0};
	my_list_s header_list	= 
		parser_parse(&status, header_raw.data_address, header_raw.data_length);
	
	if (status.errno != 0) 
	{
		/* this means that an error occured while processing the input */
		return(DBP_RESPONSE_PARSE_ERROR);
	}
	request->header_list	= header_list;
	return(DBP_RESPONSE_SUCCESS);
}

int dbp_request_read_action(dbp_request_s *request)
{
	key_value_pair_s pair   = {0};
	my_list_s list	= request->header_list;
	if (list.count > 0)
	{
		pair	= *(key_value_pair_s*) my_list_get(list, 0);
	} 
	else 
	{
		return(DBP_RESPONSE_HEADER_EMPTY);
	}

	dbp_header_keys_s action	= attribs[0];
	int actionval	= DBP_ACTION_NOTVALID;
	if (action.strlen == pair.key_length
		&& memcmp(pair.key, action.string, action.strlen) == 0) 
	{
		// now here to check the action that the client is requesting.
		actionval	= binary_search(actions
			, sizeof(dbp_header_keys_s)
			, DBP_ACTIONS_COUNT
			, pair.value , pair.value_length
			, dbp_header_code_compare);
	}

	if (actionval  == DBP_ACTION_NOTVALID)
	{
		return(DBP_RESPONSE_ACTION_INVALID);
	}
	request->action	= actions[actionval].attrib_code;
	return(DBP_RESPONSE_SUCCESS);
}


// this function should be called before dispatching the request
// to make sure that the header contains all the required key:value 
// pairs needed by the called function.
int dbp_attribs_assert(hash_table_s table, 
	enum dbp_attribs_enum *match, int count)
{
	for (long i=0; i<count; i++)
	{
		enum dbp_attribs_enum attrib	= match[i];
		hash_input_u key	= {.number = attrib};
		hash_table_bucket_s bucket	= hash_table_get(table, key, NULL_ZERO);
		if (bucket.is_occupied == 0)
		{
			return(FALSE);
		}
	}
	return(TRUE);
}


/*
 * this function will go through the list of the key:value pairs
 * and add those key value pairs to a hash table, while making
 * sure that there are not duplicates, if duplicates are found
 * the key:value pair found later is ignored.
 */
hash_table_s dbp_header_hash(my_list_s list)
{
	int count	= list.count;
	hash_table_s table  = hash_table_init(10, 0);

	for (long i=0; i<count; ++i) 
	{
		key_value_pair_s pair	=
			*(key_value_pair_s*) my_list_get(list, i);
		
		int index	= binary_search((void*)attribs
			, sizeof(dbp_header_keys_s)
			, DBP_ATTRIBS_COUNT
			, pair.key, pair.key_length
			, dbp_header_code_compare);

		// will be -1 if an attribute is not supported (YET!)
		// which is also ignored. 
		if (index != -1) 
		{  
			dbp_header_keys_s attr  = attribs[index];

			hash_table_bucket_s b   = hash_table_get(table
				, (hash_input_u) { .address = attr.string }
				, attr.strlen);

			if (b.is_occupied)
			{
				continue;
			}

			hash_table_bucket_s bucket  = {0};
			bucket.key.number	= attr.attrib_code;
			bucket.value.number	= index;

			hash_table_add(&table, bucket);
		}
	}
	return(table);
}

dbp_header_s inline dbp_header_parse8(size_t magic)
{
	/* shr by 6 bytes, and multiply by 16 to get header size */
	dbp_header_s header	= {
		.header_length	= (((magic & 0x00FF000000000000) >> (6*8)) * 16)
		, .data_length	= (magic & 0x0000FFFFFFFFFFFF)
		, .magic	= ((magic & 0xFF00000000000000) >> (7*8))
	};

	return (header);
}