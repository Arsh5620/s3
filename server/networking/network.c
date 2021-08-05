/*
 * This file will contain all the functions for successful
 * communication with the client.
 */

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <sys/socket.h>

#include "../logger/logs.h"
#include "../logger/messages.h"
#include "../memdbg/memory.h"
#include "./network.h"

/*
 * On any unrecoverable failure exit() will be called
 */
void
assert (int c1, char *fn, int err)
{
    if (c1 == GENERAL_ERROR) {
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, NETWORK_ASSERT_MESSAGE, fn, strerror (errno), errno);
    }
}

void
assert_ssl (int c1, int c2, char *fn, int err)
{
    if (c1 == c2) {
        ulong buff_len = 1024;
        char *buffer = m_calloc (buff_len);
        ERR_error_string_n (ERR_get_error (), buffer, buff_len);

        logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, NETWORK_ASSERT_SSL_MESSAGE, fn, errno, buffer);
        m_free (buffer); // unreachable code
    }
}

// @interface: null terminated interface name string
// @returns: struct sockaddr for the found interface
// check the @error for errors, only last error recorded
// check `man 7 getifaddrs` for more details
int
network_iterate_all_available_ips (char *interface,
                                   int addr_type,
                                   const char **error,
                                   struct sockaddr_in *socket_v4,
                                   struct sockaddr_in6 *socket_v6,
                                   int *sock_family,
                                   int *sock_size)
{
    struct ifaddrs *if_addresses;
    int family;

    if (getifaddrs (&if_addresses) == -1) {
        if (error != NULL) {
            *error = strerror (errno);
        }

        return FAILED;
    }

    struct ifaddrs *selected_address = NULL;
    struct ifaddrs *if_address = if_addresses;
    for (; if_address != NULL; if_address = if_address->ifa_next) {
        if (if_address->ifa_addr == NULL)
            continue;

        family = if_address->ifa_addr->sa_family;

        if ((addr_type != 0 && family != addr_type) || (family != AF_INET && family != AF_INET6)) {
            continue;
        }

        if (selected_address == NULL) {
            if (interface != NULL) {
                if (strcmp (if_address->ifa_name, interface) == 0) {
                    selected_address = if_address;
                }
            } else {
                selected_address = if_address;
            }
        }
        char *family_str = family == AF_INET ? "AF_INET" : family == AF_INET6 ? "AF_INET6" : "UNKNOWN";
        char *host = network_gethostname (family, if_address->ifa_addr, error);
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_DEBUG, NETWORK_IP_DEBUG, if_address->ifa_name, family_str, family, host);
        m_free (host);
    }

    if (selected_address != NULL) {
        switch (selected_address->ifa_addr->sa_family) {
            case AF_INET:
                *socket_v4 = *(struct sockaddr_in *) selected_address->ifa_addr;
                *sock_family = AF_INET;
                *sock_size = sizeof (struct sockaddr_in);
                break;
            case AF_INET6:
                *socket_v6 = *(struct sockaddr_in6 *) selected_address->ifa_addr;
                *sock_family = AF_INET6;
                *sock_size = sizeof (struct sockaddr_in6);
                break;
        }
    }

    freeifaddrs (if_addresses);

    return socket_v4 != NULL || socket_v6 != NULL ? SUCCESS : FAILED;
}

char *
network_gethostname (int family, struct sockaddr *sock, const char **error)
{
    char *host = m_malloc (NETWORK_HOST_MAXLENGTH);
    int struct_size = family == AF_INET ? sizeof (struct sockaddr_in) : sizeof (struct sockaddr_in6);
    int errn = getnameinfo (sock, struct_size, host, NETWORK_HOST_MAXLENGTH, NULL, 0, NI_NUMERICHOST);
    if (errn != 0) {
        if (error != NULL) {
            *error = gai_strerror (errn);
        }
        memcpy (host, "none", sizeof ("none"));
    }

    return host;
}

/*
 * This function will initialize a new socket, bind it, and then
 * setup listening on the port, it will not accept a connection.
 */
