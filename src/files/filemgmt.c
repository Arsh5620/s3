#include "filemgmt.h"
#include "files/file.h"
#include "files/path.h"
#include "general/messages.h"
#include "logging/logs.h"
#include "memdbg/memory.h"

#include <string.h>

static long temp_file_counter = 0;

enum sqltypes filemgmt_record_types[] = {
  [FILEMGMT_COLUMN_FILENAME_INDEX] = SQLTYPE_TEXT,
  [FILEMGMT_COLUMN_DATE_INDEX] = SQLTYPE_LONG,
  [FILEMGMT_COLUMN_SIZE_INDEX] = SQLTYPE_LONG,
  [FILEMGMT_COLUMN_HASH_INDEX] = SQLTYPE_TEXT,
};

int
filemgmt_file_exists_sqlite3 (char *file_name, int file_name_length)
{
    sqlite3_stmt *stmt = database_get_stmt (FILEMGMT_QUERY_SELECT_EQUALS, sizeof (FILEMGMT_QUERY_SELECT_EQUALS));
    if (stmt == NULL) {
        return FAILED;
    }

    sqlite3_bind_text (stmt, 1, file_name, file_name_length, SQLITE_TRANSIENT);

    return database_rows_affected (stmt, SQLITE_ROW, "File not found");
}

int
filemgmt_file_delete_sqlite3 (char *file_name, int file_name_length)
{
    sqlite3_stmt *stmt = database_get_stmt (FILEMGMT_QUERY_DELETE, sizeof (FILEMGMT_QUERY_DELETE));
    if (stmt == NULL) {
        return FAILED;
    }

    sqlite3_bind_text (stmt, 1, file_name, file_name_length, SQLITE_TRANSIENT);

    return database_rows_affected (stmt, SQLITE_DONE, "File could not be deleted");
}

int
filemgmt_file_add_sqlite3 (char *file_name, int file_name_length, char *hash, int hash_length, int file_length)
{
    sqlite3_stmt *stmt = database_get_stmt (FILEMGMT_QUERY_INSERT, sizeof (FILEMGMT_QUERY_INSERT));
    if (stmt == NULL) {
        return FAILED;
    }

    sqlite3_bind_text (stmt, 1, file_name, file_name_length, SQLITE_TRANSIENT);
    sqlite3_bind_int64 (stmt, 2, time (NULL));
    sqlite3_bind_int64 (stmt, 3, file_length);
    sqlite3_bind_blob (stmt, 4, hash, hash_length, SQLITE_TRANSIENT);

    return database_rows_affected (stmt, SQLITE_DONE, "Could not add file");
}

int
filemgmt_folder_exists_sqlite3 (string_s folder_name)
{
    sqlite3_stmt *stmt = database_get_stmt (FILEMGMT_QUERY_SELECT_LIKE_COUNT, sizeof (FILEMGMT_QUERY_SELECT_LIKE_COUNT));
    if (stmt == NULL) {
        return FAILED;
    }

    APPEND_STRING (my_string, folder_name, "%");

    sqlite3_bind_text (stmt, 1, my_string, folder_name.length + 1, SQLITE_TRANSIENT);

    return database_rows_affected (stmt, SQLITE_ROW, "Couldn't find the folder name given");
}

my_list_s *
filemgmt_folder_files (string_s folder_name)
{
    sqlite3_stmt *stmt = database_get_stmt (FILEMGMT_QUERY_SELECT_LIKE, sizeof (FILEMGMT_QUERY_SELECT_LIKE));
    if (stmt == NULL) {
        return NULL;
    }

    APPEND_STRING (my_string, folder_name, "%");

    sqlite3_bind_text (stmt, 1, my_string, folder_name.length + 1, SQLITE_TRANSIENT);

    my_list_s *list = m_malloc (sizeof (*list));
    my_list_s tmp_list = my_list_new (10, sizeof (filemgmt_record_s));
    memcpy (list, &tmp_list, sizeof (*list));

    database_iterate_all_rows (stmt, list, filemgmt_read_sqlite_row_into_list, "Iterating over the SQLite results failed");

    return list;
}

int
filemgmt_read_sqlite_row_into_list (void *list_void, sqlite3_stmt *stmt)
{
    my_list_s *list = (my_list_s *) list_void;
    int sql_columns = sqlite3_column_count (stmt);

    if (sql_columns < FILEMGMT_COLUMN_COUNT) {
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_ERROR, "SQL column count is less than the requested number of columns");
        return FAILED;
    } else if (sql_columns > FILEMGMT_COLUMN_COUNT) {
        logs_put (LOG_OUT_BOTH,
                  LOG_LEVEL_DEBUG,
                  "SQL column count is higher than the requested number of columns, only the first n-columns are read");
    }

    filemgmt_record_s record = {0};
    sqlite_result_s *results = (sqlite_result_s *) &record;

    for (size_t i = 0; i < FILEMGMT_COLUMN_COUNT; i++) {
        database_read_column_for_type (stmt, filemgmt_record_types[i], i, results + i);
    }

    my_list_push (list, (char *) &record);

    return SUCCESS;
}

