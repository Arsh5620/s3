#include "networking/network.h"

typedef struct device_backup_protocol {
    netconn_info_s connection;
} dbp_s;

enum connection_shutdown_type {
    DBP_CONNECT_SHUTDOWN_CLIENT 
    , DBP_CONNECT_SHUTDOWN_SERVER
    , DBP_CONNECT_SHUTDOWN_CORRUPTION
};

void dbp_read(dbp_s *read);
dbp_s dbp_init(unsigned short port);
void dbp_cleanup(dbp_s protocol_handle);
void dbp_accept_connection_loop(dbp_s *protocol);
