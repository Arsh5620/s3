#include <string.h>
#include <time.h>
#include <unistd.h>
#include "protocol.h"

// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

static key_code_pair_s attribs[] = 
{
    {"action", 6, .code = ATTRIB_ACTION}
    , {"crc", 3, .code = ATTRIB_CRC}
    , {"filename", 8, .code = ATTRIB_FILENAME}
    , {"folder", 6, .code = ATTRIB_FOLDER}
};

static key_code_pair_s actions[] = 
{
    {"create", 6, .code = ACTION_CREATE}
    , {"notification", 12, .code = ACTION_NOTIFICATION} 
    , {"request", 7, .code = ACTION_REQUEST}
    , {"update", 6, .code = ACTION_UPDATE}
};

static enum attrib_supported_enum call_asserts[]
[sizeof(attribs) / sizeof(enum attrib_supported_enum)] = {
    {ATTRIB_ACTION, ATTRIB_CRC, ATTRIB_FILENAME, ATTRIB_FOLDER} // ACTION_CREATE
    , {ATTRIB_ACTION, 0, 0} // ACTION_NOTIFICATION
    , {ATTRIB_ACTION, 0, 0} // ACTION_REQUEST
    , {ATTRIB_ACTION, 0, 0} // ACTION_UPDATE
};

dbp_s dbp_init(unsigned short port)
{
    dbp_s protocol = {0};
    protocol.logs = logs_open();
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, PROTOCOL_LOG_INIT_COMPLETE);
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, PROTOCOL_NETWORK_SBS_INIT);

    protocol.connection = network_connect_init_sync(port);
    database_connection_s connect_info  = 
		config_parse_dbc(DBP_CONFIG_FILENAME);

    error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
		, PROTOCOL_MYSQL_LOGIN_INFO_SISS
        , connect_info.host, connect_info.port
		, connect_info.user
        , connect_info.db);
        
    if(database_init(connect_info) == MYSQL_SUCCESS) {
        protocol.is_init = TRUE;
    } else {
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_CATASTROPHIC
			, PROTOCOL_MYSQL_FAILED_CONNECT);
        exit(SERVER_DATABASE_FAILURE);
    }
    return(protocol);
}

void dbp_accept_connection_loop(dbp_s *protocol)
{
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		,PROTOCOL_NETWORK_WAIT_CONNECT);
		
    while (SUCCESS == 
        network_connect_accept_sync(&(protocol->connection))){ 

		error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, PROTOCOL_NETWORK_CLIENT_CONNECT_SI
		    , inet_ntoa(protocol->connection.client_socket.sin_addr)
            , ntohs(protocol->connection.client_socket.sin_port)); 

		int shutdown	= DBP_CONNECT_SHUTDOWN_FLOW;
        for(;;) {
            int ret	= dbp_next(protocol);
			printf("dbp_next returned value: %d\n", ret);
            if(ret){
				if(ret != DBP_CONNEND_FLOW)
					shutdown	= DBP_CONNECT_SHUTDOWN_CORRUPTION;
				break;
			} 
        }

        dbp_shutdown_connection(*protocol, shutdown);
    }
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, PROTOCOL_SERVER_SHUTDOWN);
}

void dbp_shutdown_connection(dbp_s protocol
	, enum connection_shutdown_type type)
{
    if(close(protocol.connection.client) == 0) {
		char *reason;
		if(type == DBP_CONNECT_SHUTDOWN_FLOW)
			reason	= PROTOCOL_SHUTDOWN_REASON_FLOW;
		else if(type == DBP_CONNECT_SHUTDOWN_CORRUPTION)
			reason	= PROTOCOL_SHUTDOWN_REASON_CORRUPT;
		else
			reason	= PROTOCOL_SHUTDOWN_REASON_UNKNOWN;
		
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, PROTOCOL_CLIENT_CONNECT_ABORTED_S, reason);
	}
}

void dbp_cleanup(dbp_s protocol_handle)
{
    logs_close();
    return;
}

