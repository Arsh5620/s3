#include "protocol.h"

/**
 * This function will initialize the connection and everything
 * It will however exit for any error that is not recoverable.
 */
s3_protocol_s
s3_connection_initialize_sync (unsigned short port, s3_log_settings_s settings)
{
    /* variables are inited right to left first */
    s3_protocol_s protocol = {0};
    protocol.logs = logs_open (settings);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_LOG_INIT_COMPLETE);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_NETWORK_SBS_INIT);

    protocol.connection = network_connect_init_sync (port);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, PROTOCOL_DATABASE_SETUP);

    if (filemgmt_create_backup_folders () != SUCCESS)
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_CATASTROPHIC, PROTOCOL_BACKUP_FOLDER_NOT_CREATED);
    }

    if (
      database_init (FILEMGMT_ROOT_FOLDER_NAME S3_DATABASE_FILENAME) == SUCCESS
      && database_make_schema (
           2,
           AUTH_TABLE_CREATE,
           sizeof (AUTH_TABLE_CREATE),
           FILEMGMT_TABLE_CREATE,
           sizeof (FILEMGMT_TABLE_CREATE))
           == SUCCESS)
    {
        protocol.init_complete = TRUE;
    }
    else
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_CATASTROPHIC, PROTOCOL_MYSQL_FAILED_CONNECT);
    }

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, PROTOCOL_SETUP_FINISH);
    return (protocol);
}

void
s3_epollctl_add_epollin (s3_protocol_s *protocol, int socket_fd)
{
    struct epoll_event listener_epoll = {0};
    listener_epoll.events = EPOLLIN;
    listener_epoll.data.fd = socket_fd;

    if (epoll_ctl (protocol->epoll_fd, EPOLL_CTL_ADD, socket_fd, &listener_epoll) == -1)
    {
        my_print (
          MESSAGE_OUT_LOGS, LOGGER_LEVEL_CATASTROPHIC, PROTOCOL_EPOLL_CTL_FAILED, strerror (errno));
    }
}

void
s3_setup_epoll (s3_protocol_s *protocol)
{
    protocol->epoll_fd = epoll_create1 (NULL_ZERO);
    if (protocol->epoll_fd == -1)
    {
        my_print (
          MESSAGE_OUT_LOGS,
          LOGGER_LEVEL_CATASTROPHIC,
          PROTOCOL_EPOLL_CREATE_FAILED,
          strerror (errno));
    }

    s3_epollctl_add_epollin (protocol, protocol->connection.server);
    protocol->epoll_events = m_malloc (sizeof (struct epoll_event) * S3_NETWORK_MAX_EPOLL_EVENTS);
    protocol->epoll_events_count = S3_NETWORK_MAX_EPOLL_EVENTS;
}

void
s3_make_connection_async (s3_protocol_s *protocol)
{
    int fd_flags = fcntl (protocol->connection.client, F_GETFL, 0);
    fd_flags |= O_NONBLOCK;
    fcntl (protocol->connection.client, F_SETFL, fd_flags);

    s3_epollctl_add_epollin (protocol, protocol->connection.client);
}

void
s3_connection_accept_loop_async (s3_protocol_s *protocol)
{
    s3_setup_epoll (protocol);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_NETWORK_WAIT_CONNECT);

    int number_of_tries = 0;
    while (TRUE)
    {
        int number_epoll_events = epoll_wait (
          protocol->epoll_fd, protocol->epoll_events, protocol->epoll_events_count, -1);

        if (number_epoll_events == -1)
        {
            my_print (
              MESSAGE_OUT_LOGS,
              number_of_tries < 10 ? LOGGER_LEVEL_ERROR : LOGGER_LEVEL_CATASTROPHIC,
              PROTOCOL_EPOLL_WAIT_FAILED,
              strerror (errno));
            number_of_tries++;
            continue;
        }
        else
        {
            number_of_tries = 0;
        }

        for (size_t i = 0; i < number_epoll_events; i++)
        {
            struct epoll_event epoll_event_info = protocol->epoll_events[i];

            if (epoll_event_info.data.fd == protocol->connection.server)
            {
                network_connect_accept_sync (&protocol->connection);

                s3_make_connection_async (protocol);
                struct sockaddr_in client = protocol->connection.client_socket;
                char *client_ip = inet_ntoa (client.sin_addr);
                ushort client_port = ntohs (client.sin_port);

                my_print (
                  MESSAGE_OUT_LOGS,
                  LOGGER_LEVEL_INFO,
                  PROTOCOL_NETWORK_CLIENT_CONNECT,
                  client_ip,
                  client_port);
            }
            else
            {
                int shutdown = S3_SHUTDOWN_CLOSE;
                int error = 0;

                if (protocol->current.request == NULL)
                {
                    protocol->current.request = m_calloc (sizeof (s3_request_s));
                    protocol->current.request->instance = (char *) protocol;
                }

                if (protocol->current.response == NULL)
                {
                    protocol->current.response = m_calloc (sizeof (s3_response_s));
                    protocol->current.response->instance = (char *) protocol;
                    protocol->current.response->file_name = &protocol->current.request->file_name;
                }

                error = s3_next_request (protocol);
                if (
                  error == S3_RESPONSE_SUCCESS
                  && protocol->current.program_status == STATUS_RESPONSE_NOW)
                {
                    error = s3_handle_response (protocol->current.response, error);
                }

                if (error != S3_RESPONSE_SUCCESS)
                {
                    shutdown = S3_SHUTDOWN_INVALID;
                    s3_handle_close (protocol->current.request, protocol->current.response);
                    s3_connection_shutdown (*protocol, shutdown);
                }
            }
        }
    }
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_SERVER_SHUTDOWN);
}

