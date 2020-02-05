#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "database.h"
#include "../defines.h"
#include "../binarysearch.h"

static MYSQL *sql_connection;

b_search_string_s config_property[5] = {
    {"database", 8, CONFIG_DATABASE}
    , {"machine", 7, CONFIG_MACHINE}
    , {"password", 8, CONFIG_PASSWORD}
    , {"port", 4, CONFIG_PORT}
    , {"username", 8, CONFIG_USERNAME}
};


// after database_init is called a static variable sql_connection
// is initialized to the connection information and a mysql db 
// connection is attempted. 
int database_init(database_connection_s connect)
{
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library.\n");
        exit(SERVER_DATABASE_FAILURE);
    }
    sql_connection  = mysql_init(0);
    if (sql_connection == NULL) {
        fprintf(stderr, "could not initialize the mysql"
                    " connection, exiting..\n");
        exit(SERVER_DATABASE_FAILURE);
    }

    if (NULL == mysql_real_connect(sql_connection
        , connect.host , connect.user
        , connect.passwd , connect.db
        , connect.port , NULL , 0)) {
        fprintf(stderr, "could not connect to mariaDB server"
            ", check your connection settings and try again.\n");
        exit(SERVER_DATABASE_FAILURE);
    }

    return(DATABASE_SETUP_COMPLETE);
}