void
network_connect_init_sync (int port,
                           int *server,
                           struct sockaddr_in *socket_v4,
                           struct sockaddr_in6 *socket_v6,
                           int *family,
                           int *sock_size,
                           int addr_type,
                           char *interface
#ifndef DEBUG
                           ,
                           SSL_CTX **ssl_context,
                           SSL_METHOD **ssl_method
#endif
)
{
    int result = 0;

    logs_put (LOG_OUT_LOGS,
              LOG_LEVEL_DEBUG,
              NETWORK_MODE_INFORMATION,
#ifdef DEBUG
              "Plain"
#else
              "SSL"
#endif
    );

#ifndef DEBUG

    // check for certificate and private-key should already be done.
    result = SSL_library_init ();
    assert_ssl (result, NULL_ZERO, "SSL_library_init", SERVER_TLS_LIB_INIT_FAILED);

    result = SSL_load_error_strings ();
    assert_ssl (result, NULL_ZERO, "SSL_load_error_strings", SERVER_TLS_LIB_INIT_FAILED);

    *ssl_method = (SSL_METHOD *) TLS_server_method ();
    *ssl_context = SSL_CTX_new (*ssl_method);

    assert_ssl (result, NULL_ZERO, "SSL_library_init", SERVER_TLS_LIB_INIT_FAILED);

    if (SSL_CTX_use_certificate_file (*ssl_context, RSA_SERVER_CERT, SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        assert_ssl (NULL_ZERO, NULL_ZERO, "SSL_CTX_use_certificate_file", SERVER_TLS_LOAD_CERT_FAILED);
    }

    if (SSL_CTX_use_PrivateKey_file (*ssl_context, RSA_SERVER_KEY, SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        assert_ssl (NULL_ZERO, NULL_ZERO, "SSL_CTX_use_PrivateKey_file", SERVER_TLS_LOAD_KEY_FAILED);
    }

    if (SSL_CTX_check_private_key (*ssl_context) != SSL_SUCCESS) {
        assert_ssl (NULL_ZERO, NULL_ZERO, "SSL_CTX_check_private_key", SERVER_TLS_CERT_KEY_CHK_FAILED);
    }

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, NETWORK_MODE_SSL_INITED);
#endif
    const char *error = NULL;

    if (interface == NULL) {
        interface = "lo";
    }

    int errn = network_iterate_all_available_ips (interface, addr_type, &error, socket_v4, socket_v6, family, sock_size);

    if (errn != SUCCESS) {
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, "Cannot get the IP addresses available on the local machine: %s", error);
    }

    *server = socket (*family, SOCK_STREAM, IPPROTO_TCP);

    if (*family == AF_INET) {
        socket_v4->sin_port = htons (port);
    } else {
        socket_v6->sin6_port = htons (port);
    }

    assert (*server, "socket", SERVER_SOCK_INIT_FAILED);

#ifndef RELEASE_BUILD
    int enable = 1;
    result = setsockopt (*server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int));
    assert (result, "setsockopt", SERVER_SET_SOCKOPT_FAILED);

    result = setsockopt (*server, IPPROTO_TCP, TCP_NODELAY, (char *) &enable, sizeof (int));
    assert (result, "setsockopt", SERVER_SET_SOCKOPT_FAILED);
#endif

    const struct sockaddr *address = *family == AF_INET ? (struct sockaddr *) socket_v4 : (struct sockaddr *) socket_v6;
    result = bind (*server, (const struct sockaddr *) address, *sock_size);
    assert (result, "bind", SERVER_BIND_FAILED);

    result = listen (*server, NETWORK_QUEQUE);
    assert (result, "listen", SERVER_LISTEN_FAILED);

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, NETWORK_PORT_LISTENING, port, NETWORK_QUEQUE);
}

/*
 * On any unrecoverable failure exit() will be called
 * this function is sync and will block until a client connects.
 */
void
network_connect_accept_sync (network_s *connection,
                             int server
#ifndef DEBUG
                             ,
                             SSL_CTX *ssl_context
#endif
)
{
    boolean connected = FALSE;

    do {
        socklen_t len = sizeof (struct sockaddr_in);
        connection->client = accept (server, (struct sockaddr *) &connection->client_socket, &len);
        assert (connection->client, "accept", SERVER_ACCEPT_FAILED);

        connected = TRUE;
#ifndef DEBUG

        connection->ssl_tls = SSL_new (ssl_context);
        if (connection->ssl_tls == NULL) {
            assert_ssl (NULL_ZERO, NULL_ZERO, "SSL_new", SERVER_TLS_SSL_NEW_FAILED);
        }

        int result = SSL_set_fd (connection->ssl_tls, connection->client);
        assert_ssl (result, NULL_ZERO, "SSL_set_fd", SERVER_TLS_SET_FILE_DESCRIPTOR_FAILED);

        result = SSL_accept (connection->ssl_tls);
        if (result != SSL_SUCCESS) {
            SSL_free (connection->ssl_tls);
            logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, NETWORK_SSL_PLAIN_CLIENT);

            close (connection->client);
            connected = FALSE;
        } else {
            logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, NETWORK_ASSERT_SSL_LEVEL_MESSAGE, SSL_get_cipher (connection->ssl_tls));
        }
