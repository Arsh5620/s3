#include "dbp_protocol.h"
#include "../file.h"
#include "../defines.h"
#include <time.h>

int create_setup_environment();

int dbp_create(dbp_s *protocol)
{
    char tempfile[FILE_NAME_MAXLENGTH];
    // dbp setup the environment before accepting data from connection.
    if (create_setup_environment() == SUCCESS)
    {
        dbp_common_attribs_s attribs = dbp_attribs_try_find(protocol);
        if(attribs.filename.error == 0) {
            int length  = sprintf(tempfile, DBP_TEMP_FILE_FORMAT
                            , DBP_FILE_TEMP_DIR
                            , (char*)attribs.filename.address);
            FILE *temp  = file_open(tempfile, length, FILE_MODE_WRITEONLY);
            file_write_s fileinfo  = 
            {
                .current = 0
                , .size = dbp_data_length(protocol->header_magic_now)
            };
            printf("The file is %ld bytes long. Download started.... \n", fileinfo.size);

            clock_t starttime = clock();
            int fd_error   = 
                file_download(temp, &protocol->connection, &fileinfo);
            clock_t endtime = clock();

            double timeelapsed = 
                (((double)(endtime - starttime)) / CLOCKS_PER_SEC) * 1000;

            int speed = ((fileinfo.size / 1024 / 1024) * (1000 / timeelapsed)) ;
            printf("The file upload status is %d, and it took %fms, speed: %dMb/s\n"
                    , fd_error
                    , timeelapsed
                    , speed);
        }
    }
    return(FAILED);
}

// create:: setup_environment
int create_setup_environment()
{
    // first make sure the temporary file directory exists. 
    if(file_dir_mkine(DBP_FILE_TEMP_DIR) != FILE_DIR_EXISTS)
    {
        logger_write_printf("could not open \"" DBP_FILE_TEMP_DIR 
                "\" dir, check if the program has appropriate "
                "permissions.");
        return(FAILED);
    }
    return(SUCCESS);
}