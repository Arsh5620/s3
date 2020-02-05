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

int database_init(database_connection_s connect);

#endif