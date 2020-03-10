#include "protocol.h"

int dbp_protocol_notification(packet_info_s *info)
{
    netconn_data_s data_read    = 
        network_data_readstream(&info->dbp->connection
        , dbp_data_length(info->header));

    void *address = network_data_address(&data_read);

    printf("client sent a notification: \n");
    printf("%.*s\n", (int)data_read.data_length, (char*)address);

    logs_write_printf("notification: %.*s"
        , data_read.data_length, (char*) address);

    return(SUCCESS);
}

