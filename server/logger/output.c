#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "messages.h"

void
my_print (long logs_type, enum logs_level level, char *format, ...)
{
    char stdout_print = (logs_type & LOG_OUT_STDOUT) > 0;
    char logs_print = (logs_type & LOG_OUT_LOGS) > 0;

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
        logs_write (level, format, args);
        va_end (args);
    }

    if (level == LOG_LEVEL_CATASTROPHIC)
    {
        exit (LOG_LEVEL_CATASTROPHIC);
    }
}