// returns 0 for no-error, any other number for error or conn close request
int dbp_next(dbp_s *protocol)
{
    packet_info_s info	= dbp_read_headers(protocol);
    info.dbp    = protocol;
	if(info.error) {
		return(info.error);
	}
    info.header_table   = dbp_attribs_hash_table(info);

    int assert  = dbp_assert_list(info.header_list
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

    int prehook = dbp_action_prehook(&info);
    if(prehook)
        return(prehook);

    if(info.header.data_length > 0) {
        if(dbp_setup_download_env() == SUCCESS) {
            info.data_written   = create_download_file(&info);
        } else
            return DBP_CONN_SETUP_ENV_FAILED;
    }

    int posthook = dbp_action_posthook(&info);
    // file_delete(info.data_written.filename.address);
	printf("this is the output");
	
	if(info.response.dbp_response)
		dbp_response_write(&info);
    m_free(info.data_written.filename.address, MEMORY_FILE_LINE);
    return(posthook);
}

int dbp_response_write(packet_info_s *info)
{
	printf("this is the output");
	switch (info->response.dbp_response)
	{
	case DBP_RESPONSE_ACK:
		network_connection_write(&info->dbp->connection, "hi, how are you. ", 12);
		break;
	
	default:
		break;
	}
}

file_write_s create_download_file(packet_info_s *info)
{
    static int counter = 0;

    char *temp_file;
    file_write_s fileinfo  =  {0};
    fileinfo.size = info->header.data_length;

    long length  = strings_sprintf(&temp_file, DBP_TEMP_FILE_FORMAT
        , DBP_FILE_TEMP_DIR
        , ++counter);
    
    FILE *temp  = fopen(temp_file, FILE_MODE_WRITEONLY);

    if(temp == NULL || length <= 0) {
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_ERROR
			, PROTOCOL_DOWNLOAD_FILE_NOOPEN);
        fileinfo.size   = -1;
        return(fileinfo);
    }
    fileinfo.filename.address   = temp_file;
    fileinfo.filename.length    = length;

    clock_t starttime = clock();
    int download_status   = 
        file_download(temp, &info->dbp->connection, &fileinfo);
    clock_t endtime = clock();

    double time_elapsed = 
        (((double)(endtime - starttime)) / CLOCKS_PER_SEC) * 1000;

    // speed is download size in bytes / 1MB 
    // * number of times we can download this file in seconds
    double speed = (((double)fileinfo.size / 1024 / 128) 
        * (1000 / time_elapsed));

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
		, PROTOCOL_DOWNLOAD_COMPLETE_ISIIFF
		, info->attribs.filename.length 
        , info->attribs.filename.address 
		, fileinfo.size , download_status
        , time_elapsed , speed);
    fclose(temp);
    return(fileinfo);
}

int dbp_setup_download_env()
{
    // first make sure the temporary file directory exists. 
    int result  = file_dir_mkine(DBP_FILE_TEMP_DIR);
    if(result != FILE_DIR_EXISTS)
    {
        error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
			, PROTOCOL_SETUP_ENV_DIR_PERMISSIONS_S
			, DBP_FILE_TEMP_DIR);
        return(FAILED);
    }
    return(SUCCESS);
}

int dbp_action_posthook(packet_info_s *info)
{
    int result = 0;
    switch(info->action)
    {
        case ACTION_NOTIFICATION:
            result  = dbp_notification_posthook(info);
            break;
        case ACTION_CREATE:
            return(SUCCESS);
            break;
    }
    return(result);
}

int dbp_action_prehook(packet_info_s *info)
{
    int result = 0;
    switch (info->action)
    {
        case ACTION_NOTIFICATION:
            result = dbp_notification_prehook(info);
            break;
        case ACTION_CREATE:
            result = dbp_create_prehook(info);
            break;
    }
    return(result);
}

// this function should be called before dispatching the request
// to make sure that the header contains all the required key:value 
// pairs needed by the called function.
int dbp_assert_list(array_list_s list, 
    enum attrib_supported_enum *match, int match_length)
{
    int finds[match_length];
    memset(finds, 0, sizeof(finds));

    for(long i=0; i<list.index; ++i) {
        key_value_pair_s pair   = 
            *(key_value_pair_s*) my_list_get(list, i);

        int index  =  binary_search((void*) attribs
            , sizeof(key_code_pair_s)
            , sizeof(attribs) / sizeof(key_code_pair_s)
            , pair.key, pair.key_length
            , binary_search_kc_cmp);

        key_code_pair_s node  = attribs[index];

        for(long j=0; j<match_length; ++j)
        {
            if(node.code == match[j]) {
                finds[j]    = 1;
                break;
            }
        }
    }

