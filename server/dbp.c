// device backup protocol

#include <unistd.h>

#include "./dbp.h"
#include "./general/defines.h"
#include "./protocol/protocol.h"
#include "./config/config.h"

dbp_s dbp_init(unsigned short port)
{
    dbp_s protocol = {0};
    protocol.logs = logs_init();
    logs_write_printf("starting protocol initialization ...");

    protocol.connection = network_connect_init_sync(port);
    database_connection_s connect_info  = config_parse_dbc("config.a");
        
    if(database_init(connect_info) == DATABASE_SETUP_COMPLETE
        && database_verify_integrity() == MYSQL_SUCCESS) {
        protocol.is_init = TRUE;
    } else {
        fprintf(stderr, "Failed to setup database connection"
            ", refer to logs for more information.");
        exit(SERVER_DATABASE_FAILURE);
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
            int read_v	= dbp_next(protocol);
			printf("read_v %d\n", read_v);
            if(read_v) break;
        }

        dbp_shutdown_connection(*protocol, DBP_CONNECT_SHUTDOWN_FLOW);
    }
    logs_write_printf("network_connect_accept_sync failed: %s, %d."
        , __FILE__, __LINE__);
}

void dbp_shutdown_connection(dbp_s protocol
	, enum connection_shutdown_type reason)
{
    if(close(protocol.connection.client) == 0)
        logs_write_printf("client connection closed: reason(%d)", reason);
}

// returns 0 for no-error, any other number for error or conn close request
int dbp_next(dbp_s *protocol)
{
	
    packet_info_s info	= dbp_read_headers(protocol);
    info.dbp    = protocol;
	if(info.error) {
		return(info.error);
	}
    int handler = dbp_action_dispatch(info);
    return(handler);
}

void dbp_cleanup(dbp_s protocol_handle)
{
    logs_cleanup();
    return;
}