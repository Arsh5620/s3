#include "protocol.h"
#include "../ssbs/deserializer.h"
#include <unistd.h>
#include <sys/ioctl.h>

boolean sigint_action_flag = FALSE;

/**
 * This is the main functions that sets up everything, and this functions should only return when
 * success is 'true' otherwise it should kill the app. (Unless ofcourse if we have a bug)
 */
s3_protocol_s
s3_connection_initialize_sync (settings_s settings, boolean *success)
{
    *success = FALSE;

    s3_protocol_s protocol = {0};
    protocol.logs = logs_open (settings.log_settings);

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, PROTOCOL_LOG_INIT_COMPLETE);

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, PROTOCOL_NETWORK_SBS_INIT);

#ifndef DEBUG
    network_connect_init_sync (settings.ip_settings.port,
                               &protocol.server,
                               &protocol.server_sockv4,
                               &protocol.server_sockv6,
                               &protocol.sock_family,
                               &protocol.sock_size,
                               settings.ip_settings.address_type,
                               settings.ip_settings.interface,
                               &protocol.ssl_context,
                               &protocol.ssl_method);
#else
    network_connect_init_sync (settings.ip_settings.port,
                               &protocol.server,
                               &protocol.server_sockv4,
                               &protocol.server_sockv6,
                               &protocol.sock_family,
                               &protocol.sock_size,
                               settings.ip_settings.address_type,
                               settings.ip_settings.interface);
#endif

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, PROTOCOL_DATABASE_SETUP);
    protocol.client_table = hash_table_init (100, FALSE);

    if (path_mkdir_recursive (FILEMGMT_FOLDER_NAME) != SUCCESS) {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_FATAL, PROTOCOL_BACKUP_FOLDER_NOT_CREATED);
    }

    if (database_init (FILEMGMT_ROOT_FOLDER_NAME S3_DATABASE_FILENAME) == SUCCESS &&
        database_make_schema (
          2, AUTH_TABLE_CREATE, sizeof (AUTH_TABLE_CREATE), FILEMGMT_TABLE_CREATE, sizeof (FILEMGMT_TABLE_CREATE)) == SUCCESS) {
        *success = TRUE;
    } else {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_FATAL, PROTOCOL_MYSQL_FAILED_CONNECT);
    }

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, PROTOCOL_SETUP_FINISH);
    return (protocol);
}

void
s3_epollctl_add_epollin (int epoll_fd, int socket_fd)
{
    struct epoll_event listener_epoll = {0};
    listener_epoll.events = EPOLLIN | EPOLLRDHUP;
    listener_epoll.data.fd = socket_fd;

    if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, socket_fd, &listener_epoll) == -1) {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_FATAL, PROTOCOL_EPOLL_CTL_FAILED, strerror (errno));
    }
}

void
s3_setup_epoll (s3_protocol_s *protocol)
{
    protocol->epoll_fd = epoll_create1 (NULL_ZERO);
    if (protocol->epoll_fd == -1) {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_FATAL, PROTOCOL_EPOLL_CREATE_FAILED, strerror (errno));
    }

    s3_epollctl_add_epollin (protocol->epoll_fd, protocol->server);
    protocol->epoll_events = m_malloc (sizeof (struct epoll_event) * S3_NETWORK_MAX_EPOLL_EVENTS);
    protocol->epoll_events_count = S3_NETWORK_MAX_EPOLL_EVENTS;
}

void
s3_make_connection_async (int epoll_fd, int client_fd)
{
    int fd_flags = fcntl (client_fd, F_GETFL, 0);
    fd_flags |= O_NONBLOCK;
    fcntl (client_fd, F_SETFL, fd_flags);

    s3_epollctl_add_epollin (epoll_fd, client_fd);
}

void
s3_exit_sigint_fired ()
{
    sigint_action_flag = TRUE;
}

// This is the main loop of the application, everything happens here, after exiting this function
// the program will exit
void
s3_connection_accept_loop_async (s3_protocol_s *protocol)
{
    s3_setup_epoll (protocol);
    logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, PROTOCOL_NETWORK_WAIT_CONNECT);

    float average_ms[1000] = {0};
    float average_wait[1000] = {0};
    float average_data[1000] = {0};
