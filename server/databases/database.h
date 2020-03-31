// we will be using connection to mariadb for now. 
// as the project matures, this interface might be updated. 

#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

#include <mysql.h>
#include "../general/strings.h"
#include "../data-structures/hash_table.h"

typedef struct sql_connection_information
{
    const char host[64];
	const char user[32];
	const char passwd[32];
	const char db[32];
	unsigned int port;
} database_connection_s;


enum config_types {
    CONFIG_USERNAME
    , CONFIG_PASSWORD
    , CONFIG_DATABASE
    , CONFIG_MACHINE
    , CONFIG_PORT
};

typedef struct database_bind_fields {
    my_bool is_null;
    my_bool is_error;
	my_bool is_unsigned;
    enum enum_field_types type;
    size_t length;
    void *buffer;
	char *name;
	size_t name_length;
	char *table_name;
	size_t table_length;
	size_t buffer_length;
	size_t flags;
	size_t decimals;
	size_t charsetnr;
	char is_init;
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
	database_bind_fields_s *fields;
    MYSQL_BIND *bind_params;
	size_t bind_param_count;
	hash_table_s table;
	int error;
} database_table_bind_s;

#define MYSQL_SUCCESS	0
#define MYSQL_ERROR		1

#define DATABASE_DB_NAME	"dbp"
#define DATABASE_TABLE_FI_NAME	"file_information"

/* table name: file_information, column information */

#define TABLE1_FI_COLUMN_FOLDER_NAME \
	(string_s){.address="folder_name", .length=11}
#define TABLE1_FI_COLUMN_FILE_NAME \
	(string_s){.address="file_name", .length=9}
#define TABLE1_FI_COLUMN_FILE_CD \
	(string_s){.address="file_cd", .length=7}
#define TABLE1_FI_COLUMN_FILE_UD \
	(string_s){.address="file_ud", .length=7}
#define TABLE1_FI_COLUMN_FILE_LA \
	(string_s){.address="file_la", .length=7} 
#define TABLE1_FI_COLUMN_FILE_LM \
	(string_s){.address="file_lm", .length=7}
#define TABLE1_FI_COLUMN_FILE_DD \
	(string_s){.address="file_dd", .length=7}
#define TABLE1_FI_COLUMN_FILE_SIZE \
	(string_s){.address="file_size", .length=9}
#define TABLE1_FI_COLUMN_FILE_MD5 \
	(string_s){.address="file_md5", .length=8} 
#define TABLE1_FI_COLUMN_PERMISSIONS \
	(string_s){.address="permission", .length=10}
#define TABLE1_FI_COLUMN_OWNER \
	(string_s){.address="owner", .length=5}

#define DATABASE_TABLE_FI_CREATE \
	"CREATE TABLE IF NOT EXISTS " DATABASE_TABLE_FI_NAME \
	" (folder_name VARCHAR(256)" \
	", file_name VARCHAR(256)" \
	", file_cd DATE" \
	", file_ud DATE" \
	", file_la DATE" \
	", file_lm DATE" \
	", file_dd DATE" \
	", file_size BIGINT" \
	", file_md5 VARCHAR(32)" \
	", permission BIGINT" \
	", owner VARCHAR(32));"

#define DATABASE_TABLE_FI_BIND \
	"SELECT * FROM " DATABASE_TABLE_FI_NAME " LIMIT 1"

#define DATABASE_TABLE_FI_CHECKEXISTS \
	"SELECT COUNT(*) AS 'A' FROM " DATABASE_TABLE_FI_NAME

#define DATABASE_TABLE_FI_INSERT \
	"INSERT INTO "DATABASE_TABLE_FI_NAME" "\
	"VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

#define DATABASE_TABLE_FI_QUERY \
	"SELECT * FROM " DATABASE_TABLE_FI_NAME

#define DATABASE_CREATE_DATABASE \
	"CREATE DATABASE IF NOT EXISTS " DATABASE_DB_NAME

#define FLAG_ISSET(x, y) ((x & y) > 0)

int database_init(database_connection_s connect);

int database_verify_integrity();
int database_check_tables();
int database_create_tables();

MYSQL *database_get_handle();
database_table_bind_s database_get_global_bind();

int database_table_insertrow(char *query, MYSQL_BIND *bind, size_t count);
MYSQL_STMT *database_table_query(char *query
	, MYSQL_BIND *bind_in 
	, MYSQL_BIND *bind_out);
void __database_query_print_dbg(MYSQL_STMT *stmt
	, database_table_bind_s bind);

/* functions to help with variable binding in mysql*/

void database_bind_init_global();
database_table_bind_s database_bind_setup(MYSQL *mysql);
void database_bind_allocate(database_table_bind_s *bind, size_t columns);
hash_table_s database_bind_maketable(database_table_bind_s *bind_table);
database_bind_field_flags_s database_bind_set_flags(size_t flags);
database_bind_fields_s database_bind_field(MYSQL_FIELD field);
void database_bind_linkfields(database_table_bind_s *table);
database_bind_fields_s 
	database_bind_field_copy(database_bind_fields_s src);
database_table_bind_s 
	database_bind_some_copy(database_table_bind_s bind_table
	, string_s *columns
	, int count);
size_t database_bind_column_index
	(database_table_bind_s bind_table, string_s column_name);
#endif