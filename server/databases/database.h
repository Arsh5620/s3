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
, file_md DATE\
, file_ud DATE\
, file_la DATE\
, file_lm DATE\
, file_dd DATE\
, file_size BIGINT\
, file_md5 VARCHAR(32)\
, permission BIGINT\
, owner VARCHAR(32));"

typedef struct database_table1
{
    string_s folder_name;
    string_s file_name;
    MYSQL_TIME file_md;
    MYSQL_TIME file_ud;
    MYSQL_TIME file_la;
    MYSQL_TIME file_lm;
    MYSQL_TIME file_dd;
    size_t file_size;
    char file_md5[32];
    size_t permissions;
    char owner[32];
} database_table1_s;


#define DATABASE_TABLE_CHECKEXISTS1 \
"SELECT COUNT(*) AS 'a' FROM dbp_file_information;"

#define DATABASE_TABLE1_INSERT \
"INSERT INTO dbp_file_information "\
"VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"

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

void database_bind_multiple(MYSQL_BIND *bind_start_address
    , int count
    , enum enum_field_types type
    , char **buffer_pp
    , unsigned long int *length
    , my_bool *is_null
    , my_bool *error);

int database_insert_table1(database_table1_s table);

#endif