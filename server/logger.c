#include "logger.h"
#include <time.h>
#include <stdlib.h>
#include "networking/defines.h"
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include "memory.h"

logger_s logs = {0};

logger_s logger_init()
{
    if(logs.init_complete == SUCCESS) return logs;

    logger_s log    = {0};

    DIR *dir = opendir(LOG_DIR_NAME);
    if(dir) {
        closedir(dir);
    }
    else if (errno == ENOENT) {
        if(mkdir(LOG_DIR_NAME, S_IRWXU | S_IRGRP | S_IXGRP) != 0) {
            printf("Creating new directory for logs "
                    "failed, program will now exit.");
        }
    }
    else {
        printf("Could not open directory for "
                "writing logs, logging subsystem failed.\n");
    }
    
    time_t datetime = time(NULL);
    
    log.filename    = m_calloc(LOG_MAX_FILENAMELENGTH, "logger.c:logger_init");
    sprintf(log.filename, LOG_FILE_NAME , LOG_DIR_NAME, datetime);
    log.file    = fopen(log.filename, "w+");

    if(log.file == NULL) {
        fprintf(stderr, "Could not initialize the logging subsystem,"
                " program will now exit.\n");
        exit(SERVER_LOGGER_FAILURE);
    }

    log.sprint_buffer   = m_calloc(LOG_MAX_SPRINTFBUFFER, "logger.c:logger_init");
    log.init_complete   = SUCCESS;
    logs = log;
    return(log);
}

int logger_write_printf(char *string, ...) 
{
    va_list variable_args;
    va_start(variable_args, string);

    time_t t    = time(NULL);
    struct tm local_time    = *localtime(&t);

    int print_length = sprintf(logs.sprint_buffer
                                 , "[%02d-%02d-%04d %02d:%02d:%02d] :: "
                                 , local_time.tm_mon + 1
                                 , local_time.tm_mday
                                 , local_time.tm_year + 1900
                                 , local_time.tm_hour
                                 , local_time.tm_min
                                 , local_time.tm_sec);
    logs.bytes_written += fwrite(logs.sprint_buffer, print_length, 1, logs.file);

    int bytes_written = logs.bytes_written;
    print_length  = vsprintf(logs.sprint_buffer, string, variable_args);
    logs.bytes_written += fwrite(logs.sprint_buffer, print_length, 1, logs.file);
    logs.bytes_written += fwrite(LOG_FILE_NEWLINE, 2, 1, logs.file);

    fflush(logs.file);
    return(logs.bytes_written - bytes_written);
}

void logger_cleanup()
{
    m_free(logs.filename, "logger.c:logger_cleanup");
    m_free(logs.sprint_buffer, "logger.c:logger_cleanup");

    if(fclose(logs.file) != 0)
        printf("Could not close the file open for logging.\n");
    
    if(logs.bytes_written == 0)
        unlink(logs.filename);
}