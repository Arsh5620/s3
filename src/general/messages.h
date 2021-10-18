#ifndef SRC_GENERAL_MESSAGES_
#define SRC_GENERAL_MESSAGES_

#define PROTOCOL_LOG_INIT_COMPLETE "logging sub-system has been started"

#define PROTOCOL_NETWORK_SBS_INIT "starting network sub-system now"

#define PROTOCOL_SETUP_FINISH "protocol initialize sync setup complete"

#define PROTOCOL_DATABASE_SETUP "setting up connection to sqlite using version " SQLITE_VERSION

#define PROTOCOL_MYSQL_LOGIN_INFO                                                                                                \
    "attempting to connect to mysql server: \n"                                                                                  \
    "host: \"%s\"\n"                                                                                                             \
    "port: \"%d\"\n"                                                                                                             \
    "username: \"%s\"\n"                                                                                                         \
    "database: \"%s\""

#define PROTOCOL_MYSQL_FAILED_CONNECT "failed to connect to mysql server, program will now exit"

#define PROTOCOL_BACKUP_FOLDER_NOT_CREATED "failed while creating required directories for application data"

#define PROTOCOL_EPOLL_CREATE_FAILED "error trying to create epoll, %s"

#define PROTOCOL_EPOLL_CTL_FAILED "error trying to ctl epoll, %s"

#define PROTOCOL_EPOLL_WAIT_FAILED "error trying to wait for fds on epoll, %s"

#define PROTOCOL_NETWORK_WAIT_CONNECT "entering connection accept loop, waiting for a client..."

#define PROTOCOL_NETWORK_CLIENT_CONNECT "a client connected from address \"%s\" and port number (%d)"

#define PROTOCOL_SERVER_SHUTDOWN "server is about to shutdown, finishing cleanup before exiting"

#define PROTOCOL_CLIENT_CONNECT_ABORTED "client connection has been terminated for reason: \"%s\""

#define PROTOCOL_DOWNLOAD_FILE_NOOPEN "could not open the temporary file for writing data from the client"

#define PROTOCOL_DOWNLOAD_COMPLETE                                                                                               \
    "downloaded file name \"%.*s\", (%d) bytes, "                                                                                \
    "[time: %ld ms]"

#define PROTOCOL_SETUP_ENV_DIR_PERMISSIONS "could not open directory \"%s\", permission denied"

#define PROTOCOL_ABORTED_CORRUPTION "client magic is not correct, cannot identify packet: [0x%.16lx]"

#define PROTOCOL_READ_HEADERS_FAILED "network error while reading the header information"

#define REQUEST_ACTION_TYPE "Packet requests action type: %d"

#define RESPONSE_CODE "Packet response code: %d"

#define PROTOCOL_SHUTDOWN_REASON_FLOW "expected as per program flow"

#define PROTOCOL_SHUTDOWN_REASON_CORRUPT "packet corruption or out of order"

#define PROTOCOL_SHUTDOWN_REASON_UNKNOWN "reason unknown"

#define DATABASE_FILE_FOUND "database file %s, either found or created"

#define DATABASE_SCHEMA_CHECK "checking table with query: %s"

#define DESERIALIZER_PRINT_HEADERS "found %d key value pairs after deserializing header : "

#define DESERIALIZER_PRINT_HEADERS_ROW_STRING "\t\tkey, value: %.*s, %.*s, %ld, %f, %lf"

#define NOTIFICATION_GENERAL "client sent a notification: \"%.*s\""

#define NETWORK_ASSERT_MESSAGE "network function \"%s\" error: %s, errno: %d"

#define NETWORK_READ_ERROR "network returned an error while trying to perform a read operation"

#define NETWORK_INVALID_READ_ARGS "invalid arguments to network_read_stream, read_Info cannot be null"

#define NETWORK_READ_REQUEST_TOO_BIG "network read request too big to handle in-memory"

#define NETWORK_ASSERT_SSL_MESSAGE "tls ssl function \"%s\" failed errno: %d, more information:\n%s"

#define NETWORK_ASSERT_SSL_LEVEL_MESSAGE "tls ssl negotiation connected to client over \"%s\" cipher"

#define NETWORK_SSL_PLAIN_CLIENT "A client tried to connect over plain mode, rejected"

