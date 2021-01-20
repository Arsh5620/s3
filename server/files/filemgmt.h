#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "../files/file.h"
#include "../general/string.h"
#include "../databases/database.h"

enum filemgmt_errors_enum
{
    FILEMGMT_EXISTS = 1,
    FILEMGMT_NOT_FOUND,
    FILEMGMT_SQL_COULD_NOT_BIND
};

typedef struct filemgmt_file_name
{
    string_s file_name;           // The name of the file as per the client
    string_s real_file_name;      // The name of the file as the per the server
    string_s real_hash_file_name; // The name of the hash file as per the server
    string_s temp_file_name;      // Temporary file name used during file download
    string_s temp_hash_file_name; // Temporary hash file name used during download
} filemgmt_file_name_s;

#define FILEMGMT_TABLE_NAME "metadata"

#define FILEMGMT_COLUMN_FILENAME "filename"
#define FILEMGMT_COLUMN_DATE "date"
#define FILEMGMT_COLUMN_SIZE "size"
#define FILEMGMT_COLUMN_HASH "md5"

#define FILEMGMT_QUERY_SELECT_EQUALS                                                               \
    "SELECT COUNT(*) FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " = ? LIMIT 1;"

#define FILEMGMT_QUERY_SELECT_LIKE                                                                 \
    "SELECT COUNT(*) FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " LIKE '?';"

#define FILEMGMT_QUERY_DELETE                                                                      \
    "DELETE FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " = ?;"

#define FILEMGMT_QUERY_INSERT "INSERT INTO " FILEMGMT_TABLE_NAME " VALUES (?, ?, ?, ?);"

#define FILEMGMT_TABLE_CREATE                                                                      \
    "CREATE TABLE IF NOT EXISTS " FILEMGMT_TABLE_NAME " (" FILEMGMT_COLUMN_FILENAME                \
    " TEXT, " FILEMGMT_COLUMN_DATE " INTEGER, " FILEMGMT_COLUMN_SIZE                               \
    " INTEGER, " FILEMGMT_COLUMN_HASH " BLOB);"

#define FILEMGMT_ROOT_FOLDER_NAME "backup/"
#define FILEMGMT_FOLDER_NAME FILEMGMT_ROOT_FOLDER_NAME "files/"
#define FILEMGMT_FOLDER_META_NAME FILEMGMT_ROOT_FOLDER_NAME "hashes/"

#define FILEMGMT_TEMP_DIR "temp/"
#define FILEMGMT_TEMP_FORMAT "%s/file(%ld).tmp"
#define FILEMGMT_HASH_FORMAT "%s/hash(%ld).tmp"

int
filemgmt_file_exists_sqlite3 (char *file_name, int file_name_length);
int
filemgmt_file_delete_sqlite3 (char *file_name, int file_name_length);
int
filemgmt_file_add_sqlite3 (char *file_name, int file_name_length, int file_length);
int
filemgmt_file_exists (string_s file_name, string_s real_name, struct stat *file_stats);

int
filemgmt_folder_exists (string_s folder_name);
int
filemgmt_folder_exists_sqlite3 (string_s folder_name);

int
filemgmt_file_add (string_s file_name);
int
filemgmt_rename_file (string_s dest, string_s src);
int
filemgmt_remove_meta (string_s file_name);
int
filemgmt_setup_environment (string_s client_filename, filemgmt_file_name_s *file_info);
int
filemgmt_create_backup_folders ();
int
filemgmt_setup_temp_files (filemgmt_file_name_s *file_info);
int
filemgmt_mkdirs (filemgmt_file_name_s *file_info);
#endif