#endif
    } while (!connected);
}

/*
 * This function will perform memory allocation, and you will have to call
 * network_data_free when you are ready to release the memory
 */
long
network_read_stream (network_s *connection)
{
    network_read_s *read_info = &connection->data_read;
    if (read_info == NULL) {
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, NETWORK_INVALID_READ_ARGS);
        return -1;
    }

    if (read_info->length > NETWORK_READ_BUFFER && connection->temporary_file == NULL) {
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, NETWORK_READ_REQUEST_TOO_BIG);
        return -1;
    }

    ulong buffer_index = 0, temp_index = 0;
    if (connection->temporary_file != NULL) {
        buffer_index = read_info->index;
        temp_index = read_info->index;
    }

    for (; read_info->index < read_info->length;) {
        int read_amount = min (read_info->length - read_info->index, NETWORK_READ_BUFFER);
        char *buffer_offset = read_info->address + read_info->index - buffer_index;

#ifndef DEBUG
        long length = SSL_read (connection->ssl_tls, buffer_offset, read_amount);
#else
        long length = read (connection->client, buffer_offset, read_amount);
#endif

        if (length == 0) {
            if (read_info->index != read_info->length) {
                read_info->error_code = NETWORK_ERROR_EOFUNEXPECTED;
            } else {
                read_info->error_code = NETWORK_SUCCESS;
            }
            break;
        } else if (length < 0) {
            switch (errno) {
                case ECONNRESET:
                    read_info->error_code = NETWORK_ERROR_CONNRESET;
                    break;
                case EAGAIN:
#if EWOULDBLOCK != EAGAIN
                case EWOULDBLOCK:
#endif
                    read_info->error_code = NETWORK_ASYNC_WOULDBLOCK;
                    break;
                default:
                    read_info->error_code = NETWORK_ERROR_READ_ERROR;
                    break;
            }
            break;
        } else {
            read_info->index += length;
        }

        if (connection->temporary_file != NULL && read_info->index - buffer_index) {
            if (buffer_index == 0) {
                sha_256_init (&connection->sha256_context);
            }

            fwrite (read_info->address, sizeof (char), read_info->index - buffer_index, connection->temporary_file);

            sha_256_update (&connection->sha256_context, (uchar *) read_info->address, read_info->index - buffer_index);

            /*
             * Only flush when the entire file has been downloaded.
             * Flush operations include a lot of disk IO wait and we want to avoid them as much as possible
             * for latency, while also making sure that our data is getting written to disk instead of staying
             * in memory. This is afforded by the kernel and filesystem IO buffers caching the data until
             * the buffer is filled and then doing an internal async IO flush.
             */
            if (read_info->index == read_info->length) {
                fflush (connection->temporary_file);

                sha_256_finalize (&connection->sha256_context, connection->sha256_hex);
            }
        }
        buffer_index = read_info->index;
    }

    return buffer_index - temp_index;
}

int
network_write_stream (network_s *network, char *buffer, ulong buffer_length)
{
    ulong bytes_written = 0;
    for (long length = 0; bytes_written < buffer_length;) {
#ifndef DEBUG
        length = SSL_write (network->ssl_tls, buffer + bytes_written, buffer_length - bytes_written);
#else
        length = write (network->client, buffer + bytes_written, buffer_length - bytes_written);
#endif
        if (length > 0) {
            bytes_written += length;
        }
    }

    if (bytes_written != buffer_length) {
        switch (errno) {
            case ECONNRESET: {
                return (NETWORK_ERROR_CONNRESET);
            } break;

            default: {
                return (NETWORK_ERROR_READ_ERROR);
            } break;
        }
    }
    return NETWORK_SUCCESS;
}

/*
 * releases memory (if-any) allocated by network read functions
 */
void
network_read_free (network_read_s read)
{
    if (read.address) {
        m_free (read.address);
    }
}

network_read_s
network_read_with_size (int size)
{
    network_read_s read = {0};
    read.length = size;
    return read;
}