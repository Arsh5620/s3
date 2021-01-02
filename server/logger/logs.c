#include <time.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <execinfo.h>

#include "./logs.h"
#include "../general/string.h"
#include "../memdbg/memory.h"
#include "../files/file.h"

// In c static means that the variables are only available in the same translation unit
static logger_s logs = {0};
static dbp_log_settings_s log_settings;

// logs_gettime_s will allocate 64 bytes and format the current
// time according to format, check strftime for more information
// set ns (nanosecond) to 1 to use the LOG_DATE_FORMAT_NS
// use free to free memory when not in use anymore.
char *
logs_gettime_s (char *format, char ns, long *len)
{
    char *memory = malloc (LOG_MAX_TIMESTRINGL);
    if (memory == NULL)
    {
        return (NULL);
    }

    struct timespec time;
    if (clock_gettime (CLOCK_REALTIME, &time) != 0)
    {
        return (NULL);
    }

    struct tm local;
    if (localtime_r (&(time.tv_sec), &local) == NULL)
    {
        return (NULL);
    }

    int length = strftime (memory, LOG_MAX_TIMESTRINGL, format, &local);
    if (length == 0)
    {
        return (NULL);
    }

    if (len != NULL)
    {
        *len = length;
    }

    if (ns)
    {
        long length;
        char *memory2 = string_sprintf2 (LOG_DATE_FORMAT_NS, &length, memory, time.tv_nsec / 1000);
        free (memory);

        if (length < 0)
        {
            return (NULL);
        }

        if (len != NULL)
        {
            *len = length;
        }
        return (memory2);
    }
    return (memory);
}

int
logs_open_file (logger_s *log)
{
    long length = 0;
    char *dateformat = logs_gettime_s (LOG_DATE_FORMAT, FALSE, &length);

    log->filename = string_sprintf2 (LOG_FILE_NAME, NULL, LOG_DIR_NAME, dateformat);

    free (dateformat);

    log->file_p = fopen (log->filename, FILE_MODE_WRITEBINARY);
    if (log->file_p == NULL)
    {
        fprintf (stderr, "Could not initialize the logging subsystem\n");
        return (FAILED);
    }
    return (SUCCESS);
}

logger_s
logs_open (dbp_log_settings_s settings)
{
    log_settings = settings;
    if (logs.init == TRUE)
    {
        return logs;
    }

    logger_s log = {0};
    int result = file_dir_mkine (LOG_DIR_NAME);
    if (result != FILE_DIR_EXISTS)
    {
        printf ("could not open directory for writing logs, "
                "logging subsystem unavailable.\n");
        return log;
    }
    if (logs_open_file (&log) == FAILED)
    {
        printf ("could not open the file for writing logs, "
                "logging subsystem unavailable\n");
        return log;
    }

    log.init = TRUE;
    logs = log;
    return (log);
}

char *log_levels[]
  = {" [INFO] -> ", " [DEBUG] -> ", " [WARN] -> ", " [ERROR] -> ", " [CATASTROPHIC] -> "};

// this function returns TRUE for no error, and FALSE if an error occur
boolean
logs_write (enum logger_level level, char *string, va_list args)
{
    if (log_settings.print_debug_logs == FALSE && level == LOGGER_LEVEL_DEBUG)
    {
        return TRUE;
    }

    if (logs.init == FALSE && level >= LOGGER_LEVEL_WARN)
    {
        vprintf (string, args); // prints to the console output
        puts ("<---->");        // adds a new line
        return (FALSE);
    }

    boolean error = 0;
    long length = 0;
    char *date = logs_gettime_s (LOG_DATE_FORMAT, TRUE, &length);
    int write = fwrite (date, 1, length, logs.file_p);
    free (date);

    char *log_level;
    if (level >= LOGGER_LEVEL_INFO && level <= LOGGER_LEVEL_CATASTROPHIC)
    {
        log_level = log_levels[level];
    }
    else
    {
        log_level = "LOG_LEVEL_NOT_KNOWN";
    }

    fwrite (log_level, sizeof (char), strlen (log_level), logs.file_p);

    char *buffer = string_svprintf2 (string, args, &length);
    write = fwrite (buffer, sizeof (char), length, logs.file_p);
    fwrite (LOG_FILE_NEWLINE, sizeof (char), sizeof (LOG_FILE_NEWLINE) - 1, logs.file_p);

    if (log_settings.print_stack_frames)
    {
        int count = 0;
        char **data = get_backtrace (&count);

        for (size_t i = 0; i < count; i++)
        {
            fwrite (data[i], sizeof (char), strlen (data[i]), logs.file_p);
            fwrite (LOG_FILE_NEWLINE, sizeof (char), sizeof (LOG_FILE_NEWLINE) - 1, logs.file_p);
        }
    }

    free (buffer);

    if (length != write)
    {
        error = TRUE;
    }

    if (error == TRUE)
    {
        logs.init = FALSE;
        fprintf (
          stderr,
          "An error occured while writing logs, will fallback"
          " to standard output.\n");
        return (FALSE);
    }

    fflush (logs.file_p);
    return (TRUE);
}

char **
get_backtrace (int *backtrace_count)
{
#ifdef __linux__
    int backtrace_size = 256;
    void *backtrace_memory = m_malloc (backtrace_size);
    *backtrace_count = backtrace ((void **) backtrace_memory, backtrace_size);

    char **symbols = backtrace_symbols (backtrace_memory, backtrace_size);
    m_free (backtrace_memory);
    return symbols;
#else
    *backtrace_count = 0;
    return NULL;
#endif
}

void
logs_close ()
{
    free (logs.filename);
    if (fclose (logs.file_p) != 0)
    {
        printf ("Could not close the file open for logging.\n");
    }
}