# ifndef FILE_INCLUDE_GAURD
# define FILE_INCLUDE_GAURD

#include <stdio.h>
#include <sys/stat.h>

#include "../general/string.h"
#include "../networking/network.h"
#include "../data-structures/list.h"

enum file_errors_enum {
	FILE_SUCCESS
	, FILE_ERROR
	, FILE_DIR_EXISTS
	, FILE_DIR_CREATE_FAILED
	, FILE_DIR_ERROR_OPEN
	, FILE_ERROR_OPEN
	, FILE_ERROR_SEEK
	, FILE_NETWORK_ERROR
	, FILE_WRITE_ERROR
	, FILE_READ_ERROR
};

#define FILE_MODE_WRITEBINARY	"wb"
#define FILE_MODE_READBINARY	"rb"
#define FILE_MODE_WRITEEXISTING	"rb+"

#define FILE_NAME_LENGTH	(256)
#define FILE_BUFFER_LENGTH	(MB(1))
#define FILE_SHA1_SIZE		(160/8)

typedef struct file_info
{
	size_t index;
	size_t size;
	string_s name;
} file_info_s;

typedef struct file_hash_sha1
{
	my_list_s hash_list;	// sha1 hash for every 1 MB of file block
	char *hash_buffer;
	ulong hash_index, hash_size;
} file_sha1;

typedef struct {
	FILE *file;
	struct stat stats;    

	char *buffer;
	long index;
	long readlength;
	long maxlength;
	boolean eof;
} file_reader_s;

int file_dir_mkine(char *dir);
int file_delete(char *file);
int file_rename(string_s dest, string_s src);
struct stat file_stat(FILE *file);

void file_reader_close(file_reader_s *reader);
file_reader_s file_reader_init(FILE *file);
int file_reader_next(file_reader_s *reader, long index, long size);

int file_append(char *dest, char *src, ulong index, ulong size);
int file_download(FILE *file, network_s *network
	, ulong size, file_sha1 *hash
	, void (*sha1_hash)(file_sha1*, network_data_s, boolean));
string_s file_path_concat(string_s path1, string_s path2);

# endif