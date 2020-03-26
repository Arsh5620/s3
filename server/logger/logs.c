#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "./logs.h"
#include "../general/defines.h"
#include "../general/strings.h"
#include "../memdbg/memory.h"
#include "../files/file.h"

static logger_s logs = {0};

// logs_gettime_s will allocate 64 bytes and format the current
// time according to format, check strftime for more information
// set ns (nanosecond) to 1 to use the LOG_DATE_FORMAT_NS
// use m_free to free memory when not in use anymore.
char *logs_gettime_s(char *format, char ns, size_t *len)
{
    char *memory    = m_malloc(LOG_MAX_TIMESTRINGL, MEMORY_FILE_LINE);
	if(memory == NULL)
		return(NULL);

    struct timespec time;
    if(clock_gettime(CLOCK_REALTIME, &time) != 0)
        return(NULL);

    struct tm local;
    if(localtime_r(&(time.tv_sec), &local) == NULL)
        return(NULL);
    
    int length = strftime(memory, LOG_MAX_TIMESTRINGL, format, &local);

    if(length == 0) {
        return(NULL);
	}

	if(len != NULL) {
		*len	= length;
	}

    if(ns) {
        char *nsf	= m_malloc(LOG_MAX_TIMESTRINGL, MEMORY_FILE_LINE);
		if(nsf == NULL)
			return(NULL);

        int length	= sprintf(nsf, LOG_DATE_FORMAT_NS
			, memory, time.tv_nsec / 1000);
		m_free(memory, MEMORY_FILE_LINE);

		if (length < 0)
			return(NULL);
		if(len != NULL)
			*len	= length;
		return(nsf);
    }
    return(memory);
}

int logs_open_file(logger_s *log)
{
	size_t length	= 0;
    char *dateformat	= logs_gettime_s(LOG_DATE_FORMAT, FALSE, &length);

    strings_sprintf(&log->filename, 
		LOG_FILE_NAME, LOG_DIR_NAME, dateformat);   

	m_free(dateformat, MEMORY_FILE_LINE); 
    log->file_p	= fopen(log->filename, "w+");

    if(log->file_p == NULL) {
        fprintf(stderr, "Could not initialize the logging subsystem,"
                " program will now exit.\n");
        return(FAILED);
    }
    return(SUCCESS);
}

logger_s logs_open()
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
    logs	= log;
    return(log);
}

char *log_levels[]	= {
" [INFO] : "
, " [DEBUG] : "
, " [WARN] : "
, " [ERROR] : "
, " [CATASTOPHIC] : "
};

// this function returns TRUE for no error, and FALSE if an error occur
size_t logs_write(enum logger_level level
	, char *string, va_list variable_args) 
{
    if(logs.is_init == FALSE) {
        vprintf(string, variable_args); // prints to the console output
        return(FALSE);
    }
    
	char is_error	= 0;
	size_t length	= 0;
	char *dateformat	= logs_gettime_s(LOG_DATE_FORMAT, TRUE, &length);
    int write	= fwrite(dateformat, 1, length, logs.file_p);

	m_free(dateformat, MEMORY_FILE_LINE);

	char *log_level;
	if (level >= LOGGER_INFO && level <= LOGGER_CATASTROPHIC)
	{	
		log_level	= log_levels[level];
	} else 
		log_level	= "LOG_LEVEL_NOT_KNOWN";
	
	fwrite(log_level, 1, strlen(log_level), logs.file_p);

	if(length != write)
		is_error	= TRUE;
	else { 
		char *buffer	= 0;
		length  = strings_svprintf(&buffer, string, variable_args);
		write	= fwrite(buffer, 1, length, logs.file_p);
		
        fwrite(LOG_FILE_NEWLINE, 1, 2, logs.file_p);

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

    fflush(logs.file_p);
    return(TRUE);
}

void logs_close()
{
    m_free(logs.filename, MEMORY_FILE_LINE);

    if(fclose(logs.file_p) != 0)
        printf("Could not close the file open for logging.\n");
}