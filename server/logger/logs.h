#ifndef LOGS_INCLUDE_GAURD
#define LOGS_INCLUDE_GAURD

#include <stdarg.h>
#include <stdio.h>
#include "../general/define.h"

#define LOG_DIR_NAME "logs/"
// DD-MM-YYYY HH:MM:SS.msms
#define LOG_DATE_FORMAT "%F %T"
#define LOG_DATE_FORMAT_NS "%s [%.6ld]"
#define LOG_FILE_NAME "%s/" APPLICATION_NAME " %s.log"
#define LOG_FILE_NEWLINE "\r\n"
#define LOG_FILE_OUTPUT "[" LOG_DATE_FORMAT ".%4d] %s "
#define LOG_MAX_FILENAMELENGTH 256
#define LOG_MAX_SPRINTFBUFFER 2048
#define LOG_MAX_TIMESTRINGL 64

typedef struct logger
{
    FILE *file_p;
    char *filename;
    boolean init;
} logger_s;

enum logger_level
{
    LOGGER_LEVEL_INFO = 0,
    LOGGER_LEVEL_DEBUG,
    LOGGER_LEVEL_WARN,
    LOGGER_LEVEL_ERROR,
    LOGGER_LEVEL_CATASTROPHIC // The logging subsystem will kill the program
};

typedef struct
{
    boolean print_debug_logs;
    boolean print_stack_frames;
} s3_log_settings_s;

logger_s
logs_open (s3_log_settings_s settings);
void
logs_close ();
char **
get_backtrace (int *backtrace_count);
char *
logs_gettime_s (char *format, char ns, long *len);
boolean
logs_write (enum logger_level level, char *string, va_list args);

#endif