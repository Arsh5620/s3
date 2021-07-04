#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "../databases/database.h"
#include "../files/file.h"
#include "../general/string.h"

enum filemgmt_errors_enum { FILEMGMT_EXISTS = 1, FILEMGMT_NOT_FOUND, FILEMGMT_SQL_COULD_NOT_BIND };

typedef struct filemgmt_file_name {
    string_s file_name;      // The name of the file as per the client
    string_s real_file_name; // The name of the file as the per the server
    string_s temp_file_name; // Temporary file name used during file download
} filemgmt_file_name_s;

#define FILEMGMT_TABLE_NAME "metadata"

#define FILEMGMT_COLUMN_FILENAME "filename"
#define FILEMGMT_COLUMN_DATE "date"
#define FILEMGMT_COLUMN_SIZE "size"
#define FILEMGMT_COLUMN_HASH "md5"

#define FILEMGMT_COLUMN_FILENAME_INDEX 0
#define FILEMGMT_COLUMN_DATE_INDEX 1
#define FILEMGMT_COLUMN_SIZE_INDEX 2
#define FILEMGMT_COLUMN_HASH_INDEX 3

typedef struct __attribute__ ((aligned (8))) sqlite_result {
    union result_value {
        char _char;
        short _short;
        int _int32;
        long _int64;
        float _float;
        double _double;
        char *_char_p;
        void *_void_p;
    } value;
    int size;
} sqlite_result_s;

typedef struct filemgmt_record {
    sqlite_result_s filename;
    sqlite_result_s date;
    sqlite_result_s size;
    sqlite_result_s md5;
} filemgmt_record_s;

#define FILEMGMT_COLUMN_COUNT sizeof (filemgmt_record_s) / sizeof (sqlite_result_s)

extern enum sqltypes filemgmt_record_types[FILEMGMT_COLUMN_COUNT];

#define FILEMGMT_QUERY_SELECT_EQUALS                                                                                             \
    "SELECT COUNT(*) FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " = ? LIMIT 1;"

#define FILEMGMT_QUERY_SELECT_LIKE_COUNT "SELECT COUNT(*) FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " LIKE ?;"

#define FILEMGMT_QUERY_SELECT_LIKE "SELECT * FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " LIKE ?;"

#define FILEMGMT_QUERY_DELETE "DELETE FROM " FILEMGMT_TABLE_NAME " WHERE " FILEMGMT_COLUMN_FILENAME " = ?;"

#define FILEMGMT_QUERY_INSERT "INSERT INTO " FILEMGMT_TABLE_NAME " VALUES (?, ?, ?, ?);"

#define FILEMGMT_TABLE_CREATE                                                                                                    \
    "CREATE TABLE IF NOT EXISTS " FILEMGMT_TABLE_NAME " (" FILEMGMT_COLUMN_FILENAME " TEXT, " FILEMGMT_COLUMN_DATE               \
    " INTEGER, " FILEMGMT_COLUMN_SIZE " INTEGER, " FILEMGMT_COLUMN_HASH " BLOB);"

#define FILEMGMT_ROOT_FOLDER_NAME "backup/"
#define FILEMGMT_FOLDER_NAME FILEMGMT_ROOT_FOLDER_NAME "files/"

#define FILEMGMT_TEMP_DIR "temp/"
#define FILEMGMT_TEMP_FORMAT "%s/file(%ld).tmp"

int
filemgmt_file_exists_sqlite3 (char *file_name, int file_name_length);
int
filemgmt_file_delete_sqlite3 (char *file_name, int file_name_length);
int
filemgmt_file_add_sqlite3 (char *file_name, int file_name_length, char *hash, int hash_length, int file_length);
int
filemgmt_file_exists (string_s file_name, string_s real_name, struct stat *file_stats);

int
filemgmt_folder_exists (string_s folder_name);
int
filemgmt_folder_exists_sqlite3 (string_s folder_name);
my_list_s *
filemgmt_folder_files (string_s folder_name);
int
filemgmt_read_sqlite_row_into_list (void *list_void, sqlite3_stmt *stmt);
void
database_read_column_for_type (sqlite3_stmt *stmt, enum sqltypes type, int column, sqlite_result_s *result);

int
filemgmt_file_add (string_s file_name, string_s hash, long file_size);
int
filemgmt_rename_file (string_s dest, string_s src);
int
filemgmt_remove_meta (string_s file_name);
int
filemgmt_set_real_path (string_s client_filename, filemgmt_file_name_s *file);
int
filemgmt_set_tmp_path (filemgmt_file_name_s *file_info);
int
filemgmt_mkdirs (filemgmt_file_name_s *file_info);
#endif