    char is_found   = TRUE;
    for (size_t i = 0; i < match_length; i++)
    {
        if(finds[i] == 0 && match[i] != 0) {
            is_found    = FALSE;
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
hash_table_s dbp_attribs_hash_table(packet_info_s info)
{
    array_list_s list   = info.header_list;
    int length  = list.index;

    hash_table_s table  = hash_table_inits();

    for(int i=0; i<length; ++i) {
        key_value_pair_s pair   =
            *(key_value_pair_s*)my_list_get(list, i);
        
        int attrib  =  binary_search((void*) attribs
            , sizeof(key_code_pair_s)
            , sizeof(attribs) / sizeof(key_code_pair_s)
            , pair.key, pair.key_length
            , binary_search_kc_cmp);

		// will be -1 if an attribute is not supported (YET!)
		// which is also ignored. 
        if(attrib != -1) {  
            key_code_pair_s attr  = attribs[attrib];
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
    hash_table_s table  = info.header_table;

    for(size_t i=0; i<sizeof(attribs) / sizeof(key_code_pair_s); ++i) {
        key_code_pair_s attrib	= attribs[i];
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
		case ATTRIB_FOLDER:
            if(bucket.value_len > FILE_NAME_MAXLENGTH 
                || bucket.value_len <= 0)
                attributes.error   = DBP_ATTRIBS_ERR_NAMETOOLONG;
            else {
                attributes.folder_name.address = bucket.value;
                attributes.folder_name.length  = bucket.value_len;
            }
            break;
        case ATTRIB_CRC: 
            if(bucket.value_len != 8)
                attributes.error   =
                            DBP_ATTRIBS_ERR_CRC32NOTFOUND;
			else 
            	attributes.crc32	= *(unsigned int*) bucket.value;
            break;
        }
    }
    return(attributes);
}

packet_info_s dbp_read_headers(dbp_s *protocol)
{
	packet_info_s info  = {0};
    dbp_header_s header = {0};

    data_types_s data   = network_data_read_long(&protocol->connection);
	long magic	= data.data_types_u._long;
    header.data_length  = dbp_data_length(magic);
    header.header_length    = dbp_header_length(magic);
    header.magic    = dbp_header_magic(magic);

    info.header = header;
    
    if(header.magic != DBP_PROTOCOL_MAGIC) {
        // the header size we have received is 
        // not correct, possible protocol corruption.
        // end connection with the client. 
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_INFO
        	, PROTOCOL_ABORTED_CORRUPTION_L
			, header.magic);
        info.error  = DBP_CONNEND_CORRUPT;
        return(info);
    }

    netconn_data_s header1   = 
        network_data_readstream(&protocol->connection, header.header_length);

    char *address = network_data_address(&header1);
    array_list_s header_list    = 
        parser_parse(address, header1.data_length);
        
    info.action = -1;
    
    key_value_pair_s pair   = {0};
    if(header_list.index > 0){
        pair    = *(key_value_pair_s*) my_list_get(header_list, 0);
	    info.header_list    = header_list;
    } else {
        info.error  = DBP_CONN_EMPTYPACKET;
        return(info);
    }

    if(memcmp(pair.key, attribs[0].string, attribs[0].strlen) == 0) {
        // now here to check the action that the client is requesting.
        info.action = binary_search(actions
            , sizeof(key_code_pair_s)
            , sizeof(actions) / sizeof(key_code_pair_s)
            , pair.value , pair.value_length
            , binary_search_kc_cmp);
    }

    if(info.action  == -1) {
        info.error  = DBP_CONN_INVALID_ACTION;
    }
    return(info);
}

// for the dbp header length the actual value is the header length from 
// client multiplied by 16
inline short dbp_header_length(size_t magic)
{
    return(((magic & 0x00FF000000000000) >> (6*8)) * 16);
}

inline char dbp_header_magic(size_t magic)
{
    return((magic & 0xFF00000000000000) >> (7*8));
}

inline size_t dbp_data_length(size_t magic)
{
    return(magic & 0x0000FFFFFFFFFFFF);
}