#define AVERAGE_SIZE (sizeof (average_ms) / sizeof (average_ms[0]))
    long counter = 0;
    struct timespec end_time;

    while (TRUE && sigint_action_flag == FALSE) {
        int epoll_count = epoll_wait (protocol->epoll_fd, protocol->epoll_events, protocol->epoll_events_count, 1000);

        struct timespec start_time;
        clock_gettime (CLOCK_REALTIME, &start_time);

        if (counter == 0) {
            printf ("Waiting to start...");
            fflush (stdout);
        } else {
            int counter_mod = min (counter, AVERAGE_SIZE);

            float average_time = 0;
            float average_mbps = 0;
            float average_idle = 0;

            float average_time_max = 0;
            float average_idle_max = 0;
            for (size_t i = 0; i < counter_mod; i++) {
                average_time += average_ms[i];
                average_mbps += average_data[i];
                average_idle += average_wait[i];

                average_time_max = max (average_time_max, average_ms[i]);
                average_idle_max = max (average_idle_max, average_wait[i]);
            }

            printf ("\33[2K\r[Last 1000rq] Avg DL: %.2f MB/s, Avg loop: %.2f ms, Avg load: %.2f %%",
                    (average_time / (average_time + average_idle)) * ((average_mbps / (average_time / 1000)) / MB (1)),
                    average_time / counter_mod,
                    average_time / average_idle);
            fflush (stdout);

            long seconds_consumed = start_time.tv_sec - end_time.tv_sec;
            long ns_consumed = start_time.tv_nsec - end_time.tv_nsec;

            average_wait[counter % AVERAGE_SIZE] = seconds_consumed * 1000 + (ns_consumed / 1000000.0f);
        }

        if (epoll_count == -1) {
            logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, PROTOCOL_EPOLL_WAIT_FAILED, strerror (errno));
            continue;
        }

        long total_data_read = 0;

        for (size_t i = 0; i < epoll_count; i++) {
            struct epoll_event epoll_event = protocol->epoll_events[i];

            if (epoll_event.data.fd == protocol->server) {
                network_s *new_connection = m_calloc (sizeof (network_s));
#ifndef DEBUG
                network_connect_accept_sync (new_connection, protocol->server, protocol->ssl_context);
#else
                network_connect_accept_sync (new_connection, protocol->server);
#endif

                s3_make_connection_async (protocol->epoll_fd, new_connection->client);
                struct sockaddr_in client = new_connection->client_socket;
                char *client_ip = inet_ntoa (client.sin_addr);
                ushort client_port = ntohs (client.sin_port);

                logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, PROTOCOL_NETWORK_CLIENT_CONNECT, client_ip, client_port);

                new_connection->data_read.address = m_malloc (NETWORK_READ_BUFFER);
                new_connection->data_read.length = S3_PROTOCOL_MAGIC_LEN;
                new_connection->network_completed_action = s3_connection_magic_read;

                hash_table_bucket_s bucket;
                bucket.key.number = new_connection->client;
                bucket.key_len = 0;
                bucket.value.address = (char *) new_connection;
                bucket.value_len = sizeof (*new_connection);
                hash_table_add (&protocol->client_table, bucket);
            } else {
                hash_input_u key = {.number = epoll_event.data.fd};
                hash_table_bucket_s bucket = hash_table_get (protocol->client_table, key, 0);
                network_s *client_network = (network_s *) bucket.value.address;

                if (epoll_event.events & EPOLLERR || epoll_event.events & EPOLLHUP || epoll_event.events & EPOLLRDHUP) {
                    logs_put (LOG_OUT_LOGS, LOG_LEVEL_WARN, "A remote client hung up");

                    s3_connection_reset_new_packet (client_network);
                    s3_connection_abort (client_network, &protocol->client_table);
                    continue;
                }

                total_data_read += network_read_stream (client_network);

                if (client_network->data_read.index == client_network->data_read.length &&
                    client_network->network_completed_action (client_network) != SUCCESS) {
                    // TODO: Actually terminate the remote client
                    logs_put (LOG_OUT_LOGS, LOG_LEVEL_WARN, "A remote connection has been terminated");

                    s3_connection_reset_new_packet (client_network);
                    s3_connection_abort (client_network, &protocol->client_table);
                }
            }
        }

        average_data[counter % AVERAGE_SIZE] = total_data_read;

        clock_gettime (CLOCK_REALTIME, &end_time);

        long seconds_consumed = end_time.tv_sec - start_time.tv_sec;
        long ns_consumed = end_time.tv_nsec - start_time.tv_nsec;

        average_ms[counter % AVERAGE_SIZE] = seconds_consumed * 1000 + (ns_consumed / 1000000.0f);
        counter++;
    }

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, PROTOCOL_SERVER_SHUTDOWN);

