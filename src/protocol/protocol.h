#ifndef SRC_PROTOCOL_PROTOCOL_
#define SRC_PROTOCOL_PROTOCOL_

#include "auth.h"
#include "data/hash_table.h"
#include "database/database.h"
#include "files/file.h"
#include "files/filemgmt.h"
#include "files/path.h"
#include "general/common.h"
#include "general/messages.h"
#include "general/string.h"
#include "logger/logs.h"
#include "memdbg/memory.h"
#include "networking/network.h"
#include "ssbs/list.h"
#include "ssbs/serializer.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

#define S3_PROTOCOL_MAGIC 0xD0
#define S3_PROTOCOL_MAGIC_LEN (8)
#define S3_PROTOCOL_HEADER_MAXLEN (256 << 4)
#define S3_NETWORK_MAX_EPOLL_EVENTS 100
#define S3_DATABASE_FILENAME "schema.db"
#define S3_RESPONSE_KEY_NAME "response"

// attribute name keys
#define S3_ATTRIBNAME_ACTION "action"
#define S3_ATTRIBNAME_CRC "crc"
#define S3_ATTRIBNAME_DIRNAME "dirname"
#define S3_ATTRIBNAME_APPEND "append"
#define S3_ATTRIBNAME_FILENAME "filename"
#define S3_ATTRIBNAME_UPDATEAT "updateat"
#define S3_ATTRIBNAME_UPDATETRIM "trim"
#define S3_ATTRIBNAME_USERNAME "username"
#define S3_ATTRIBNAME_SECRET "secret"

// one-to-one mapping to the actions_supported
enum s3_actions_enum {
    S3_ACTION_CREATE = 0,
    S3_ACTION_DELETE,
    S3_ACTION_DIR,
    S3_ACTION_LOGIN,
    S3_ACTION_NOTIFICATION,
    S3_ACTION_REQUEST,
    S3_ACTION_SERVER,
    S3_ACTION_UPDATE,
    S3_ACTION_INVALID = -1
};

enum s3_attribs_enum {
    S3_ATTRIB_ACTION = 0,
    S3_ATTRIB_APPEND,
    S3_ATTRIB_CRC,
    S3_ATTRIB_DIRNAME,
    S3_ATTRIB_FILENAME,
    S3_ATTRIB_UPDATEAT,
    S3_ATTRIB_UPDATETRIM,
    S3_ATTRIB_USERNAME,
    S3_ATTRIB_PASSWORD
};

enum s3_shutdown_enum { S3_SHUTDOWN_CLOSE, S3_SHUTDOWN_INVALID };

enum s3_response_code {
    S3_RESPONSE_SUCCESS,           // Internal, NOT USED
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
    S3_RESPONSE_FOLDER_NOT_FOUND,
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
    S3_RESPONSE_INVALID_COMMUNICATION,
    S3_RESPONSE_SETUP_ENVIRONMENT_FAILED,
    S3_RESPONSE_SERVER_INTERNAL_ERROR,
    S3_RESPONSE_SERVER_DIR_ERROR,
    S3_RESPONSE_SERVER_FILE_ERROR,
    S3_RESPONSE_SQLITE_INTERNAL_ERROR,
    S3_RESPONSE_SQLITE_NO_CONNECTION,
    S3_RESPONSE_NETWORK_ERROR_WRITE,
    S3_RESPONSE_NETWORK_ERROR_READ,
    S3_RESPONSE_CANNOT_CREATE_TEMP_FILE,
    S3_RESPONSE_ERROR_WRITING_HEADERS,

    // Do not add anything after here
    __S3_RESPONSE_END_ITEMS,
};

#define RESPONSE_RESET_RETURN(statement, network, response_code)                                                                 \
    if (statement) {                                                                                                             \
        s3_connection_reset_with_response (network, response_code);                                                              \
        return response_code > S3_RESPONSE_ERRORS ? FAILED : SUCCESS;                                                            \
    }

extern char *attribs[];
extern char *actions[];
extern char **s3_asserts;
extern char **s3_response_messages;
extern const int attribs_count;
extern const int actions_count;

typedef struct {
    size_t data_length;
    unsigned short header_length;
    unsigned char magic;
} s3_header_s;

typedef struct {
    char *key;
    char *value;
    size_t key_len;
    size_t value_len;
} kvpair_s;

#define KVPAIR(_key, _key_len, _value, _value_len)                                                                               \
    (kvpair_s) { .key = _key, .key_len = _key_len, .value = _value, .value_len = _value_len }

typedef struct request_s {
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
    char *header_raw;
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
} s3_request_s;

typedef struct response_writer {
    char *address;
    size_t index;
    size_t capacity;
} response_writer_s;

typedef struct response_s {
    my_list_s header_list;
    s3_header_s header_info;

    /*
     * response_code is the response indicator, which idicates success or
     * failure after a call to the requested action.
     */
    long response_code;
    filemgmt_file_name_s *file_name;
    response_writer_s writer_buffer;
    long total_write_completed;
} s3_response_s;

typedef struct {
    long update_at;
    boolean trim;
} s3_action_update_s;

