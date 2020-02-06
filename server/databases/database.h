// we will be using connection to mariadb for now. 
// as the project matures, this interface might be updated. 

#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

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

#define DATABASE_TABLE_CHECKEXISTS1 \
"SELECT COUNT(*) AS 'a' FROM dbp_file_information;"

int database_init(database_connection_s connect);
int database_verify_integrity();

#endif