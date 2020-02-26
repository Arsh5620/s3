// device backup protocol

#include "dbp.h"
#include "defines.h"
#include "logs.h"
#include <unistd.h>
#include "./protocol/dbp_protocol.h"
#include "databases/database.h"
#include "config.h"

dbp_s dbp_init(unsigned short port)
{
    dbp_s protocol = {0};

    logs_init();
    logs_write_printf("starting protocol initialization ...");

    protocol.connection = network_connect_init_sync(port);

    database_connection_s connect_info  = 
        config_parse_dbc("CONFIGFORMAT");
        
    if(database_init(connect_info) == DATABASE_SETUP_COMPLETE
        && database_verify_integrity() == MYSQL_SUCCESS) {
        protocol.is_init = TRUE;
    }
    return(protocol);
}

void dbp_accept_connection_loop(dbp_s *protocol)
{
    logs_write_printf("waiting for the client to connect ...");
    while (SUCCESS == 
        network_connect_accept_sync(&(protocol->connection))) {

        logs_write_printf("client connected: {%s(%d)}"
            , inet_ntoa(protocol->connection.client_socket.sin_addr)
            , ntohs(protocol->connection.client_socket.sin_port)); 

        for(;;) {
            if(dbp_read(protocol)) break;
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

// returns 0 for success, any other number for error or conn close request
int dbp_read(dbp_s *protocol)
{
    data_types_s data   = network_data_read_long(&(protocol->connection));
    int header_size = dbp_magic_check(data.data_types_u._long);
    // printf("%lx is the pointer\n", data.data_u._long);
    
    if(header_size == -1) {
        // the header size we have received is 
        // not correct, possible protocol corruption.
        // end connection with the client. 
        logs_write_printf("connection closed as possible"
            " corruption detected(header: 0x%.16lx)."
            , data.data_types_u._long);
        
        return(DBP_CONNEND_CORRUPT);
    }

    array_list_s header_list    = dbp_headers_read(protocol, header_size);
    protocol->headers   = header_list;
    protocol->header_magic_now  = data.data_types_u._long;

    int action = 0;
    key_value_pair_s pair   = {0};
    if(header_list.index > 0){
        pair    = *(key_value_pair_s*)my_list_get(header_list, 0);
        action  = dbp_headers_action(protocol, pair);
    }
    else
        return(DBP_CONN_EMPTYPACKET);

    if(action == DBP_ACTION_NOID)
        return(DBP_CONN_INVALID_ACTION);

    int handler = dbp_action_dispatch(protocol, action);
    return(handler);
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

array_list_s dbp_headers_read(dbp_s *protocol, int length)
{
    netconn_data_s header   = 
        network_data_readxbytes(&protocol->connection, length);
    void *address = network_netconn_data_address(&header);
    array_list_s header_list    = 
        parser_parse(address, header.data_length);
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
    return(magic & 0x0000FFFFFFFFFFFF);
}

void dbp_cleanup(dbp_s protocol_handle)
{
    logs_cleanup();
    return;
}