/**
 * For this to work, the structure that is used to store the results must have its members aligned to a common
 * byte boundary. We are using alignment of 8 for now.
 */
void
database_read_column_for_type (sqlite3_stmt *stmt, enum sqltypes type, int column, sqlite_result_s *result)
{
    switch (type) {
        case SQLTYPE_LONG: {
            result->value._int64 = sqlite3_column_int64 (stmt, column);
            result->size = sizeof (sqlite3_int64);
        } break;
        case SQLTYPE_TEXT: {
            const unsigned char *sql_copy = sqlite3_column_text (stmt, column);
            int size = sqlite3_column_bytes (stmt, column);

            result->value._char_p = m_malloc (size + 1); // For \0 terminator
            memcpy (result->value._char_p, sql_copy, size);
            result->value._char_p[size] = 0;

            result->size = size;
        } break;
        case SQLTYPE_BLOB: {
            const void *sql_copy = sqlite3_column_blob (stmt, column);
            int size = sqlite3_column_bytes (stmt, column);

            result->value._void_p = m_malloc (size);
            memcpy (result->value._void_p, sql_copy, size);

            result->size = size;
        } break;

        default:
            logs_put (LOG_OUT_BOTH, LOG_LEVEL_DEBUG, "Unknown sqltype %d", type);
            break;
    }
}

int
filemgmt_file_exists (string_s file_name, string_s real_name, struct stat *file_stats)
{
    int result = filemgmt_file_exists_sqlite3 (file_name.address, file_name.length);

    if (result == SUCCESS) {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, FILEMGMT_RECORD_EXISTS, file_name.length, file_name.address);
    } else {
        return FALSE;
    }

    FILE *file = fopen (real_name.address, FILE_MODE_READBINARY);

    if (file == NULL) {
        return FALSE;
    }

    if (file_stats != NULL) {
        struct stat f1 = file_stat (file);
        *file_stats = f1;
    }
    fclose (file);

    return TRUE;
}

int
filemgmt_folder_exists (string_s folder_name)
{
    return filemgmt_folder_exists_sqlite3 (folder_name) == SUCCESS ? TRUE : FALSE;
}

int
filemgmt_file_add (string_s file_name, string_s hash, long file_size)
{
    return filemgmt_file_add_sqlite3 (file_name.address, file_name.length, hash.address, hash.length, file_size);
}

int
filemgmt_rename_file (string_s dest, string_s src)
{
    if (file_rename (dest, src) == FAILED) {
        return FAILED;
    }
    return SUCCESS;
}

int
filemgmt_remove_meta (string_s file_name)
{
    return filemgmt_file_delete_sqlite3 (file_name.address, file_name.length);
}

int
filemgmt_mkdirs (filemgmt_file_name_s *file_info)
{
    return path_mkdir_recursive (file_info->real_file_name.address);
}

int
filemgmt_set_real_path (string_s filename, filemgmt_file_name_s *file)
{
    char *full_path = path_get_absolute_from_home (FILEMGMT_FOLDER_NAME);
    if (path_mkdir_recursive (full_path) != SUCCESS) {
        m_free (full_path);
        return FAILED;
    }

    my_list_s path_list = path_parse (filename).path_list;
    string_s file_name = path_construct (path_list, FALSE);
    my_list_free (path_list);

    if (file_name.address == NULL) {
        m_free (full_path);
        return FAILED;
    }

    string_s real_file_name = file_path_concat (STRING2 (full_path, strlen (full_path)), file_name);

    file->file_name = file_name;
    file->real_file_name = real_file_name;

    m_free (full_path);
    return SUCCESS;
}

int
filemgmt_set_tmp_path (filemgmt_file_name_s *file_info)
{
    if (path_mkdir_recursive (FILEMGMT_TEMP_DIR) != SUCCESS) {
        return (FAILED);
    }

    temp_file_counter++;

    long length;
    char *full_temp_path = path_get_absolute_from_home (FILEMGMT_TEMP_DIR);
    char *temp = string_sprintf (FILEMGMT_TEMP_FORMAT, &length, full_temp_path, temp_file_counter);
    m_free (full_temp_path);

    if (temp == NULL) {
        return (FAILED);
    }

    file_info->temp_file_name = (string_s){.address = temp, .length = length};
    return (SUCCESS);
}