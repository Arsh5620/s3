#include "protocol.h"

int
s3_preprocess_notification (s3_request_s *request)
{
    if (
      request->header_info.data_length > 0 && request->header_info.data_length < FILE_BUFFER_LENGTH)
    {
        return (SUCCESS);
    }
    else
    {
        return (DBP_RESPONSE_NOTIFY_TOOBIG);
    }
}

int
s3_postprocess_notification (s3_request_s *request, s3_response_s *response)
{
    FILE *file = fopen (request->file_name.temp_file_name.address, FILE_MODE_READBINARY);

    file_reader_s reader = file_reader_init (file);
    file_reader_next (&reader, 0, request->header_info.data_length);

    my_print (
      MESSAGE_OUT_BOTH,
      LOGGER_LEVEL_INFO,
      NOTIFICATION_GENERAL,
      (int) reader.readlength,
      (char *) reader.buffer);

    file_reader_close (&reader);
    return (SUCCESS);
}