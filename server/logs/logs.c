#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#include "logs.h"
#include "defines.h"
#include "memory.h"
#include "file.h"
#include "strings.h"

static logger_s logs = {0};

int logs_open_file(logger_s *log)
{
    time_t datetime = time(NULL);
    
    strings_sprintf(&log->filename, 
		LOG_FILE_NAME, LOG_DIR_NAME, datetime);    
    log->file	= fopen(log->filename, "w+");

    if(log->file == NULL) {
        fprintf(stderr, "Could not initialize the logging subsystem,"
                " program will now exit.\n");
        return(FAILED);
    }
    return(SUCCESS);
}

logger_s logs_init()
{
    if(logs.is_init == TRUE) 
		return logs;

    logger_s log    = {0};
    if(file_dir_mkine(LOG_DIR_NAME) != FILE_DIR_EXISTS)
    { 
        printf("could not open directory for writing logs, "
                "logging subsystem unavailable.");
        return log;
    }
    if(logs_open_file(&log) == FAILED) {
        printf("could not open the file for writing logs, "
                "logging subsystem unavailable");
        return log;
    }

    log.is_init	= TRUE;
	log.is_empty	= TRUE;
    logs	= log;
    return(log);
}

/*
 * this function returns TRUE for no error, and FALSE if an error occur
 */
int logs_write_printf(char *string, ...) 
{
    va_list variable_args;
    va_start(variable_args, string);

    if(logs.is_init == FALSE) {
        vprintf(string, variable_args); // prints to the console output
        return(FALSE);
    }
    
    time_t t    = time(NULL);
    struct tm local_time    = *localtime(&t);

	char *buffer;
	char is_error;
    int length = strings_sprintf(&buffer
    	, LOG_FILE_OUTPUT
    	, local_time.tm_mon + 1
    	, local_time.tm_mday
    	, local_time.tm_year + 1900
    	, local_time.tm_hour
    	, local_time.tm_min
    	, local_time.tm_sec);

    int write	= fwrite(buffer, 1, length, logs.file);
	m_free(buffer, MEMORY_FILE_LINE);

	if(length != write)
		is_error	= TRUE;
	else {
		buffer	= NULL;
		length  = strings_svprintf(&buffer, string, variable_args);
		write	= fwrite(buffer, 1, length, logs.file);
		fwrite(LOG_FILE_NEWLINE, 1, 2, logs.file);
		m_free(buffer, MEMORY_FILE_LINE);
		if(length != write)
			is_error	= TRUE;
	}

	if(is_error == TRUE){
		logs.is_init	= FALSE;
		printf("An error occured while writing logs, will fallback"
			" to standard output.\n");
		return(FALSE);
	}
	logs.is_empty	= FALSE;
    fflush(logs.file);
    return(TRUE);
}

void logs_cleanup()
{
    m_free(logs.filename, MEMORY_FILE_LINE);

    if(fclose(logs.file) != 0)
        printf("Could not close the file open for logging.\n");
    
    if(logs.is_empty == TRUE)
        file_delete(logs.filename);
}