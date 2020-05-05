// we will be using connection to mariadb for now. 
// as the project matures, this interface might be updated. 

#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

#include <stddef.h>
#include <mysql.h>
#include "../general/string.h"
#include "../data-structures/hash_table.h"
#include "../dataparser/data.h"

typedef struct sql_connection_information
{
	char *host;
	char *user;
	char *passwd;
	char *db;
	unsigned int port;
} database_connection_s;

enum database_error_exit_enum 
{
	DATABASE_MYSQL_LIBINIT = 64
	, DATABASE_MYSQL_INIT 
	, DATABASE_LOGIN_FAIL
	, DATABASE_INTEGRITY_CHECK_FAIL
	, DATABASE_BIND_QUERY_FAIL
	, DATABASE_FAILURE_OTHER
};

enum config_types {
	CONFIG_USERNAME
	, CONFIG_PASSWORD
	, CONFIG_DATABASE
	, CONFIG_MACHINE
	, CONFIG_PORT
};

#define CONFIG_COUNT	sizeof(config_property)/sizeof(struct config_parse)

typedef struct database_bind_fields {
	/*
	 * all of the elements of this structure are required to complete binds
	 */
	size_t length;
	my_bool is_null;	
	my_bool is_error;
	my_bool is_unsigned;

	enum enum_field_types type;
	string_s name;
	string_s buffer;

	size_t flags;
	char init_complete;
} database_bind_fields_s;

typedef struct database_bind_field_flags {
	my_bool is_numeric;
	my_bool is_primary;
	my_bool is_unique;
	my_bool is_multikey;
	my_bool is_binary;
	my_bool is_autoincr;
	my_bool is_not_null;
	my_bool has_no_defaults;
} database_bind_field_flags_s;

typedef struct database_table_bind {
	/**
	 * database_bind_fields_s contain the actual information about the binds
	 * where as bind_params is the format that mysql api accepts the bind in
	 */
	int error;
	size_t count;
	string_s table_name;
	MYSQL_BIND *bind_params;
	hash_table_s hash_table;
	database_bind_fields_s *fields;
} database_table_bind_s;

enum database_status_enum 
{
	MYSQL_SUCCESS
	, MYSQL_ERROR
	, MYSQL_TABLE_NOT_FOUND
};

#define DATABASE_DB_NAME		"dbp"

#define DATABASE_COMMON_BIND_STMT(x) \
	"SELECT * FROM " x " LIMIT 1"

#define DATABASE_COMMON_TABLE_EXISTS(x) \
	"SELECT COUNT(*) AS 'table' FROM " x

#define DATABASE_CREATE_DATABASE \
	"CREATE DATABASE IF NOT EXISTS " DATABASE_DB_NAME

#define DATABASE_CREATE_TABLE(x, y) \
	"CREATE TABLE IF NOT EXISTS " x " ( " y " );"

#define FLAG_ISSET(x, y) ((x & y) > 0)

int database_init();

int database_verify_integrity();
int database_create_tables(char *statement, char *table);
int database_check_tables(char *statment, char *table);
int database_table_verify(char *check_table_query
	, char *create_table_query
	, char *table_name
	, int (*binds_setup)(MYSQL*));

MYSQL *database_get_handle();

/*
 * database_function is the function that will be called
 * after the statement is executed and before the statement is closed. 
 * you can leave it NULL, if you don't intend to call any functions
 */
int database_table_insert(int (*database_function)(MYSQL_STMT *)
	, string_s query , MYSQL_BIND *bind, size_t count);

int database_table_query(int (*database_function)(MYSQL_STMT *)
	, string_s query, MYSQL_BIND *bind_in, uint bind_in_count
	, MYSQL_BIND *bind_out);
int database_table_stmt(string_s query
	, MYSQL_BIND *bind_in, uint bind_in_count);
int database_table_row_exists(MYSQL_STMT *stmt);
/* functions to help with variable binding in mysql*/

void database_bind_init_global();
database_table_bind_s database_bind_setup(MYSQL *mysql, char *select_query);
void database_bind_allocate(database_table_bind_s *bind, size_t columns);
hash_table_s database_bind_maketable(database_table_bind_s *bind_table);
database_bind_field_flags_s database_bind_set_flags(size_t flags);
database_bind_fields_s database_bind_field(MYSQL_FIELD *field
	, database_bind_fields_s *src);
database_table_bind_s database_bind_select_copy(
	database_table_bind_s bind_table, string_s *columns, int count);
void database_bind_free(database_table_bind_s bind);
int database_bind_add_data(database_table_bind_s bind_table
	, string_s column_name, string_s data);
size_t database_bind_column_index(database_table_bind_s bind_table
	, string_s column_name);
void database_bind_clean(database_table_bind_s bind_table);
void database_bind_link_fields(database_table_bind_s *table);

#endif