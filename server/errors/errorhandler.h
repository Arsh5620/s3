#ifndef ERRORHANDLER_INCLUDE_GAURD
#define ERRORHANDLER_INCLUDE_GAURD

#include "../logger/logs.h"
#include "../general/defines.h"

/* 
 * error message string name must begin with the filename prefix
 * and must end with the arguments required for the printf call. 
 * for example: ERRORHANDLER_INCLUDEEXCEPTION_SS, means the error 
 * message is for file name errorhandler.[c|h] and it requires two 
 * arguments both of which are string. another example would be
 * PROTOCOL_NULLPACKET_SI, protocol.c [string, int]
 * and no suffix if the string would not require argument
 * all suffixes must be notated in this document with their acronyms
 * String: S
 * Int: I
 * Unsigned Int: i
 * Long: L
 * Unsigned Long: l
 * Char: C
 * Unsigned Char: c
 * Short: S
 * Unsigned Short: s
 * Double: D
 * Float: F
 */

#define PROTOCOL_LOG_INIT_COMPLETE \
	"logging subsystem has been started"
#define PROTOCOL_NETWORK_SBS_INIT \
	"setting up network subsystem"
#define PROTOCOL_MYSQL_LOGIN_INFO_SISS \
	"attempting to connect to mysql server: \n" \
	"host: \"%s\"\n" \
	"port: \"%d\"\n" \
	"username: \"%s\"\n" \
	"database: \"%s\""
#define PROTOCOL_MYSQL_FAILED_CONNECT \
	"failed to connect to mysql, program will now exit"
#define PROTOCOL_NETWORK_WAIT_CONNECT \
	"server is waiting for a client to connect"
#define PROTOCOL_NETWORK_CLIENT_CONNECT_SI \
	"a client connected from host \"%s\" and port number %d"
#define PROTOCOL_SERVER_SHUTDOWN \
	"server is about to shutdown, finishing cleanup before exit"
#define PROTOCOL_CLIENT_CONNECT_ABORTED_S \
	"client connection has been terminated, reason: \"%s\""
#define PROTOCOL_DOWNLOAD_FILE_NOOPEN \
	"could not open the temporary file for writing upload from the client"
#define PROTOCOL_DOWNLOAD_COMPLETE_ISIIFF \
	"data for file name \"%.*s\" uploaded %d bytes " \
	"[status: %d, time: %.3fms, speed: %.2fMb/s]"
#define PROTOCOL_SETUP_ENV_DIR_PERMISSIONS_S \
	"could not open directory \"%s\", could be directory permissions issue"
#define PROTOCOL_ABORTED_CORRUPTION_L \
	"connection terminated because of corruption: [0x%.16lx]"
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
#define DATABASE_MYSQL_AUTH_FAILED_S \
	"could not connect or authenticate with mysql server, error: %s"
#define DATABASE_MYSQL_CONNECTED \
	"successfully connected to the mysql server"
#define DATABASE_INTEGRITY_CHECK \
	"checking database for tables and schema"
#define DATABASE_INTEGRITY_FAILED \
	"failed to create database/table schema, integrity check failed"
#define DATABASE_INTEGRITY_PING	\
	"checking database for connection and response, ping"
#define DATABASE_CONNECTED_SERVER_S \
	"connected to mysql database, ping successful, server name: \"%s\""
#define DATABASE_INT_DB_SELECT_FAILED_S \
	"failed to select the correct database, error: %s"
#define DATABASE_INT_DB_CREATED_S \
	"database not found, created database: \"%s\""
#define DATABASE_INT_DB_CREATE_FAILED_S \
	"database \"%s\" already exists"
#define DATABASE_INT_DB_CREATE_ACCESS \
	"selecting database failed, " \
	"mysql \"CREATE\" access required"
#define DATABASE_INT_TABLE_CREATED_S \
	"table \"%s\" has been created"
#define DATABASE_INT_TABLE_CREATE_FAILED_SS \
	"table \"%s\" could not be created, error: %s"
#define DATABASE_INT_TABLE_FOUND_S \
	"table \"%s\" already exists"
#define DATABASE_INT_TABLE_NOT_FOUND_S \
	"table \"%s\" does not exists"
#define NOTIFICATION_GENERAL_IS \
	"client sent a notification: \"%.*s\""
#define NETWORK_ASSERT_MESSAGE_SSI \
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

#define ERRORS_HANDLE_STDOUT	0b0001
#define ERRORS_HANDLE_LOGS		0b0010
#define ERRORS_HANDLE_STDOLOG	0b0011

void error_handle(long handle_type
	, enum logger_level log_level, char *format, ...);
#endif // ERRORHANDLER_INCLUDE_GAURD