#include "../protocol.h"
#include "../build.h"

#define SERVER_INFO_CONST_RESPONSE                                                                 \
    STRING ("DBP server version " __SERVER_VERSION__ ", " __SERVER_INFO__)

long
dbp_server_request_writer (dbp_response_s *response)
{
    string_s string = SERVER_INFO_CONST_RESPONSE;
    return (dbp_response_writer_update (response, string));
}

int
dbp_preprocess_serverinfo (dbp_request_s *request)
{
    if (request->header_info.data_length != 0)
    {
        return (DBP_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT);
    }
    return (SUCCESS);
}

int
dbp_postprocess_serverinfo (dbp_request_s *request, dbp_response_s *response)
{
    response->response_code = DBP_RESPONSE_PACKET_DATA_READY;
    if (dbp_response_write (response, dbp_server_request_writer) != SUCCESS)
    {
        return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
    }
    return (SUCCESS);
}