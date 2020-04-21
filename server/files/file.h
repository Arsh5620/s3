# ifndef FILE_INCLUDE_GAURD
# define FILE_INCLUDE_GAURD

#include <stdio.h>
#include <sys/stat.h>

#include "../networking/network.h"
#include "../general/strings.h"

enum file_errors_enum {
	FILE_SUCCESS
	, FILE_ERROR
	, FILE_DIR_EXISTS
	, FILE_DIR_CREATE_FAILED
	, FILE_DIR_ERROR_OPEN
	, FILE_ERROR_OPEN
	, FILE_NETWORK_ERROR
	, FILE_WRITE_ERROR
	, FILE_READ_ERROR
};

#define FILE_MODE_READONLY  "r"
#define FILE_MODE_WRITEONLY "w"
#define FILE_MODE_APPENDONLY    "a"

#define FILE_MODE_READWRITE 	"r+"
#define FILE_MODE_READOVERWRITE "w+"
#define FILE_MODE_READAPPEND    "a+"

#define FILE_NAME_LENGTH     256
#define FILE_BUFFER_LENGTH   (1 MB)

typedef struct file_write_helper
{
	size_t index;
	size_t size;
	string_s filename;
} file_write_s;

typedef struct {
	FILE *file;
	struct stat stats;    

	char *buffer;
	long index
		, readlength
		, maxlength;
	char is_eof;
} file_reader_s;

int file_append(char *dest, char *src, ulong size);
int file_dir_mkine(char *dir_name);
int file_delete(char *filename);
struct stat file_read_stat(FILE *file);
file_reader_s file_init_reader(FILE *file);
int file_rename(string_s dest, string_s src);
void file_close_reader(file_reader_s *reader);
FILE *file_open(char *name, int length, char *mode);
int file_download(FILE *file, network_s *network, file_write_s *info);
int file_reader_fill(file_reader_s *reader, long fill_at, long fill_size);
string_s file_path_concat(string_s dir1, string_s dir2, string_s file_name);

# endif