#ifdef DEBUG
    if (sigint_action_flag == TRUE) {
        printf ("\n\n*************************************\n"
                "Please wait while we try to close everything...\n"
                "If this is taking too long, open another terminal and use: kill -SIGKILL %d\n"
                "*************************************\n",
                getpid ());
        memory_debug_print_unfreed ();
    }
#endif
    printf ("\nGoodbye!!\n");
}

void
s3_connection_abort (network_s *network, hash_table_s *client_table)
{
#ifndef DEBUG
    SSL_shutdown (network->ssl_tls);
    SSL_free (network->ssl_tls);
#endif

    close (network->client);
    hash_table_remove (client_table, (hash_input_u){.number = network->client}, 0);
    m_free (network->data_read.address);
    m_free (network);
}

int
s3_connection_magic_read (network_s *network)
{
    size_t magic = *(size_t *) (network->data_read.address);

    boolean valid;
    s3_header_s header_magic = s3_request_read_magic (magic, &valid);

    if (valid) {
        // setup connection
        s3_connection_alloc (network);

        ((s3_request_s *) network->request)->header_info = header_magic;
        network->data_read.index = 0;
        network->data_read.length = header_magic.header_length;
        network->network_completed_action = s3_connection_headers_read;

        return SUCCESS;
    } else {
        return FAILED;
    }
}

int
s3_connection_headers_read (network_s *network)
{
    my_list_s header_list = my_list_new (10, sizeof (deserializer_value_t));
    deserializer_t deserializer = deserializer_init (network->data_read.address, network->data_read.length, FALSE);
    deserialize_all (&deserializer, &header_list);

    s3_print_headers (header_list);
    s3_request_s *request = ((s3_request_s *) network->request);
    request->header_list = header_list;
    request->header_table = make_hash_table (header_list);

    int error = s3_request_read_action (request);

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, REQUEST_ACTION_TYPE, request->action);

    char **params_assert = s3_asserts + (request->action * attribs_count);
    boolean assert = s3_attribs_assert (request->header_table, params_assert, attribs_count);
    RESPONSE_RESET_RETURN (assert == FALSE, network, S3_RESPONSE_MISSING_ATTRIBS);

    error = s3_auth_transaction (request);
    RESPONSE_RESET_RETURN (error != SUCCESS, network, error);

    error = s3_setup_environment (request);
    RESPONSE_RESET_RETURN (error != SUCCESS, network, error);

    if (s3_connection_temp_file_required (request->action)) {
        network->temporary_file = fopen (request->file_name.temp_file_name.address, FILE_MODE_WRITEBINARY);
    } else {
        network->temporary_file = NULL;
    }

    network->response->file_name = &request->file_name;

    error = s3_action_preprocess (request);
    RESPONSE_RESET_RETURN (error != SUCCESS, network, error);

    if (request->header_info.data_length) {
        error = s3_handle_response (network, S3_RESPONSE_DATA_SEND);
        RESPONSE_RESET_RETURN (error != SUCCESS, network, S3_RESPONSE_NETWORK_ERROR_WRITE);
    }

    if (request->header_info.data_length > 0) {
        network->data_read.index = 0;
        network->data_read.length = request->header_info.data_length;
        network->network_completed_action = s3_connection_confirm_upload;

        return SUCCESS;
    } else {
        return s3_connection_confirm_upload (network);
    }
}

boolean
s3_connection_temp_file_required (enum s3_actions_enum action)
{
    switch (action) {
        case S3_ACTION_CREATE:
        case S3_ACTION_UPDATE:
            return TRUE;
        default:
            return FALSE;
    }
}

/**
 * Even though the function is named confirm_upload, the function will only do any real-work when
 * data_write_confirm is set to true by a preprocess function, otherwise it will fall through to
 * next function and so on
 */
int
s3_connection_confirm_upload (network_s *network)
{
    s3_request_s *request = network->request;

    if (request->data_write_confirm == TRUE) {
        int error = s3_handle_response (network, S3_RESPONSE_PACKET_DATA_MORE);
        RESPONSE_RESET_RETURN (error != SUCCESS, network, S3_RESPONSE_NETWORK_ERROR_WRITE);

        network->data_read.index = 0;
        network->network_completed_action = s3_connection_upload;
        network->data_read.length = sizeof (long);

        return SUCCESS;
    } else {
        return s3_connection_upload (network);
    }
}

