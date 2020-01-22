// device backup protocol

#include "dbp.h"
#include "networking/defines.h"
#include "logger.h"
int dbp_magic_check(long magic);

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
        
        dbp_read(protocol);
    }
    logger_write_printf("network_connect_accept_sync failed.");
}

void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason)
{
    if(shutdown(protocol.connection.client, SHUT_RDWR) == 0){
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