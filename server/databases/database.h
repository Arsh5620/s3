#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

#include <stddef.h>
#include <sqlite3.h>

sqlite3 *
database_get_sqlite3 ();
int
database_init (const char *database_filename);
int
database_make_schema (int number_tables, ...);
sqlite3_stmt *
database_get_stmt (const char *query, int query_length, int *error);
int
database_finish_stmt (sqlite3_stmt *stmt, int response_type, char *error_message);
#endif