#include "database.h"
#include "general/messages.h"
#include "logging/logs.h"

#include <stdarg.h>

sqlite3 *sqlite_connection;

void
database_print_sqlite_error (char *error_message, long log_level)
{
    logs_put (LOG_OUT_BOTH, log_level, error_message, sqlite3_errmsg (sqlite_connection));
}

int
database_init (const char *database_filename)
{
    int error = sqlite3_open (database_filename, &sqlite_connection);
    if (error != SQLITE_OK) {
        sqlite3_close (sqlite_connection);
        database_print_sqlite_error ("Sqlite open failed, error message : %s", LOG_LEVEL_FATAL);
        return FAILED;
    }

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, DATABASE_FILE_FOUND, database_filename);
    return SUCCESS;
}

int
database_run_query_noresult (char *query, long query_length)
{
    sqlite3_stmt *stmt;

    int error = sqlite3_prepare_v2 (sqlite_connection, query, query_length, &stmt, NULL);

    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, DATABASE_SCHEMA_CHECK, query);

    if (error != SQLITE_OK) {
        database_print_sqlite_error ("Sqlite prepare failed, error message : %s", LOG_LEVEL_FATAL);
        sqlite3_close (sqlite_connection);
        return FAILED;
    }

    error = sqlite3_step (stmt);

    if (error != SQLITE_DONE) {
        database_print_sqlite_error ("Sqlite step failed, error message : %s", LOG_LEVEL_FATAL);
        sqlite3_finalize (stmt);
        return FAILED;
    }

    sqlite3_finalize (stmt);

    return SUCCESS;
}

sqlite3_stmt *
database_get_stmt (const char *query, int query_length)
{
    sqlite3_stmt *stmt;
    int error = sqlite3_prepare_v2 (sqlite_connection, query, query_length, &stmt, NULL);

    if (error != SQLITE_OK) {
        database_print_sqlite_error ("Sqlite failed preparing query transaction: %s", LOG_LEVEL_ERROR);
        sqlite3_finalize (stmt);

        return NULL;
    }

    return stmt;
}

int
database_rows_affected (sqlite3_stmt *stmt, int response_type, char *error_message)
{
    int error = sqlite3_step (stmt);
    if (error != response_type) {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, error_message);
        sqlite3_finalize (stmt);
        return FAILED;
    }

    int response = 1;
    if (response_type == SQLITE_ROW) {
        response = sqlite3_column_int64 (stmt, 0) != 0;
    }

    sqlite3_finalize (stmt);

    return response ? SUCCESS : FAILED;
}

int
database_iterate_all_rows (sqlite3_stmt *stmt, void *memory, int (*row_callback) (void *, sqlite3_stmt *), char *error_message)
{
    int step = SQLITE_OK;
    int failed = FALSE;

    while (TRUE) {
        step = sqlite3_step (stmt);

        if (step == SQLITE_ROW) {
            if (row_callback (memory, stmt) == FAILED) {
                failed = TRUE;
                break;
            }
        } else if (step == SQLITE_DONE) {
            break;
        } else {
            logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, "SQLite error: %s", error_message);
            failed = TRUE;
            break;
        }
    }

    sqlite3_finalize (stmt);

    return failed ? FAILED : SUCCESS;
}

sqlite3 *
database_get_sqlite3 ()
{
    return sqlite_connection;
}