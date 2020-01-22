// device backup protocol

#include "dbp.h"

dbp_s dbp_init(unsigned short port)
{
    dbp_s protocol = {0};

    protocol.connection = network_connect_init_sync(port);

    return(protocol);
}