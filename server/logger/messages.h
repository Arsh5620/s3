#ifndef MESSAGES_INCLUDE_GAURD
#define MESSAGES_INCLUDE_GAURD

#include "../logger/logs.h"
#include "../general/define.h"

#define PROTOCOL_LOG_INIT_COMPLETE "logging sub-system has been started"

#define PROTOCOL_NETWORK_SBS_INIT "setting up network sub-system now"

#define PROTOCOL_MYSQL_LOGIN_INFO                                                                  \
    "attempting to connect to mysql server: \n"                                                    \
    "host: \"%s\"\n"                                                                               \
    "port: \"%d\"\n"                                                                               \
    "username: \"%s\"\n"                                                                           \
    "database: \"%s\""

#define PROTOCOL_MYSQL_FAILED_CONNECT "failed to connect to mysql server, program will now exit"

#define PROTOCOL_NETWORK_WAIT_CONNECT "server is waiting for a client to connect"

#define PROTOCOL_NETWORK_CLIENT_CONNECT                                                            \
    "a client connected from address \"%s\" and port number (%d)"

#define PROTOCOL_SERVER_SHUTDOWN "server is about to shutdown, finishing cleanup before exit"

#define PROTOCOL_CLIENT_CONNECT_ABORTED "client connection has been terminated for reason: \"%s\""

#define PROTOCOL_DOWNLOAD_FILE_NOOPEN                                                              \
    "could not open the temporary file for writing data from the client"

#define PROTOCOL_DOWNLOAD_COMPLETE                                                                 \
    "client sent file name \"%.*s\" for (%d) bytes "                                               \
    "[status: %d, time: %.3fms, speed: %.2fMb/s]"

#define PROTOCOL_SETUP_ENV_DIR_PERMISSIONS "could not open directory \"%s\", permission denied"

#define PROTOCOL_ABORTED_CORRUPTION "connection terminated because of corruption: [0x%.16lx]"

#define PROTOCOL_READ_HEADERS_FAILED "error while reading the key value pairs in the buffer"

#define PROTOCOL_SHUTDOWN_REASON_FLOW "expected as per program flow"

#define PROTOCOL_SHUTDOWN_REASON_CORRUPT "packet corruption or out of order"

#define PROTOCOL_SHUTDOWN_REASON_UNKNOWN "reason unknown"

#define DATABASE_MYSQL_LIB_INIT_FAILED "could not initialize mysql client library"

#define DATABASE_MYSQL_INIT_FAILED "could not initialize a mysql handle to attempt connection"

#define DATABASE_MYSQL_AUTH_FAILED "could not connect or authenticate with mysql server, error: %s"

#define DATABASE_MYSQL_CONNECTED "successfully connected to the mysql server"

#define DATABASE_INTEGRITY_CHECK "checking database for tables and schema"

#define DATABASE_INTEGRITY_FAILED "failed to create database/table schema, integrity check failed"

#define DATABASE_INTEGRITY_PING "checking database for connection and response, pinging server"

#define DATABASE_CONNECTED_SERVER                                                                  \
    "connected to mysql database, ping successful, server name: \"%s\""

#define DATABASE_INT_DB_SELECT_FAILED "failed to select the correct database, error: %s"

#define DATABASE_INT_DB_CREATED "database not found, created database: \"%s\""

#define DATABASE_INT_DB_CREATE_FAILED "database \"%s\" already exists"

#define DATABASE_INT_DB_CREATE_ACCESS                                                              \
    "selecting database failed, "                                                                  \
    "mysql \"CREATE\" access required"

#define DATABASE_INT_TABLE_CREATED "table \"%s\" has been created"

#define DATABASE_INT_TABLE_CREATE_FAILED "table \"%s\" could not be created, error: %s"

#define DATABASE_INT_TABLE_FOUND "table \"%s\" already exists"

#define DATABASE_INT_TABLE_NOT_FOUND "table \"%s\" does not exists"

#define NOTIFICATION_GENERAL "client sent a notification: \"%.*s\""

#define NETWORK_ASSERT_MESSAGE "network function \"%s\" error: %s, errno: %d"

#define NETWORK_ASSERT_SSL_MESSAGE "tls ssl function \"%s\" failed errno: %d, more information:\n%s"

