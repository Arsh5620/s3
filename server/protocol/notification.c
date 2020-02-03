#include "dbp_protocol.h"

int dbp_protocol_notification(dbp_s *protocol)
{
    netconn_data_s data_read    = 
                    network_data_readxbytes(&protocol->connection
                        , dbp_data_length(protocol->header_magic_now));

    void *address = network_netconn_data_address(&data_read);

    printf("Client sent a notification: \n");
    printf("%.*s\n\n", data_read.data_length, (char*)address);

    logs_write_printf("notification: %.*s"
                            , data_read.data_length, (char*) address);

    return(0);
}

