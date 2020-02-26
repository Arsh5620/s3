# ifndef DBP_INCLUDE_GAURD
# define DBP_INCLUDE_GAURD
#include "networking/network.h"
#include "strings.h"
#include "list.h"
#include "parser/parser.h"

typedef struct device_backup_protocol {
    netconn_info_s connection;
    array_list_s headers;
    unsigned long header_magic_now;
    char is_init;
} dbp_s;

enum connection_shutdown_type {
    DBP_CONNECT_SHUTDOWN_FLOW
    , DBP_CONNECT_SHUTDOWN_CORRUPTION
};

int dbp_read(dbp_s *_read);
dbp_s dbp_init(unsigned short port);
void dbp_cleanup(dbp_s protocol_handle);
void dbp_accept_connection_loop(dbp_s *protocol);

// internal functions -- might not need to be used outside. 
// only use outside of the scope if the function has no side effects. 

int dbp_magic_check(long magic);
void dbp_shutdown_connection(dbp_s protocol
            , enum connection_shutdown_type reason);
int dbp_headers_action(dbp_s *_read, key_value_pair_s pair);
int dbp_action_dispatch(dbp_s *_read, int action);
long int dbp_data_length(unsigned long magic);
array_list_s dbp_headers_read(dbp_s *_read, int length);

#endif