#define NETWORK_ASSERT_SSL_LEVEL_MESSAGE                                                           \
    "tls ssl negotiation connected to client over \"%s\" cipher"

#define NETWORK_PORT_LISTENING                                                                     \
    "network initialization complete on port number (%d), "                                        \
    "server now listening with a queque length of (%d)"

#define MYSQLBIND_QUERY_FAILED                                                                     \
    "mysql bind setup failed, setup required"                                                      \
    " for querying the server, error: %s"

#define MYSQLBIND_QUERY_RESULT_FAILED                                                              \
    "mysql bind setup failed, error when performing"                                               \
    " mysql_store_result, error: %s"

#define MYSQLBIND_QUERY_COLUMN_DISCOVERED "column discovery for bind setup found: [%.*s::%.*s]"

#define MYSQLBIND_COLUMN_COUNT_ERROR                                                               \
    "selective bind columns requesting more column count than avail"                               \
    ", column count avail is %d and request is for %d columns"

#define MYSQLBIND_COLUMN_NOT_FOUND                                                                 \
    "requested column for find does not exists in "                                                \
    "the table, column name: %.*s and length %d"

#define MYSQLBIND_BIND_COPY_REQUEST "copy of binds requested for %d columns"

#define MYSQLBIND_BIND_COPY_REQUEST_INFO                                                           \
    "column information for bind copy: name \"%.*s\" with length %d"

#define MYSQLBIND_BIND_FREE "bind release requested for %d columns"

#define FILEMGMT_RECORD_EXISTS "file \"%.*s\" already exists"

#define MEMORY_ALLOCATION_LOG                                                                      \
    "{\n\t{address:\"%p\", new address:\"%p\"}\n\t, "                                              \
    "{request type: \"%s\", request size: %d}\n\t, "                                               \
    "{file name: \"%s\", file line number: %d}\n}"

#define MEMORY_ALLOCATION_ERROR MEMORY_ALLOCATION_LOG ", error: %s"

#define MEMORY_ALLOCATION_NOENTRY "address not found in table"

#define DBP_CONNECTION_SHUTDOWN_CLEANUP "closing sockets / closing logging sub-system"

// RESPONSE STRINGS

#define DBP_RESPONSE_STRING_ACTION_INVALID "Invalid action type."

#define DBP_RESPONSE_STRING_HEADER_EMPTY "Packet is empty."

#define DBP_RESPONSE_STRING_PARSE_ERROR "Invalid header format."

#define DBP_RESPONSE_STRING_THIN_ATTRIBS "Required attributes not present."

#define DBP_RESPONSE_STRING_DATA_SEND "Send data"

#define DBP_RESPONSE_STRING_PACKET_OK "OK"

#define DBP_RESPONSE_STRING_CORRUPTED_PACKET "Packet header corrupted, conn-rejected."

#define DBP_RESPONSE_STRING_CORRUPTED_DATA_HEADERS "Data header corrupted, conn-rejected."

#define DBP_RESPONSE_STRING_SETUP_ENV_FAILED "Server error, cannot create environment."

#define DBP_RESPONSE_STRING_ATTIB_VALUE_INVALID "Invalid values."

#define DBP_RESPONSE_STRING_FILE_EXISTS_ALREADY "File already exists."

#define DBP_RESPONSE_STRING_GENERAL_SERVER_ERROR "Server error."

#define DBP_RESPONSE_STRING_FILE_NOT_FOUND "File does not exists"

#define DBP_RESPONSE_STRING_FILE_UPDATE_OUTOFBOUNDS "Update index out of bounds."

#define PARSER_STDOUT_ERROR_STRING "status code: %ld, line no: %ld, index no: %ld, line length: %d"

#define DBP_RESPONSE_STRING_DATA_NONE_NEEDED "Data not required."

#define DBP_RESPONSE_STRING_FAILED_AUTHENTICATION "Failed authentication."

#define MESSAGE_OUT_STDOUT 0b0001
#define MESSAGE_OUT_LOGS 0b0010
#define MESSAGE_OUT_BOTH 0b0011

void
my_print (long log_type, enum logger_level log_level, char *format, ...);
#endif 