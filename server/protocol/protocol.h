#ifndef PROTOCOL_INCLUDE_GAURD
#define PROTOCOL_INCLUDE_GAURD

#include <stdio.h>
#include "../networking/network.h"
#include "../errors/errorhandler.h"
#include "../files/file.h"
#include "../general/binarysearch.h"
#include "../general/defines.h"
#include "../data-structures/hash_table.h"
#include "../memdbg/memory.h"
#include "../general/strings.h"
#include "../data-structures/list.h"
#include "../parser/parser.h"
#include "../databases/database.h"
#include "../config/config.h"

#define DBP_FILE_TEMP_DIR "temp"

#define DBP_ACTION_NOID  -1
#define DBP_ATTRIBS_ERR_NAMETOOLONG 0x0003
#define DBP_ATTRIBS_ERR_CRC32NOTFOUND   0x0004
#define DBP_PROTOCOL_MAGIC 0xd0
#define DBP_TEMP_FILE_FORMAT "%s/download-fn(%ld).tmp"
#define DBP_CONFIG_FILENAME "config.a"

#define DBP_CONNEND_FLOW    1
#define DBP_CONNEND_CORRUPT 2
#define DBP_CONN_EMPTYPACKET    3
#define DBP_CONN_INVALID_ACTION 4
#define DBP_CONN_INSUFFICENT_ATTRIB 5
#define DBP_CONN_SETUP_ENV_FAILED   6
#define DBP_CONN_NOTIFICATION_SIZE  7

enum dbp_response_enum {
	DBP_RESPONSE_NONE
	, DBP_RESPONSE_ACK
};

// one-to-one mapping to the actions_supported
enum actions_supported_enum {
	ACTION_CREATE = 0
	, ACTION_NOTIFICATION
	, ACTION_REQUEST
	, ACTION_UPDATE
};

enum attrib_supported_enum {
	ATTRIB_ACTION = 128
	, ATTRIB_FILENAME
	, ATTRIB_FOLDER
	, ATTRIB_CRC
};

typedef struct {
	netconn_info_s connection;
	logger_s logs;
	char is_init;
} dbp_s; // device backup protocol

typedef struct dbp_common_attribs {
	string_s filename;
	string_s folder_name;
	unsigned int crc32;
	int error;
} dbp_common_attribs_s;

typedef struct {
	size_t data_length;
	unsigned short header_length;
	unsigned char magic;
} dbp_header_s;

typedef struct {
	enum dbp_response_enum dbp_response;
} dbp_response_s;

typedef struct {
	dbp_header_s header;
	my_list_s header_list;
	hash_table_s header_table;
	file_write_s data_written;
	int action;
	int error;
	dbp_s *dbp;
	dbp_common_attribs_s attribs;
	dbp_response_s response;
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
int dbp_response_write(packet_info_s *info);
int dbp_action_prehook(packet_info_s *info);
packet_info_s dbp_read_headers(dbp_s *protocol);

int dbp_notification_posthook(packet_info_s *info);
int dbp_notification_prehook(packet_info_s *info);
int dbp_create_prehook(packet_info_s *protocol);

int dbp_assert_list(my_list_s list, 
	enum attrib_supported_enum *match, int match_length);
hash_table_s dbp_attribs_hash_table(packet_info_s info);
dbp_common_attribs_s dbp_attribs_parse_all(packet_info_s info);
file_write_s create_download_file(packet_info_s *info);
int create_setup_environment();

short dbp_header_length(size_t magic);
char dbp_header_magic(size_t magic);
size_t dbp_data_length(size_t magic);

int dbp_setup_download_env();
int dbp_action_posthook(packet_info_s *info);

#endif //PROTOCOL_INCLUDE_GAURD