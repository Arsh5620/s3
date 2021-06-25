#ifndef NETWORK_INCLUDE_GAURD
#define NETWORK_INCLUDE_GAURD

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>

#include "../general/define.h"

#define NETWORK_READ_BUFFER KB (4)
#define NETWORK_WRITE_BUFFER KB (4)
#define RSA_SERVER_KEY "certificates/CA-key.pem"
#define RSA_SERVER_CERT "certificates/CA-cert.pem"
#define SSL_SUCCESS 1
#define N_READ_SET(x) ({0, x})

enum network_errors_enum {
    NETWORK_SUCCESS = 0,
    NETWORK_ASYNC_WOULDBLOCK = 1,
    NETWORK_ERROR_EOFUNEXPECTED = 128,
    NETWORK_ERROR_CONNRESET,
    NETWORK_ERROR_READ_ERROR,
    NETWORK_REQUEST_TOO_BIG
};

typedef struct {
    char *address;
    ulong index;
    ulong length;
    enum network_errors_enum error_code;
} network_read_s;

/**
 * client - client file pointer
 * client_socket -  socket struct sockaddr_in for the client
 */
typedef struct network_connection {
    int client;
    struct sockaddr_in client_socket;

#ifndef DEBUG
    SSL *ssl_tls;
#endif

    // We prioritize writing to temporary files over writing to in-memory buffer to save memory
    // This is necessary as we are not going to allocate any more than 4K memory per client.
    network_read_s data_read;
    FILE *temporary_file;
    void (*network_completed_action) (struct network_connection *);

    void *response;
    void *request;
} network_s;

void
network_connect_init_sync (int port,
                           int *server,
                           struct sockaddr_in *server_socket
#ifndef DEBUG
                           ,
                           SSL_CTX **ssl_context,
                           SSL_METHOD **ssl_method
#endif
);
void
network_connect_accept_sync (network_s *connection,
                             int server
#ifndef DEBUG
                             ,
                             SSL_CTX *ssl_context
#endif
);
void
network_complete_async_read (network_read_s *read);
network_read_s
network_read_with_size (int size);
void
network_read_stream (network_s *connection);
void
network_read_free (network_read_s data);

int
network_write_stream (network_s *network, char *buffer, ulong buffer_length);
#endif