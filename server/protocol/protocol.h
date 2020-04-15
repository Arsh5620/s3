#ifndef PROTOCOL_INCLUDE_GAURD
#define PROTOCOL_INCLUDE_GAURD

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
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

#define DBP_PROTOCOL_MAGIC		0xD0
#define DBP_TEMP_FORMAT			"%s/download-fn(%ld).tmp"
#define DBP_TEMP_DIR			"temp"
#define DBP_CONFIG_FILENAME		"config.a"
#define DBP_RESPONSE_FORMAT		"response=%3d\r\n"
 
enum dbp_errors_enum {
	DBP_CONNECTION_NOERROR	= 0
	, DBP_CONNECTION_WARN
	, DBP_CONNECTION_ERROR_READ
	, DBP_CONNECTION_ERROR_SETUP_ENV_FAILED
	, DBP_CONNECTION_ERROR_CORRUPTED_PACKET
	, DBP_CONNECTION_ERROR_CORRUPTED_DATAH
	, DBP_CONNECTION_ERROR_WRITEFAILED
};

enum dbp_warns_enum {
	DBP_CONNECTION_NOWARN	= 4096
	, DBP_CONNECTION_WARN_PARSE_ERROR
	, DBP_CONNECTION_WARN_HEADER_EMPTY
	, DBP_CONNECTION_WARN_ACTION_INVALID
	, DBP_CONNECTION_WARN_THIN_ATTRIBS
	, DBP_CONNECTION_WARN_ATTRIB_VALUE_INVALID
};

// one-to-one mapping to the actions_supported
enum dbp_actions_enum {
	DBP_ACTION_CREATE = 128
	, DBP_ACTION_NOTIFICATION
	, DBP_ACTION_REQUEST
	, DBP_ACTION_UPDATE
	, DBP_ACTION_NOTVALID	= -1
};

enum dbp_attribs_enum {
	DBP_ATTRIB_ACTION = 128
	, DBP_ATTRIB_FILENAME
	, DBP_ATTRIB_FOLDER
	, DBP_ATTRIB_CRC
};

enum dbp_shutdown_enum {
	DBP_CONNECTION_SHUTDOWN_FLOW
	, DBP_CONNECTION_SHUTDOWN_CORRUPTION
};

enum dbp_response_code {
	DBP_RESPONSE_DATA_SEND	= 1
	, DBP_RESPONSE_PACKET_OK
	/* warnings but we can continue the connection */
	, DBP_RESPONSE_ACTION_INVALID = 32
	, DBP_RESPONSE_HEADER_EMPTY
	, DBP_RESPONSE_PARSE_ERROR
	, DBP_RESPONSE_THIN_ATTRIBS
	, DBP_RESPONSE_ATTIB_VALUE_INVALID
	/* errors and the connection will need to be closed */
	, DBP_RESPONSE_CORRUPTED_PACKET = 128
	, DBP_RESPONSE_CORRUPTED_DATAH
	, DBP_RESPONSE_SETUP_ENV_FAILED
};

typedef struct {
	size_t data_length;
	unsigned short header_length;
	unsigned char magic;
} dbp_header_s;

typedef struct {
	char *string;
	ulong strlen;
	enum dbp_attribs_enum attrib_code;
} dbp_header_keys_s;

typedef struct {
	string_s file_name;
	string_s folder_name;
	uint crc32;
} dbp_protocol_attribs_s;

#define DBP_ACTIONS_COUNT	4
#define DBP_ATTRIBS_COUNT	4
#define DBP_ATTRIBS_STRUCT_COUNT	3

extern dbp_header_keys_s attribs[];
extern dbp_header_keys_s actions[];
extern enum dbp_attribs_enum dbp_call_asserts[][DBP_ATTRIBS_COUNT];
extern struct config_parse attribs_parse[];

#define DBP_STRINGKEY(str, code) \
	(dbp_header_keys_s) { \
		.string = str \
		, .strlen = sizeof(str) - 1 \
		, .attrib_code = code \
	}

