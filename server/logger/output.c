#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "messages.h"

void
my_print (long handle_type, enum logger_level log_level, char *format, ...)
{
    char stdout_print = (handle_type & MESSAGE_OUT_STDOUT) > 0;
    char logs_print = (handle_type & MESSAGE_OUT_LOGS) > 0;

    if (stdout_print == 0 && logs_print == 0)
    {
        return; // do nothing.
    }

    if (stdout_print)
    {
        va_list args;
        va_start (args, format);
        vfprintf (stdout, format, args);
        fprintf (stdout, LOG_FILE_NEWLINE);
        va_end (args);
    }

    if (logs_print)
    {
        va_list args;
        va_start (args, format);
        logs_write (LOG_LEVEL (log_level), format, args);
        va_end (args);

        if (LOG_LEVEL (log_level) == LOGGER_LEVEL_CATASTROPHIC)
        {
            exit (LOG_EXIT_GET (log_level));
        }
    }
}