int
s3_connection_upload (network_s *network)
{
    s3_request_s *request = network->request;

    int error = SUCCESS;

    if (request->data_write_confirm == TRUE) {
        error = s3_response_accept_status (network);
    }

    RESPONSE_RESET_RETURN (error != SUCCESS, network, S3_RESPONSE_NETWORK_ERROR_WRITE);

    if (error == SUCCESS) {
        error = s3_action_send (network);

        RESPONSE_RESET_RETURN (error != SUCCESS, network, error);
    }

    return s3_connection_post_process (network);
}

int
s3_connection_post_process (network_s *network)
{
    // This function is where all of the heavy-lifting should happen.
    // Preferably on a different thread to avoid blocking the rest of the requests

    int error = s3_action_postprocess (network);
    RESPONSE_RESET_RETURN (error != SUCCESS, network, error);

    if (network->response->response_code != S3_RESPONSE_PACKET_DATA_READY &&
        s3_handle_response (network, S3_RESPONSE_PACKET_OK) != SUCCESS) {
        RESPONSE_RESET_RETURN (TRUE, network, S3_RESPONSE_NETWORK_ERROR_WRITE);
    }

    s3_connection_reset_new_packet (network);
    return SUCCESS;
}

void
s3_connection_reset_with_response (network_s *network, enum s3_response_code response_code)
{
    s3_handle_response (network, response_code);
    s3_connection_reset_new_packet (network);
}

void
s3_connection_reset_new_packet (network_s *network)
{
    if (network->temporary_file != NULL) {
        fclose (network->temporary_file);
        network->temporary_file = NULL;
    }

    if (network->request != NULL) {
        // Don't bother checking if file successfully removed
        unlink (network->request->file_name.temp_file_name.address);
    }

    // We just finished processing this packet, reset everything
    network->data_read.index = 0;
    network->data_read.length = S3_PROTOCOL_MAGIC_LEN;
    network->network_completed_action = s3_connection_magic_read;

    s3_connection_free (network);
}

/***
 * This function should be called before dispatching the request
 * to make sure that the header contains all the required key:value
 * pairs needed by the called function.
 */
int
s3_attribs_assert (hash_table_s table, char **asserts, int count)
{
    for (long i = 0; i < count; i++) {
        char *attrib = asserts[i];

        if (attrib != NULL && s3_attrib_contains (table, attrib) == FALSE) {
            return (FALSE);
        }
    }
    return (TRUE);
}

// returns TRUE on success and FALSE otherwise
int
s3_attrib_contains (hash_table_s table, char *attrib)
{
    if (attrib == NULL) {
        return FALSE;
    }

    hash_table_bucket_s bucket = hash_table_get (table, (hash_input_u){.address = attrib}, NULL_ZERO);
    // this is intentional for clarity on the return values
    return bucket.is_occupied ? TRUE : FALSE;
}

/**
 * This function will return the RAW value for a given attribute, any data processing (such as deserializing long/int etc.)
 * must be done by the function responsible for the call
 *
 * Also note that if the attrib cannot be found the out_length will not be modified
 *
 * There are two ways to confirm if the return value of the function is correct, if you are expecting a blob-value then you
 * can check the return value of the function, for value types you can check the is_valid param.
 *
 * The function does checks if is_valid is NULL before setting, so you can safely pass it in as NULL if you won't use it
 */
char *
s3_attrib_get (hash_table_s table, char *attrib, size_t *out_length, boolean *is_valid)
{
    if (is_valid != NULL) {
        *is_valid = FALSE;
    }

    if (attrib == NULL) {
        return NULL;
    }

    hash_table_bucket_s bucket = hash_table_get (table, (hash_input_u){.address = attrib}, strlen (attrib));

    if (bucket.is_occupied) {
        if (out_length != NULL) {
            *out_length = bucket.value_len;
        }
        if (is_valid != NULL) {
            *is_valid = TRUE;
        }
        return bucket.value.address;
    }

    return NULL;
}

hash_table_s
make_hash_table (my_list_s list)
{
    hash_table_s table = hash_table_init (10, TRUE);

    for (long i = 0; i < list.count; ++i) {
        deserializer_value_t pair = *(deserializer_value_t *) my_list_get (list, i);
        hash_table_bucket_s bucket = hash_table_get (table, (hash_input_u){.address = pair.key}, pair.key_length);

        if (bucket.is_occupied) {
            // This means that a same key has already been added, ignore and continue
            continue;
        }

        hash_table_bucket_s new_item = (hash_table_bucket_s){
          .key.address = (char *) pair.key,
          .key_len = pair.key_length,
          .value.address = pair.value_pointer,
          .value_len = pair.value_long,
        };

        hash_table_add (&table, new_item);
    }
    return (table);
}

