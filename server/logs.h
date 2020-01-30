# ifndef LOGS_INCLUDE_GAURD
# define LOGS_INCLUDE_GAURD

#include <stdio.h>

#define LOG_DIR_NAME "./logs/"
#define LOG_FILE_NAME "%slog%ld.log"
#define LOG_FILE_NEWLINE "\r\n"

#define LOG_MAX_FILENAMELENGTH  256
#define LOG_MAX_SPRINTFBUFFER   2048

typedef struct logger
{
    FILE *file;
    size_t bytes_written;
    char *filename;
    char *sprint_buffer;

    char init_complete;
} logger_s;

logger_s logger_init();
void logger_cleanup();
int logger_write_printf(char *string, ...) ;

#endif