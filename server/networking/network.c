/*
 * This file will contain all the functions for successful
 * communication with the client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "./network.h"
#include "../memdbg/memory.h"
#include "../output/output.h"
#include "../logger/logs.h"

/*
 * On any unrecoverable failure exit() will be called
 */
void
assert (int c1, char *fn, int err)
{
    if (c1 == GENERAL_ERROR)
    {
        output_handle (
          OUTPUT_HANDLE_BOTH,
          LOG_EXIT_SET (LOGGER_LEVEL_CATASTROPHIC, err),
          NETWORK_ASSERT_MESSAGE,
          fn,
          strerror (errno),
          errno);
    }
}

void
assert_ssl (int c1, int c2, char *fn, int err)
{
    if (c1 == c2)
    {
        ulong buff_len = 1024;
        char *buffer = m_calloc (buff_len, MEMORY_FILE_LINE);
        ERR_error_string_n (ERR_get_error (), buffer, buff_len);

        output_handle (
          OUTPUT_HANDLE_BOTH,
          LOG_EXIT_SET (LOGGER_LEVEL_CATASTROPHIC, err),
          NETWORK_ASSERT_SSL_MESSAGE,
          fn,
          errno,
          buffer);
        free (buffer); // unreachable code
    }
}

/*
 * this function will initialize a new socket, bind it, and then
 * setup listening on the port, it will not accept a connection.
 */
network_s
network_connect_init_sync (int port)
{
    network_s connection = {0};
    int result = 0;

#ifndef DEBUG

    // check for certificate and private-key should already be done.
    result = SSL_library_init ();
    assert_ssl (result, NULL_ZERO, "SSL_library_init", SERVER_TLS_LIB_INIT_FAILED);

    result = SSL_load_error_strings ();
    assert_ssl (result, NULL_ZERO, "SSL_load_error_strings", SERVER_TLS_LIB_INIT_FAILED);

    connection.ssl_method = (SSL_METHOD *) TLS_server_method ();
    connection.ssl_context = SSL_CTX_new (connection.ssl_method);

    assert_ssl (result, NULL_ZERO, "SSL_library_init", SERVER_TLS_LIB_INIT_FAILED);

    if (
      SSL_CTX_use_certificate_file (connection.ssl_context, RSA_SERVER_CERT, SSL_FILETYPE_PEM)
      != SSL_SUCCESS)
    {
        assert_ssl (
          NULL_ZERO, NULL_ZERO, "SSL_CTX_use_certificate_file", SERVER_TLS_LOAD_CERT_FAILED);
    }

    if (
      SSL_CTX_use_PrivateKey_file (connection.ssl_context, RSA_SERVER_KEY, SSL_FILETYPE_PEM)
      != SSL_SUCCESS)
    {
        assert_ssl (
          NULL_ZERO, NULL_ZERO, "SSL_CTX_use_PrivateKey_file", SERVER_TLS_LOAD_KEY_FAILED);
    }

    if (SSL_CTX_check_private_key (connection.ssl_context) != SSL_SUCCESS)
    {
        assert_ssl (
          NULL_ZERO, NULL_ZERO, "SSL_CTX_check_private_key", SERVER_TLS_CERT_KEY_CHK_FAILED);
    }

#endif

    connection.server = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert (connection.server, "socket", SERVER_SOCK_INIT_FAILED);

    connection.server_socket.sin_addr.s_addr = INADDR_ANY;
    connection.server_socket.sin_family = AF_INET;
    connection.server_socket.sin_port = htons (port);

#ifndef RELEASE_BUILD
    int enable = 1;
    result = setsockopt (connection.server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int));
    assert (result, "setsockopt", SERVER_SET_SOCKOPT_FAILED);

    result
      = setsockopt (connection.server, IPPROTO_TCP, TCP_NODELAY, (char *) &enable, sizeof (int));
    assert (result, "setsockopt", SERVER_SET_SOCKOPT_FAILED);
#endif

    result = bind (
      connection.server,
      (struct sockaddr *) &connection.server_socket,
      sizeof (struct sockaddr_in));
    assert (result, "bind", SERVER_BIND_FAILED);

    result = listen (connection.server, NETWORK_QUEQUE);
    assert (result, "listen", SERVER_LISTEN_FAILED);

    output_handle (
      OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO, NETWORK_PORT_LISTENING, port, NETWORK_QUEQUE);
    return connection;
}

/*
 * On any unrecoverable failure exit() will be called
 * this function is sync and will block until a client connects.
 */
