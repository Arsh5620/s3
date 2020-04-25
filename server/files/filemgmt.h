#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "../files/file.h"
#include "../general/strings.h"

enum filemgmt_errors_enum {
	FILEMGMT_EXISTS	= 1
	, FILEMGMT_NOT_FOUND
	, FILEMGMT_SQL_COULD_NOT_BIND
};

#define FILEMGMT_QUERY_FILEFOLDEREXISTS  \
	"SELECT * FROM " DATABASE_TABLE_FI_NAME \
	" WHERE file_name = ?"

#define FILEMGMT_QUERY_DELETE \
	"DELETE FROM " DATABASE_TABLE_FI_NAME \
	" WHERE file_name = ? LIMIT 1;"

#define FILEMGMT_QUERY_INSERT \
	"INSERT INTO " DATABASE_TABLE_FI_NAME \
	" VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

#define FILEMGMT_FOLDER_NAME	"backup/"

int filemgmt_file_exists(string_s file_name);
int filemgmt_file_add(string_s file_name);
int filemgmt_rename_file(string_s dest, string_s src);
int filemgmt_remove_meta(string_s file_name);
#endif