#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "errorhandler.h"

void error_handle(long handle_type
	, enum logger_level log_level, char *format, ...)
{
	va_list list;
	va_start(list, format);

	char stdout_print	= (handle_type & ERRORS_HANDLE_STDOUT) > 0;
	char logs_print	= (handle_type & ERRORS_HANDLE_LOGS) > 0;

	if (stdout_print == 0 && logs_print == 0)
		return; // do nothing.

	if (stdout_print)
	{
		va_list args;
		va_copy(args, list);
		vfprintf(stdout, format, args);
		fprintf(stdout, LOG_FILE_NEWLINE);
		va_end(args);
	}
	if (logs_print) 
	{
		logs_write(LOG_LEVEL(log_level), format, list);

		if (LOG_LEVEL(log_level) == LOGGER_LEVEL_CATASTROPHIC) 
		{
			exit(LOG_LEVEL_EXIT_GET(log_level));
		}
	}
}