void
s3_connection_alloc (network_s *data)
{
    data->request = m_calloc (sizeof (s3_request_s));
    data->response = m_calloc (sizeof (s3_response_s));
}

void
s3_connection_free (network_s *network)
{
    if (network->request != NULL) {
        m_free_null_check (network->request->file_name.file_name.address);
        m_free_null_check (network->request->file_name.temp_file_name.address);
        m_free_null_check (network->request->file_name.real_file_name.address);

        my_list_free (network->request->header_list);
        hash_table_free (network->request->header_table);
        m_free (network->request);
    }

    if (network->response != NULL) {
        my_list_free (network->response->header_list);
        m_free (network->response);
    }

    network->request = NULL;
    network->response = NULL;
}

uchar *
hexstring (uchar *string, int string_length)
{
    uchar *memory = m_malloc (string_length * 2 + 1);
    memory[string_length * 2] = 0;

    for (size_t i = 0; i < string_length; i++) {
        memory[i * 2] = sha_uchar_to_hex ((string[i] & 0xF0) >> 4);
        memory[i * 2 + 1] = sha_uchar_to_hex (string[i] & 0xF);
    }

    return memory;
}

void
hexdump (char *string, int string_length, int line_width)
{
    for (size_t i = 0; i < string_length; i++) {
        printf ("%02x ", (unsigned char) *(string + i));
        if (i > 0 && i % line_width == 0) {
            printf ("\n");
        }
    }
    printf ("\n");
}

// void
// s3_handle_close (s3_request_s *request, s3_response_s *response)
// {
//     m_free (request->header_raw);
//     my_list_free (request->header_list);
//     my_list_free (response->header_list);
//     hash_table_free (request->header_table);
//     if (request->additional_data) {
//         m_free (request->additional_data);
//         request->additional_data = NULL_ZERO;
//     }
//     m_free_null_check (request->file_name.file_name.address);
//     m_free_null_check (request->file_name.real_file_name.address);
//     m_free_null_check (request->file_name.temp_file_name.address);
// }

long
s3_handle_response_string (network_s *network)
{
    char *response_message = s3_response_messages[network->response->response_code];
    if (response_message != NULL) {
        string_s response_string = STRING2 (response_message, strlen (response_message));
        return s3_response_writer_update (network->response, response_string);
    } else {
        return SUCCESS;
    }
}

int
s3_handle_response (network_s *network, enum s3_response_code code)
{
    if (code == SUCCESS) {
        return SUCCESS;
    }

    s3_response_s *response = network->response;
    response->response_code = code;

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, RESPONSE_CODE, code);

    if (s3_response_write (network, s3_handle_response_string) != SUCCESS) {
        return S3_RESPONSE_NETWORK_ERROR_WRITE;
    }

    return SUCCESS;
}

// void
// s3_connection_shutdown (s3_protocol_s protocol, enum s3_shutdown_enum type)
// {
//     if (shutdown (protocol.connection.client, SHUT_RDWR) == 0)
//     {
//         char *reason;
//         switch (type)
//         {
//         case S3_SHUTDOWN_CLOSE:
//             reason = PROTOCOL_SHUTDOWN_REASON_FLOW;
//             break;
//         case S3_SHUTDOWN_INVALID:
//             reason = PROTOCOL_SHUTDOWN_REASON_CORRUPT;
//             break;
//         default:
//             reason = PROTOCOL_SHUTDOWN_REASON_UNKNOWN;
//             break;
//         }
//         logs_put (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, PROTOCOL_CLIENT_CONNECT_ABORTED, reason);
//     }
// }

void
s3_close (s3_protocol_s protocol)
{
    // close (protocol.connection.client);
    // close (protocol.connection.server);
    // logs_put (MESSAGE_OUT_LOGS, LOGS_LEVEL_INFO, S3_CONNECTION_SHUTDOWN_CLEANUP);
    logs_close ();
}

