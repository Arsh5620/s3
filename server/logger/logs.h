# ifndef LOGS_INCLUDE_GAURD
# define LOGS_INCLUDE_GAURD
#include <stdarg.h>

#define LOG_DIR_NAME "logs"
// DD-MM-YYYY HH:MM:SS.msms
#define LOG_DATE_FORMAT "%F %T"
#define LOG_DATE_FORMAT_NS "%s [%.6ld]"
#define LOG_FILE_NAME "%s/dbp %s.log"
#define LOG_FILE_NEWLINE "\r\n"
#define LOG_FILE_OUTPUT "[" LOG_DATE_FORMAT ".%4d] %s "
#define LOG_MAX_FILENAMELENGTH  256
#define LOG_MAX_SPRINTFBUFFER   2048
#define LOG_MAX_TIMESTRINGL  64

typedef struct logger
{
	FILE *file_p;
	char *filename;
	char is_init;
} logger_s;

enum logger_level {
	LOGGER_INFO
	, LOGGER_DEBUG
	, LOGGER_WARN
	, LOGGER_ERROR 
	, LOGGER_CATASTROPHIC // program WILL exit. 
};

logger_s logs_open();
void logs_close();
char *logs_gettime_s(char *format, char ns, size_t *len);
size_t logs_write(enum logger_level level
	, char *string, va_list variable_args);

#endif