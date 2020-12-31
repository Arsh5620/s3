#include "database.h"
#include "../logger/messages.h"
#include <stdarg.h>

sqlite3 *sqlite_connection;

void
database_print_sqlite_error (char *error_message)
{
    my_print (
      MESSAGE_OUT_BOTH,
      LOGGER_LEVEL_CATASTROPHIC,
      error_message,
      sqlite3_errmsg (sqlite_connection));
}

int
database_init (const char *database_filename)
{
    int error = sqlite3_open (database_filename, &sqlite_connection);
    if (error != SQLITE_OK)
    {
        sqlite3_close (sqlite_connection);
        database_print_sqlite_error ("Sqlite open failed, error message : %s");
        return FAILED;
    }
    return SUCCESS;
}

int
database_make_schema (int number_tables, ...)
{
    va_list vargs;
    va_start (vargs, number_tables);

    for (size_t i = 0; i < number_tables; i++)
    {
        sqlite3_stmt *stmt;

        char *query = va_arg (vargs, char *);
        int query_length = va_arg (vargs, int);
        int error = sqlite3_prepare_v2 (sqlite_connection, query, query_length, &stmt, NULL);

        if (error != SQLITE_OK)
        {
            database_print_sqlite_error ("Sqlite prepare failed, error message : %s");
            sqlite3_close (sqlite_connection);
            return FAILED;
        }

        error = sqlite3_step (stmt);

        if (error != SQLITE_DONE)
        {
            database_print_sqlite_error ("Sqlite step failed, error message : %s");
            sqlite3_finalize (stmt);
            return FAILED;
        }

        sqlite3_finalize (stmt);
    }
    return SUCCESS;
}

sqlite3_stmt *
database_get_stmt (const char *query, int query_length, int *error_out)
{
    sqlite3_stmt *stmt;
    int error = sqlite3_prepare_v2 (sqlite_connection, query, query_length, &stmt, NULL);

    if (error != SQLITE_OK)
    {
        my_print (
          MESSAGE_OUT_LOGS,
          LOGGER_LEVEL_ERROR,
          "Sqlite failed preparing query transaction: %s",
          sqlite3_errmsg (sqlite_connection));
        sqlite3_finalize (stmt);
        if (error_out != NULL)
        {
            *error_out = FAILED;
        }
        return NULL;
    }

    if (error_out != NULL)
    {
        *error_out = SUCCESS;
    }
    return stmt;
}

int
database_finish_stmt (sqlite3_stmt *stmt, int response_type, char *error_message)
{
    int error = sqlite3_step (stmt);
    if (error != response_type)
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, error_message);
        sqlite3_finalize (stmt);
        return FAILED;
    }

    int response = 1;
    if (response_type == SQLITE_ROW)
    {
        response = sqlite3_column_int64 (stmt, 0) != 0;
    }

    sqlite3_finalize (stmt);

    return response ? SUCCESS : FAILED;
}

sqlite3 *
database_get_sqlite3 ()
{
    return sqlite_connection;
}