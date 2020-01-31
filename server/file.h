# ifndef FILE_INCLUDE_GAURD
# define FILE_INCLUDE_GAURD

#define FILE_DIR_EXISTS 0x00
#define FILE_DIR_CREATE_FAILED  0x02
#define FILE_DIR_ERR_OPEN 0x03

int file_dir_mkine(char *dir_name);

# endif