void
s3_handle_close (s3_request_s *request, s3_response_s *response)
{
    m_free (request->header_raw);
    my_list_free (request->header_list);
    my_list_free (response->header_list);
    hash_table_free (request->header_table);
    if (request->additional_data)
    {
        m_free (request->additional_data);
        request->additional_data = NULL_ZERO;
    }
    m_free_null_check (request->file_name.file_name.address);
    m_free_null_check (request->file_name.real_hash_file_name.address);
    m_free_null_check (request->file_name.real_file_name.address);
    m_free_null_check (request->file_name.temp_file_name.address);
    m_free_null_check (request->file_name.temp_hash_file_name.address);
}

long
s3_handle_response_string (s3_response_s *response)
{
    string_s link = {0};

    switch (response->response_code)
    {
        S3_CASE (link, S3_RESPONSE_DATA_SEND, S3_RESPONSE_STRING_DATA_SEND);

        S3_CASE (link, S3_RESPONSE_PACKET_OK, S3_RESPONSE_STRING_PACKET_OK);

        S3_CASE (link, S3_RESPONSE_ACTION_INVALID, S3_RESPONSE_STRING_ACTION_INVALID);

        S3_CASE (link, S3_RESPONSE_HEADER_EMPTY, S3_RESPONSE_STRING_HEADER_EMPTY);

        S3_CASE (link, S3_RESPONSE_DESERIALIZER_ERROR, S3_RESPONSE_STRING_DESERIALIZER_ERROR);

        S3_CASE (link, S3_RESPONSE_MISSING_ATTRIBS, S3_RESPONSE_STRING_MISSING_ATTRIBS);

        S3_CASE (link, S3_RESPONSE_ATTRIB_VALUE_INVALID, S3_RESPONSE_STRING_ATTIB_VALUE_INVALID);

        S3_CASE (link, S3_RESPONSE_FILE_EXISTS_ALREADY, S3_RESPONSE_STRING_FILE_EXISTS_ALREADY);

        S3_CASE (link, S3_RESPONSE_FILE_NOT_FOUND, S3_RESPONSE_STRING_FILE_NOT_FOUND);

        S3_CASE (
          link, S3_RESPONSE_FILE_UPDATE_OUTOFBOUNDS, S3_RESPONSE_STRING_FILE_UPDATE_OUTOFBOUNDS);

        S3_CASE (link, S3_RESPONSE_CORRUPTED_PACKET, S3_RESPONSE_STRING_CORRUPTED_PACKET);

        S3_CASE (
          link, S3_RESPONSE_CORRUPTED_DATA_HEADERS, S3_RESPONSE_STRING_CORRUPTED_DATA_HEADERS);

        S3_CASE (
          link, S3_RESPONSE_SETUP_ENVIRONMENT_FAILED, S3_RESPONSE_STRING_SETUP_ENVIRONMENT_FAILED);

        S3_CASE (link, S3_RESPONSE_SERVER_INTERNAL_ERROR, S3_RESPONSE_STRING_SERVER_INTERNAL_ERROR);

        S3_CASE (
          link,
          S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT,
          S3_RESPONSE_STRING_UNEXPECTED_DATA_FROM_CLIENT);

        S3_CASE (link, S3_RESPONSE_FAILED_AUTHENTICATION, S3_RESPONSE_STRING_FAILED_AUTHENTICATION);

    default:
        link = (string_s){0};
    }

    return (s3_response_writer_update (response, link));
}

