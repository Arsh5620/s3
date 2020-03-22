#include "protocol.h"
#include "../files/file.h"
#include "../general/defines.h"
#include "../files/filemgmt.h"
#include <time.h>

int dbp_create(packet_info_s *info)
{
    // dbp setup the environment before accepting data from connection.
    if (create_setup_environment() == SUCCESS)
    {
        dbp_common_attribs_s attribs = info->attribs;
        if(attribs.filename.address == 0 || attribs.filename.length == 0)
            return(FAILED);

        file_write_s w_info = 
            create_download_file(info);

        filemgmt_file_exists(0, &attribs.filename);
    } else {
        // we will still need to flush the data or close the connection.
    }
    return(SUCCESS);
}

file_write_s create_download_file(packet_info_s *info)
{
    file_write_s fileinfo  =  {0};
    fileinfo.size = dbp_data_length(info->header);
    char *temp_file;
    string_s filename  = info->attribs.filename;

    if(filename.error == 0) {
        long length  = strings_sprintf(&temp_file, DBP_TEMP_FILE_FORMAT
                                , DBP_FILE_TEMP_DIR
                                , (int)filename.length
                                , (char*)filename.address);
        
        FILE *temp  = file_open(temp_file
                                    , length
                                    , FILE_MODE_WRITEONLY);

        if(temp == NULL || length <= 0) {
            logs_write_printf("could not open file for writing");
            fileinfo.size   = -1;
            return(fileinfo);
        }
        
        logs_write_printf("file upload {\"%.*s\" uploaded %ld bytes} ..."
                , filename.length , filename.address , fileinfo.size);
        printf("download size: %ld\n", fileinfo.size);

        clock_t starttime = clock();
        int download_status   = 
            file_download(temp, &info->dbp->connection, &fileinfo);
        clock_t endtime = clock();

        double time_elapsed = 
            (((double)(endtime - starttime)) / CLOCKS_PER_SEC) * 1000;

        // speed is download size in bytes / 1MB 
        // * number of times we can download this file in seconds
        int speed = (((double)fileinfo.size / 1024 / 128) 
                        * (1000 / time_elapsed));

        logs_write_printf(
                "file upload, status"
                ":(%d) in %.3f ms @ speed: %dMb/s\n"
                , download_status
                , time_elapsed , speed);

        fclose(temp);
    }
    return(fileinfo);
}

// create:: setup_environment
int create_setup_environment()
{
    // first make sure the temporary file directory exists. 
    int result  = file_dir_mkine(DBP_FILE_TEMP_DIR);
    if(result != FILE_DIR_EXISTS)
    {
        perror("opendir");
        logs_write_printf("could not open \"" DBP_FILE_TEMP_DIR 
                "\" dir, check if the program has appropriate "
                "permissions.");
        return(FAILED);
    }
    return(SUCCESS);
}