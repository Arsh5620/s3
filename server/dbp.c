// device backup protocol

#include "dbp.h"
#include "defines.h"
#include "logs.h"
#include <unistd.h>
#include "./protocol/dbp_protocol.h"

dbp_s dbp_init(unsigned short port)
{
    dbp_s protocol = {0};

    logs_init();
    logs_write_printf("starting protocol initialization ...");

    protocol.connection = network_connect_init_sync(port);
    return(protocol);
}

void dbp_accept_connection_loop(dbp_s *protocol)
{
    logs_write_printf("waiting for the client to connect ...");
    while (network_connect_accept_sync(&(protocol->connection)) == SUCCESS) {
        logs_write_printf("client connected: %s : %d"
            , inet_ntoa(protocol->connection.client_socket.sin_addr)
            , ntohs(protocol->connection.client_socket.sin_port)); 

        for(;;) {
            if(dbp_read(protocol) == -1) break;
        }

        dbp_shutdown_connection(*protocol, DBP_CONNECT_SHUTDOWN_FLOW);
    }
    logs_write_printf("network_connect_accept_sync failed: %s, %d.", __FILE__, __LINE__);
}

void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason)
{
    if(close(protocol.connection.client) == 0){
        logs_write_printf("client connection closed: reason(%d)", reason);
    }
}

// returns -1 if the client is asking for the connection to be closed.
int dbp_read(dbp_s *_read)
{
    data_types_s data = network_data_read_long(&(_read->connection));
    int header_size     = dbp_magic_check(data.data_types_u._long);
    // printf("%lx is the pointer\n", data.data_u._long);
    
    if(header_size == -1) {
        // the header size we have received is 
        // not correct, possible protocol corruption.
        // end connection with the client. 
        logs_write_printf("connection closed as header invalid 0x%.16lx."
                , data.data_types_u._long);
        dbp_shutdown_connection(*_read, DBP_CONNECT_SHUTDOWN_CORRUPTION);
        return(-1);
    }

    array_list_s header_list    = dbp_headers_read(_read, header_size);
    _read->headers  = header_list;
    _read->header_magic_now  = data.data_types_u._long;

    key_value_pair_s first_record   = 
                        *(key_value_pair_s*)my_list_get(header_list, 0);

    int action = 0;
    if(header_list.index > 0)
        action = dbp_headers_action(_read, first_record);
    else
        return(-1);

    if(action == DBP_ACTION_ERR) {
        dbp_shutdown_connection(*_read, DBP_CONNECT_SHUTDOWN_CORRUPTION);
        return(action);
    }

    int handler = dbp_action_dispatch(_read,action);
    return(0);
}

int dbp_action_dispatch(dbp_s *protocol, int action)
{
    int result = 0;
    switch (action)
    {
    case ACTION_NOTIFICATION:
        result = dbp_protocol_notification(protocol);
        break;
    case ACTION_CREATE:
        result = dbp_create(protocol);
        break;
    }
    return(result);
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
    logs_cleanup();
    return;
}