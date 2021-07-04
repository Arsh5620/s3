#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

#include <sqlite3.h>
#include <stddef.h>

enum sqltypes {
    SQLTYPE_LONG,
    SQLTYPE_TEXT,
    SQLTYPE_BLOB,
};

sqlite3 *
database_get_sqlite3 ();
int
database_init (const char *database_filename);
int
database_make_schema (int number_tables, ...);
sqlite3_stmt *
database_get_stmt (const char *query, int query_length, int *error);
int
database_rows_affected (sqlite3_stmt *stmt, int response_type, char *error_message);
int
database_iterate_all_rows (sqlite3_stmt *stmt, void *memory, int (*row_callback) (void *, sqlite3_stmt *), char *error_message);
#endif