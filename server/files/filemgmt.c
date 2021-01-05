#include <string.h>
#include "./filemgmt.h"
#include "../logger/messages.h"
#include "../files/path.h"

int
filemgmt_file_exists_sqlite3 (char *file_name, int file_name_length)
{
    int error;
    sqlite3_stmt *stmt
      = database_get_stmt (FILEMGMT_QUERY_EXISTS, sizeof (FILEMGMT_QUERY_EXISTS), &error);

    if (error != SUCCESS)
    {
        return error;
    }

    sqlite3_bind_text (stmt, 1, file_name, file_name_length, SQLITE_TRANSIENT);

    return database_finish_stmt (stmt, SQLITE_ROW, "File not found");
}

int
filemgmt_file_delete_sqlite3 (char *file_name, int file_name_length)
{
    int error;
    sqlite3_stmt *stmt
      = database_get_stmt (FILEMGMT_QUERY_DELETE, sizeof (FILEMGMT_QUERY_DELETE), &error);

    if (error != SUCCESS)
    {
        return error;
    }

    sqlite3_bind_text (stmt, 1, file_name, file_name_length, SQLITE_TRANSIENT);

    return database_finish_stmt (stmt, SQLITE_DONE, "File could not be deleted");
}

int
filemgmt_file_add_sqlite3 (char *file_name, int file_name_length, int file_length)
{
    int error;
    sqlite3_stmt *stmt
      = database_get_stmt (FILEMGMT_QUERY_INSERT, sizeof (FILEMGMT_QUERY_INSERT), &error);

    if (error != SUCCESS)
    {
        return error;
    }

    sqlite3_bind_text (stmt, 1, file_name, file_name_length, SQLITE_TRANSIENT);
    sqlite3_bind_int64 (stmt, 2, time (NULL));
    sqlite3_bind_int64 (stmt, 3, file_length);
    sqlite3_bind_null (stmt, 4);

    return database_finish_stmt (stmt, SQLITE_DONE, "Could not add file");
}

int
filemgmt_file_exists (string_s file_name, string_s real_name, struct stat *file_stats)
{
    int result = filemgmt_file_exists_sqlite3 (file_name.address, file_name.length);

    if (result == SUCCESS)
    {
        my_print (
          MESSAGE_OUT_LOGS,
          LOGGER_LEVEL_DEBUG,
          FILEMGMT_RECORD_EXISTS,
          file_name.length,
          file_name.address);
    }
    else
    {
        return (FALSE);
    }

    FILE *file = fopen (real_name.address, FILE_MODE_READBINARY);

    if (file == NULL)
    {
        return (FALSE);
    }

    if (file_stats != NULL)
    {
        struct stat f1 = file_stat (file);
        *file_stats = f1;
    }
    fclose (file);

    return (TRUE);
}

int
filemgmt_file_add (string_s file_name)
{
    // TODO: instead of 0x8100 add the actual file size
    return filemgmt_file_add_sqlite3 (file_name.address, file_name.length, 0x8100);
}

int
filemgmt_rename_file (string_s dest, string_s src)
{
    path_mkdir_recursive (FILEMGMT_FOLDER_NAME);
    if (file_rename (dest, src) == FAILED)
    {
        return (FAILED);
    }
    return (SUCCESS);
}

int
filemgmt_remove_meta (string_s file_name)
{
    return filemgmt_file_delete_sqlite3 (file_name.address, file_name.length);
}

int
filemgmt_mkdirs (filemgmt_file_name_s *file_info)
{
    int result = path_mkdir_recursive (file_info->real_file_name.address);
    if (result != SUCCESS)
    {
        return (result);
    }

    result = path_mkdir_recursive (file_info->real_hash_file_name.address);

    return (result);
}

int
filemgmt_create_backup_folders ()
{
    if (path_mkdir_recursive (FILEMGMT_FOLDER_NAME) != SUCCESS)
    {
        return (FAILED);
    }

    if (path_mkdir_recursive (FILEMGMT_FOLDER_META_NAME) != SUCCESS)
    {
        return (FAILED);
    }

    return SUCCESS;
}

int
filemgmt_setup_environment (string_s client_filename, filemgmt_file_name_s *file_info)
{
    if (filemgmt_create_backup_folders () != SUCCESS)
    {
        return FAILED;
    }

    my_list_s path_list = path_parse (client_filename).path_list;
    string_s file_name = path_construct (path_list);
    my_list_free (path_list);

    if (file_name.address == NULL)
    {
        return (FAILED);
    }

    string_s real_file_name = file_path_concat (STRING (FILEMGMT_FOLDER_NAME), file_name);
    string_s real_hash_file_name = file_path_concat (STRING (FILEMGMT_FOLDER_META_NAME), file_name);

    file_info->file_name = file_name;
    file_info->real_file_name = real_file_name;
    file_info->real_hash_file_name = real_hash_file_name;

    return (SUCCESS);
}

int
filemgmt_setup_temp_files (filemgmt_file_name_s *file_info)
{
    if (path_mkdir_recursive (FILEMGMT_TEMP_DIR) != SUCCESS)
    {
        return (FAILED);
    }
    static long counter = 0;

    long length;
    char *temp_file_p
      = string_sprintf (FILEMGMT_TEMP_FORMAT, &length, FILEMGMT_TEMP_DIR, ++counter);

    string_s temp_file = {0, .address = temp_file_p, .length = length};

    if (temp_file_p == NULL)
    {
        return (FAILED);
    }

    char *hash_file_p = string_sprintf (FILEMGMT_HASH_FORMAT, &length, FILEMGMT_TEMP_DIR, counter);
    string_s hash_file = {0, .address = hash_file_p, .length = length};

    if (hash_file_p == NULL)
    {
        return (FAILED);
    }

    file_info->temp_hash_file_name = hash_file;
    file_info->temp_file_name = temp_file;

    return (SUCCESS);
}