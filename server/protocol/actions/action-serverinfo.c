#include "../protocol.h"
#include "../build.h"

#define SERVER_INFO_CONST_RESPONSE                                                                 \
    STRING ("S3 server version " __SERVER_VERSION__ ", " __SERVER_INFO__)

long
s3_server_request_writer (s3_response_s *response)
{
    string_s string = SERVER_INFO_CONST_RESPONSE;
    return (s3_response_writer_update (response, string));
}

int
s3_preprocess_serverinfo (s3_request_s *request)
{
    if (request->header_info.data_length != 0)
    {
        return (S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT);
    }
    return (SUCCESS);
}

int
s3_postprocess_serverinfo (s3_request_s *request, s3_response_s *response)
{
    response->response_code = S3_RESPONSE_PACKET_DATA_READY;
    if (s3_response_write (response, s3_server_request_writer) != SUCCESS)
    {
        return (S3_RESPONSE_NETWORK_ERROR_WRITE);
    }
    return (SUCCESS);
}