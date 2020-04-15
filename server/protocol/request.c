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

ulong dbp_request_readheaders(dbp_protocol_s protocol, dbp_request_s *request)
{
	network_data_atom_s header_read	= network_read_long(&protocol.connection);
	dbp_header_s header_1 = {0};

	long magic	= header_read._u.long_t;
	header_1.data_length	= dbp_data_length(magic);
	header_1.header_length	= dbp_header_length(magic);
	header_1.magic	= dbp_header_magic(magic);

	request->header_info = header_1;
	
	if (header_1.magic != DBP_PROTOCOL_MAGIC)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_ABORTED_CORRUPTION
			, header_1.magic);
		return(DBP_CONNECTION_ERROR_CORRUPTED_PACKET);
	}

	network_data_s header_raw	= 
		network_read_stream(&protocol.connection, header_1.header_length);
	
	if (header_raw.error_code)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_READ_HEADERS_FAILED);
		return(DBP_CONNECTION_ERROR_READ);
	}

	lexer_status_s status	= {0};
	my_list_s header_list	= 
		parser_parse(&status, header_raw.data_address, header_raw.data_length);
	
	if (status.errno != 0) 
	{
		/* this means that an error occured while processing the input */
		return(DBP_CONNECTION_WARN_PARSE_ERROR);
	}
	request->header_list	= header_list;
	return(DBP_CONNECTION_NOERROR);
}

int dbp_read_action(dbp_request_s *request)
{
	key_value_pair_s pair   = {0};
	my_list_s list	= request->header_list;
	if (list.count > 0)
	{
		pair	= *(key_value_pair_s*) my_list_get(list, 0);
	} 
	else 
	{
		return(DBP_CONNECTION_WARN_HEADER_EMPTY);
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
		actionval	= actions[actionval].attrib_code;
	}

	if (actionval  == DBP_ACTION_NOTVALID)
	{
		return(DBP_CONNECTION_WARN_ACTION_INVALID);
	}
	request->action	= actionval;
	return(DBP_CONNECTION_NOERROR);
}


// this function should be called before dispatching the request
// to make sure that the header contains all the required key:value 
// pairs needed by the called function.
int dbp_list_assert(hash_table_s table, 
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
hash_table_s dbp_headers_make_table(my_list_s list)
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

inline short dbp_header_length(size_t magic)
{
	/* shr by 6 bytes, and multiply by 16 to get header size */
	return(((magic & 0x00FF000000000000) >> (6*8)) * 16);
}

inline char dbp_header_magic(size_t magic)
{
	/* shr by 7 to get the magic byte */
	return((magic & 0xFF00000000000000) >> (7*8));
}

inline size_t dbp_data_length(size_t magic)
{
	/* the last 6 bytes are used to store the size of the data */
	return(magic & 0x0000FFFFFFFFFFFF);
}