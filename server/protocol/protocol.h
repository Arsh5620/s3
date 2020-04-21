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
	, DBP_ATTRIB_UPDATEAT
};

enum dbp_shutdown_enum {
	DBP_CONNECTION_SHUTDOWN_FLOW
	, DBP_CONNECTION_SHUTDOWN_CORRUPTION
};

enum dbp_response_code {
	DBP_RESPONSE_SUCCESS
	, DBP_RESPONSE_DATA_SEND	= 1
	, DBP_RESPONSE_PACKET_OK
	/* warnings but we can continue the connection */
	, DBP_RESPONSE_WARNINGS	 = 32
	, DBP_RESPONSE_ACTION_INVALID
	, DBP_RESPONSE_HEADER_EMPTY
	, DBP_RESPONSE_PARSE_ERROR
	, DBP_RESPONSE_THIN_ATTRIBS
	, DBP_RESPONSE_ATTRIB_VALUE_INVALID
	, DBP_RESPONSE_FILE_EXISTS_ALREADY
	, DBP_RESPONSE_FILE_NOT_FOUND
	, DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS
	, DBP_RESPONSE_NOTIFY_TOOBIG
	/* errors and the connection will need to be closed */
	, DBP_RESPONSE_ERRORS	= 128
	, DBP_RESPONSE_CORRUPTED_PACKET
	, DBP_RESPONSE_CORRUPTED_DATA_HEADERS
	, DBP_RESPONSE_SETUP_ENV_FAILED
	, DBP_RESPONSE_GENERAL_SERVER_ERROR
	, DBP_RESPONSE_ERROR_WRITE
	, DBP_RESPONSE_ERROR_READ
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
	long update_at;
} dbp_protocol_attribs_s;

#define DBP_ACTIONS_COUNT	4
#define DBP_ATTRIBS_COUNT	5
#define DBP_ATTRIBS_STRUCT_COUNT	4
#define DBP_KEY_FILENAME	"file_name"

#define DBP_CASE_LINK_CODE(src, code, string) \
	case code: \
	{ \
		src->data_string = STRING_S(string); \
	} \
	break;

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
	 * header_* will have all the information related to the entire header
	 * header_info is the information about the size of the header table, 
	 * and the size of the expected data. 
	 * header_list is the key value pairs of the header that we received. 
	 * header_list is the same key value pairs in a hash table for quick access
	 */
	my_list_s header_list;
	network_data_s header_raw;
	dbp_header_s header_info;
	hash_table_s header_table;
	hash_table_s additional_data;

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
void dbp_print_counter();

void dbp_close(dbp_protocol_s protocol);
void dbp_handle_close(dbp_request_s *request, dbp_response_s *response);
ulong dbp_next_request(dbp_protocol_s *protocol);
dbp_protocol_s dbp_connection_initialize_sync(unsigned short port);
void dbp_connection_accept_loop(dbp_protocol_s *protocol);
void dbp_connection_shutdown(dbp_protocol_s protocol
	, enum dbp_shutdown_enum type);

ulong dbp_request_read_headers(dbp_protocol_s protocol
	, dbp_request_s *request);
int dbp_request_read_action(dbp_request_s *request);

int dbp_posthook_notification(dbp_request_s *request
	, dbp_response_s *response);
int dbp_posthook_create(dbp_request_s *request, dbp_response_s *response);
int dbp_posthook_update(dbp_request_s *request, dbp_response_s *response);

int dbp_prehook_notification(dbp_request_s *request);
int dbp_prehook_create(dbp_request_s *request);
int dbp_prehook_action(dbp_request_s *request);
int dbp_prehook_update(dbp_request_s *request);

int dbp_attribs_assert(hash_table_s table, 
	enum dbp_attribs_enum *match, int count);
int dbp_file_setup_environment();
file_write_s dbp_file_download(dbp_request_s *request);

hash_table_s dbp_header_hash(my_list_s list);
dbp_header_s dbp_header_parse8(size_t magic);

int dbp_action_prehook(dbp_request_s *request);
int dbp_action_posthook(dbp_request_s *request, dbp_response_s *response);

int dbp_handle_response(dbp_response_s *response, enum dbp_response_code code);

int dbp_response_write(dbp_response_s *response);
void dbp_response_make_header(dbp_response_s *response
	, char *buffer, ulong header_length);
ulong dbp_response_make_magic(dbp_response_s *response);
ulong dbp_response_header_length(dbp_response_s *response);

int dbp_request_data(dbp_protocol_s *protocol, dbp_request_s *request);
int dbp_request_data_headers(dbp_protocol_s *protocol, dbp_request_s *request);
#endif //PROTOCOL_INCLUDE_GAURD