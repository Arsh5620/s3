#ifndef DEFINE_INCLUDE_GAURD
#define DEFINE_INCLUDE_GAURD

#define FAILED		1
#define SUCCESS		0

#define FALSE       0
#define TRUE		1

#define INVALID_SOCKET	-1
#define BIND_ERROR  	-1
#define GENERAL_ERROR  	-1
#define NULL_ZERO		(0)

#define MB(x)		(x*1024*1024)

#define NETWORK_QUEQUE	8
#define NETWORK_PORT	4096

enum exit_codes_enum 
{
	SERVER_ERROR_TOLOGS = 1
	, SERVER_SOCK_INIT_FAILED
	, SERVER_SET_SOCKOPT_FAILED
	, SERVER_BIND_FAILED
	, SERVER_LISTEN_FAILED
	, SERVER_ACCEPT_FAILED
};

#define IFELSERETURN(x) if ((x) != SUCCESS) { return (FAILED); }
#define MFREEIFNOTNULL(x) if ((x) != NULL) { m_free(x, MEMORY_FILE_LINE);}

typedef char boolean;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#endif