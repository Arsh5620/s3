#ifndef PROTOCOL_INCLUDE_GAURD
#define PROTOCOL_INCLUDE_GAURD

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../networking/network.h"
#include "../output/output.h"
#include "../files/file.h"
#include "../general/binarysearch.h"
#include "../general/define.h"
#include "../data-structures/hash_table.h"
#include "../memdbg/memory.h"
#include "../general/string.h"
#include "../data-structures/list.h"
#include "../parser/parser.h"
#include "../databases/database.h"
#include "../dataparser/data.h"
#include "../files/path.h"
#include "../files/filemgmt.h"

#define DBP_PROTOCOL_MAGIC		0xD0
#define DBP_PROTOCOL_MAGIC_LEN	(8)
#define DBP_PROTOCOL_HEADER_MAXLEN	(256 << 4)
#define DBP_CONFIG_FILENAME		"config.a"
#define DBP_RESPONSE_FORMAT_STRING	"%.*s=\"%.*s\"\r\n"
#define DBP_RESPONSE_FORMAT_LONG	"%.*s=\"%ld\"\r\n"
#define DBP_RESPONSE_KEY_NAME	"response"

// one-to-one mapping to the actions_supported
enum dbp_actions_enum {
	DBP_ACTION_CREATE = 128
	, DBP_ACTION_DELETE
	, DBP_ACTION_NOTIFICATION
	, DBP_ACTION_REQUEST
	, DBP_ACTION_UPDATE
	, DBP_ACTION_NOTVALID	= -1
};

enum dbp_attribs_enum {
	DBP_ATTRIB_ACTION = 128
	, DBP_ATTRIB_FILENAME
	, DBP_ATTRIB_CRC
	, DBP_ATTRIB_UPDATEAT
	, DBP_ATTRIB_UPDATETRIM
};

enum dbp_shutdown_enum {
	DBP_CONNECTION_SHUTDOWN_FLOW
	, DBP_CONNECTION_SHUTDOWN_CORRUPTION
};

enum dbp_response_code {
	DBP_RESPONSE_SUCCESS
	, DBP_RESPONSE_DATA_SEND	= 1
	, DBP_RESPONSE_PACKET_OK
	, DBP_RESPONSE_PACKET_DATA_MORE
	, DBP_RESPONSE_PACKET_DATA_READY
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
	, DBP_RESPONSE_DATA_NONE_NEEDED
	, DBP_RESPONSE_DATA_NOT_ACCEPTED
	/* errors and the connection will need to be closed */
	, DBP_RESPONSE_ERRORS	= 128
	, DBP_RESPONSE_CORRUPTED_PACKET
	, DBP_RESPONSE_CORRUPTED_DATA_HEADERS
	, DBP_RESPONSE_SETUP_ENV_FAILED
	, DBP_RESPONSE_GENERAL_SERVER_ERROR
	, DBP_RESPONSE_GENERAL_DIR_ERROR
	, DBP_RESPONSE_GENERAL_FILE_ERROR
	, DBP_RESPONSE_ERROR_WRITE
	, DBP_RESPONSE_ERROR_READ
	, DBP_RESPONSE_CANNOT_CREATE_TEMP_FILE
	, DBP_RESPONSE_ERROR_WRITING_HEADERS
};

typedef struct {
	size_t data_length;
	unsigned short header_length;
	unsigned char magic;
} dbp_header_s;

typedef struct {
	string_s file_name;
	uint crc32;
	long update_at;
	boolean trim; // 0 means false, every other value is true
} dbp_protocol_attribs_s;

#define DBP_ACTIONS_COUNT	5
#define DBP_ATTRIBS_COUNT	5
#define DBP_ATTRIBS_STRUCT_COUNT	DBP_ATTRIBS_COUNT - 1
#define DBP_KEY_FILENAME	"file_name"

#define DBP_ASSIGN(string_dest, code, string) \
	case code: \
	{ \
		string_dest = STRING(string); \
	} \
	break;

extern data_keys_s attribs[];
extern data_keys_s actions[];
extern enum dbp_attribs_enum dbp_call_asserts[][DBP_ATTRIBS_COUNT];

typedef struct {
	long update_at;
	boolean trim;
} dbp_action_update_s;

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

	char *additional_data; // action level data that can be set and used.

	data_result_s data_result; // both the list and the hash of the header values
	/*
	 * for big file writes, we have to first confirm that the client
	 * accepts the file before we can continue, the prehook function 
	 * must set this value to TRUE for the protocol to wait for client
	 * confirmation.
	 */
	boolean data_write_confirm;
	filemgmt_file_name_s file_info;

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
	long response_code;
	filemgmt_file_name_s *file_info;
	string_s writer;
	long data_written;
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
int dbp_posthook_delete(dbp_request_s *request, dbp_response_s *response);
int dbp_posthook_request(dbp_request_s *request, dbp_response_s *response);

int dbp_prehook_notification(dbp_request_s *request);
int dbp_prehook_create(dbp_request_s *request);
int dbp_prehook_action(dbp_request_s *request);
int dbp_prehook_update(dbp_request_s *request);
int dbp_prehook_delete(dbp_request_s *request);
int dbp_prehook_request(dbp_request_s *request);

int dbp_attribs_assert(hash_table_s table, 
	enum dbp_attribs_enum *match, int count);
int dbp_file_setup_environment();
int dbp_file_download(dbp_request_s *request);

hash_table_s dbp_header_hash(my_list_s list);
dbp_header_s dbp_header_parse8(size_t magic);

int dbp_action_prehook(dbp_request_s *request);
int dbp_action_posthook(dbp_request_s *request, dbp_response_s *response);

int dbp_handle_response(dbp_response_s *response, enum dbp_response_code code);

int dbp_response_write(dbp_response_s *response
	, long (*writer)(dbp_response_s *in));
long dbp_response_write_header(dbp_response_s *response
	, char *buffer, ulong buffer_length);
ulong dbp_response_make_magic(dbp_response_s *response);

int dbp_request_data(dbp_protocol_s *protocol, dbp_request_s *request);
int dbp_request_data_headers(dbp_protocol_s *protocol, dbp_request_s *request);
#endif //PROTOCOL_INCLUDE_GAURD