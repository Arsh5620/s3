#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "../files/file.h"
#include "../general/strings.h"

#define FILE_EXISTS	1
#define FILE_NOTFOUND	0
#define FILE_SQL_COULD_NOT_BIND	2

#define FILEMGMT_QUERY_FILEFOLDEREXISTS  \
	"SELECT * FROM " DATABASE_TABLE_FI_NAME \
	" WHERE file_name = ? and folder_name = ?"

#define FILEMGMT_FOLDER_NAME	"backup"

int filemgmt_file_exists(string_s *folder_name, string_s *file_name);
int filemgmt_file_add(string_s *folder_name, string_s *file_name);
#endif