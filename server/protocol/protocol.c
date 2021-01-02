#include "protocol.h"

/**
 * This function will initialize the connection and everything
 * It will however exit for any error that is not recoverable.
 */
dbp_protocol_s
dbp_connection_initialize_sync (unsigned short port)
{
    /* variables are inited right to left first */
    dbp_protocol_s protocol = {0};
    protocol.logs = logs_open ();

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_LOG_INIT_COMPLETE);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_NETWORK_SBS_INIT);

    protocol.connection = network_connect_init_sync (port);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, PROTOCOL_DATABASE_SETUP);

    if (
      database_init (DBP_DATABASE_FILENAME) == SUCCESS
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
dbp_connection_accept_loop (dbp_protocol_s *protocol)
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

        int shutdown = DBP_CONNECTION_SHUTDOWN_FLOW;

        for (int error = 0; error == SUCCESS;)
        {
            if (++profiler >= profiler_exit)
            {
                exit (1);
            }

            dbp_response_s response = {0};
            dbp_request_s request = {0};
            protocol->current_response = &response;
            protocol->current_request = &request;
            request.instance = (char *) protocol;
            response.instance = (char *) protocol;
            response.file_name = &request.file_name;

            error = dbp_next_request (protocol);
            error = dbp_handle_response (&response, error);

            if (error != SUCCESS)
            {
                shutdown = DBP_CONNECTION_SHUTDOWN_CORRUPTION;
            }

            dbp_handle_close (&request, &response);
        }
        dbp_connection_shutdown (*protocol, shutdown);
    }
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, PROTOCOL_SERVER_SHUTDOWN);
}

void
dbp_handle_close (dbp_request_s *request, dbp_response_s *response)
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
dbp_handle_response_string (dbp_response_s *response)
{
    string_s link = {0};

    switch (response->response_code)
    {
        DBP_CASE (link, DBP_RESPONSE_DATA_SEND, DBP_RESPONSE_STRING_DATA_SEND);

        DBP_CASE (link, DBP_RESPONSE_PACKET_OK, DBP_RESPONSE_STRING_PACKET_OK);

        DBP_CASE (link, DBP_RESPONSE_ACTION_INVALID, DBP_RESPONSE_STRING_ACTION_INVALID);

        DBP_CASE (link, DBP_RESPONSE_HEADER_EMPTY, DBP_RESPONSE_STRING_HEADER_EMPTY);

        DBP_CASE (link, DBP_RESPONSE_DESERIALIZER_ERROR, DBP_RESPONSE_STRING_DESERIALIZER_ERROR);

        DBP_CASE (link, DBP_RESPONSE_MISSING_ATTRIBS, DBP_RESPONSE_STRING_MISSING_ATTRIBS);

        DBP_CASE (link, DBP_RESPONSE_ATTRIB_VALUE_INVALID, DBP_RESPONSE_STRING_ATTIB_VALUE_INVALID);

        DBP_CASE (link, DBP_RESPONSE_FILE_EXISTS_ALREADY, DBP_RESPONSE_STRING_FILE_EXISTS_ALREADY);

        DBP_CASE (link, DBP_RESPONSE_FILE_NOT_FOUND, DBP_RESPONSE_STRING_FILE_NOT_FOUND);

        DBP_CASE (
          link, DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS, DBP_RESPONSE_STRING_FILE_UPDATE_OUTOFBOUNDS);

        DBP_CASE (link, DBP_RESPONSE_CORRUPTED_PACKET, DBP_RESPONSE_STRING_CORRUPTED_PACKET);

        DBP_CASE (
          link, DBP_RESPONSE_CORRUPTED_DATA_HEADERS, DBP_RESPONSE_STRING_CORRUPTED_DATA_HEADERS);

        DBP_CASE (
          link,
          DBP_RESPONSE_SETUP_ENVIRONMENT_FAILED,
          DBP_RESPONSE_STRING_SETUP_ENVIRONMENT_FAILED);

        DBP_CASE (
          link, DBP_RESPONSE_SERVER_INTERNAL_ERROR, DBP_RESPONSE_STRING_SERVER_INTERNAL_ERROR);

        DBP_CASE (
          link,
          DBP_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT,
          DBP_RESPONSE_STRING_UNEXPECTED_DATA_FROM_CLIENT);

        DBP_CASE (
          link, DBP_RESPONSE_FAILED_AUTHENTICATION, DBP_RESPONSE_STRING_FAILED_AUTHENTICATION);

    default:
        link = (string_s){0};
    }

    return (dbp_response_writer_update (response, link));
}

