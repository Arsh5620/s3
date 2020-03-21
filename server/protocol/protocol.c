#include <string.h>
#include "protocol.h"

// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

static b_search_string_s attribs[] = 
{
    {"action", 6, .code = ATTRIB_ACTION}
    , {"crc", 3, .code = ATTRIB_CRC}
    , {"filename", 8, .code = ATTRIB_FILENAME}
};

static b_search_string_s actions[] = 
{
    {"create", 6, .code = ACTION_CREATE}
    , {"notification", 12, .code = ACTION_NOTIFICATION} 
    , {"request", 7, .code = ACTION_REQUEST}
    , {"update", 6, .code = ACTION_UPDATE}
};

static int call_asserts[][4] = {
    {ATTRIB_ACTION, ATTRIB_CRC, ATTRIB_FILENAME} // ACTION_CREATE
    , {ATTRIB_ACTION} // ACTION_NOTIFICATION
    , {ATTRIB_ACTION} // ACTION_REQUEST
    , {ATTRIB_ACTION} // ACTION_UPDATE
};


int dbp_action_dispatch(packet_info_s info)
{
    int assert  = dbp_assert_list(info.header_list, attribs
        , sizeof(attribs) / sizeof(b_search_string_s)
        , call_asserts[info.action]
        , sizeof(call_asserts[0]) / sizeof(int));
    
    if (assert == 0)
    {
        return(DBP_CONN_INSUFFICENT_ATTRIB);
    }
    
	//right after we have confirmed that all the required attribs
	//are present for the function call to succeed, we can parse them
	//to the structure for dbp_common_attribs_s
    dbp_common_attribs_s attribs = dbp_attribs_parse_all(info);
	info.attribs	= attribs;

    int result = 0;
    switch (info.action)
    {
    case ACTION_NOTIFICATION:
        result = dbp_protocol_notification(&info);
        break;
    case ACTION_CREATE:
        result = dbp_create(&info);
        break;
    }
    return(result);
}

// this function should be called before dispatching the request
// to make sure that the header contains all the required key:value 
// pairs needed by the called function.
int dbp_assert_list(array_list_s list, 
    b_search_string_s *codes, int code_length, 
    int *match, int match_length)
{
    int finds[match_length];
    memset(finds, 0, sizeof(finds));

    for(long i=0; i<list.index; ++i) {
        key_value_pair_s pair   = 
            *(key_value_pair_s*) my_list_get(list, i);
        int index  =  b_search(codes, code_length
                        , pair.key, pair.key_length);
        b_search_string_s node  = codes[index];
        for(long j=0; j<match_length; ++j)
        {
            if(node.code == match[j]) {
                finds[j]    = 1;
                break;
            }
        }
    }

    char is_found   = 1;
    for (size_t i = 0; i < match_length; i++)
    {
        if(finds[i] == 0 && match[i] != 0) {
            is_found    = 0;
            break;
        }
    }

    return(is_found);
}


/*
 * this function will go through the list of the key:value pairs
 * and add those key value pairs to a hash table, while making
 * sure that there are not duplicates, if duplicates are found
 * the key:value pair found later is ignored.
 */
hash_table_s dbp_attribs_find(packet_info_s info)
{
    array_list_s list   = info.header_list;
    int length  = list.index;

    hash_table_s table  = hash_table_inits();

    for(int i=0; i<length; ++i) {
        key_value_pair_s pair   =
            *(key_value_pair_s*)my_list_get(list, i);
        
        int attrib  =  b_search(attribs
                        , sizeof(attribs) 
                        / sizeof(b_search_string_s)
                        , pair.key, pair.key_length);

		// will be -1 if an attribute is not supported (YET!)
		// which is also ignored. 
        if(attrib != -1) {  
            b_search_string_s attr  = attribs[attrib];
            hash_table_bucket_s b   = 
                hash_table_get(table, attr.string, attr.strlen);
            if(b.is_occupied){
				//ignore and continue to the next pair
				continue;
            }
            hash_table_bucket_s b1  = {0};
            b1.key  = attr.string;
            b1.key_len  = attr.strlen;
            b1.value    = pair.value;
			b1.value_len	= pair.value_length;

            hash_table_add(&table, b1);
        }
    }
	return(table);
}

/*
 * this function will try to parse as many attributes it can from
 * the hash table of attributes that we can use, ignoring all non-recognized
 * attributes, and if any error occurs, it will stop processing and 
 * set the error flag accordingly. 
 */
dbp_common_attribs_s dbp_attribs_parse_all(packet_info_s info)
{
    dbp_common_attribs_s attributes = {0};
    hash_table_s table  = dbp_attribs_find(info);

    for(size_t i=0; i<sizeof(attribs) / sizeof(b_search_string_s); ++i) {
        b_search_string_s attrib	= attribs[i];
		hash_table_bucket_s bucket	= 
			hash_table_get(table, attrib.string, attrib.strlen);

        switch (attrib.code)
        {
		case -1:
			//ignore such as "action"
			break;
        case ATTRIB_FILENAME:
            if(bucket.value_len > FILE_NAME_MAXLENGTH 
                || bucket.value_len <= 0)
                attributes.error   = DBP_ATTRIBS_ERR_NAMETOOLONG;
            else {
                attributes.filename.address = bucket.value;
                attributes.filename.length  = bucket.value_len;
            }
            break;
        case ATTRIB_CRC: 
            if(bucket.value_len != 8)
                attributes.filename.error   =
                            DBP_ATTRIBS_ERR_CRC32NOTFOUND;
			else 
            	attributes.crc32	= *(unsigned int*) bucket.value;
            break;
        }
    }
    return(attributes);
}

packet_info_s dbp_read_headers(dbp_s *protocol, long header_magic)
{
	packet_info_s info  = {0};
    int header_size = dbp_magic_check(header_magic);
    // printf("%lx is the pointer\n", data.data_u._long);
    
    if(header_size < 0) {
        // the header size we have received is 
        // not correct, possible protocol corruption.
        // end connection with the client. 
        logs_write_printf("connection closed as possible"
            " corruption detected(header: 0x%.16lx)."
            , header_magic);

        info.error  = DBP_CONNEND_CORRUPT;
        return(info);
    }

    netconn_data_s header   = 
        network_data_readstream(&protocol->connection, header_size);
    char *address = network_data_address(&header);
    array_list_s header_list    = 
        parser_parse(address, header.data_length);
        
    info.action = -1;
    
    key_value_pair_s pair   = {0};
    if(header_list.index > 0){
        pair    = *(key_value_pair_s*)my_list_get(header_list, 0);
	    info.header = header_magic;
	    info.header_list    = header_list;
    } else {
        info.error  = DBP_CONN_EMPTYPACKET;
        return(info);
    }

    if(memcmp(pair.key, attribs[0].string, attribs[0].strlen) == 0) {
        // now here to check the action that the client is requesting.
        info.action = b_search(actions
            , sizeof(actions)/sizeof(b_search_string_s)
            , pair.value
            , pair.value_length);
    }
    if(info.action  == -1) {
        info.error  = DBP_CONN_INVALID_ACTION;
    }
    return(info);
}

/**
 * return: will return "header-size" or "-1" for failure, see "defines.h"
 */
int dbp_magic_check(long magic)
{
    if(((magic >> 56)& 0xD0) == 0xD0) {
        return (magic>>48) & 0xFF0;
    }
    return (-1);
}

long int dbp_data_length(unsigned long magic)
{
    return(magic & 0x0000FFFFFFFFFFFF);
}