typedef struct {
	/*
	 * action is the type of action that the client has requested to perform
	 * this can include but is not limited to "notification", "request"
	 * , and "update" etc. 
	 */
	enum dbp_actions_enum action;

	/*
	 * the difference between error and warn is that in case of an error
	 * the entire connection will be terminated, and in case of warn we
	 * can still continue the connection for more requests 
	 */
	enum dbp_errors_enum error;

	/*
	 * header_* will have all the information related to the entire header
	 * header_info is the information about the size of the header table, 
	 * and the size of the expected data. 
	 * header_list is the key value pairs of the header that we received. 
	 * header_list is the same key value pairs in a hash table for quick access
	 */
	my_list_s header_list;
	dbp_header_s header_info;
	hash_table_s header_table;

	dbp_protocol_attribs_s attribs;

	/*
	 * temp_file will contain all the information such as name, size, 
	 * for the temporary file written to the temp folder
	 */
	file_write_s temp_file;

	/*
	 * instance is a pointer to the dbp_protocol_s that will have 
	 * information in regards of the current network and its status
	 */
	char *instance;
} dbp_request_s;

typedef struct {
	my_list_s header_list;
	dbp_header_s header_info;

	/*
	 * response_code is the response indicator, which idicates success or
	 * failure after a call to the requested action. 
	 */
	int response_code;
	string_s data_string;
	char *instance;
} dbp_response_s;

typedef struct {
	char init_complete;

	network_s connection;
	logger_s logs;

	dbp_request_s *current_request;
	dbp_response_s *current_response;
} dbp_protocol_s;

// typedef struct dbp_common_attribs {
// 	string_s filename;
// 	string_s folder_name;
// 	unsigned int crc32;
// 	int error;
// } dbp_common_attribs_s;

void dbp_cleanup(dbp_protocol_s protocol);
ulong dbp_protocol_nextrequest(dbp_protocol_s *protocol);
dbp_protocol_s dbp_connection_initialize_sync(unsigned short port);
void dbp_accept_connection_loop(dbp_protocol_s *protocol);

// internal functions -- should not be used outside. 

void dbp_shutdown_connection(dbp_protocol_s protocol
	, enum dbp_shutdown_enum type);
ulong dbp_request_readheaders(dbp_protocol_s protocol, dbp_request_s *request);

int dbp_posthook_notification(dbp_request_s *request, dbp_response_s *response);

int dbp_prehook_notification(dbp_request_s *request);
int dbp_prehook_create(dbp_request_s *request);
int dbp_prehook_action(dbp_request_s *request);
int dbp_read_action(dbp_request_s *request);

int dbp_list_assert(hash_table_s table, 
	enum dbp_attribs_enum *match, int count);
hash_table_s dbp_headers_make_table(my_list_s list);
file_write_s dbp_download_file(dbp_request_s *request);
int create_setup_environment();

short dbp_header_length(size_t magic);
char dbp_header_magic(size_t magic);
size_t dbp_data_length(size_t magic);

int dbp_setup_environment();
int dbp_action_prehook(dbp_request_s *request);
int dbp_action_posthook(dbp_request_s *request, dbp_response_s *response);
void dbp_request_cleanup();
int dbp_handle_warns(dbp_protocol_s *protocol, enum dbp_warns_enum warn);
void dbp_handle_errors(dbp_response_s *response, 
	enum dbp_errors_enum error, int *shutdown);
	
int dbp_response_write(dbp_response_s *response);
string_s dbp_response_make_header(dbp_response_s *response);
ulong dbp_response_make_magic(dbp_response_s *response);

int dbp_request_data(dbp_protocol_s *protocol, dbp_request_s *request);
int dbp_request_data_headers(dbp_protocol_s *protocol, dbp_request_s *request);
int dbp_handle_response(dbp_response_s *response, enum dbp_response_code code);
#endif //PROTOCOL_INCLUDE_GAURD