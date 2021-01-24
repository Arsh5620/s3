#ifndef DEFINE_INCLUDE_GAURD
#define DEFINE_INCLUDE_GAURD

#define FAILED 1
#define SUCCESS 0

#define FALSE 0
#define TRUE 1

#define INVALID_SOCKET -1
#define BIND_ERROR -1
#define GENERAL_ERROR -1
#define NULL_ZERO (0)

#define MB(x) (x * 1024 * 1024)

#define NETWORK_QUEQUE 8
#define NETWORK_PORT 4096
#define APPLICATION_NAME "S3"
enum exit_codes_enum
{
    SERVER_ERROR_TOLOGS = 1,
    SERVER_SOCK_INIT_FAILED,
    SERVER_SET_SOCKOPT_FAILED,
    SERVER_BIND_FAILED,
    SERVER_LISTEN_FAILED,
    SERVER_ACCEPT_FAILED,
    SERVER_TLS_LIB_INIT_FAILED,
    SERVER_TLS_LOAD_CERT_FAILED,
    SERVER_TLS_LOAD_KEY_FAILED,
    SERVER_TLS_CERT_KEY_CHK_FAILED,
    SERVER_TLS_SSL_NEW_FAILED,
    SERVER_TLS_SET_FILE_DESCRIPTOR_FAILED,
    SERVER_TLS_SSL_ACCEPT_FAILED
};

#define ASSERT(x)                                                                                  \
    if ((x) != SUCCESS)                                                                            \
    {                                                                                              \
        return (FAILED);                                                                           \
    }
#define m_free_null_check(x)                                                                       \
    if ((x) != NULL)                                                                               \
    {                                                                                              \
        m_free (x);                                                                                \
    }

typedef char boolean;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#endif