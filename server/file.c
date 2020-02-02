#include "file.h"
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

//file dir make if not exists
int file_dir_mkine(char *dir_name)
{
    DIR *dir = opendir(dir_name);
    if(dir) {
        closedir(dir);
        return(FILE_DIR_EXISTS);
    }
    else if (errno == ENOENT) {
        if(mkdir(dir_name, S_IRWXU | S_IRGRP | S_IXGRP) != 0) {
            return FILE_DIR_CREATE_FAILED;
        }
        else return(FILE_DIR_EXISTS);
    }
    
    return(FILE_DIR_ERR_OPEN);
}

FILE *file_open(char *name, int length, char *mode)
{
    if(length > FILE_NAME_MAXLENGTH)
        return(NULL);

    char filename[length + 1];
    filename[length] = 0;

    memcpy(filename, name, length);

    FILE *file  = fopen(filename, mode);
    return(file);
}

int file_delete(char *filename)
{
    return(unlink(filename));
}

int file_download(FILE *file
                    , netconn_info_s *network
                    , file_write_s *info)
{
    int read_required = 0;
    do{
    read_required = info->size - info->current;
    if(read_required > FILE_DOWNLOAD_BUFFER)
        read_required = FILE_DOWNLOAD_BUFFER;

    if(read_required == 0) break;
    netconn_data_s data_read = 
        network_data_readxbytes(network, read_required);

    void *data_address  = network_netconn_data_address(&data_read);
    if(data_read.read_status & DATA_READ_ERROR){
        return(FILE_DOWNLOAD_ERR);
    }else if(data_read.read_status & DATA_READ_EOF){
        if(info->current + data_read.data_length < info->size)
            return(FILE_DOWNLOAD_ERR);
    }

    int bytes_written   = 
        fwrite(data_address, 1, data_read.data_length, file);
    fflush(file);

    if(bytes_written != data_read.data_length)
        return(FILE_DOWNLOAD_ERR);

    info->current += data_read.data_length;
    } while(info->current < info->size);
    return(FILE_DOWNLOAD_COMPLETE);    
}