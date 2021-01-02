#ifndef PROTOCOL_INCLUDE_GAURD
#define PROTOCOL_INCLUDE_GAURD

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "./auth.h"
#include "../networking/network.h"
#include "../logger/messages.h"
#include "../files/file.h"
#include "../general/binarysearch.h"
#include "../general/define.h"
#include "../data-structures/hash_table.h"
#include "../memdbg/memory.h"
#include "../general/string.h"
#include "../ssbs/list.h"
#include "../parser/parser.h"
#include "../databases/database.h"
#include "../dataparser/data.h"
#include "../files/path.h"
#include "../files/filemgmt.h"

#define DBP_PROTOCOL_MAGIC 0xD0
#define DBP_PROTOCOL_MAGIC_LEN (8)
#define DBP_PROTOCOL_HEADER_MAXLEN (256 << 4)
#define DBP_DATABASE_FILENAME "schema.db"
#define DBP_RESPONSE_FORMAT_STRING "%.*s=\"%.*s\"\r\n"
#define DBP_RESPONSE_FORMAT_LONG "%.*s=\"%ld\"\r\n"
#define DBP_RESPONSE_KEY_NAME "response"

// attribute name keys
#define DBP_ATTRIBNAME_ACTION "action"
#define DBP_ATTRIBNAME_CRC "crc"
#define DBP_ATTRIBNAME_FILENAME "filename"
#define DBP_ATTRIBNAME_UPDATEAT "updateat"
#define DBP_ATTRIBNAME_UPDATETRIM "trim"
#define DBP_ATTRIBNAME_USERNAME "username"
#define DBP_ATTRIBNAME_SECRET "secret"

// one-to-one mapping to the actions_supported
enum dbp_actions_enum
{
    DBP_ACTION_CREATE = 1,
    DBP_ACTION_DELETE,
    DBP_ACTION_NOTIFICATION,
    DBP_ACTION_REQUEST,
    DBP_ACTION_SERVER,
    DBP_ACTION_UPDATE,
    DBP_ACTION_INVALID = -1
};

enum dbp_attribs_enum
{
    DBP_ATTRIB_ACTION = 1,
    DBP_ATTRIB_FILENAME,
    DBP_ATTRIB_CRC,
    DBP_ATTRIB_UPDATEAT,
    DBP_ATTRIB_UPDATETRIM,
    DBP_ATTRIB_USERNAME,
    DBP_ATTRIB_PASSWORD
};

enum dbp_shutdown_enum
{
    DBP_CONNECTION_SHUTDOWN_FLOW,
    DBP_CONNECTION_SHUTDOWN_CORRUPTION
};

enum dbp_response_code
{
    DBP_RESPONSE_SUCCESS,   // Internal, no response to the client
    DBP_RESPONSE_DATA_SEND, // The server has acknowledged the request and is now expecting data
                            // packets
    DBP_RESPONSE_PACKET_OK, // The packet was received and processed, no
                            // further action required on server end
    DBP_RESPONSE_PACKET_DATA_MORE,  // Requesting send permission from client
    DBP_RESPONSE_PACKET_DATA_READY, // Sending data

    /* warnings but we can continue the connection */
    DBP_RESPONSE_WARNINGS = 32,
    DBP_RESPONSE_ACTION_INVALID,
    DBP_RESPONSE_HEADER_EMPTY,
    DBP_RESPONSE_DESERIALIZER_ERROR,
    DBP_RESPONSE_MISSING_ATTRIBS,
    DBP_RESPONSE_ATTRIB_VALUE_INVALID,
    DBP_RESPONSE_FILE_EXISTS_ALREADY,
    DBP_RESPONSE_FILE_NOT_FOUND,
    DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS,
    DBP_RESPONSE_NOTIFY_TOOBIG,
    DBP_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT,
    DBP_RESPONSE_DATA_NOT_ACCEPTED,
    DBP_RESPONSE_MIX_AUTH_ERROR,
    DBP_RESPONSE_SERVER_ERROR_NOAUTH,
    DBP_RESPONSE_FAILED_AUTHENTICATION,

    /* errors and the connection will need to be closed */
    DBP_RESPONSE_ERRORS = 128,
    DBP_RESPONSE_CORRUPTED_PACKET,
    DBP_RESPONSE_CORRUPTED_DATA_HEADERS,
    DBP_RESPONSE_SETUP_ENVIRONMENT_FAILED,
    DBP_RESPONSE_SERVER_INTERNAL_ERROR,
    DBP_RESPONSE_SERVER_DIR_ERROR,
    DBP_RESPONSE_SERVER_FILE_ERROR,
    DBP_RESPONSE_SQLITE_INTERNAL_ERROR,
    DBP_RESPONSE_SQLITE_NO_CONNECTION,
    DBP_RESPONSE_NETWORK_ERROR_WRITE,
    DBP_RESPONSE_NETWORK_ERROR_READ,
    DBP_RESPONSE_CANNOT_CREATE_TEMP_FILE,
    DBP_RESPONSE_ERROR_WRITING_HEADERS
};

typedef struct
{
    size_t data_length;
    unsigned short header_length;
    unsigned char magic;
} dbp_header_s;

typedef struct
{
    string_s file_name;
    uint crc32;
    long update_at;
    boolean trim; // 0 means false, every other value is true
} dbp_protocol_attribs_s;

