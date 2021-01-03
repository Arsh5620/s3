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

    if (
      database_init (S3_DATABASE_FILENAME) == SUCCESS
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

static long profiler = 0;
const long profiler_exit = 1000000;

void
s3_connection_accept_loop (s3_protocol_s *protocol)
{
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_NETWORK_WAIT_CONNECT);

    while (network_connect_accept_sync (&protocol->connection), TRUE)
    {
        struct sockaddr_in client = protocol->connection.client_socket;
        char *client_ip = inet_ntoa (client.sin_addr);
        ushort client_port = ntohs (client.sin_port);

        my_print (
          MESSAGE_OUT_LOGS,
          LOGGER_LEVEL_INFO,
          PROTOCOL_NETWORK_CLIENT_CONNECT,
          client_ip,
          client_port);

        int shutdown = S3_CONNECTION_SHUTDOWN_FLOW;

        for (int error = 0; error == SUCCESS;)
        {
            if (++profiler >= profiler_exit)
            {
                exit (1);
            }

            s3_response_s response = {0};
            s3_request_s request = {0};
            protocol->current_response = &response;
            protocol->current_request = &request;
            request.instance = (char *) protocol;
            response.instance = (char *) protocol;
            response.file_name = &request.file_name;

            error = s3_next_request (protocol);
            error = s3_handle_response (&response, error);

            if (error != SUCCESS)
            {
                shutdown = S3_CONNECTION_SHUTDOWN_CORRUPTION;
            }

            s3_handle_close (&request, &response);
        }
        s3_connection_shutdown (*protocol, shutdown);
    }
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_SERVER_SHUTDOWN);
}

void
s3_handle_close (s3_request_s *request, s3_response_s *response)
{
    m_free (request->header_raw.data_address);
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
          link,
          S3_RESPONSE_SETUP_ENVIRONMENT_FAILED,
          S3_RESPONSE_STRING_SETUP_ENVIRONMENT_FAILED);

        S3_CASE (
          link, S3_RESPONSE_SERVER_INTERNAL_ERROR, S3_RESPONSE_STRING_SERVER_INTERNAL_ERROR);

        S3_CASE (
          link,
          S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT,
          S3_RESPONSE_STRING_UNEXPECTED_DATA_FROM_CLIENT);

        S3_CASE (
          link, S3_RESPONSE_FAILED_AUTHENTICATION, S3_RESPONSE_STRING_FAILED_AUTHENTICATION);

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
        case S3_CONNECTION_SHUTDOWN_FLOW:
            reason = PROTOCOL_SHUTDOWN_REASON_FLOW;
            break;
        case S3_CONNECTION_SHUTDOWN_CORRUPTION:
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
    client_filename = data_get_string_s (
      request->header_list, request->header_table, S3_ATTRIB_FILENAME, &error);

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
    s3_request_s *request = protocol->current_request;
    s3_response_s *response = protocol->current_response;

    int result = s3_request_read_headers (*protocol, request);
    if (result != S3_RESPONSE_SUCCESS)
    {
        return (result);
    }

    request->instance = (char *) protocol;
    request->header_table = data_make_table (request->header_list, attribs, S3_ATTRIBS_COUNT);

    result = s3_request_read_action (request);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, REQUEST_ACTION_TYPE, request->action);

    if (result != S3_RESPONSE_SUCCESS)
    {
        return (result);
    }

    enum s3_attribs_enum *asserts = s3_call_asserts[request->action - S3_ACTION_CREATE];
    boolean assert = s3_attribs_assert (request->header_table, asserts, S3_ATTRIBS_COUNT);

    if (assert == FALSE)
    {
        return (S3_RESPONSE_MISSING_ATTRIBS);
    }

    result = s3_auth_transaction (request);

    if (result != S3_RESPONSE_SUCCESS)
    {
        return (result);
    }

    result = s3_setup_environment (request);
    if (result != SUCCESS)
    {
        return (result);
    }

    result = s3_action_preprocess (request);

    if (result != S3_RESPONSE_SUCCESS)
    {
        return (result);
    }

    if (request->header_info.data_length)
    {
        if (s3_handle_response (response, S3_RESPONSE_DATA_SEND) != SUCCESS)
        {
            return (S3_RESPONSE_NETWORK_ERROR_WRITE);
        }

        result = s3_request_data (protocol, request);
        if (result != S3_RESPONSE_SUCCESS)
        {
            return (result);
        }
    }

    if (request->data_write_confirm == TRUE)
    {
        if (s3_handle_response (response, S3_RESPONSE_PACKET_DATA_MORE) != SUCCESS)
        {
            return (S3_RESPONSE_NETWORK_ERROR_WRITE);
        }

        if (s3_response_accept_status (response) == SUCCESS)
        {
            result = s3_action_send (request, response);

            if (result != S3_RESPONSE_SUCCESS)
            {
                return result;
            }
        }
    }

    result = s3_action_postprocess (request, response);

    if (result != S3_RESPONSE_SUCCESS)
    {
        return (result);
    }

    if (
      response->response_code != S3_RESPONSE_PACKET_DATA_READY
      && s3_handle_response (response, S3_RESPONSE_PACKET_OK) != SUCCESS)
    {
        return (S3_RESPONSE_NETWORK_ERROR_WRITE);
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
    case S3_ACTION_UPDATE:
        result = s3_postprocess_update (request, response);
        break;
    case S3_ACTION_DELETE:
        result = s3_postprocess_delete (request, response);
        break;
    case S3_ACTION_REQUEST:
        return S3_RESPONSE_SUCCESS;
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