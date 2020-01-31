#include "file.h"
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

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