#define DBP_ACTIONS_COUNT 6
#define DBP_ATTRIBS_COUNT 7
#define DBP_ATTRIBS_STRUCT_COUNT DBP_ATTRIBS_COUNT - 1
#define DBP_KEY_FILENAME "file_name"

#define DBP_CASE(string_dest, code, string)                                                        \
    case code:                                                                                     \
    {                                                                                              \
        string_dest = STRING (string);                                                             \
    }                                                                                              \
    break;

extern data_keys_s attribs[];
extern data_keys_s actions[];
extern enum dbp_attribs_enum dbp_call_asserts[][DBP_ATTRIBS_COUNT];

typedef struct
{
    long update_at;
    boolean trim;
} dbp_action_update_s;

typedef struct
{
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

    /*
     * for big file writes, we have to first confirm that the client
     * accepts the file before we can continue, the prehook function
     * must set this value to TRUE for the protocol to wait for client
     * confirmation.
     * Then once we receive a confirmation, it is set in the same variable
     * so in the data send function we can check the value of this to
     * check if we should start writing
     */
    boolean data_write_confirm;
    filemgmt_file_name_s file_name;

    /*
     * instance is a pointer to the dbp_protocol_s that will have
     * information in regards of the current network and its status
     */
    char *instance;
} dbp_request_s;

typedef struct
{
    my_list_s header_list;
    dbp_header_s header_info;

    /*
     * response_code is the response indicator, which idicates success or
     * failure after a call to the requested action.
     */
    long response_code;
    filemgmt_file_name_s *file_name;
    string_s writer_buffer;
    long total_write_completed;
    char *instance;
} dbp_response_s;

typedef struct
{
    char init_complete;

    network_s connection;
    logger_s logs;

    dbp_request_s *current_request;
    dbp_response_s *current_response;
} dbp_protocol_s;

typedef struct
{
    boolean print_debug_logs;
} dbp_protocol_settings_s;

void
dbp_print_counter ();

void
dbp_close (dbp_protocol_s protocol);
void
dbp_handle_close (dbp_request_s *request, dbp_response_s *response);
ulong
dbp_next_request (dbp_protocol_s *protocol);
dbp_protocol_s
dbp_connection_initialize_sync (unsigned short port);
void
dbp_connection_accept_loop (dbp_protocol_s *protocol);
void
dbp_connection_shutdown (dbp_protocol_s protocol, enum dbp_shutdown_enum type);

ulong
dbp_request_read_headers (dbp_protocol_s protocol, dbp_request_s *request);
int
dbp_request_read_action (dbp_request_s *request);

long
dbp_action_request_writer (dbp_response_s *in);

int
dbp_posthook_notification (dbp_request_s *request, dbp_response_s *response);
int
dbp_posthook_create (dbp_request_s *request, dbp_response_s *response);
int
dbp_posthook_update (dbp_request_s *request, dbp_response_s *response);
int
dbp_posthook_delete (dbp_request_s *request, dbp_response_s *response);
int
dbp_posthook_request (dbp_request_s *request, dbp_response_s *response);
int
dbp_posthook_serverinfo (dbp_request_s *request, dbp_response_s *response);

int
dbp_prehook_notification (dbp_request_s *request);
int
dbp_prehook_create (dbp_request_s *request);
int
dbp_prehook_action (dbp_request_s *request);
int
dbp_prehook_update (dbp_request_s *request);
int
dbp_prehook_delete (dbp_request_s *request);
int
dbp_prehook_request (dbp_request_s *request);
int
dbp_prehook_serverinfo (dbp_request_s *request);

int
dbp_attribs_assert (hash_table_s table, enum dbp_attribs_enum *match, int count);
int
dbp_attrib_contains (hash_table_s table, int attrib);

int
dbp_file_setup_environment ();
int
dbp_file_download (dbp_request_s *request);

hash_table_s
dbp_header_hash (my_list_s list);
dbp_header_s
dbp_header_parse8 (size_t magic);
my_list_s
dbp_deserialize_headers (network_data_s headers, int *error);
void
dbp_print_headers (my_list_s list);

int
dbp_action_prehook (dbp_request_s *request);
int
dbp_action_posthook (dbp_request_s *request, dbp_response_s *response);
int
dbp_action_send (dbp_request_s *request, dbp_response_s *response);

int
dbp_handle_response (dbp_response_s *response, enum dbp_response_code code);

int
dbp_response_write (dbp_response_s *response, long (*writer) (dbp_response_s *in));
long
dbp_response_write_header (dbp_response_s *response, char *buffer, ulong buffer_length);
ulong
dbp_response_make_magic (dbp_response_s *response);
int
dbp_response_accept_status (dbp_response_s *response);
int
dbp_response_writer_update (dbp_response_s *response, string_s write);

int
dbp_request_data (dbp_protocol_s *protocol, dbp_request_s *request);
int
dbp_request_data_headers (dbp_protocol_s *protocol, dbp_request_s *request);

int
dbp_auth_query (dbp_request_s *request);
int
dbp_auth_transaction (dbp_request_s *request);
int
dbp_auth_query_sqlite3 (char *username, int username_length, char *password, int password_length);

int
dbp_copy_keyvaluepairs (my_list_s source_list, my_list_s *dest_list);
#endif // PROTOCOL_INCLUDE_GAURD