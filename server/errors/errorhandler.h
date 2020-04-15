#ifndef ERRORHANDLER_INCLUDE_GAURD
#define ERRORHANDLER_INCLUDE_GAURD

#include "../logger/logs.h"
#include "../general/defines.h"

#define PROTOCOL_LOG_INIT_COMPLETE \
	"logging subsystem has been started"

#define PROTOCOL_NETWORK_SBS_INIT \
	"setting up network subsystem"

#define PROTOCOL_MYSQL_LOGIN_INFO \
	"attempting to connect to mysql server: \n" \
	"host: \"%s\"\n" \
	"port: \"%d\"\n" \
	"username: \"%s\"\n" \
	"database: \"%s\""

#define PROTOCOL_MYSQL_FAILED_CONNECT \
	"failed to connect to mysql, program will now exit"

#define PROTOCOL_NETWORK_WAIT_CONNECT \
	"server is waiting for a client to connect"

#define PROTOCOL_NETWORK_CLIENT_CONNECT \
	"a client connected from host \"%s\" and port number %d"

#define PROTOCOL_SERVER_SHUTDOWN \
	"server is about to shutdown, finishing cleanup before exit"

#define PROTOCOL_CLIENT_CONNECT_ABORTED \
	"client connection has been terminated, reason: \"%s\""

#define PROTOCOL_DOWNLOAD_FILE_NOOPEN \
	"could not open the temporary file for writing upload from the client"

#define PROTOCOL_DOWNLOAD_COMPLETE \
	"data for file name \"%.*s\" uploaded %d bytes " \
	"[status: %d, time: %.3fms, speed: %.2fMb/s]"

#define PROTOCOL_SETUP_ENV_DIR_PERMISSIONS\
	"could not open directory \"%s\", could be directory permissions issue"

#define PROTOCOL_ABORTED_CORRUPTION \
	"connection terminated because of corruption: [0x%.16lx]"

#define PROTOCOL_READ_HEADERS_FAILED \
	"error while reading the stream for key value pairs in headers"

#define PROTOCOL_SHUTDOWN_REASON_FLOW \
	"expected as per program flow"

#define PROTOCOL_SHUTDOWN_REASON_CORRUPT \
	"packet corruption or out of order"

#define PROTOCOL_SHUTDOWN_REASON_UNKNOWN \
	"reason unknown"

#define DATABASE_MYSQL_LIB_INIT_FAILED \
	"could not initialize mysql client library"

#define DATABASE_MYSQL_INIT_FAILED \
	"could not init a mysql handle"

#define DATABASE_MYSQL_AUTH_FAILED \
	"could not connect or authenticate with mysql server, error: %s"

#define DATABASE_MYSQL_CONNECTED \
	"successfully connected to the mysql server"

#define DATABASE_INTEGRITY_CHECK \
	"checking database for tables and schema"

#define DATABASE_INTEGRITY_FAILED \
	"failed to create database/table schema, integrity check failed"

#define DATABASE_INTEGRITY_PING	\
	"checking database for connection and response, ping"

#define DATABASE_CONNECTED_SERVER \
	"connected to mysql database, ping successful, server name: \"%s\""

#define DATABASE_INT_DB_SELECT_FAILED \
	"failed to select the correct database, error: %s"

#define DATABASE_INT_DB_CREATED \
	"database not found, created database: \"%s\""

#define DATABASE_INT_DB_CREATE_FAILED \
	"database \"%s\" already exists"

#define DATABASE_INT_DB_CREATE_ACCESS \
	"selecting database failed, " \
	"mysql \"CREATE\" access required"

#define DATABASE_INT_TABLE_CREATED \
	"table \"%s\" has been created"

#define DATABASE_INT_TABLE_CREATE_FAILED \
	"table \"%s\" could not be created, error: %s"

#define DATABASE_INT_TABLE_FOUND \
	"table \"%s\" already exists"

#define DATABASE_INT_TABLE_NOT_FOUND \
	"table \"%s\" does not exists"

#define NOTIFICATION_GENERAL \
	"client sent a notification: \"%.*s\""

#define NETWORK_ASSERT_MESSAGE \
	"network function \"%s\" error: %s, errno: %d"

#define NETWORK_PORT_LISTENING \
	"network init complete on port %d, " \
	"and is listening with queque length of %d"

#define MYSQLBIND_QUERY_FAILED \
	"mysql bind setup failed, bind setup is required" \
	" for parameterized queries, error: %s"

#define MYSQLBIND_QUERY_RESULT_FAILED \
	"mysql bind setup failed, error when performing" \
	" mysql_store_result, error: %s"

#define MYSQLBIND_QUERY_COLUMN_DISCOVERED \
	"column discovery for bind setup found: %.*s.%.*s"

#define MYSQLBIND_COLUMN_COUNT_ERROR \
	"selective bind columns requesting more column count than avail"\
	", column count avail is %d and request is for %d columns"

#define MYSQLBIND_COLUMN_NOT_FOUND \
	"requested column for find does not exists in "\
	"the table, column name: %.*s and length %d"

#define MYSQLBIND_BIND_COPY_REQUEST \
	"copy of binds requested, for %d columns"

#define MYSQLBIND_BIND_COPY_REQUEST_INFO \
	"column information for bind copy: name \"%.*s\" with length %d"

#define MYSQLBIND_BIND_FREE \
	"bind free for params count %d"

#define FILEMGMT_RECORD_STATUS \
	"record with folder name \"%.*s\", and file name \"%.*s\" has status %d"

#define MEMORY_ALLOCATION_LOG \
	"{\n\t{address:\"%p\", new address:\"%p\"}\n\t, " \
	"{request type: \"%s\", request size: %d}\n\t, " \
	"{file name: \"%s\", file line number: %d}\n}"

#define MEMORY_ALLOCATION_ERROR \
	MEMORY_ALLOCATION_LOG ", error: %s"

#define MEMORY_ALLOCATION_NOENTRY \
	"address not found in table"

#define DBP_CONNECTION_SHUTDOWN_CLEANUP \
	"closing sockets / closing logging subsystem"

// RESPONSE STRINGS

#define DBP_RESPONSE_STRING_ACTION_INVALID \
	"the packet received does not contain a valid action type"

#define DBP_RESPONSE_STRING_HEADER_EMPTY \
	"the packet received was empty, and has been discarded"

#define DBP_RESPONSE_STRING_PARSE_ERROR \
	"the packet received does not follow the correct header key format"

#define DBP_RESPONSE_STRING_THIN_ATTRIBS \
	"the packet received does not contain the " \
	"required attributes for the requested action"

#define DBP_RESPONSE_STRING_DATA_SEND \
	"send data"

#define DBP_RESPONSE_STRING_PACKET_OK \
	"packet ok"

#define DBP_RESPONSE_STRING_CORRUPTED_PACKET \
	"packet header corrupted, connection rejected"

#define DBP_RESPONSE_STRING_CORRUPTED_DATAH \
	"data header corrupted, connection rejected"

#define DBP_RESPONSE_STRING_SETUP_ENV_FAILED \
	"setting up environment failed, connection rejected"

#define DBP_RESPONSE_STRING_ATTIB_VALUE_INVALID \
	"values passed to the packet as headers are rejected, try again"
	
#define ERRORS_HANDLE_STDOUT	0b0001
#define ERRORS_HANDLE_LOGS		0b0010
#define ERRORS_HANDLE_STDOLOG	0b0011

void error_handle(long handle_type
	, enum logger_level log_level, char *format, ...);
#endif // ERRORHANDLER_INCLUDE_GAURD