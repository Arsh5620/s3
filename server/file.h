# ifndef FILE_INCLUDE_GAURD
# define FILE_INCLUDE_GAURD

#include <stdio.h>
#include "./networking/network.h"

#define FILE_DIR_EXISTS 0x00
#define FILE_DIR_CREATE_FAILED  0x02
#define FILE_DIR_ERR_OPEN 0x03

#define FILE_MODE_READONLY  "r"
#define FILE_MODE_WRITEONLY "w"
#define FILE_MODE_APPENDONLY    "a"

#define FILE_MODE_READWRITE "r+"
#define FILE_MODE_READOVERWRITE "w+"
#define FILE_MODE_READAPPEND    "a+"

#define FILE_NAME_MAXLENGTH     256
#define FILE_UPLOAD_BUFFER    (1<<20)

#define FILE_UPLOAD_ERR   0x02
#define FILE_UPLOAD_COMPLETE  0x00

typedef struct file_write_helper
{
    size_t current;
    size_t size;
} file_write_s;

int file_dir_mkine(char *dir_name);
int file_delete(char *filename);
FILE *file_open(char *name, int length, char *mode);
int file_download(FILE *file
                    , netconn_info_s *network
                    , file_write_s *info);

# endif