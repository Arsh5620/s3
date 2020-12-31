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
    string_s file_name;
    string_s real_file_name;
    string_s real_hash_file_name;
    string_s temp_file_name;
    string_s temp_hash_file_name;
} filemgmt_file_name_s;

#define FILEMGMT_TABLE_NAME "file_information"

#define FILEMGMT_QUERY_EXISTS "SELECT * FROM " FILEMGMT_TABLE_NAME " WHERE file_name = ?"

#define FILEMGMT_QUERY_DELETE "DELETE FROM " FILEMGMT_TABLE_NAME " WHERE file_name = ? LIMIT 1;"

#define FILEMGMT_QUERY_INSERT                                                                      \
    "INSERT INTO " FILEMGMT_TABLE_NAME " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

#define FILEMGMT_COLUMN_FILE_NAME STRING ("file_name")
#define FILEMGMT_COLUMN_FILE_CD STRING ("file_cd")
#define FILEMGMT_COLUMN_FILE_UD STRING ("file_ud")
#define FILEMGMT_COLUMN_FILE_LA STRING ("file_la")
#define FILEMGMT_COLUMN_FILE_LM STRING ("file_lm")
#define FILEMGMT_COLUMN_FILE_DD STRING ("file_dd")
#define FILEMGMT_COLUMN_FILE_SIZE STRING ("file_size")
#define FILEMGMT_COLUMN_FILE_MD5 STRING ("file_md5")
#define FILEMGMT_COLUMN_PERMISSIONS STRING ("permission")
#define FILEMGMT_COLUMN_OWNER STRING ("owner")

#define FILEMGMT_TABLE_CREATE                                                                      \
    DATABASE_CREATE_TABLE (                                                                        \
      FILEMGMT_TABLE_NAME,                                                                         \
      " file_name VARCHAR(256)"                                                                    \
      ", file_cd DATE"                                                                             \
      ", file_ud DATE"                                                                             \
      ", file_la DATE"                                                                             \
      ", file_lm DATE"                                                                             \
      ", file_dd DATE"                                                                             \
      ", file_size BIGINT"                                                                         \
      ", file_md5 VARCHAR(32)"                                                                     \
      ", permission BIGINT"                                                                        \
      ", owner VARCHAR(32)")

#define FILEMGMT_BIND DATABASE_COMMON_BIND_STMT (FILEMGMT_TABLE_NAME)

#define FILEMGMT_TABLE_CHECK DATABASE_COMMON_TABLE_EXISTS (FILEMGMT_TABLE_NAME)

#define FILEMGMT_FOLDER_NAME "backup/"
#define FILEMGMT_HASH_FOLDER "backup-sha/"
#define FILEMGMT_TEMP_FORMAT "%s/download-fn(%ld).tmp"
#define FILEMGMT_TEMP_DIR "temp"
#define FILEMGMT_HASH_FORMAT "%s/temp-(%d).sha1"

int
filemgmt_binds_setup (MYSQL *mysql);
int
filemgmt_file_exists (string_s file_name, string_s real_name, struct stat *file_stats);
int
filemgmt_file_add (string_s file_name);
int
filemgmt_rename_file (string_s dest, string_s src);
int
filemgmt_remove_meta (string_s file_name);
int
filemgmt_setup_environment (string_s client_filename, filemgmt_file_name_s *file_info);
int
filemgmt_setup_temp_files (filemgmt_file_name_s *file_info);
int
filemgmt_mkdirs (filemgmt_file_name_s *file_info);
#endif