void
network_connect_accept_sync (network_s *connection)
{
    socklen_t len = sizeof (struct sockaddr_in);
    connection->client
      = accept (connection->server, (struct sockaddr *) &connection->client_socket, &len);
    assert (connection->client, "accept", SERVER_ACCEPT_FAILED);

#ifndef DEBUG

    connection->ssl_tls = SSL_new (connection->ssl_context);
    if (connection->ssl_tls == NULL)
    {
        assert_ssl (NULL_ZERO, NULL_ZERO, "SSL_new", SERVER_TLS_SSL_NEW_FAILED);
    }

    int result = SSL_set_fd (connection->ssl_tls, connection->client);
    assert_ssl (result, NULL_ZERO, "SSL_set_fd", SERVER_TLS_SET_FILE_DESCRIPTOR_FAILED);

    result = SSL_accept (connection->ssl_tls);
    if (result != SSL_SUCCESS)
    {
        assert_ssl (NULL_ZERO, NULL_ZERO, "SSL_accept", SERVER_TLS_SSL_ACCEPT_FAILED);
    }

    output_handle (
      OUTPUT_HANDLE_LOGS,
      LOGGER_LEVEL_INFO,
      NETWORK_ASSERT_SSL_LEVEL_MESSAGE,
      SSL_get_cipher (connection->ssl_tls));

#endif
}

/*
 * @returns netconn_data_s which will have all data information.
 * this function will automatically perform memory allocation,
 * and you will have to call network_data_free after release memory
 * when no longer in use
 */
network_data_s
network_read_stream (network_s *connection, ulong size)
{
    network_data_s data = {0};
    if (size > NETWORK_READ_BUFFER)
    {
        size = NETWORK_READ_BUFFER;
        data.error_code = NETWORK_REQUEST_TOO_BIG;
    }

    char *memory = m_malloc (size, MEMORY_FILE_LINE);
    size_t data_read = 0;
    for (; data_read < size;)
    {
#ifndef DEBUG
        ulong length = SSL_read (connection->ssl_tls, memory + data_read, (size - data_read));
#else
        ulong length = read (connection->client, memory + data_read, (size - data_read));
#endif
        if (length == 0)
        {
            if (data_read != size)
            {
                data.error_code = NETWORK_ERROR_READ_PARTIAL;
            }
            else
            {
                data.error_code = NETWORK_ERROR_SUCCESS;
            }
            break;
        }
        else if (length < 0)
        {
            switch (errno)
            {
            case ECONNRESET:
                data.error_code = NETWORK_ERROR_READ_CONNRESET;
                break;

            default:
                data.error_code = NETWORK_ERROR_READ_ERROR;
                break;
            }
            break;
        }
        else
        {
            data_read += length;
        }
    }

    data.data_address = memory;
    data.data_length = data_read;
    return (data);
}

int
network_write_stream (network_s *network, char *buffer, ulong buffer_length)
{
    ulong bytes_written = 0;
    for (ulong length = 0; bytes_written < buffer_length;)
    {
#ifndef DEBUG
        length = SSL_write (network->ssl_tls, buffer, buffer_length);
#else
        length = write (network->client, buffer, buffer_length);
#endif
        if (length > 0)
        {
            bytes_written += length;
        }
    }

    if (bytes_written != buffer_length)
    {
        switch (errno)
        {
        case ECONNRESET:
        {
            return (NETWORK_ERROR_READ_CONNRESET);
        }
        break;

        default:
        {
            return (NETWORK_ERROR_READ_ERROR);
        }
        break;
        }
    }
    return (NETWORK_ERROR_SUCCESS);
}

/*
 * releases memory (if-any) allocated by network_data_readxbytes
 */
void
network_data_free (network_data_s data)
{
    if (data.data_address)
    {
        m_free (data.data_address, MEMORY_FILE_LINE);
    }
}

#define NETWORK_READ(type)                                                                         \
    network_data_atom_s network_read_##type (network_s *network)                                   \
    {                                                                                              \
        network_data_s d0 = network_read_stream (network, sizeof (type));                          \
        network_data_atom_s d1 = {0};                                                              \
        if (d0.error_code == NETWORK_ERROR_SUCCESS)                                                \
        {                                                                                          \
            d1.u.type##_t = *(type *) d0.data_address;                                             \
        }                                                                                          \
        d1.error_code = d0.error_code;                                                             \
        network_data_free (d0);                                                                    \
        return d1;                                                                                 \
    }

NETWORK_READ (char);
NETWORK_READ (short);
NETWORK_READ (int);
NETWORK_READ (long);