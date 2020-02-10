#include "logs.h"
#include <time.h>
#include <stdlib.h>
#include "defines.h"
#include <stdarg.h>
#include "memory.h"
#include "file.h"

logger_s logs = {0};
int open_log_file(logger_s *log)
{
    time_t datetime = time(NULL);
    
    log->filename    = m_calloc(LOG_MAX_FILENAMELENGTH, "logger.c:logger_init");
    sprintf(log->filename, LOG_FILE_NAME , LOG_DIR_NAME, datetime);
    log->file    = fopen(log->filename, "w+");

    if(log->file == NULL) {
        fprintf(stderr, "Could not initialize the logging subsystem,"
                " program will now exit.\n");
        return(FAILED);
    }
    return(SUCCESS);
}

logger_s logs_init()
{
    if(logs.init_complete == TRUE) return logs;

    logger_s log    = {0};

    if(file_dir_mkine(LOG_DIR_NAME) != FILE_DIR_EXISTS)
    { 
        printf("could not open directory for writing logs, "
                "logging subsystem unavailable.");
        return log;
    }
    if(open_log_file(&log) == FAILED) {
        printf("could not open the file for writing logs, "
                "logging subsystem unavailable");
        return log;
    }

    log.sprint_buffer   = m_calloc(LOG_MAX_SPRINTFBUFFER, "logger.c:logger_init");
    log.init_complete   = TRUE;
    logs = log;
    return(log);
}

/*
 * this function returns as result number of bytes written. 
 * -1 if no bytes were written to a log file. 
 */
int logs_write_printf(char *string, ...) 
{
    va_list variable_args;
    va_start(variable_args, string);

    if(logs.init_complete == FALSE) {
        vprintf(string, variable_args);
        return(-1);
    }
    
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
    logs.bytes_written += fwrite(logs.sprint_buffer,1,  print_length, logs.file);
    int bytes_written = logs.bytes_written;
    print_length  = vsprintf(logs.sprint_buffer, string, variable_args);
    logs.bytes_written += fwrite(logs.sprint_buffer, print_length, 1, logs.file);
    logs.bytes_written += fwrite(LOG_FILE_NEWLINE, 2, 1, logs.file);

    fflush(logs.file);
    return(logs.bytes_written - bytes_written);
}

void logs_cleanup()
{
    m_free(logs.filename, "logger.c:logger_cleanup");
    m_free(logs.sprint_buffer, "logger.c:logger_cleanup");

    if(fclose(logs.file) != 0)
        printf("Could not close the file open for logging.\n");
    
    if(logs.bytes_written == 0)
        file_delete(logs.filename);
}