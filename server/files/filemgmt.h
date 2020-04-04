#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "../files/file.h"
#include "../general/strings.h"

#define FILE_EXISTS 1
#define FILE_NOTFOUND  0
#define FILE_INPUT_ERROR 2

#define FILEMGMT_QUERY_FILEFOLDEREXISTS  \
	"SELECT * FROM " DATABASE_TABLE_FI_NAME \
	" WHERE file_name = ? and folder_name = ?"

int filemgmt_file_exists(string_s *folder_name, string_s *file_name);
#endif