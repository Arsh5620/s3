#ifndef SRC_LOGGER_LOGS_
#define SRC_LOGGER_LOGS_

#include "general/common.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct logs {
    FILE *file_p;
    char *filename;
    boolean init;
} logs_s;

enum log_types {
    LOG_OUT_STDOUT = 1,
    LOG_OUT_LOGS = 2,
    LOG_OUT_BOTH = 3,
};

enum logs_level {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL // The logging subsystem will kill the program
};

typedef struct {
    boolean print_debug_logs;
    boolean print_stack_frames;
} s3_log_settings_s;

logs_s
logs_open (s3_log_settings_s settings);
void
logs_close ();
void
logs_enable_backtrace ();
void
logs_disable_backtrace ();
char **
get_backtrace (int *backtrace_count);
char *
logs_gettime_s (char *format, char ns, long *len);
boolean
logs_write_internal (enum logs_level level, char *string, va_list args);
void
logs_put (enum log_types logs_type, enum logs_level log_level, char *format, ...);
#endif // SRC_LOGGER_LOGS_