int
dbp_handle_response (dbp_response_s *response, enum dbp_response_code code)
{
    if (code == DBP_RESPONSE_SUCCESS)
    {
        return (SUCCESS);
    }

    response->response_code = code;

    if (dbp_response_write (response, dbp_handle_response_string) != SUCCESS)
    {
        return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
    }

    if (response->response_code > DBP_RESPONSE_ERRORS)
    {
        return (response->response_code);
    }
    else
    {
        return (SUCCESS);
    }
}

void
dbp_connection_shutdown (dbp_protocol_s protocol, enum dbp_shutdown_enum type)
{
    if (shutdown (protocol.connection.client, SHUT_RDWR) == 0)
    {
        char *reason;
        switch (type)
        {
        case DBP_CONNECTION_SHUTDOWN_FLOW:
            reason = PROTOCOL_SHUTDOWN_REASON_FLOW;
            break;
        case DBP_CONNECTION_SHUTDOWN_CORRUPTION:
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
dbp_close (dbp_protocol_s protocol)
{
    close (protocol.connection.client);
    close (protocol.connection.server);
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_INFO, DBP_CONNECTION_SHUTDOWN_CLEANUP);
    logs_close ();
}

int
dbp_setup_environment (dbp_request_s *request)
{
    string_s client_filename = {0};
    if (filemgmt_setup_temp_files (&request->file_name) != SUCCESS)
    {
        return (DBP_RESPONSE_CANNOT_CREATE_TEMP_FILE);
    }

    int error;
    client_filename = data_get_string_s (
      request->header_list, request->header_table, DBP_ATTRIB_FILENAME, &error);

    if (error == SUCCESS)
    {
        if (client_filename.address != NULL && client_filename.length == 0)
        {
            return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
        }
        error = filemgmt_setup_environment (client_filename, &request->file_name);

        if (error != SUCCESS)
        {
            return (DBP_RESPONSE_SETUP_ENVIRONMENT_FAILED);
        }

        switch (request->action)
        {
        case DBP_ACTION_CREATE:
        case DBP_ACTION_UPDATE:
            break;

        default:
        {
            return (SUCCESS);
        }
        }

        error = filemgmt_mkdirs (&request->file_name);
        if (error != SUCCESS)
        {
            return (DBP_RESPONSE_SETUP_ENVIRONMENT_FAILED);
        }
    }
    return (SUCCESS);
}

// returns 0 for no-error, any other number for error or conn close request
ulong
dbp_next_request (dbp_protocol_s *protocol)
{
    dbp_request_s *request = protocol->current_request;
    dbp_response_s *response = protocol->current_response;

    int result = dbp_request_read_headers (*protocol, request);
    if (result != DBP_RESPONSE_SUCCESS)
    {
        return (result);
    }

    request->instance = (char *) protocol;
    request->header_table = data_make_table (request->header_list, attribs, DBP_ATTRIBS_COUNT);

    result = dbp_request_read_action (request);

    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, REQUEST_ACTION_TYPE, request->action);

    if (result != DBP_RESPONSE_SUCCESS)
    {
        return (result);
    }

    enum dbp_attribs_enum *asserts = dbp_call_asserts[request->action - DBP_ACTION_CREATE];
    boolean assert = dbp_attribs_assert (request->header_table, asserts, DBP_ATTRIBS_COUNT);

    if (assert == FALSE)
    {
        return (DBP_RESPONSE_MISSING_ATTRIBS);
    }

    result = dbp_auth_transaction (request);

    if (result != DBP_RESPONSE_SUCCESS)
    {
        return (result);
    }

    result = dbp_setup_environment (request);
    if (result != SUCCESS)
    {
        return (result);
    }

    result = dbp_action_prehook (request);

    if (result != DBP_RESPONSE_SUCCESS)
    {
        return (result);
    }

    if (request->header_info.data_length)
    {
        if (dbp_handle_response (response, DBP_RESPONSE_DATA_SEND) != SUCCESS)
        {
            return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
        }

        result = dbp_request_data (protocol, request);
        if (result != DBP_RESPONSE_SUCCESS)
        {
            return (result);
        }
    }

    if (request->data_write_confirm == TRUE)
    {
        if (dbp_handle_response (response, DBP_RESPONSE_PACKET_DATA_MORE) != SUCCESS)
        {
            return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
        }

        if (dbp_response_accept_status (response) == SUCCESS)
        {
            result = dbp_action_send (request, response);

            if (result != DBP_RESPONSE_SUCCESS)
            {
                return result;
            }
        }
    }

    result = dbp_action_posthook (request, response);

    if (result != DBP_RESPONSE_SUCCESS)
    {
        return (result);
    }

    if (
      response->response_code != DBP_RESPONSE_PACKET_DATA_READY
      && dbp_handle_response (response, DBP_RESPONSE_PACKET_OK) != SUCCESS)
    {
        return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
    }

    return (DBP_RESPONSE_SUCCESS);
}

int
dbp_action_send (dbp_request_s *request, dbp_response_s *response)
{
    int result = 0;
    response->response_code = DBP_RESPONSE_PACKET_DATA_READY;
    long (*writer) (dbp_response_s *) = NULL;

    switch (request->action)
    {
    case DBP_ACTION_REQUEST:
        writer = dbp_action_request_writer;
        break;

    default:
        result = DBP_RESPONSE_SUCCESS;
        break;
    }

    if (writer != NULL && dbp_response_write (response, writer) != SUCCESS)
    {
        return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
    }
    return result;
}

int
dbp_action_posthook (dbp_request_s *request, dbp_response_s *response)
{
    int result = 0;
    switch (request->action)
    {
    case DBP_ACTION_NOTIFICATION:
        result = dbp_posthook_notification (request, response);
        break;
    case DBP_ACTION_CREATE:
        result = dbp_posthook_create (request, response);
        break;
    case DBP_ACTION_UPDATE:
        result = dbp_posthook_update (request, response);
        break;
    case DBP_ACTION_DELETE:
        result = dbp_posthook_delete (request, response);
        break;
    case DBP_ACTION_REQUEST:
        return DBP_RESPONSE_SUCCESS;
        break;
    case DBP_ACTION_SERVER:
        result = dbp_posthook_serverinfo (request, response);
        break;
    case DBP_ACTION_INVALID:
        result = DBP_RESPONSE_SERVER_INTERNAL_ERROR;
        break;
    }
    return (result);
}

int
dbp_action_prehook (dbp_request_s *request)
{
    int result = 0;
    switch (request->action)
    {
    case DBP_ACTION_NOTIFICATION:
        result = dbp_prehook_notification (request);
        break;
    case DBP_ACTION_CREATE:
        result = dbp_prehook_create (request);
        break;
    case DBP_ACTION_UPDATE:
        result = dbp_prehook_update (request);
        break;
    case DBP_ACTION_DELETE:
        result = dbp_prehook_delete (request);
        break;
    case DBP_ACTION_REQUEST:
        result = dbp_prehook_request (request);
        break;
    case DBP_ACTION_SERVER:
        result = dbp_prehook_serverinfo (request);
        break;
    case DBP_ACTION_INVALID:
        result = DBP_RESPONSE_SERVER_INTERNAL_ERROR;
        break;
    }
    return (result);
}