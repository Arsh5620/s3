#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "../files/file.h"
#include "../general/string.h"

enum filemgmt_errors_enum {
	FILEMGMT_EXISTS	= 1
	, FILEMGMT_NOT_FOUND
	, FILEMGMT_SQL_COULD_NOT_BIND
};

typedef struct filemgmt_file_name
{
	string_s file_name;
	string_s real_file_name;
	string_s real_hash_file_name;
	string_s temp_file_name;
	string_s temp_hash_file_name;
} filemgmt_file_name_s;


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
#define FILEMGMT_HASH_FOLDER	"backup-sha1/"
#define FILEMGMT_TEMP_FORMAT	"%s/download-fn(%ld).tmp"
#define FILEMGMT_TEMP_DIR		"temp"
#define FILEMGMT_HASH_FORMAT	"%s/temp-(%d).sha1"

int filemgmt_file_exists(string_s file_name
	, string_s real_name,  struct stat *file_stats);
int filemgmt_file_add(string_s file_name);
int filemgmt_rename_file(string_s dest, string_s src);
int filemgmt_remove_meta(string_s file_name);
int filemgmt_setup_environment(string_s client_filename
	, filemgmt_file_name_s *file_info);
int filemgmt_setup_temp_files(filemgmt_file_name_s *file_info);
int filemgmt_mkdirs(filemgmt_file_name_s *file_info);
#endif