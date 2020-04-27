#include <time.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "./logs.h"
#include "../general/define.h"
#include "../general/string.h"
#include "../memdbg/memory.h"
#include "../files/file.h"

static logger_s logs = {0};

// logs_gettime_s will allocate 64 bytes and format the current
// time according to format, check strftime for more information
// set ns (nanosecond) to 1 to use the LOG_DATE_FORMAT_NS
// use free to free memory when not in use anymore.
char *logs_gettime_s(char *format, char ns, long *len)
{
	char *memory    = malloc(LOG_MAX_TIMESTRINGL);
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
		char *nsf	= malloc(LOG_MAX_TIMESTRINGL);
		if(nsf == NULL)
			return(NULL);

		int length	= sprintf(nsf, LOG_DATE_FORMAT_NS
			, memory, time.tv_nsec / 1000);
		free(memory);

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
	long length	= 0;
	char *dateformat	= logs_gettime_s(LOG_DATE_FORMAT, FALSE, &length);

	log->filename	= string_sprintf2(LOG_FILE_NAME, NULL
		, LOG_DIR_NAME, dateformat);   

	free(dateformat); 

	log->file_p	= fopen(log->filename, "w+");
	if(log->file_p == NULL) {
		fprintf(stderr, "Could not initialize the logging subsystem\n");
		return(FAILED);
	}
	return(SUCCESS);
}

logger_s logs_open()
{
	if(logs.is_init == TRUE) 
		return logs;

	logger_s log    = {0};
	int result 	= file_dir_mkine(LOG_DIR_NAME);
	if(result != FILE_DIR_EXISTS)
	{ 
		printf("could not open directory for writing logs, "
				"logging subsystem unavailable.\n");
		return log;
	}
	if(logs_open_file(&log) == FAILED) {
		printf("could not open the file for writing logs, "
				"logging subsystem unavailable\n");
		return log;
	}

	log.is_init	= TRUE;
	logs	= log;
	return(log);
}

char *log_levels[]	= {
" [INFO] -> "
, " [DEBUG] -> "
, " [WARN] -> "
, " [ERROR] -> "
, " [CATASTROPHIC] -> "
};

// this function returns TRUE for no error, and FALSE if an error occur
size_t logs_write(enum logger_level level
	, char *string, va_list variable_args) 
{
	if(logs.is_init == FALSE) {
		vprintf(string, variable_args); // prints to the console output
		puts(""); // adds a new line
		return(FALSE);
	}
	
	char is_error	= 0;
	long length	= 0;
	char *dateformat	= logs_gettime_s(LOG_DATE_FORMAT, TRUE, &length);
	int write	= fwrite(dateformat, 1, length, logs.file_p);

	free(dateformat);

	char *log_level;
	if (level >= LOGGER_LEVEL_INFO && level <= LOGGER_LEVEL_CATASTROPHIC)
	{	
		log_level	= log_levels[level];
	} else 
		log_level	= "LOG_LEVEL_NOT_KNOWN";
	
	fwrite(log_level, 1, strlen(log_level), logs.file_p);

	if(length != write)
		is_error	= TRUE;
	else { 
		char *buffer	= string_svprintf2(string, variable_args, &length);
		write	= fwrite(buffer, 1, length, logs.file_p);
		
		fwrite(LOG_FILE_NEWLINE, 1, 2, logs.file_p);

		free(buffer);
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
	free(logs.filename);

	if(fclose(logs.file_p) != 0)
		printf("Could not close the file open for logging.\n");
}