// device backup protocol

#include "dbp.h"
#include "networking/defines.h"
#include "logger.h"
#include <string.h>
#include <unistd.h>
#include "binarysearch.h"

int dbp_magic_check(long magic);
long int dbp_data_length(unsigned long magic);
void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason);
int dbp_headers_action(dbp_s *_read, key_value_pair_s pair);
array_list_s dbp_headers_read(dbp_s *_read, int length);


dbp_s dbp_init(unsigned short port)
{
    dbp_s protocol = {0};

    logger_init();
    logger_write_printf("starting protocol initialization ...");

    protocol.connection = network_connect_init_sync(port);
    return(protocol);
}

void dbp_accept_connection_loop(dbp_s *protocol)
{
    logger_write_printf("waiting for the client to connect ...");
    while (network_connect_accept_sync(&(protocol->connection)) == SUCCESS) {
        logger_write_printf("client connected: %s : %d"
            , inet_ntoa(protocol->connection.client_socket.sin_addr)
            , ntohs(protocol->connection.client_socket.sin_port)); 

        for(;;) {
            if(dbp_read(protocol) == -1) break;
        }

        dbp_shutdown_connection(*protocol, DBP_CONNECT_SHUTDOWN_FLOW);
    }
    logger_write_printf("network_connect_accept_sync failed: %s, %d.", __FILE__, __LINE__);
}

void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason)
{
    if(close(protocol.connection.client) == 0){
        logger_write_printf("client connection closed: reason(%d)", reason);
    }
}

// MAKE SURE THAT THIS LIST IS SORTED

static b_search_string_s actions_supported[4] = 
    {
        {"create", 6}
        , {"notification", 12} 
        , {"request",7}
        , {"update", 6}
    };
 
// returns -1 if the client is asking for the connection to be closed.
int dbp_read(dbp_s *_read)
{
    netconn_data_basetypes_s data = network_data_read_long(&(_read->connection));
    int header_size     = dbp_magic_check(data.data_u._long);
    int packet_length   = dbp_data_length(data.data_u._long);
    // printf("%lx is the pointer\n", data.data_u._long);
    
    if(header_size == -1) {
        // the header size we have received is 
        // not correct, possible protocol corruption.
        // end connection with the client. 
        logger_write_printf("connection closed as header invalid.");
        dbp_shutdown_connection(*_read, DBP_CONNECT_SHUTDOWN_CORRUPTION);
        return(-1);
    }

    array_list_s header_list    = dbp_headers_read(_read, header_size);
    _read->headers  = header_list;

    key_value_pair_s first_record   = 
                        *(key_value_pair_s*)list_get(header_list, 0);

    int action = dbp_headers_action(_read, first_record);

    if(action == -1)
        return(action);

    
        
    for(int i=1; i<header_list.index; ++i){
        key_value_pair_s pair = *(key_value_pair_s*)list_get(header_list, i);
        logger_write_printf("%.*s : %.*s", pair.key_length, pair.key, pair.value_length, pair.value);
    }

    netconn_data_s data_read    = network_data_readxbytes(&_read->connection
                                    , dbp_data_length(data.data_u._long));

    void *address = network_netconn_data_address(&data_read);
    printf("Client sent a notification: \n");
    printf("%.*s\n\n", data_read.data_length, (char*)address);
    return(0);
}

array_list_s dbp_headers_read(dbp_s *_read, int length)
{
    netconn_data_s header    = network_data_readxbytes
                                (&(_read->connection), length);

    void *address = network_netconn_data_address(&header);
    tolowercase(address, header.data_length);

    array_list_s header_list   = string_key_value_pairs(address
                                    , header.data_length);
                                        
    return(header_list);
}

int dbp_headers_action(dbp_s *_read, key_value_pair_s pair)
{
    if(memcmp(pair.key, "action", 6) == 0){
        // now here to check the action that the client is requesting.
        int action = b_search(actions_supported
                        , sizeof(actions_supported)/sizeof(b_search_string_s)
                        , pair.value
                        , pair.value_length);

        logger_write_printf("action requested: serial code %d", action);
        if(action != -1) return(action);
    }
    
    logger_write_printf("connection corruption,"
                            " first key:value was not action.");
    dbp_shutdown_connection(*_read, DBP_CONNECT_SHUTDOWN_CORRUPTION);
    return(-1);
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
    return(magic & 0x000FFFFFFFFFFFFF);
}

void dbp_cleanup(dbp_s protocol_handle)
{
    logger_cleanup();
    return;
}