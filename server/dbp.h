# ifndef DBP_INCLUDE_GAURD
# define DBP_INCLUDE_GAURD
#include "networking/network.h"
#include "strings.h"
#include "list.h"
#include "parser/parser.h"

#define DBP_CONN_EMPTYPACKET    0
#define DBP_CONNEND_FLOW    1
#define DBP_CONNEND_CORRUPT 2
#define DBP_CONN_INVALID_ACTION 3

typedef struct {
    netconn_info_s connection;
    char is_init;
} dbp_s; // device backup protocol

typedef struct {
    long header;
    array_list_s header_list;
    int action;
    int error;
    dbp_s *dbp;
} packet_info_s;

enum connection_shutdown_type {
    DBP_CONNECT_SHUTDOWN_FLOW
    , DBP_CONNECT_SHUTDOWN_CORRUPTION
};

int dbp_next(dbp_s *protocol);
dbp_s dbp_init(unsigned short port);
void dbp_cleanup(dbp_s protocol_handle);
void dbp_accept_connection_loop(dbp_s *protocol);

// internal functions -- should not be used outside. 

void dbp_shutdown_connection(dbp_s protocol 
    , enum connection_shutdown_type reason);
int dbp_action_dispatch(packet_info_s info);
packet_info_s dbp_read_headers(dbp_s *protocol, long header_magic);

#endif