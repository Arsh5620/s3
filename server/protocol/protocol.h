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
#include "../databases/database.h"
#include "../data/data.h"
#include "../files/path.h"
#include "../files/filemgmt.h"

#define S3_PROTOCOL_MAGIC 0xD0
#define S3_PROTOCOL_MAGIC_LEN (8)
#define S3_PROTOCOL_HEADER_MAXLEN (256 << 4)
#define S3_DATABASE_FILENAME "schema.db"
#define S3_RESPONSE_KEY_NAME "response"

// attribute name keys
#define S3_ATTRIBNAME_ACTION "action"
#define S3_ATTRIBNAME_CRC "crc"
#define S3_ATTRIBNAME_DIRNAME "dirname"
#define S3_ATTRIBNAME_FILENAME "filename"
#define S3_ATTRIBNAME_UPDATEAT "updateat"
#define S3_ATTRIBNAME_UPDATETRIM "trim"
#define S3_ATTRIBNAME_USERNAME "username"
#define S3_ATTRIBNAME_SECRET "secret"

// one-to-one mapping to the actions_supported
enum s3_actions_enum
{
    S3_ACTION_CREATE = 1,
    S3_ACTION_DELETE,
    S3_ACTION_DIR,
    S3_ACTION_NOTIFICATION,
    S3_ACTION_REQUEST,
    S3_ACTION_SERVER,
    S3_ACTION_UPDATE,
    S3_ACTION_INVALID = -1
};

enum s3_attribs_enum
{
    S3_ATTRIB_ACTION = 1,
    S3_ATTRIB_CRC,
    S3_ATTRIB_DIRNAME,
    S3_ATTRIB_FILENAME,
    S3_ATTRIB_UPDATEAT,
    S3_ATTRIB_UPDATETRIM,
    S3_ATTRIB_USERNAME,
    S3_ATTRIB_PASSWORD
};

enum s3_shutdown_enum
{
    S3_SHUTDOWN_CLOSE,
    S3_SHUTDOWN_INVALID
};

enum s3_response_code
{
    S3_RESPONSE_SUCCESS,           // Internal, no response to the client
    S3_RESPONSE_DATA_SEND,         // The server is expecting client to send data
    S3_RESPONSE_PACKET_OK,         // Server has processed the action
    S3_RESPONSE_PACKET_DATA_MORE,  // Requesting send permission from client
    S3_RESPONSE_PACKET_DATA_READY, // Sending data

    /* warnings but we can continue the connection */
    S3_RESPONSE_WARNINGS = 32,
    S3_RESPONSE_ACTION_INVALID,
    S3_RESPONSE_HEADER_EMPTY,
    S3_RESPONSE_DESERIALIZER_ERROR,
    S3_RESPONSE_MISSING_ATTRIBS,
    S3_RESPONSE_ATTRIB_VALUE_INVALID,
    S3_RESPONSE_FILE_EXISTS_ALREADY,
    S3_RESPONSE_FILE_NOT_FOUND,
    S3_RESPONSE_FILE_UPDATE_OUTOFBOUNDS,
    S3_RESPONSE_NOTIFY_TOOBIG,
    S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT,
    S3_RESPONSE_DATA_NOT_ACCEPTED,
    S3_RESPONSE_MIX_AUTH_ERROR,
    S3_RESPONSE_SERVER_ERROR_NOAUTH,
    S3_RESPONSE_FAILED_AUTHENTICATION,

    /* errors and the connection will need to be closed */
    S3_RESPONSE_ERRORS = 128,
    S3_RESPONSE_CORRUPTED_PACKET,
    S3_RESPONSE_CORRUPTED_DATA_HEADERS,
    S3_RESPONSE_SETUP_ENVIRONMENT_FAILED,
    S3_RESPONSE_SERVER_INTERNAL_ERROR,
    S3_RESPONSE_SERVER_DIR_ERROR,
    S3_RESPONSE_SERVER_FILE_ERROR,
    S3_RESPONSE_SQLITE_INTERNAL_ERROR,
    S3_RESPONSE_SQLITE_NO_CONNECTION,
    S3_RESPONSE_NETWORK_ERROR_WRITE,
    S3_RESPONSE_NETWORK_ERROR_READ,
    S3_RESPONSE_CANNOT_CREATE_TEMP_FILE,
    S3_RESPONSE_ERROR_WRITING_HEADERS
};

typedef struct
{
    size_t data_length;
    unsigned short header_length;
    unsigned char magic;
} s3_header_s;

#define S3_CASE(string_dest, code, string)                                                         \
    case code:                                                                                     \
    {                                                                                              \
        string_dest = STRING (string);                                                             \
    }                                                                                              \
    break;

extern data_keys_s attribs[];
extern data_keys_s actions[];
extern enum s3_attribs_enum *s3_asserts;
extern const int attribs_count;
extern const int actions_count;

typedef struct
{
    long update_at;
    boolean trim;
} s3_action_update_s;

