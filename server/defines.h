// This file contains the error codes that the functions will return.
// Keep in mind that the exit status is limited to 8 bit width.

#define FAILED				0x000001
#define SUCCESS				0x000000

#define FALSE               0x000000
#define TRUE                0x000001

#define REALLOC_FAILED				0x000002
#define MALLOC_FAILED				0x000003
#define SERVER_OUT_OF_MEMORY        0x000004
#define SERVER_LOGGER_FAILURE       0x000005
#define SERVER_ERROR_REFER_TO_LOGS  0x000006
#define SERVER_DATABASE_FAILURE     0x000007

#define	SERVER_WSA_STARTUP_FAILED	0x000010
#define SERVER_SOCK_INIT_FAILED		0x000011
#define SERVER_BIND_FAILED			0x000021
#define SERVER_LISTEN_FAILED		0x000031
#define SERVER_ACCEPT_FAILED		0x000041
#define SERVER_ACCEPT_TRYAGAIN		0x000042
#define SERVER_SELECT_FAILED		0X000051
#define SERVER_SEND_FAILED			0x000061

#define SERVER_FILE_OPEN_FAILED		0X000151
#define SERVER_FILE_WRITE_FAILED	0x000161

#define SERVER_MYSQL_SUCCESS		0X000200
#define SERVER_MYSQL_CONNECT_FAILED	0X000201
#define SERVER_MYSQL_DB_ERROR		0X000202
#define SERVER_MYSQL_INIT_ERROR		0X000203
#define SERVER_MYSQL_INIT_REPEAT	0X000204
#define SERVER_MYSQL_EXEC_FAILED	0X000205
#define SERVER_MYSQL_PREPARE_FAILED	0X000206
#define SERVER_MYSQL_STMT_FAILED	0X000207

#define SERVER_AUTH_USER_BLOCKED	0X000300
#define SERVER_AUTH_NAME_INCORRECT	0X000301
#define SERVER_AUTH_PASS_INCORRECT	0X000302

#define SERVER_NETWORK_STATUS_NONVALID 0x000400
#define SERVER_FLOW_OUT_OF_CONTEXT  0X000500


#define MEGABYTES(x)    (x*1024*1024)

#define MAX_ALLOWED_NETWORK_BUFFER  MEGABYTES(8)

#define INVALID_SOCKET	-1
#define BIND_ERROR  	-1
#define GENERAL_ERROR  	-1
#define NULL_ZERO		0