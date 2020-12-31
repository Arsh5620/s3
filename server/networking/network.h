#ifndef NETWORK_INCLUDE_GAURD
#define NETWORK_INCLUDE_GAURD

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../general/define.h"

#define NETWORK_READ_BUFFER MB (1)
#define NETWORK_WRITE_BUFFER MB (1)
#define RSA_SERVER_KEY "certificates/CA-key.pem"
#define RSA_SERVER_CERT "certificates/CA-cert.pem"
#define SSL_SUCCESS 1

enum network_errors_enum
{
    NETWORK_SUCCESS = 0,
    NETWORK_ERROR_READ_PARTIAL = 128,
    NETWORK_ERROR_READ_CONNRESET,
    NETWORK_ERROR_READ_ERROR,
    NETWORK_REQUEST_TOO_BIG
};

/**
 * server - server file pointer
 * client - client file pointer
 * server_socket -  socket struct sockaddr_in for the server
 * client_socket -  socket struct sockaddr_in for the client
 * error_code -     int to indicate the error code
 */
typedef struct network_connection
{
    int server;
    int client;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;
    uint error_code;

#ifndef DEBUG
    // tls data that we need
    SSL_CTX *ssl_context;
    SSL *ssl_tls;
    SSL_METHOD *ssl_method;
    X509 *ssl_certificate;
#endif
} network_s;

typedef struct network_connection_data
{
    char *data_address;
    ulong data_length;
    ulong data_maxlen;
    uint error_code;
} network_data_s;

network_s
network_connect_init_sync (int port);
void
network_connect_accept_sync (network_s *connection);

network_data_s
network_read_stream (network_s *connection, ulong size);
/**
 * size must be a power of 2, so 1, 2, 4, 8
 */
long
network_read_primitives (network_s *network, int size, int *error);
void
network_data_free (network_data_s data);

int
network_write_stream (network_s *network, char *buffer, ulong buffer_length);
#endif