int
s3_handle_response (s3_response_s *response, enum s3_response_code code)
{
    if (code == S3_RESPONSE_SUCCESS)
    {
        return (SUCCESS);
    }

    response->response_code = code;

    if (s3_response_write (response, s3_handle_response_string) != SUCCESS)
    {
        return (S3_RESPONSE_NETWORK_ERROR_WRITE);
    }

    if (response->response_code > S3_RESPONSE_ERRORS)
    {
        return (response->response_code);
    }
    else
    {
        return (SUCCESS);
    }
}

void
s3_connection_shutdown (s3_protocol_s protocol, enum s3_shutdown_enum type)
{
    if (shutdown (protocol.connection.client, SHUT_RDWR) == 0)
    {
        char *reason;
        switch (type)
        {
        case S3_SHUTDOWN_CLOSE:
            reason = PROTOCOL_SHUTDOWN_REASON_FLOW;
            break;
        case S3_SHUTDOWN_INVALID:
            reason = PROTOCOL_SHUTDOWN_REASON_CORRUPT;
            break;
        default:
            reason = PROTOCOL_SHUTDOWN_REASON_UNKNOWN;
            break;
        }
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, PROTOCOL_CLIENT_CONNECT_ABORTED, reason);
    }
}

void
s3_close (s3_protocol_s protocol)
{
    close (protocol.connection.client);
    close (protocol.connection.server);
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, S3_CONNECTION_SHUTDOWN_CLEANUP);
    logs_close ();
}

int
s3_setup_environment (s3_request_s *request)
{
    string_s client_filename = {0};
    if (filemgmt_setup_temp_files (&request->file_name) != SUCCESS)
    {
        return (S3_RESPONSE_CANNOT_CREATE_TEMP_FILE);
    }

    int error;
    client_filename
      = data_get_string_s (request->header_list, request->header_table, S3_ATTRIB_FILENAME, &error);

    if (error == SUCCESS)
    {
        if (client_filename.address != NULL && client_filename.length == 0)
        {
            return (S3_RESPONSE_ATTRIB_VALUE_INVALID);
        }
        error = filemgmt_setup_environment (client_filename, &request->file_name);

        if (error != SUCCESS)
        {
            return (S3_RESPONSE_SETUP_ENVIRONMENT_FAILED);
        }

        switch (request->action)
        {
        case S3_ACTION_CREATE:
        case S3_ACTION_UPDATE:
            break;

        default:
        {
            return (SUCCESS);
        }
        }

        error = filemgmt_mkdirs (&request->file_name);
        if (error != SUCCESS)
        {
            return (S3_RESPONSE_SETUP_ENVIRONMENT_FAILED);
        }
    }
    return (SUCCESS);
}