int
s3_setup_environment (s3_request_s *request)
{
    if (filemgmt_set_tmp_path (&request->file_name) != SUCCESS) {
        return S3_RESPONSE_CANNOT_CREATE_TEMP_FILE;
    }

    size_t filename_length;
    char *filename = s3_attrib_get (request->header_table, S3_ATTRIBNAME_FILENAME, &filename_length, NULL);

    if (filename != NULL) {
        if (filename_length == 0) {
            return S3_RESPONSE_ATTRIB_VALUE_INVALID;
        }

        int error = filemgmt_set_real_path (STRING2 (filename, filename_length), &request->file_name);

        if (error != SUCCESS) {
            return S3_RESPONSE_SETUP_ENVIRONMENT_FAILED;
        }

        switch (request->action) {
            case S3_ACTION_CREATE:
            case S3_ACTION_UPDATE:
                break;

            default:
                return SUCCESS;
        }

        error = filemgmt_mkdirs (&request->file_name);
        if (error != SUCCESS) {
            return S3_RESPONSE_SETUP_ENVIRONMENT_FAILED;
        }
    }
    return SUCCESS;
}

int
s3_action_send (network_s *network)
{
    s3_request_s *request = network->request;
    s3_response_s *response = network->response;

    int result = 0;
    long (*writer) (network_s *) = NULL;

    switch (request->action) {
        case S3_ACTION_REQUEST:
            response->response_code = S3_RESPONSE_PACKET_DATA_READY;
            writer = s3_action_request_writer;
            break;
        case S3_ACTION_DIR:
            response->response_code = S3_RESPONSE_PACKET_DATA_READY;
            writer = s3_action_dirlist_writer;
            break;

        default:
            result = SUCCESS;
            break;
    }

    if (writer != NULL && s3_response_write (network, writer) != SUCCESS) {
        return S3_RESPONSE_NETWORK_ERROR_WRITE;
    }

    return result;
}

/**
 * Post process should contain logic for processing of the data that is
 * downloaded, including verifying the correctness of such data.
 *
 * For any action types that don't want to do post processing should
 * just return SUCCESS immediately.
 */
int
s3_action_postprocess (network_s *network)
{
    s3_request_s *request = network->request;

    int result = 0;
    switch (request->action) {
        case S3_ACTION_NOTIFICATION:
            result = s3_postprocess_notification (network);
            break;
        case S3_ACTION_CREATE:
            result = s3_postprocess_create (network);
            break;
        case S3_ACTION_DIR:
            result = SUCCESS;
            break;
        case S3_ACTION_UPDATE:
            result = s3_postprocess_update (network);
            break;
        case S3_ACTION_DELETE:
            result = s3_postprocess_delete (network);
            break;
        case S3_ACTION_REQUEST:
            result = SUCCESS;
            break;
        case S3_ACTION_SERVER:
            result = s3_postprocess_serverinfo (network);
            break;
        case S3_ACTION_LOGIN:
            result = s3_postprocess_login (network);
            break;
        case S3_ACTION_INVALID:
            result = S3_RESPONSE_SERVER_INTERNAL_ERROR;
            break;
    }

    return result;
}

/**
 * Pre process is called before the data download is started.
 * For action types that don't contain any data and don't have a
 * custom response writer, both the pre process and post process
 * are the same, so post processs must be avoided.
 *
 * Here we should try to validate the correctness of the rest of the
 * packet and we should return an error if the packet state is invalid,
 * so that the data download for such packets can be avoided.
 *
 * Also preprocess should set any variables required if there is
 * a custom response writer attached to the action, as the custom
 * response writer is called between the pre process and the post
 * process
 */
int
s3_action_preprocess (s3_request_s *request)
{
    int result = 0;
    switch (request->action) {
        case S3_ACTION_NOTIFICATION:
            result = s3_preprocess_notification (request);
            break;
        case S3_ACTION_CREATE:
            result = s3_preprocess_create (request);
            break;
        case S3_ACTION_DIR:
            result = s3_preprocess_dirlist (request);
            break;
        case S3_ACTION_UPDATE:
            result = s3_preprocess_update (request);
            break;
        case S3_ACTION_DELETE:
            result = s3_preprocess_delete (request);
            break;
        case S3_ACTION_REQUEST:
            result = s3_preprocess_request (request);
            break;
        case S3_ACTION_SERVER:
            result = s3_preprocess_serverinfo (request);
            break;
        case S3_ACTION_LOGIN:
            result = s3_preprocess_login (request);
            break;
        case S3_ACTION_INVALID:
            result = S3_RESPONSE_SERVER_INTERNAL_ERROR;
            break;
    }
    return (result);
}