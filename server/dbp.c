// device backup protocol

#include "dbp.h"
#include "networking/defines.h"
#include "logger.h"
#include "strings.h"
#include <string.h>
#include <unistd.h>

int dbp_magic_check(long magic);
void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason);


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
            dbp_read(protocol);
        }
        dbp_shutdown_connection(*protocol, DBP_CONNECT_SHUTDOWN_FLOW);
    }
    logger_write_printf("network_connect_accept_sync failed.");
}

void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason)
{
    if(shutdown(protocol.connection.client, SHUT_RDWR) == 0 
        && close(protocol.connection.client) == 0){
        logger_write_printf("client connection closed: reason(%d)", reason);
    }
}

void dbp_read(dbp_s *read)
{
    netconn_data_basetypes_s data = network_data_read_long(&(read->connection));
    int header_size = dbp_magic_check(data.data_u._long);
    if(header_size == -1) {
        // the header size we have received is 
        // not correct, possible protocol corruption.
        // end connection with the client. 
        dbp_shutdown_connection(*read, DBP_CONNECT_SHUTDOWN_CORRUPTION);
        return;
    }

    netconn_data_s header    = network_data_readxbytes
                                (&(read->connection), header_size);
    tolowercase(header.data_address, header.data_length);

    if(memcmp(header.data_address, "action", 6) == 0){
        // now here to check the action that the client is requesting.
        int action = 
    } else {
        logger_write_printf("connection corruption,"
                                " first key:value was not action.");
        dbp_shutdown_connection(*read, DBP_CONNECT_SHUTDOWN_CORRUPTION);
        return;
    }
    array_list_s header_list   = string_key_value_pairs(header.data_address
                                    , header.data_length);



    for(int i=0; i<header_list.index; ++i){
        key_value_pair_s pair = *(key_value_pair_s*)list_get(header_list, i);
        logger_write_printf("%.*s : %.*s", pair.key_length, pair.key, pair.value_length, pair.value);
    }
}

int dbp_headers_action(key_value_pair_s *pair)
{
    char *action    = pair->value;
    int length  = pair->value_length;

    
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

void dbp_cleanup(dbp_s protocol_handle)
{
    logger_cleanup();
    return;
}