// returns 0 for no-error, any other number for error or conn close request
ulong
s3_next_request (s3_protocol_s *protocol)
{
    s3_request_s *request = protocol->current.request;
    s3_response_s *response = protocol->current.response;
    int error = 0;

    switch (protocol->current.program_status)
    {
    case STATUS_CONNECTION_ACCEPTED:
    case STATUS_HEADER_MAGIC_READ:
    {
        error = s3_request_read_magic (protocol, request);
        if (error != S3_RESPONSE_SUCCESS || protocol->current.read_status != STATUS_ASYNC_COMPLETED)
        {
            return error;
        }
        protocol->current.program_status = STATUS_HEADER_SERIALIZED_READ;
        return S3_RESPONSE_SUCCESS;
    }

    case STATUS_HEADER_SERIALIZED_READ:
    {
        int error = s3_request_read_headers (protocol, request);
        if (error != S3_RESPONSE_SUCCESS || protocol->current.read_status != STATUS_ASYNC_COMPLETED)
        {
            return error;
        }

        request->header_table = data_make_table (request->header_list, attribs, attribs_count);

        error = s3_request_read_action (request);

        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, REQUEST_ACTION_TYPE, request->action);

        if (error != S3_RESPONSE_SUCCESS)
        {
            return (error);
        }

        /**
         * s3_asserts is one dimensional array of enums (int) so to go to
         * second index just add the attribs_count to the s3_asserts pointer
         */
        enum s3_attribs_enum *params_assert
          = s3_asserts + attribs_count * (request->action - S3_ACTION_CREATE);
        boolean assert = s3_attribs_assert (request->header_table, params_assert, attribs_count);

        if (assert == FALSE)
        {
            return (S3_RESPONSE_MISSING_ATTRIBS);
        }

        error = s3_auth_transaction (request);

        if (error != S3_RESPONSE_SUCCESS)
        {
            return (error);
        }

        error = s3_setup_environment (request);
        if (error != SUCCESS)
        {
            return (error);
        }
        protocol->current.program_status = STATUS_REQUEST_PREPROCESS;
    }

    case STATUS_REQUEST_PREPROCESS:
    {
        // Generally these actions are expensive
        // But they don't require any data from the client
        error = s3_action_preprocess (request);

        if (error != S3_RESPONSE_SUCCESS)
        {
            return (error);
        }

        if (request->header_info.data_length)
        {
            if (s3_handle_response (response, S3_RESPONSE_DATA_SEND) != SUCCESS)
            {
                return (S3_RESPONSE_NETWORK_ERROR_WRITE);
            }
        }

        protocol->current.program_status = STATUS_REQUEST_DOWNLOADING;
    }

    case STATUS_REQUEST_DOWNLOADING:
    {
        if (request->header_info.data_length)
        {
            error = s3_request_data (protocol, request);
            if (
              error != S3_RESPONSE_SUCCESS
              || protocol->current.read_status != STATUS_ASYNC_COMPLETED)
            {
                return (error);
            }
        }
        protocol->current.program_status = STATUS_REQUEST_UPLOADING;
    }

    case STATUS_REQUEST_UPLOADING:
    {
        boolean send_data = FALSE;
        if (request->data_write_confirm == TRUE)
        {
            if (s3_handle_response (response, S3_RESPONSE_PACKET_DATA_MORE) != SUCCESS)
            {
                return (S3_RESPONSE_NETWORK_ERROR_WRITE);
            }

            error = s3_response_accept_status (response);
            if (error != SUCCESS || protocol->current.read_status != STATUS_ASYNC_COMPLETED)
            {
                return (S3_RESPONSE_NETWORK_ERROR_WRITE);
            }

            send_data = TRUE;
        }

        if (send_data == TRUE || request->data_write_confirm == FALSE)
        {
            error = s3_action_send (request, response);

            if (error != S3_RESPONSE_SUCCESS)
            {
                return error;
            }
        }
        protocol->current.program_status = STATUS_REQUEST_POSTPROCESS;
    }

    case STATUS_REQUEST_POSTPROCESS:
    {
        // Again no network IO just some heavy lifting for other IO
        error = s3_action_postprocess (request, response);

        if (error != S3_RESPONSE_SUCCESS)
        {
            return (error);
        }

        if (
          response->response_code != S3_RESPONSE_PACKET_DATA_READY
          && s3_handle_response (response, S3_RESPONSE_PACKET_OK) != SUCCESS)
        {
            return (S3_RESPONSE_NETWORK_ERROR_WRITE);
        }
        protocol->current.program_status = STATUS_RESPONSE_NOW;
    }
    case STATUS_RESPONSE_NOW:
        break;
    }
    return (S3_RESPONSE_SUCCESS);
}

int
s3_action_send (s3_request_s *request, s3_response_s *response)
{
    int result = 0;
    response->response_code = S3_RESPONSE_PACKET_DATA_READY;
    long (*writer) (s3_response_s *) = NULL;

    switch (request->action)
    {
    case S3_ACTION_REQUEST:
        writer = s3_action_request_writer;
        break;

    default:
        result = S3_RESPONSE_SUCCESS;
        break;
    }

    if (writer != NULL && s3_response_write (response, writer) != SUCCESS)
    {
        return (S3_RESPONSE_NETWORK_ERROR_WRITE);
    }
    return result;
}

/**
 * Post process should contain logic for processing of the data that is
 * downloaded, including verifying the correctness of such data.
 *
 * For any action types that don't want to do post processing should
 * just return S3_RESPONSE_SUCCESS immediately.
 */
int
s3_action_postprocess (s3_request_s *request, s3_response_s *response)
{
    int result = 0;
    switch (request->action)
    {
    case S3_ACTION_NOTIFICATION:
        result = s3_postprocess_notification (request, response);
        break;
    case S3_ACTION_CREATE:
        result = s3_postprocess_create (request, response);
        break;
    case S3_ACTION_DIR:
        result = S3_RESPONSE_SUCCESS;
        break;
    case S3_ACTION_UPDATE:
        result = s3_postprocess_update (request, response);
        break;
    case S3_ACTION_DELETE:
        result = s3_postprocess_delete (request, response);
        break;
    case S3_ACTION_REQUEST:
        result = S3_RESPONSE_SUCCESS;
        break;
    case S3_ACTION_SERVER:
        result = s3_postprocess_serverinfo (request, response);
        break;
    case S3_ACTION_INVALID:
        result = S3_RESPONSE_SERVER_INTERNAL_ERROR;
        break;
    }
    return (result);
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
    switch (request->action)
    {
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
    case S3_ACTION_INVALID:
        result = S3_RESPONSE_SERVER_INTERNAL_ERROR;
        break;
    }
    return (result);
}