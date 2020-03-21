# ifndef LOGS_INCLUDE_GAURD
# define LOGS_INCLUDE_GAURD

#include <stdio.h>

#define LOG_DIR_NAME "log-output"
#define LOG_FILE_NAME "%s/log-%ld.log"
#define LOG_FILE_NEWLINE "\r\n"
#define LOG_FILE_OUTPUT "[%02d-%02d-%04d %02d:%02d:%02d] :: "
#define LOG_MAX_FILENAMELENGTH  256
#define LOG_MAX_SPRINTFBUFFER   2048

typedef struct logger
{
    FILE *file;
    char *filename;
    char is_init;
    char is_empty;
} logger_s;

logger_s logs_init();
void logs_cleanup();
int logs_write_printf(char *string, ...) ;

#endif