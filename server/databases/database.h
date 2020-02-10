// we will be using connection to mariadb for now. 
// as the project matures, this interface might be updated. 

#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

#include <mysql.h>
#include "../strings.h"

typedef struct sql_connection_information
{
    const char host[32];
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

#define DATABASE_SETUP_COMPLETE 0

#define MYSQL_SUCCESS	0
#define MYSQL_ERROR		1

#define DATABASE_DB_NAME	"dbp"
#define DATABASE_TABLE_NAME	"dbp_file_information"

#define DATABASE_TABLE_1 \
"CREATE TABLE IF NOT EXISTS " DATABASE_TABLE_NAME \
" (folder_name VARCHAR(256)\
, file_name VARCHAR(256)\
, file_cd DATE\
, file_ud DATE\
, file_la DATE\
, file_lm DATE\
, file_dd DATE\
, file_size BIGINT\
, file_md5 VARCHAR(32)\
, permission BIGINT\
, owner VARCHAR(32));"

enum datetime_values_set {
    DATETIME_CREATE_DATE    = 0b00001
    , DATETIME_UPLOAD_DATE  = 0b00010
    , DATETIME_LASTACCESS_DATE  = 0b00100
    , DATETIME_LASTCHANGE_DATE  = 0b01000
    , DATETIME_DELETION_DATE    = 0b10000
};
typedef struct database_table1
{
    string_s folder_name;
    string_s file_name;
    MYSQL_TIME file_cd;
    MYSQL_TIME file_ud;
    MYSQL_TIME file_la;
    MYSQL_TIME file_lm;
    MYSQL_TIME file_dd;
    enum datetime_values_set file_times_set;
    size_t file_size;
    char file_md5[32];
    size_t permissions;
    char owner[32];
} database_table1_s;

enum schema_table1_index {
    TABLE1_INDEX_FOLDER_NAME    = 0
    , TABLE1_INDEX_FILE_NAME    = 1
    , TABLE1_INDEX_FILE_CD  = 2
    , TABLE1_INDEX_FILE_UD  = 3
    , TABLE1_INDEX_FILE_LA  = 4
    , TABLE1_INDEX_FILE_LM  = 5
    , TABLE1_INDEX_FILE_DD  = 6
    , TABLE1_INDEX_FILE_SIZE    = 7
    , TABLE1_INDEX_FILE_MD5 = 8
    , TABLE1_INDEX_PERMISSIONS  = 9
    , TABLE1_INDEX_OWNER    = 10
};

#define DATABASE_TABLE_CHECKEXISTS1 \
"SELECT COUNT(*) AS 'a' FROM dbp_file_information;"

#define DATABASE_TABLE1_INSERT \
"INSERT INTO "DATABASE_TABLE_NAME" "\
"VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

#define DATABASE_TABLE1_QUERY \
"SELECT * FROM " DATABASE_TABLE_NAME

#define DATABASE_TABLE1_COLUMNCOUNT 11
#define DATABASE_FOLDERNAME_LEN 256
#define DATABASE_FILENAME_LEN 256

typedef struct database_table_stmt_group {
    my_bool is_null[DATABASE_TABLE1_COLUMNCOUNT];
    my_bool is_error[DATABASE_TABLE1_COLUMNCOUNT];
    enum enum_field_types types[DATABASE_TABLE1_COLUMNCOUNT];
    unsigned long int length[DATABASE_TABLE1_COLUMNCOUNT];
    unsigned long int *length_ptrs[DATABASE_TABLE1_COLUMNCOUNT];
    char *buffers[DATABASE_TABLE1_COLUMNCOUNT];
    MYSQL_BIND bind_params[DATABASE_TABLE1_COLUMNCOUNT];
} db_table_stmt_s;

int database_init(database_connection_s connect);
int database_verify_integrity();

int database_check_tables();
int database_create_tables();

void database_bind_param(MYSQL_BIND *bind
    , enum enum_field_types type
    , char *buffer_pointer
    , unsigned long int *length
    , my_bool *is_null
    , my_bool *error);

int database_table1_insert(
    database_table1_s table
    , db_table_stmt_s * table1);

db_table_stmt_s *database_table1_bind_get(database_table1_s *table);
void database_table1_bind_free(db_table_stmt_s *table);

database_table1_s *database_table1_allocate();
void database_table1_free(database_table1_s *table);

MYSQL_STMT *database_table1_query(db_table_stmt_s *binds
    , char *query, MYSQL_BIND *in_bind);
void __database_query_print_dbg(MYSQL_STMT *stmt
    , db_table_stmt_s *binds);
db_table_stmt_s *database_table1_bind_getselective(
    database_table1_s *table
    , int *select
    , int count);
#endif