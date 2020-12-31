#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./protocol/protocol.h"

int
main (int argc, char *argv[])
{
    dbp_protocol_s protocol = dbp_connection_initialize_sync (NETWORK_PORT);
    if (protocol.init_complete)
        dbp_connection_accept_loop (&protocol);
    dbp_close (protocol);
}