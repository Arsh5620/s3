#include "dbp_protocol.h"

int dbp_protocol_notification(dbp_s *protocol)
{
    //     for(int i=1; i<header_list.index; ++i){
    //     key_value_pair_s pair = *(key_value_pair_s*)list_get(header_list, i);
    //     logger_write_printf("%.*s : %.*s", pair.key_length, pair.key, pair.value_length, pair.value);
    // }

    netconn_data_s data_read    = 
                    network_data_readxbytes(&protocol->connection
                        , dbp_data_length(protocol->header_magic_now));

    void *address = network_netconn_data_address(&data_read);

    printf("Client sent a notification: \n");
    printf("%.*s\n\n", data_read.data_length, (char*)address);
    
    return(0);
}

