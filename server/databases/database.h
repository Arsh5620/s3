// we will be using connection to mariadb for now. 
// as the project matures, this interface might be updated. 

#ifndef DATABASE_INCLUDE_GAURD
#define DATABASE_INCLUDE_GAURD

typedef struct sql_connection_information
{
    const char *host;
	const char *user;
	const char *passwd;
	const char *db;
	unsigned int port;
} database_connection_s;

#define DATABASE_SETUP_COMPLETE 0

int database_init(database_connection_s connect);

#endif