#define NETWORK_PORT_LISTENING                                                                                                   \
    "network initialization complete on port number (%d), "                                                                      \
    "server now listening with a queque length of (%d)"

#define NETWORK_MODE_PLAIN_INITED "network successfully initialized Plain mode"

#define NETWORK_MODE_SSL_INITED "network successfully initialized SSL mode"

#define NETWORK_IP_DEBUG "Discovered interface %-8s %-10s (%2d) [%s]"

#define MYSQLBIND_QUERY_FAILED                                                                                                   \
    "mysql bind setup failed, setup required"                                                                                    \
    " for querying the server, error: %s"

#define MYSQLBIND_QUERY_RESULT_FAILED                                                                                            \
    "mysql bind setup failed, error when performing"                                                                             \
    " mysql_store_result, error: %s"

#define MYSQLBIND_QUERY_COLUMN_DISCOVERED "column discovery for bind setup found: [%.*s::%.*s]"

#define MYSQLBIND_COLUMN_COUNT_ERROR                                                                                             \
    "selective bind columns requesting more column count than avail"                                                             \
    ", column count avail is %d and request is for %d columns"

#define MYSQLBIND_COLUMN_NOT_FOUND                                                                                               \
    "requested column for find does not exists in "                                                                              \
    "the table, column name: %.*s and length %d"

#define MYSQLBIND_BIND_FREE "bind release requested for %d columns"

#define FILEMGMT_RECORD_EXISTS "file \"%.*s\" found"

#define MEMORY_ALLOCATION_LOG                                                                                                    \
    "{\n\tAddress:\"%p [%p]\"\n"                                                                                                 \
    "\tRequest: \"%s (%d)\"\n"                                                                                                   \
    "\tLocation: \"%s:%d\"\n}"

#define MEMORY_UNFREE_LOG_TITLE "ADDRESS:REALLOC-ADDRESS [LAST-TYPE]"

#define MEMORY_UNFREE_LOG "* %p:%p [%s]"

#define MEMORY_UNFREE_LOG_UPDATES_TITLE ">\tFILE:LINE [SIZE:TYPE]"

#define MEMORY_UNFREE_LOG_UPDATES ">\t%s:%d [%d:%s]"

#define MEMORY_ALLOCATION_ERROR MEMORY_ALLOCATION_LOG ", error: %s"

#define MEMORY_ALLOCATION_NOENTRY "address not found in table"

#define S3_CONNECTION_SHUTDOWN_CLEANUP "closing sockets / closing logging sub-system"

// response strings

#define S3_RESPONSE_STRING_ACTION_INVALID "Invalid action type."

#define S3_RESPONSE_STRING_HEADER_EMPTY "Packet is empty."

#define S3_RESPONSE_STRING_DESERIALIZER_ERROR "Invalid header format."

#define S3_RESPONSE_STRING_MISSING_ATTRIBS "Required attributes not present."

#define S3_RESPONSE_STRING_DATA_SEND "Send data"

#define S3_RESPONSE_STRING_PACKET_OK "OK"

#define S3_RESPONSE_STRING_CORRUPTED_PACKET "Packet header corrupted, conn-rejected."

#define S3_RESPONSE_STRING_CORRUPTED_DATA_HEADERS "Data header corrupted, conn-rejected."

#define S3_RESPONSE_STRING_SETUP_ENVIRONMENT_FAILED "Server error, cannot find or create the upload folder"

#define S3_RESPONSE_STRING_ATTIB_VALUE_INVALID "Invalid values."

#define S3_RESPONSE_STRING_FILE_EXISTS_ALREADY "File already exists."

#define S3_RESPONSE_STRING_SERVER_INTERNAL_ERROR "Internal server error."

#define S3_RESPONSE_STRING_FILE_NOT_FOUND "File does not exists"

#define S3_RESPONSE_STRING_FILE_UPDATE_OUTOFBOUNDS "Update index out of bounds."

#define PARSER_STDOUT_ERROR_STRING "status code: %ld, line no: %ld, index no: %ld, line length: %d"

#define S3_RESPONSE_STRING_UNEXPECTED_DATA_FROM_CLIENT "Unexpected data from client for the requested action type."

#define S3_RESPONSE_STRING_FAILED_AUTHENTICATION "Failed authentication."
#endif // SRC_GENERAL_MESSAGES_