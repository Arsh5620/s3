#include "protocol.h"

int
s3_preprocess_notification (s3_request_s *request)
{
    if (request->header_info.data_length > 0 && request->header_info.data_length < FILE_BUFFER_LENGTH) {
        return (SUCCESS);
    } else {
        return (S3_RESPONSE_NOTIFY_TOOBIG);
    }
}

int
s3_postprocess_notification (network_s *network)
{
    // s3_request_s *request = network->request;
    // TODO: fix this
    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, NOTIFICATION_GENERAL);
    return (SUCCESS);
}