typedef struct {
    int epoll_fd;
    int epoll_events_count;
    struct epoll_event *epoll_events;

    int server;
    struct sockaddr_in server_sockv4;
    struct sockaddr_in6 server_sockv6;
    int sock_family;
    int sock_size;

    // These variables are only required when using SSL, so they are not included in debug builds
    SSL_CTX *ssl_context;

    SSL_METHOD *ssl_method;
    X509 *ssl_certificate;
    boolean plain_mode;

    logs_s logs;
    hash_table_s client_table;
} s3_protocol_s;

typedef struct settings {
    s3_log_settings_s log_settings;
    struct {
        int address_type;
        char *interface;
        unsigned short port;
    } ip_settings;
    boolean plain_mode;
} settings_s;

void
s3_print_counter ();

boolean
s3_verify_db ();
void
s3_generate_directories (char **folder_path, char **schema_path);
void
s3_close (s3_protocol_s protocol);
void
s3_handle_close (s3_request_s *request, s3_response_s *response);
ulong
s3_next_request (s3_protocol_s *protocol);
s3_protocol_s
s3_connection_initialize_sync (settings_s settings, boolean *success);
void
s3_connection_accept_loop_async (s3_protocol_s *protocol);
void
s3_connection_shutdown (s3_protocol_s protocol, enum s3_shutdown_enum type);
void
s3_setup_epoll (s3_protocol_s *protocol);
void
s3_epollctl_add_epollin (int epoll_fd, int socket_fd);
s3_header_s
s3_request_read_magic (size_t magic, boolean *valid);
int
s3_request_read_headers (s3_protocol_s *protocol, s3_request_s *request);
int
s3_request_read_action (s3_request_s *request);

void
s3_network_data_free (s3_protocol_s *protocol);
int
s3_network_read_stream_async (s3_protocol_s *protocol, char *memory, int size);

long
s3_action_request_writer (network_s *network);
long
s3_action_dirlist_writer (network_s *response);

int
s3_postprocess_notification (network_s *network);
int
s3_postprocess_create (network_s *network);
int
s3_postprocess_update (network_s *network);
int
s3_postprocess_delete (network_s *network);
int
s3_postprocess_request (network_s *network);
int
s3_postprocess_serverinfo (network_s *network);
int
s3_preprocess_dirlist (s3_request_s *request);
int
s3_preprocess_login (s3_request_s *request);

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
s3_postprocess_login (network_s *network);

int
s3_attribs_assert (hash_table_s table, char **match, int count);
int
s3_attrib_contains (hash_table_s table, char *attrib);

int
s3_file_setup_environment ();
int
s3_file_download (s3_request_s *request);

hash_table_s
s3_header_hash (my_list_s list);
s3_header_s
s3_header_parse8 (size_t magic);
my_list_s
s3_deserialize_headers (char *address, int length, int *error);
void
s3_print_headers (my_list_s list);

int
s3_action_preprocess (s3_request_s *request);
int
s3_action_postprocess (network_s *network);
int
s3_action_send (network_s *network);

int
s3_handle_response (network_s *network, enum s3_response_code code);
int
s3_response_write (network_s *network, long (*writer) (network_s *));
long
s3_response_write_header (s3_response_s *response, char *buffer, ulong buffer_length);
ulong
s3_response_make_magic (s3_response_s *response);
int
s3_response_accept_status (network_s *response);
int
s3_response_writer_update (s3_response_s *response, string_s write);

void
s3_connection_alloc (network_s *data);
void
s3_connection_free (network_s *data);
hash_table_s
make_hash_table (my_list_s list);
int
s3_request_data (s3_protocol_s *protocol, s3_request_s *request);
int
s3_request_data_headers (s3_protocol_s *protocol, s3_request_s *request);
int
s3_connection_confirm_upload (network_s *network);
int
s3_connection_upload (network_s *network);
int
s3_connection_post_process (network_s *network);
void
s3_connection_reset_new_packet (network_s *network);

int
s3_auth_query (s3_request_s *request);
int
s3_auth_transaction (s3_request_s *request);
int
s3_auth_add_user (char *username, char *passphrase, int username_length, int passphrase_length);
int
s3_auth_query_sqlite3 (char *username, int username_length, char *password, int password_length);
int
s3_auth_insert_sqlite3 (char *username, int username_length, char *password, int password_length);

int
s3_copy_keyvaluepairs (my_list_s source_list, my_list_s *dest_list);

int
s3_connection_magic_read (network_s *data);
int
s3_connection_headers_read (network_s *data);

int
s3_setup_environment (s3_request_s *request);
char *
s3_attrib_get (hash_table_s table, char *attrib, size_t *out_length, boolean *is_valid);
void
s3_connection_abort (network_s *network, hash_table_s *client_table);
void
s3_connection_reset_with_response (network_s *network, enum s3_response_code response_code);
void
s3_dirlist_serialize_sqltype (enum sqltypes type, sqlite_result_s *result, serializer_t *serializer);
boolean
s3_connection_temp_file_required (enum s3_actions_enum action);
void
s3_exit_sigint_fired ();
#endif // SRC_PROTOCOL_PROTOCOL_