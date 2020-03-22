# ifndef DBP_INCLUDE_GAURD
# define DBP_INCLUDE_GAURD

#include "./networking/network.h"
#include "./general/strings.h"
#include "./data-structures/list.h"
#include "./parser/parser.h"
#include "./databases/database.h"
#include "./logs/logs.h"

#define DBP_CONN_EMPTYPACKET    0
#define DBP_CONNEND_FLOW    1
#define DBP_CONNEND_CORRUPT 2
#define DBP_CONN_INVALID_ACTION 3
#define DBP_CONN_INSUFFICENT_ATTRIB 4

typedef struct {
    netconn_info_s connection;
    logger_s logs;
    char is_init;
} dbp_s; // device backup protocol

typedef struct dbp_common_attribs {
    string_s filename;
    unsigned int crc32;
    int error;
} dbp_common_attribs_s;

typedef struct {
    size_t data_length;
    short header_length;
    char magic;
} dbp_header_s;

typedef struct {
    dbp_header_s header;
    array_list_s header_list;
    int action;
    int error;
    dbp_s *dbp;
    dbp_common_attribs_s attribs;
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
packet_info_s dbp_read_headers(dbp_s *protocol);

#endif