typedef struct
{
    /*
     * action is the type of action that the client has requested to perform
     * this can include but is not limited to "notification", "request"
     * , and "update" etc.
     */
    enum s3_actions_enum action;

    /*
     * header_* will have all the information related to the entire header
     * header_info is the information about the size of the header table,
     * and the size of the expected data.
     * header_list is the key value pairs of the header that we received.
     * header_list is the same key value pairs in a hash table for quick access
     */
    my_list_s header_list;
    network_data_s header_raw;
    s3_header_s header_info;
    hash_table_s header_table;

    char *additional_data; // action level data that can be set and used.

    /*
     * for big file writes, we have to first confirm that the client
     * accepts the file before we can continue, the preprocess function
     * must set this value to TRUE for the protocol to wait for client
     * confirmation.
     * Then once we receive a confirmation, it is set in the same variable
     * so in the data send function we can check the value of this to
     * check if we should start writing
     */
    boolean data_write_confirm;
    filemgmt_file_name_s file_name;

    /*
     * instance is a pointer to the s3_protocol_s that will have
     * information in regards of the current network and its status
     */
    char *instance;
} s3_request_s;

typedef struct
{
    my_list_s header_list;
    s3_header_s header_info;

    /*
     * response_code is the response indicator, which idicates success or
     * failure after a call to the requested action.
     */
    long response_code;
    filemgmt_file_name_s *file_name;
    string_s writer_buffer;
    long total_write_completed;
    char *instance;
} s3_response_s;

typedef struct
{
    char init_complete;

    network_s connection;
    logger_s logs;

    s3_request_s *current_request;
    s3_response_s *current_response;
} s3_protocol_s;

void
s3_print_counter ();

void
s3_close (s3_protocol_s protocol);
void
s3_handle_close (s3_request_s *request, s3_response_s *response);
ulong
s3_next_request (s3_protocol_s *protocol);
s3_protocol_s
s3_connection_initialize_sync (unsigned short port, s3_log_settings_s settings);
void
s3_connection_accept_loop (s3_protocol_s *protocol);
void
s3_connection_shutdown (s3_protocol_s protocol, enum s3_shutdown_enum type);

ulong
s3_request_read_headers (s3_protocol_s protocol, s3_request_s *request);
int
s3_request_read_action (s3_request_s *request);

long
s3_action_request_writer (s3_response_s *in);

int
s3_postprocess_notification (s3_request_s *request, s3_response_s *response);
int
s3_postprocess_create (s3_request_s *request, s3_response_s *response);
int
s3_postprocess_update (s3_request_s *request, s3_response_s *response);
int
s3_postprocess_delete (s3_request_s *request, s3_response_s *response);
int
s3_postprocess_request (s3_request_s *request, s3_response_s *response);
int
s3_postprocess_serverinfo (s3_request_s *request, s3_response_s *response);

int
s3_preprocess_notification (s3_request_s *request);
int
s3_preprocess_create (s3_request_s *request);
int
s3_preprocess_action (s3_request_s *request);
int
s3_preprocess_update (s3_request_s *request);
int
s3_preprocess_delete (s3_request_s *request);
int
s3_preprocess_request (s3_request_s *request);
int
s3_preprocess_serverinfo (s3_request_s *request);

int
s3_attribs_assert (hash_table_s table, enum s3_attribs_enum *match, int count);
int
s3_attrib_contains (hash_table_s table, int attrib);

int
s3_file_setup_environment ();
int
s3_file_download (s3_request_s *request);

hash_table_s
s3_header_hash (my_list_s list);
s3_header_s
s3_header_parse8 (size_t magic);
my_list_s
s3_deserialize_headers (network_data_s headers, int *error);
void
s3_print_headers (my_list_s list);

int
s3_action_preprocess (s3_request_s *request);
int
s3_action_postprocess (s3_request_s *request, s3_response_s *response);
int
s3_action_send (s3_request_s *request, s3_response_s *response);

int
s3_handle_response (s3_response_s *response, enum s3_response_code code);

int
s3_response_write (s3_response_s *response, long (*writer) (s3_response_s *in));
long
s3_response_write_header (s3_response_s *response, char *buffer, ulong buffer_length);
ulong
s3_response_make_magic (s3_response_s *response);
int
s3_response_accept_status (s3_response_s *response);
int
s3_response_writer_update (s3_response_s *response, string_s write);

int
s3_request_data (s3_protocol_s *protocol, s3_request_s *request);
int
s3_request_data_headers (s3_protocol_s *protocol, s3_request_s *request);

int
s3_auth_query (s3_request_s *request);
int
s3_auth_transaction (s3_request_s *request);
int
s3_auth_query_sqlite3 (char *username, int username_length, char *password, int password_length);

int
s3_copy_keyvaluepairs (my_list_s source_list, my_list_s *dest_list);
#endif // PROTOCOL_INCLUDE_GAURD